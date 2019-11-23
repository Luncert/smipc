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

void logWarn(char *msg) {
    printf("[WARN] %s\n", msg);
}

void logError(char *msg) {
    printf("[ERROR] %s\n", msg);
}

void logFatal(char *msg) {
    printf("[FATAL] %s\n", msg);
    exit(-1);
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
