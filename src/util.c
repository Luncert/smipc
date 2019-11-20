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
