//
// Created by Luncert on 2019/11/19.
//

#ifndef SMIPC_UTIL_H
#define SMIPC_UTIL_H

#include <windows.h>

#define TRUE 1
#define FALSE 0

void logInfo(char* msg);
void logError(char* msg);

typedef struct _String {
    int len;
    int cap;
    char* val;
} *String;

String newEmptyString(int cap);
String parseConstToString(const char *raw);
String parseToString(const char *raw, int len);
String cloneString(String s);
void appendString(String s, const char *t, int len);
String appendToNewString(String s, const char *t, int len);
String concatString(String s1, String s2);
int equalString(String a, String b);
void releaseString(String s);

typedef struct _SyncBuf {
    HANDLE hWriteSem, hReadSem;
    char *buf;
    int bufSz;
    int rc, wc;
} *SyncBuf;

SyncBuf newSyncBuf(String semName, char *buf, int bufSz);
void writeSyncBuf(SyncBuf syncBuf, char *data, int len);
int readSyncBuf(SyncBuf syncBuf, char *buf, int n);
int readSyncBufB(SyncBuf syncBuf, char *buf, int n);
void releaseSyncBuf(SyncBuf syncBuf);

#endif //SMIPC_UTIL_H
