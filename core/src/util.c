//
// Created by Luncert on 2019/11/19.
//

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "util.h"

// log

const char *logLevelNames[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};
int logLevel = LOG_LEVEL_DEBUG;

void setLogLevel(int l) {
    if (l < LOG_LEVEL_TRACE || l > LOG_LEVEL_DISABLE) {
        printf("[ERROR] Invalid log level value %d.\n", l);
        return;
    }
    logLevel = l;
}

inline int allowLog(int level) {
   return level >= logLevel;
}

inline void _log(int l, char *msg) {
    if (l >= logLevel) {
        printf("[%s] %s\n", logLevelNames[l], msg);
    }
}
\
void logTrace(char *msg) {
    _log(LOG_LEVEL_TRACE, msg);
}

void logDebug(char *msg) {
    _log(LOG_LEVEL_DEBUG, msg);
}

void logInfo(char* msg) {
    _log(LOG_LEVEL_INFO, msg);
}

void logWarn(char *msg) {
    _log(LOG_LEVEL_WARN, msg);
}

void logError(char *msg) {
    _log(LOG_LEVEL_ERROR, msg);
}

void logFatal(char *msg) {
    _log(LOG_LEVEL_FATAL, msg);
    exit(-1);
}

void logWinError(char *msg) {
    if (LOG_LEVEL_ERROR >= logLevel) {
        void * lpMsgBuf;
        FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char *)&lpMsgBuf, 0, NULL);
        printf("[ERROR] %s%s", msg, lpMsgBuf);
    }
}

// String

String newEmptyString(int cap) {
    if (cap <= 0) {
        cap = 0;
    }
    String r = (String)malloc(sizeof(struct _String));
    r->len = 0;
    r->cap = cap;
    r->val = (char*)malloc(r->len + 1);
    memset(r->val, 0, r->cap);
    return r;
}

String parseConstToString(const char *raw) {
    int len = 0;
    while (raw[++len] != '\0');
    return parseToString((char*)raw, len);
}

String parseToString(const char *raw, int len) {
    String r = (String)malloc(sizeof(struct _String));
    r->cap = r->len = len;
    r->val = (char*)malloc(r->len + 1);
    memcpy(r->val, raw, len);
    r->val[r->len] = 0;
    return r;
}

String cloneString(String s) {
    String r = (String)malloc(sizeof(struct _String));
    r->len = s->len;
    r->cap = s->cap;
    r->val = (char*)malloc(s->len + 1);
    memcpy(r->val, s->val, r->len);
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
        memcpy(s->val, val, s->len);
        free(val);
    }
    memcpy(s->val + s->len, t, len);
    s->len += len;
    s->val[s->len] = 0;
}

String appendToNewString(String s, const char *t, int len) {
    String r = (String)malloc(sizeof(struct _String));
    r->cap = r->len = s->len + len;
    r->val = (char*)malloc(r->len + 1);
    memcpy(r->val, s->val, s->len);
    memcpy(r->val + s->len, t, len);
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
    return memcmp(a->val, b->val, a->len) == 0 ? TRUE : FALSE;
}

void releaseString(String s) {
    s->len = 0;
    s->cap = 0;
    free(s->val);
    s->val = NULL;
    free(s);
}
