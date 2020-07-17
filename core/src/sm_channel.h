#ifndef SMIPC_SM_CHANNEL_H
#define SMIPC_SM_CHANNEL_H

#define SMIPC_DLL_API __declspec(dllexport)

#include <windows.h>
#include "map.h"
#include "util.h"

#define OP_SUCCEED 0
#define OP_FAILED -1
#define OPPOSITE_END_CLOSED -2

#define LOG_DISABLE 0 // no log
#define LOG_BASIC   1 // log basic event info: channel created, channel closed .etc
#define LOG_ALL     2 // log all, include write and read details.

#define CHAN_R 0
#define CHAN_W 1

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
    HANDLE hStopEvt1, hStopEvt2;
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

SMIPC_DLL_API void initLibrary(int logMode);
SMIPC_DLL_API void cleanLibrary();
SMIPC_DLL_API int openChannel(const char *cid, int mode, int chanSz);
SMIPC_DLL_API int writeChannel(const char *cid, signed char *data, int start, int end);
SMIPC_DLL_API int readChannel(const char *cid, signed char *buf, int start, int end, unsigned char blocking);
SMIPC_DLL_API int onChannelData(const char *cid, Callback callback);
SMIPC_DLL_API int removeListener(const char *cid);
SMIPC_DLL_API int printChannelStatus(const char *cid);
SMIPC_DLL_API void closeChannel(const char *cid);

SyncBuf newSyncBuf(char *shareMem, int bufSz, int mode, String semName, char isNewMem);
int initSyncBufEvent(String namePrefix, HANDLE *hREvt, HANDLE *hWEvt);
int get_buf_readable(SyncBuf s);
int get_buf_writeable(SyncBuf s);
int asyncReadRoutine(LPVOID lpParam);
int readSyncBuf(SyncBuf syncBuf, signed char *buf, int start, int end);
int readSyncBufB(SyncBuf s, signed char *buf, int start, int end);
int writeSyncBuf(SyncBuf syncBuf, signed char *data, int start, int end);
int releaseSyncBuf(SyncBuf s, int mode);

HANDLE lock(String mutexName);
void unlock(HANDLE mutex);

void sb_read_n(SyncBuf s, signed char *buf, int n);
void sb_write_n(SyncBuf s, signed char *data, int n);
int sb_inc_rc(SyncBuf s, int delta);
int sb_inc_wc(SyncBuf s, int delta);

#endif //SMIPC_SM_CHANNEL_H