//
// Created by Luncert on 2019/11/19.
//

#ifndef SMIPC_UTIL_H
#define SMIPC_UTIL_H

#include <windows.h>

#define TRUE 1
#define FALSE 0

void logInfo(char* msg);
void logWarn(char *msg);
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

#endif //SMIPC_UTIL_H
