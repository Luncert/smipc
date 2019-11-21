#ifndef SMIPC_LIBRARY_H
#define SMIPC_LIBRARY_H

#include <windows.h>
#include "map.h"
#include "util.h"

#define OP_SUCCEED 0
#define OP_FAILED -1
#define OPPOSITE_END_CLOSED -2

#define CHAN_R 0
#define CHAN_W 1

typedef struct syncBuf {
    HANDLE hWriteSem, hReadSem;
    struct _shared {
        unsigned char mark;
        int bufSz, rc, wc;
        char *buf;
    } *shared;
} *SyncBuf;

typedef struct channel {
    HANDLE hShareMem;
    int mode;
    SyncBuf syncBuf;
} *Channel;

void initLibrary();
void cleanLibrary();
int openChannel(char *cid, int mode, int chanSz);
int writeChannel(char *cid, char *data, int len);
int readChannel(char *cid, char *buf, int n, char blocking);
int printChannelStatus(char *cid);
int closeChannel(char *cid);

int createRWSemaphore(String namePrefix, HANDLE *hRSem, HANDLE *hWSem, int count);
SyncBuf newSyncBuf(char *shareMem, int memSz, int mode, String semName, char isNewMem);
int writeSyncBuf(SyncBuf syncBuf, const char *data, int len);
int readSyncBuf(SyncBuf syncBuf, char *buf, int n);
int readSyncBufB(SyncBuf syncBuf, char *buf, int n);

HANDLE lock(String mutexName);
void unlock(HANDLE mutex);

#endif //SMIPC_LIBRARY_H