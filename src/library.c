
#include <stdio.h>
#include <windows.h>
#include "map.h"
#include "util.h"
#include "library.h"

const unsigned char MARK_READER_OPEN     = 0b1000;
const unsigned char MARK_WRITER_OPEN     = 0b0100;
const unsigned char MARK_READER_CLOSE    = 0b0010;
const unsigned char MARK_WRITER_CLOSE    = 0b0001;

#define is_reader_open(syncBuf)     (syncBuf->shared->mark & MARK_READER_OPEN)
#define is_writer_open(syncBuf)     (syncBuf->shared->mark & MARK_WRITER_OPEN)
#define is_reader_close(syncBuf)    (syncBuf->shared->mark & MARK_READER_CLOSE)
#define is_writer_close(syncBuf)    (syncBuf->shared->mark & MARK_WRITER_CLOSE)
#define set_reader_open(syncBuf)    (syncBuf->shared->mark |= MARK_READER_OPEN)
#define set_writer_open(syncBuf)    (syncBuf->shared->mark |= MARK_WRITER_OPEN)
#define set_reader_close(syncBuf)   (syncBuf->shared->mark |= MARK_READER_CLOSE)
#define set_writer_close(syncBuf)   (syncBuf->shared->mark |= MARK_WRITER_CLOSE)
#define get_buf_readable(syncBuf)   (syncBuf->shared->wc > syncBuf->shared->rc ? syncBuf->shared->wc - syncBuf->shared->rc : syncBuf->shared->bufSz + syncBuf->shared->wc - syncBuf->shared->rc)
#define sb_buf(syncBuf) (syncBuf->shared->buf)
#define sb_bufSz(syncBuf) (syncBuf->shared->bufSz)
#define sb_rc(syncBuf) (syncBuf->shared->rc)
#define sb_wc(syncBuf) (syncBuf->shared->wc)

typedef map_t(Channel) channel_map_t;

/**
 * channel_map_t: map_set会把值做一个拷贝这样就可以自己管理值需要的内存，map_get则会返回自己管理的值的内存的指针，而不是值
 */
channel_map_t* channelMap;

void initLibrary() {
    if (channelMap == NULL) {
        channelMap = (channel_map_t*)malloc(sizeof(channel_map_t));
        map_init(channelMap);
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
        logError("invalid channel mode, must be CHAN_R(0) or CHAN_W(1).");
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
        if (chanSz < 128) {
            chanSz = 128;
            logWarn("chanSz must be bigger than 128Bytes, auto adjusted.");
        }
        hShareMem = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, chanSz + sizeof(struct syncBuf), cid);
        // check error
        if (hShareMem == INVALID_HANDLE_VALUE || hShareMem == NULL) {
            logError("Failed to create shared memory.");
            return OP_FAILED;
        }
        DWORD err = GetLastError();
        if (err == ERROR_FILE_INVALID) {
            logError("Duplicate channel ID, which has been used by another object (shared memory, mutex, semaphore or critical area).");
            return OP_FAILED;
        } else if (err == ERROR_ALREADY_EXISTS) {
            logError("Duplicate channel ID, which has been used to create a shared memory.");
            return OP_FAILED;
        } else {
            printf("[DEBUG] New channel was created with ID %s\n", cid);
        }

        // get shared memory's pointer
        shareMem = (char*)MapViewOfFile(hShareMem,FILE_MAP_WRITE|FILE_MAP_READ,
                                          0,0,0);
        memset(shareMem, 0, sizeof(struct syncBuf));
        isNewMem = TRUE;
    } else {
        shareMem = (char*)MapViewOfFile(hShareMem,FILE_MAP_WRITE|FILE_MAP_READ,
                                        0,0,0);
    }

    // lock
    String mutexName = parseConstToString(cid);
    appendString(mutexName, "/mutex", 6);
    HANDLE mutex = lock(mutexName);
    releaseString(mutexName);

    String semName = parseConstToString(cid);
    appendString(semName, "/sem", 4);
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
    return writeSyncBuf(channel->syncBuf, data, len);
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
    printf(" hReadSem=%s", syncBuf->hReadSem != NULL ? "valid" : "invalid");
    printf(" hWriteSem=%s", syncBuf->hWriteSem != NULL ? "valid" : "invalid");
    printf(" bufSz=%d readCursor=%d writeCursor=%d", syncBuf->shared->bufSz, syncBuf->shared->rc, syncBuf->shared->wc);
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

    // release windows resources
    if (CloseHandle(syncBuf->hWriteSem) == FALSE) {
        DWORD err = GetLastError();
        printf("[ERROR] Failed to close windows resource, err=%ld\n", err);
        return OP_FAILED;
    }
    if (CloseHandle(syncBuf->hReadSem) == FALSE) {
        DWORD err = GetLastError();
        printf("[ERROR] Failed to close windows resource, err=%ld\n", err);
        return OP_FAILED;
    }
    if (CloseHandle(channel->hShareMem) == FALSE) {
        DWORD err = GetLastError();
        printf("[ERROR] Failed to close windows resource, err=%ld\n", err);
        return OP_FAILED;
    }

    free(syncBuf);
    free(channel);

    printf("[DEBUG] Channel closed: %s\n", cid);
    return OP_SUCCEED;
}

// SyncBuf

SyncBuf newSyncBuf(char *shareMem, int bufSz, int mode, String semName, char isNewMem) {
    SyncBuf syncBuf = (SyncBuf)malloc(sizeof(struct syncBuf));
    syncBuf->shared = (struct _shared*)shareMem;
    if (isNewMem) {
        // set mark
        if (mode == CHAN_R) {
            set_reader_open(syncBuf);
        } else {
            set_writer_open(syncBuf);
        }
        // rc and wc should be set zero and we have done that.
        syncBuf->shared->bufSz = bufSz;
        syncBuf->shared->buf = shareMem + sizeof(struct _shared);
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
    // create sem
    if (createRWSemaphore(semName, &syncBuf->hWriteSem, &syncBuf->hReadSem, syncBuf->shared->bufSz) != OP_SUCCEED) {
        return NULL;
    }
    return syncBuf;
}

int createRWSemaphore(String namePrefix, HANDLE *hWSem, HANDLE *hRSem, int count) {
    int ret = OP_SUCCEED;

    String writeSemName = appendToNewString(namePrefix, "#W", 2);
    String readSemName = appendToNewString(namePrefix, "#R", 2);

    *hWSem = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, writeSemName->val);
    if (*hWSem == NULL) {
        *hWSem = CreateSemaphore(NULL, count, count, writeSemName->val);
        if (*hWSem == INVALID_HANDLE_VALUE || *hWSem == NULL) {
            logError("Failed to create write semaphore for SyncBuf.");
            ret = OP_FAILED;
        }
    }

    if (ret == OP_SUCCEED) {
        *hRSem = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, readSemName->val);
        if (*hRSem == NULL) {
            *hRSem = CreateSemaphore(NULL, 0, count, readSemName->val);
            if (*hRSem == INVALID_HANDLE_VALUE || *hRSem == NULL) {
                logError("Failed to create read semaphore for SyncBuf.");
                CloseHandle(*hWSem);
                ret = OP_FAILED;
            }
        }
    }

    releaseString(writeSemName);
    releaseString(readSemName);
    return ret;
}

int writeSyncBuf(SyncBuf syncBuf, const char *data, int len) {
    for (int i = 0; i < len;) {
        if (is_reader_close(syncBuf)) {
            return OPPOSITE_END_CLOSED;
        }
        if (WaitForSingleObject(syncBuf->hWriteSem, INFINITE) == WAIT_FAILED) {
            logError("Failed to wait write sem.");
            return OP_FAILED;
        }
        sb_buf(syncBuf)[sb_wc(syncBuf)] = data[i++];
        sb_wc(syncBuf) = (sb_wc(syncBuf) + 1) % sb_bufSz(syncBuf);
        ReleaseSemaphore(syncBuf->hReadSem, 1, NULL);
    }
    return OP_SUCCEED;
}

int readSyncBuf(SyncBuf syncBuf, char *buf, int n) {
    if (n <= 0) {
        return 0;
    }
    int readable = get_buf_readable(syncBuf);
    if (readable == 0 && is_writer_close(syncBuf)) {
        return OPPOSITE_END_CLOSED;
    }
    if (n > readable) {
        n = readable;
    }
    for (int i = 0; i < n; i++) {
        if (WaitForSingleObject(syncBuf->hReadSem, INFINITE) == WAIT_FAILED) {
            logError("Failed to wait read sem.");
            return OP_FAILED;
        }
        buf[i] = syncBuf->shared->buf[syncBuf->shared->rc];
        syncBuf->shared->rc = (syncBuf->shared->rc + 1) % syncBuf->shared->bufSz;
        ReleaseSemaphore(syncBuf->hWriteSem, 1, NULL);
    }
    return n;
}

int readSyncBufB(SyncBuf syncBuf, char *buf, int n) {
    if (n <= 0) {
        return 0;
    }
    for (int i = 0; i < n; i++) {
        if (is_writer_close(syncBuf)) {
            int tmp = readSyncBuf(syncBuf, buf + i, n - i);
            if (tmp >= 0) {
                tmp += i;
            }
            return tmp;
        }
        if (WaitForSingleObject(syncBuf->hReadSem, INFINITE) == WAIT_FAILED) {
            logError("Failed to wait read sem.");
            return OP_FAILED;
        }
        buf[i] = sb_buf(syncBuf)[sb_rc(syncBuf)];
        sb_rc(syncBuf) = (sb_rc(syncBuf) + 1) % sb_bufSz(syncBuf);
        ReleaseSemaphore(syncBuf->hWriteSem, 1, NULL);
    }
    return n;
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