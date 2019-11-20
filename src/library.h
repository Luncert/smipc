#ifndef SMIPC_LIBRARY_H
#define SMIPC_LIBRARY_H

#include <windows.h>
#include "map.h"
#include "util.h"

const int OP_SUCCEED = 0;
const int OP_FAILED = -1;
const int OPPOSITE_END_CLOSED = -2;

const unsigned char MARK_READER_OPEN     = 0b1000;
const unsigned char MARK_WRITER_OPEN     = 0b0100;
const unsigned char MARK_READER_CLOSE    = 0b0010;
const unsigned char MARK_WRITER_CLOSE    = 0b0001;

const int CHAN_R = 0;
const int CHAN_W = 1;

map_void_t* channelMap;

typedef struct syncBuf {
    unsigned char mark;
    HANDLE hWriteSem, hReadSem;
    int bufSz, rc, wc;
    char *buf;
} *SyncBuf;

typedef struct channel {
    HANDLE hShareMem;
    int mode;
    SyncBuf syncBuf;
} *Channel;

void initLibrary();
void cleanLibrary();
int openChannel(char *cid, int mode, int memSz);
int writeChannel(char *cid, char *data, int len);
int readChannel(char *cid, char *buf, int n, char blocking);
int closeChannel(char *cid);

int createRWSemaphore(String namePrefix, HANDLE *hRSem, HANDLE *hWSem, int count);
SyncBuf newSyncBuf(char *shareMem, int memSz, int mode, String semName, char isNewMem);
int writeSyncBuf(SyncBuf syncBuf, const char *data, int len);
int readSyncBuf(SyncBuf syncBuf, char *buf, int n);
int readSyncBufB(SyncBuf syncBuf, char *buf, int n);

HANDLE lock(String mutexName);
void unlock(HANDLE mutex);

#endif //SMIPC_LIBRARY_H