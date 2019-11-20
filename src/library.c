#include "library.h"

#include <stdio.h>
#include <windows.h>
#include "map.h"
#include "util.h"

void initLibrary() {
    if (channelMap == NULL) {
        channelMap = (map_void_t*)malloc(sizeof(map_void_t));
        map_init(channelMap);
    }
}

void cleanLibrary() {
    if (channelMap != NULL) {
        map_deinit(channelMap);
        free(channelMap);
        channelMap = NULL;
    }
}

/**
 * openChannel
 * @param cid channel id
 * @param mode channel mode, CHAN_R or CHAN_W
 * @param memSz vailable memory size for channel
 * @return OP_SUCCEED, OP_FAILED
 */
int openChannel(char *cid, int mode, int memSz) {
    if (mode != CHAN_R && mode != CHAN_W) {
        logError("invalid channel mode, must be CHAN_R(0) or CHAN_W(1).");
        return OP_FAILED;
    }

    Channel channel = (Channel)map_get(channelMap, cid);
    if (channel != NULL) {
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
        if (memSz < 128) {
            memSz = 128;
            logWarn("memSz must be bigger than 128Bytes, auto adjusted.");
        }
        hShareMem = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, memSz, cid);
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
        memset(shareMem, 0, memSz);
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
    SyncBuf syncBuf = newSyncBuf(shareMem, memSz, mode, semName, isNewMem);
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
    Channel channel = (Channel)map_get(channelMap, cid);
    if (channel == NULL) {
        logError("Channel doesn't exist, make sure open it at first.");
        return OP_FAILED;
    }
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
    Channel channel = (Channel)map_get(channelMap, cid);
    if (channel == NULL) {
        logError("Channel doesn't exist, make sure open it at first.");
        return OP_FAILED;
    }
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
 * closeChannel
 * @param cid channel id
 * @return OP_SUCCEED, OP_FAILED
 */
int closeChannel(char *cid) {
    Channel channel = (Channel)map_get(channelMap, cid);
    if (channel == NULL) {
        logError("Cannot close nonexistent channel.");
        return OP_FAILED;
    }
    map_remove(channelMap, cid);
    // release SyncBuf
    SyncBuf syncBuf = channel->syncBuf;
    // release respective windows resources if the opposite end has been closed.
    if ((channel->mode == CHAN_R && syncBuf->mark & MARK_WRITER_CLOSE)
        || syncBuf->mark & MARK_READER_CLOSE) {
        CloseHandle(syncBuf->hWriteSem);
        CloseHandle(syncBuf->hReadSem);
        syncBuf->hWriteSem = NULL;
        syncBuf->hReadSem = NULL;
        CloseHandle(channel->hShareMem);
    } else {
        // update mark
        if (channel->mode == CHAN_R) {
            syncBuf->mark |= MARK_READER_CLOSE;
        } else {
            syncBuf->mark |= MARK_WRITER_CLOSE;
        }
    }
    channel->hShareMem = NULL;
    channel->syncBuf = NULL;
    free(channel);
    return OP_SUCCEED;
}

// SyncBuf

SyncBuf newSyncBuf(char *shareMem, int memSz, int mode, String semName, char isNewMem) {
    SyncBuf syncBuf = (SyncBuf)shareMem;
    if (isNewMem) {
        // set mark
        if (mode == CHAN_R) {
            syncBuf->mark |= MARK_READER_OPEN;
        } else {
            syncBuf->mark |= MARK_WRITER_OPEN;
        }
        // rc and wc should be set zero and we have done that.
        syncBuf->bufSz = memSz - (int)sizeof(struct syncBuf);
        // create sem
        if (createRWSemaphore(semName, &syncBuf->hWriteSem, &syncBuf->hReadSem, syncBuf->bufSz) !=  OP_SUCCEED) {
            return NULL;
        }
        syncBuf->buf = shareMem + sizeof(struct syncBuf);
    } else {
        // check memory mark
        if (mode == CHAN_R) {
            if (syncBuf->mark & MARK_READER_CLOSE) {
                logError("Channel has been closed from reader's side.");
                return NULL;
            } else if (syncBuf->mark & MARK_READER_OPEN) {
                logError("Channel has been opened from reader's side by another process.");
                return NULL;
            }
        } else {
            if (syncBuf->mark & MARK_WRITER_CLOSE) {
                logError("Channel has been closed from writer's side.");
                return NULL;
            } else if (syncBuf->mark & MARK_WRITER_OPEN) {
                logError("Channel has been opened from writer's side by another process.");
                return NULL;
            }
        }
        // every attr of SyncBuf could be read from the share memory.
    }
    return syncBuf;
}

int createRWSemaphore(String namePrefix, HANDLE *hRSem, HANDLE *hWSem, int count) {
    String writeSemName = appendToNewString(namePrefix, "#W", 2);
    String readSemName = appendToNewString(namePrefix, "#R", 2);

    *hWSem = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, writeSemName->val);
    if (*hWSem == NULL) {
        hWSem = CreateSemaphore(NULL, count, count, writeSemName->val);
        if (hWSem == INVALID_HANDLE_VALUE || hWSem == NULL) {
            logError("Failed to create write semaphore for SyncBuf.");
            goto errHandle;
        }
    }

    *hRSem = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, readSemName->val);
    if (*hRSem == NULL) {
        hRSem = CreateSemaphore(NULL, 0, count, readSemName->val);
        if (hRSem == INVALID_HANDLE_VALUE || hRSem == NULL) {
            logError("Failed to create read semaphore for SyncBuf.");
            CloseHandle(*hWSem);
            goto errHandle;
        }
    }

    return OP_SUCCEED;

    errHandle:
    releaseString(writeSemName);
    releaseString(readSemName);
    return OP_FAILED;
}

int writeSyncBuf(SyncBuf syncBuf, const char *data, int len) {
    for (int i = 0; i < len;) {
        if (syncBuf->mark & MARK_READER_CLOSE) {
            return OPPOSITE_END_CLOSED;
        }
        WaitForSingleObject(syncBuf->hWriteSem, INFINITE);
        syncBuf->buf[syncBuf->wc] = data[i++];
        syncBuf->wc = (syncBuf->wc + 1) % syncBuf->bufSz;
        ReleaseSemaphore(syncBuf->hReadSem, 1, NULL);
    }
    return OP_SUCCEED;
}

int readSyncBuf(SyncBuf syncBuf, char *buf, int n) {
    if (n <= 0) {
        return 0;
    }
    int readable = syncBuf->wc > syncBuf->rc ? syncBuf->wc - syncBuf->rc
                                             : syncBuf->bufSz + syncBuf->wc - syncBuf->rc;
    if (readable == 0 && syncBuf->mark & MARK_WRITER_CLOSE) {
        return OPPOSITE_END_CLOSED;
    }
    if (n > readable) {
        n = readable;
    }
    for (int i = 0; i < n; i++) {
        WaitForSingleObject(syncBuf->hReadSem, INFINITE);
        buf[i] = syncBuf->buf[syncBuf->rc];
        syncBuf->rc = (syncBuf->rc + 1) % syncBuf->bufSz;
        ReleaseSemaphore(syncBuf->hWriteSem, 1, NULL);
    }
    return n;
}

int readSyncBufB(SyncBuf syncBuf, char *buf, int n) {
    if (n <= 0) {
        return 0;
    }
    for (int i = 0; i < n; i++) {
        if (syncBuf->mark & MARK_WRITER_CLOSE) {
            int tmp = readSyncBuf(syncBuf, buf + i, n - i);
            if (tmp >= 0) {
                tmp += i;
            }
            return tmp;
        }
        WaitForSingleObject(syncBuf->hReadSem, INFINITE);
        buf[i] = syncBuf->buf[syncBuf->rc];
        syncBuf->rc = (syncBuf->rc + 1) % syncBuf->bufSz;
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