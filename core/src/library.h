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
    HANDLE hWEvt, hREvt;
    char *buf;
    struct _shared {
        int bufSz, rc, wc;
        unsigned char state, mark;
    } *shared;
} *SyncBuf;

typedef struct channel {
    HANDLE hShareMem;
    int mode;
    SyncBuf syncBuf;
} *Channel;

void initLibrary(int isTraceMode);
void cleanLibrary();
int openChannel(char *cid, int mode, int chanSz);
int writeChannel(char *cid, char *data, int len);
int readChannel(char *cid, char *buf, int n, char blocking);
int printChannelStatus(char *cid);
int closeChannel(char *cid);

#endif //SMIPC_LIBRARY_H