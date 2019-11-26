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

//#define SMIPC_TRACE

typedef void (*Callback)(char *data, int sz);

typedef struct syncBuf {
    HANDLE hWEvt, hREvt;
    char *buf;
    struct _shared {
        int bufSz, rc, wc;
        unsigned char state, mark;
    } *shared;
} *SyncBuf;

typedef struct dataListener {
    DWORD threadID;
    HANDLE hThread, hStopEvt1, hStopEvt2;
} *DataListener;

typedef struct asyncReadInfo {
    SyncBuf syncBuf;
    HANDLE hStopEvt1, hStopEvt2;
    Callback callback;
} *AsyncReadInfo;

typedef struct channel {
    int mode;
    HANDLE hShareMem;
    SyncBuf syncBuf;
    DataListener dataListener;
} *Channel;

void initLibrary(int isTraceMode);
void cleanLibrary();
int openChannel(char *cid, int mode, int chanSz);
int writeChannel(char *cid, char *data, int len);
int readChannel(char *cid, char *buf, int n, char blocking);
int onChannelData(char *cid, Callback callback);
int removeListener(char *cid);
int printChannelStatus(char *cid);
void closeChannel(char *cid);

SyncBuf newSyncBuf(char *shareMem, int bufSz, int mode, String semName, char isNewMem);
int initSyncBufEvent(String namePrefix, HANDLE *hREvt, HANDLE *hWEvt);
int get_buf_readable(SyncBuf s);
int get_buf_writeable(SyncBuf s);
int asyncReadRoutine(LPVOID lpParam);
int readSyncBuf(SyncBuf syncBuf, char *buf, int sz);
int readSyncBufB(SyncBuf s, char *buf, int sz);
int writeSyncBuf(SyncBuf syncBuf, const char *data, int sz);
int releaseSyncBuf(SyncBuf s, int mode);

HANDLE lock(String mutexName);
void unlock(HANDLE mutex);

void sb_read_n(SyncBuf s, char *buf, int n);
void sb_write_n(SyncBuf s, char *data, int n);
int sb_inc_rc(SyncBuf s, int delta);
int sb_inc_wc(SyncBuf s, int delta);
#endif //SMIPC_LIBRARY_H