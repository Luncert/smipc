//
// Created by Luncert on 2019/11/19.
//

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "util.h"

// log

void logInfo(char* msg) {
    printf("[INFO] %s\n", msg);
}

void logError(char* msg) {
    printf("[ERROR] %s\n", msg);
}

// String

String newEmptyString(int cap) {
    if (cap <= 0) {
        cap = 0;
    }
    String r = (String) malloc(sizeof(struct _String));
    r->len = 0;
    r->cap = cap;
    r->val = (char*)malloc(r->cap + 1);
    memset(r->val, 0, r->cap);
    return r;
}

String parseConstToString(const char *raw) {
    int len = 0;
    while (raw[len] != '\0') {
        len++;
    }
    return parseToString((char*)raw, len);
}

String parseToString(const char *raw, int len) {
    String r = (String) malloc(sizeof(struct _String));
    r->cap = r->len = len;
    r->val = (char*)malloc(len);

    for (int i = 0; i < len; i++) {
        r->val[i] = raw[i];
    }
    r->val[r->len] = 0;
    return r;
}

String cloneString(String s) {
    String r = (String) malloc(sizeof(struct _String));
    r->len = s->len;
    r->cap = s->cap;
    r->val = (char*)malloc(s->len + 1);

    for (int i = 0; i < r->len; i++) {
        r->val[i] = s->val[i];
    }
    r->val[r->len] = 0;

    return r;
}

void appendString(String s, const char *t, int len) {
    if (len <= 0) {
        return;
    }
    if (s->cap - s->len < len) {
        char *val = s->val;
        s->cap = (int)((s->len + len) * 1.5) + 1;
        s->val = (char*)malloc(s->cap + 1);

        int i = 0;
        for (; i < s->len; i++) {
            s->val[i] = val[i];
        }
        free(val);

        for (int limit = s->len + len; i < limit; i++) {
            s->val[i] = t[i - s->len];
        }
        s->len += len;
        s->val[s->len] = 0;
    } else {
        for (int i = 0; i < len; i++) {
            s->val[s->len++] = t[i];
        }
    }
}

String appendToNewString(String s, const char *t, int len) {
    String r = (String)malloc(sizeof(struct _String));
    r->cap = r->len = s->len + len;
    r->val = (char*)malloc(r->cap);

    int i = 0;
    for (; i < s->len; i++) {
        r->val[i] = s->val[i];
    }
    for (; i < r->len; i++) {
        r->val[i] = t[i - s->len];
    }
    r->val[r->len] = 0;
    return r;
}

String concatString(String s1, String s2) {
    return appendToNewString(s1, s2->val, s2->len);
}

int equalString(String a, String b) {
    if (a->len != b->len) {
        return FALSE;
    }
    for (int i = 0; i < a->len; i++) {
        if (a->val[i] != b->val[i]) {
            return FALSE;
        }
    }
    return TRUE;
}

void releaseString(String s) {
    s->len = 0;
    s->cap = 0;
    free(s->val);
    s->val = NULL;
    free(s);
}

// SyncBuf
// TODO: 将SyncBuf映射到共享内存上，通过信号量检测Channel是否关闭

SyncBuf newSyncBuf(String semName, char *buf, int bufSz) {
    String writeSemName = appendToNewString(semName, "#W", 2);
    String readSemName = appendToNewString(semName, "#R", 2);

    HANDLE hWriteSem, hReadSem;
    hWriteSem = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, writeSemName->val);
    if (hWriteSem == NULL) {
        hWriteSem = CreateSemaphore(NULL, bufSz, bufSz, writeSemName->val);
        if (hWriteSem == INVALID_HANDLE_VALUE || hWriteSem == NULL) {
            logError("Failed to create write semaphore for SyncBuf.");
            goto errHandle;
        }
    }
    hReadSem = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, readSemName->val);
    if (hReadSem == NULL) {
        hReadSem = CreateSemaphore(NULL, 0, bufSz, readSemName->val);
        if (hReadSem == INVALID_HANDLE_VALUE || hReadSem == NULL) {
            logError("Failed to create read semaphore for SyncBuf.");
            goto errHandle;
        }
    }

    SyncBuf syncBuf = (SyncBuf)malloc(sizeof(struct _SyncBuf));
    syncBuf->hWriteSem = hWriteSem;
    syncBuf->hReadSem = hReadSem;
    syncBuf->buf = buf;
    syncBuf->bufSz = bufSz;
    syncBuf->rc = syncBuf->wc = 0;
    return syncBuf;

    errHandle:
    releaseString(writeSemName);
    releaseString(readSemName);
    return NULL;
}

void writeSyncBuf(SyncBuf syncBuf, char *data, int len) {
    for (int i = 0; i < len;) {
        WaitForSingleObject(syncBuf->hWriteSem, INFINITE);
        syncBuf->buf[syncBuf->wc] = data[i++];
        syncBuf->wc = (syncBuf->wc + 1) % syncBuf->bufSz;
        ReleaseSemaphore(syncBuf->hReadSem, 1, NULL);
    }
}

int readSyncBuf(SyncBuf syncBuf, char *buf, int n) {
    if (n <= 0) {
        return 0;
    }
    int readable = syncBuf->wc > syncBuf->rc ? syncBuf->wc - syncBuf->rc
            : syncBuf->bufSz + syncBuf->wc - syncBuf->rc;
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
        WaitForSingleObject(syncBuf->hReadSem, INFINITE);
        buf[i] = syncBuf->buf[syncBuf->rc];
        syncBuf->rc = (syncBuf->rc + 1) % syncBuf->bufSz;
        ReleaseSemaphore(syncBuf->hWriteSem, 1, NULL);
    }
    return n;
}

void releaseSyncBuf(SyncBuf syncBuf) {
    CloseHandle(syncBuf->hWriteSem);
    CloseHandle(syncBuf->hReadSem);
    syncBuf->hWriteSem = NULL;
    syncBuf->hReadSem = NULL;
    syncBuf->buf = NULL;
    syncBuf->bufSz = 0;
    syncBuf->rc = syncBuf->wc = 0;
    free(syncBuf);
}
