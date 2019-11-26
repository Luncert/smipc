
#include <stdio.h>
#include <windows.h>
#include "map.h"
#include "util.h"
#include "library.h"

const u_char MARK_READER_OPEN     = 0b1000;
const u_char MARK_WRITER_OPEN     = 0b0100;
const u_char MARK_READER_CLOSE    = 0b0010;
const u_char MARK_WRITER_CLOSE    = 0b0001;

const u_char STATE_FULL     = 0x10;
const u_char STATE_EMPTY    = 0x01;

#define MAX_CHAN_SZ 536870911L // =sizeof(long), it's limited by the CreateSemaphore's param

#define is_reader_open(syncBuf)     (syncBuf->shared->mark & MARK_READER_OPEN)
#define is_writer_open(syncBuf)     (syncBuf->shared->mark & MARK_WRITER_OPEN)
#define is_reader_close(syncBuf)    (syncBuf->shared->mark & MARK_READER_CLOSE)
#define is_writer_close(syncBuf)    (syncBuf->shared->mark & MARK_WRITER_CLOSE)
#define set_reader_open(syncBuf)    (syncBuf->shared->mark |= MARK_READER_OPEN)
#define set_writer_open(syncBuf)    (syncBuf->shared->mark |= MARK_WRITER_OPEN)
#define set_reader_close(syncBuf)   (syncBuf->shared->mark |= MARK_READER_CLOSE)
#define set_writer_close(syncBuf)   (syncBuf->shared->mark |= MARK_WRITER_CLOSE)
#define sb_bufSz(syncBuf)           (syncBuf->shared->bufSz)
#define sb_rc(syncBuf)              (syncBuf->shared->rc)
#define sb_wc(syncBuf)              (syncBuf->shared->wc)
#define is_buf_full(syncBuf)        (syncBuf->shared->state & STATE_FULL)
#define is_buf_empty(syncBuf)       (syncBuf->shared->state & STATE_EMPTY)
#define set_buf_full(syncBuf)       (syncBuf->shared->state = STATE_FULL)
#define set_buf_empty(syncBuf)      (syncBuf->shared->state = STATE_EMPTY)

typedef map_t(Channel) channel_map_t;

/**
 * channel_map_t: map_set会把值做一个拷贝这样就可以自己管理值需要的内存，map_get则会返回自己管理的值的内存的指针，而不是值
 */
channel_map_t* channelMap;

void initLibrary(int allowLog) {
    if (channelMap == NULL) {
        channelMap = (channel_map_t*)malloc(sizeof(channel_map_t));
        map_init(channelMap);
    }
    if (!allowLog) {
        setLogLevel(LOG_DISABLE);
    }
}

void cleanLibrary() {
    if (channelMap != NULL) {
        const char *key;
        Channel channel;

        map_iter_t iter = map_iter(&m);
        while ((key = map_next(channelMap, &iter))) {
            channel = *map_get(channelMap, key);
            closeChannel((char*)key);
            free(channel);
        }

        map_deinit(channelMap);
        free(channelMap);
        channelMap = NULL;
    }
}

/**
 * openChannel
 * @param cid channel id
 * @param mode channel mode, CHAN_R or CHAN_W
 * @param chanSz vailable memory size for channel
 * @return OP_SUCCEED, OP_FAILED
 */
int openChannel(char *cid, int mode, int chanSz) {
    if (mode != CHAN_R && mode != CHAN_W) {
        logError("Invalid channel mode, must be CHAN_R(0) or CHAN_W(1).");
        return OP_FAILED;
    }
    if (chanSz <= 0) {
        logError("Invalid chanSz, must be positive.");
        return OP_FAILED;
    }

    Channel channel, *p;
    p = map_get(channelMap, cid);
    if (p != NULL) {
        channel = *p;
        // check channel's mode
        if (channel->mode != mode) {
            logError("Specified channel has been opened in different mode.");
            return OP_FAILED;
        }
        return OP_SUCCEED;
    }

    char *shareMem = NULL;
    char isNewMem = FALSE;

    HANDLE hShareMem = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, cid);
    if (hShareMem == NULL || hShareMem == INVALID_HANDLE_VALUE) {
        // Failed to open channel's shared memory, try to create
        if (chanSz > MAX_CHAN_SZ) {
            chanSz = MAX_CHAN_SZ;
            if (allowLog(LOG_WARN)) {
                printf("[WARN] chanSz should be at most %ldBytes, auto adjusted.", MAX_CHAN_SZ);
            }
        }
        hShareMem = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, chanSz + sizeof(struct syncBuf), cid);
        // check error
        if (hShareMem == INVALID_HANDLE_VALUE || hShareMem == NULL) {
            logWinError("Failed to create shared memory.");
            return OP_FAILED;
        }
        if (allowLog(LOG_INFO)) {
            printf("[INFO] Channel(%s) created\n", cid);
        }

        /*
        DWORD err = GetLastError();
        if (err == ERROR_FILE_INVALID) {
            logError("Duplicate channel ID, which has been used by another object (shared memory, mutex, semaphore or critical area).");
            return OP_FAILED;
        } else if (err == ERROR_ALREADY_EXISTS) {
            logError("Duplicate channel ID, which has been used to create a shared memory.");
            return OP_FAILED;
        }
         */

        // get shared memory's pointer
        shareMem = (char*)MapViewOfFile(hShareMem,FILE_MAP_WRITE|FILE_MAP_READ,
                                          0,0,0);
        memset(shareMem, 0, sizeof(struct syncBuf));
        isNewMem = TRUE;
    } else {
        if (allowLog(LOG_INFO)) {
            printf("[INFO] Channel(%s) open\n", cid);
        }
        shareMem = (char*)MapViewOfFile(hShareMem,FILE_MAP_WRITE|FILE_MAP_READ,
                                        0,0,0);
    }

    // lock
    String mutexName = parseConstToString(cid);
    appendString(mutexName, "/mutex", 6);
    HANDLE mutex = lock(mutexName);
    releaseString(mutexName);

    String semName = parseConstToString(cid);
    appendString(semName, "/event", 6);
    SyncBuf syncBuf = newSyncBuf(shareMem, chanSz, mode, semName, isNewMem);
    if (syncBuf == NULL) {
        CloseHandle(hShareMem);

        unlock(mutex);
        return OP_FAILED;
    }
    releaseString(semName);

    // create channel
    channel = (Channel)malloc(sizeof(struct channel));
    channel->mode = mode;
    channel->hShareMem = hShareMem;
    channel->syncBuf = syncBuf;
    map_set(channelMap, cid, channel);

    // unlock
    unlock(mutex);

    return OP_SUCCEED;
}

/**
 * readChannel
 * @param cid channel id
 * @param buf
 * @param n
 * @param blocking
 * @return n, OPPOSITE_END_CLOSED, OP_FAILED
 */
int readChannel(char *cid, char *buf, int n, char blocking) {
    Channel channel, *p;
    p = map_get(channelMap, cid);
    if (p == NULL) {
        logError("Channel doesn't exist, make sure open it at first.");
        return OP_FAILED;
    }
    channel = *p;
    // check channel's mode
    if (channel->mode != CHAN_R) {
        logError("Channel doesn't support read operation.");
        return OP_FAILED;
    }
    if (blocking) {
        return readSyncBufB(channel->syncBuf, buf, n);
    } else {
        return readSyncBuf(channel->syncBuf, buf, n);
    }
}

/**
 * writeChannel
 * @param cid channel id
 * @param data
 * @param len
 * @return OP_SUCCEED, OPPOSITE_END_CLOSED, OP_FAILED
 */
int writeChannel(char *cid, char *data, int len) {
    Channel channel, *p;
    p = map_get(channelMap, cid);
    if (p == NULL) {
        logError("Channel doesn't exist, make sure open it at first.");
        return OP_FAILED;
    }
    channel = *p;

    // check channel's mode
    if (channel->mode != CHAN_W) {
        logError("Channel doesn't support write operation.");
        return OP_FAILED;
    }
    if (len <= 0) {
        logError("Invalid parameter, len should be positive.");
        return OP_FAILED;
    }
    return writeSyncBuf(channel->syncBuf, data, len);
}

int printChannelStatus(char *cid) {
    Channel channel, *p;
    p = map_get(channelMap, cid);
    if (p == NULL) {
        logError("Channel doesn't exist, make sure open it at first.");
        return OP_FAILED;
    }
    channel = *p;

    printf("Channel(%s): mode=%c hShareMem=%s syncBuf=", cid, channel->mode == CHAN_R ? 'R' : 'W',
            channel->hShareMem != NULL ? "valid" : "invalid");
    SyncBuf syncBuf = channel->syncBuf;
    printf("{");
    printf("RO=%c", is_reader_open(syncBuf) ? '1' : '0');
    printf(" WO=%c", is_writer_open(syncBuf) ? '1' : '0');
    printf(" RC=%c", is_reader_close(syncBuf) ? '1' : '0');
    printf(" WC=%c", is_writer_close(syncBuf) ? '1' : '0');
    printf(" hREvt=%s", syncBuf->hREvt != NULL ? "valid" : "invalid");
    printf(" hWEvt=%s", syncBuf->hWEvt != NULL ? "valid" : "invalid");
    printf(" bufSz=%d rc=%d wc=%d", syncBuf->shared->bufSz, syncBuf->shared->rc, syncBuf->shared->wc);
    printf(" state=%s", is_buf_empty(syncBuf) ? "empty" : (is_buf_full(syncBuf) ? "full" : "normal"));
    printf("}\n");
    return OP_SUCCEED;
}

/**
 * closeChannel
 * @param cid channel id
 * @return OP_SUCCEED, OP_FAILED
 */
int closeChannel(char *cid) {
    Channel channel, *p;
    p = map_get(channelMap, cid);
    if (p == NULL) {
        logError("Cannot close nonexistent channel.");
        return OP_FAILED;
    }
    channel = *p;

    map_remove(channelMap, cid);

    // release SyncBuf
    SyncBuf syncBuf = channel->syncBuf;
    // update mark
    if (channel->mode == CHAN_R) {
        set_reader_close(syncBuf);
    } else {
        set_writer_close(syncBuf);
    }
    // prevent dead waiting in writeSyncBuf and readSyncBufB.
    SetEvent(syncBuf->hREvt);
    SetEvent(syncBuf->hWEvt);

    int flag = TRUE;
    if (CloseHandle(syncBuf->hREvt) == FALSE) {
        flag = FALSE;
    }
    if (CloseHandle(syncBuf->hWEvt) == FALSE) {
        flag = FALSE;
    }
    if (CloseHandle(channel->hShareMem) == FALSE) {
        flag = FALSE;
    }
    if (flag == FALSE) {
        logWinError("Failed to clean windows resource,");
        return OP_FAILED;
    }

    free(syncBuf);
    free(channel);

    if (allowLog(LOG_INFO)) {
        printf("[INFO] Channel(%s) closed\n", cid);
    }
    return OP_SUCCEED;
}

// SyncBuf

SyncBuf newSyncBuf(char *shareMem, int bufSz, int mode, String semName, char isNewMem) {
    SyncBuf syncBuf = (SyncBuf)malloc(sizeof(struct syncBuf));
    syncBuf->shared = (struct _shared*)shareMem;
    if (isNewMem) {
        sb_bufSz(syncBuf) = bufSz;
        sb_rc(syncBuf) = 0;
        sb_wc(syncBuf) = 0;
        set_buf_empty(syncBuf);
        // set mark
        if (mode == CHAN_R) {
            set_reader_open(syncBuf);
        } else {
            set_writer_open(syncBuf);
        }
    } else {
        // check memory mark
        if (mode == CHAN_R) {
            if (is_reader_close(syncBuf)) {
                logError("Channel has been closed from reader's side.");
                return NULL;
            } else if (is_reader_open(syncBuf)) {
                logError("Channel has been opened from reader's side by another process.");
                return NULL;
            } else {
                set_reader_open(syncBuf);
            }
        } else {
            if (is_writer_close(syncBuf)) {
                logError("Channel has been closed from writer's side.");
                return NULL;
            } else if (is_writer_open(syncBuf)) {
                logError("Channel has been opened from writer's side by another process.");
                return NULL;
            } else {
                set_writer_open(syncBuf);
            }
        }
        // every attr of SyncBuf could be read from the share memory.
    }
    // 由于共享内存的指针是从物理地址映射而来虚拟地址，所以在python和winexe两种环境中，这两个值可能不同.
    syncBuf->buf = shareMem + sizeof(struct _shared);
    // create sem
    if (initSyncBufEvent(semName, &syncBuf->hREvt, &syncBuf->hWEvt) != OP_SUCCEED) {
        return NULL;
    }
    return syncBuf;
}

int initSyncBufEvent(String namePrefix, HANDLE *hREvt, HANDLE *hWEvt) {
    int ret = OP_SUCCEED;

    String rEvtName = appendToNewString(namePrefix, "#R", 2);
    String wEvtName = appendToNewString(namePrefix, "#W", 2);

    *hREvt = OpenEvent(EVENT_ALL_ACCESS, FALSE, rEvtName->val);
    if (*hREvt == NULL) {
        // auto reset, init = no signal
        *hREvt = CreateEvent(NULL, FALSE, FALSE, rEvtName->val);
        if (*hREvt == INVALID_HANDLE_VALUE || *hREvt == NULL) {
            logError("Failed to create read event for SyncBuf.");
            ret = OP_FAILED;
        }
    }

    if (ret == OP_SUCCEED) {
        *hWEvt = OpenEvent(SEMAPHORE_ALL_ACCESS, FALSE, wEvtName->val);
        if (*hWEvt == NULL) {
            *hWEvt = CreateEvent(NULL, FALSE, FALSE, wEvtName->val);
            if (*hWEvt == INVALID_HANDLE_VALUE || *hWEvt == NULL) {
                logError("Failed to create read semaphore for SyncBuf.");
                CloseHandle(*hREvt);
                ret = OP_FAILED;
            }
        }
    }

    releaseString(rEvtName);
    releaseString(wEvtName);
    return ret;
}

int get_buf_readable(SyncBuf s) {
    if (sb_wc(s) == sb_rc(s)) {
        if (is_buf_full(s)) {
            return sb_bufSz(s);
        } else if (is_buf_empty(s)) {
            return 0;
        } else {
            logError("SyncBuf is in invalid state.");
            return -1;
        }
    } else if (sb_wc(s) > sb_rc(s)) {
        return sb_wc(s) - sb_rc(s);
    } else {
        return sb_wc(s) + sb_bufSz(s) - sb_rc(s);
    }
}

int get_buf_writeable(SyncBuf s) {
    if (sb_wc(s) == sb_rc(s)) {
        if (is_buf_full(s)) {
            return 0;
        } else if (is_buf_empty(s)) {
            return sb_bufSz(s);
        } else {
            logError("SyncBuf is in invalid state.");
            return -1;
        }
    } else if (sb_wc(s) > sb_rc(s)) {
        return sb_rc(s) + sb_bufSz(s) - sb_wc(s);
    } else {
        return sb_rc(s) - sb_wc(s);
    }
}

// 我一开始把这两个函数写成了宏定义的，但是总是有两个字节会写出错；结果改成函数调用就没问题了。。。
inline void sb_read_n(SyncBuf s, char *buf, int n) {
    int t = sb_bufSz(s) - sb_rc(s);
    if (t < n) {
        memcpy(buf, s->buf + sb_rc(s), t);
        memcpy(buf + t, s->buf, n - t);
    } else {
        memcpy(buf, s->buf + sb_rc(s), n);
    }
}

inline void sb_write_n(SyncBuf s, char *data, int n) {
    int t = sb_bufSz(s) - sb_wc(s);
    if (t < n) {
        memcpy(s->buf + sb_wc(s), data, t);
        memcpy(s->buf, data + t, n - t);
    } else {
        memcpy(s->buf + sb_wc(s), data, n);
    }
}

inline int sb_inc_rc(SyncBuf s, int delta) {
    sb_rc(s) = (sb_rc(s) + delta) % sb_bufSz(s);
    if (sb_rc(s) == sb_wc(s)) {
        set_buf_empty(s);
#ifdef SMIPC_TRACE
        printf("[TRACE] read: buffer -> empty.\n");
#endif
    }
    if (!SetEvent(s->hREvt)) {
        logWinError("Failed to set read event.");
        return OP_FAILED;
    }
#ifdef SMIPC_TRACE
    printf("[TRACE] read: set RE.\n");
#endif
    return OP_SUCCEED;
}

inline int sb_inc_wc(SyncBuf s, int delta) {
    sb_wc(s) = (sb_wc(s) + delta) % sb_bufSz(s);
    if (sb_rc(s) == sb_wc(s)) {
        set_buf_full(s);
#ifdef SMIPC_TRACE
        printf("[TRACE] write: buffer -> full.\n");
#endif
    }
    if (!SetEvent(s->hWEvt)) {
        logWinError("Failed to set write event.");
        return OP_FAILED;
    }
#ifdef SMIPC_TRACE
    printf("[TRACE] write: set WE.\n");
#endif
    return OP_SUCCEED;
}

int readSyncBuf(SyncBuf s, char *buf, int sz) {
    int n = get_buf_readable(s);
    if (n == 0) {
        if (is_writer_close(s)) {
            logError("Opposite end has been closed.");
            return OPPOSITE_END_CLOSED;
        }
        return 0;
    }
    if (n > sz) {
        n = sz;
    }
    sb_read_n(s, buf, n);
    sb_inc_rc(s, n);
    return n;
}

// NOTE: be careful to read writer's data in reader (You may read dirty data).
int readSyncBufB(SyncBuf s, char *buf, int sz) {
    int offset = 0;
    while (sz > 0) {
        int n = get_buf_readable(s);
        // wait for write event
        if (n == 0) {
            if (is_writer_close(s)) {
                logError("Opposite end has been closed.");
                return OPPOSITE_END_CLOSED;
            } else if (WaitForSingleObject(s->hWEvt, INFINITE) != WAIT_OBJECT_0) {
                logWinError("Failed to wait for write event.");
                return OP_FAILED;
            }
#ifdef SMIPC_TRACE
            printf("[TRACE] read: got WE.\n");
#endif
        } else {
#ifdef SMIPC_TRACE
            printf("[TRACE] read: in[sz=%d ava=%d buf=%p off=%d] rc=%d.\n", sz, n, buf, offset, sb_rc(s));
#endif
            if (n > sz) {
                n = sz;
            }
            sb_read_n(s, buf + offset, n);
            if (sb_inc_rc(s, n) != OP_SUCCEED) {
                return OP_FAILED;
            }
            offset += n;
            sz -= n;
        }
    }
    return offset;
}

int writeSyncBuf(SyncBuf s, const char *data, int sz) {
    int offset = 0;
    while (sz > 0) {
        if (is_reader_close(s)) {
            printf("%d\n", sz);
            logError("Opposite end has been closed.");
            return OPPOSITE_END_CLOSED;
        }
        int n = get_buf_writeable(s);
        // wait for read event
        if (n == 0) {
            if (WaitForSingleObject(s->hREvt, INFINITE) != WAIT_OBJECT_0) {
                logWinError("Failed to wait for read event.");
                return OP_FAILED;
            }
#ifdef SMIPC_TRACE
            printf("[TRACE] write: got RE.\n");
#endif
        } else {
#ifdef SMIPC_TRACE
            printf("[TRACE] write: in[sz=%d ava=%d data=%p off=%d] wc=%d\n", sz, n, data, offset, sb_wc(s));
#endif
            if (n > sz) {
                n = sz;
            }
            sb_write_n(s, (char*)data + offset, n);
            if (sb_inc_wc(s, n) != OP_SUCCEED) {
                return OP_FAILED;
            }
            offset += n;
            sz -= n;
        }
    }
    return OP_SUCCEED;
}

// Mutex

HANDLE lock(String mutexName) {
    HANDLE mutex = CreateMutex(NULL, FALSE, mutexName->val);
    WaitForSingleObject(mutex, INFINITE);
    return mutex;
}

void unlock(HANDLE mutex) {
    ReleaseMutex(mutex);
    CloseHandle(mutex);
}