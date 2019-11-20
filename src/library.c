#include "library.h"

#include <stdio.h>
#include <windows.h>
#include "map.h"
#include "util.h"

const int OP_SUCCEED = 0;
const int OP_FAILED = 1;
const int ERR_INVALID_PARAM = 2;

typedef struct channel {
    HANDLE hShareMem;
    char *buf;
    int memSz;
} *Channel;

map_void_t* channelMap;

void initLibrary() {
    if (channelMap == NULL) {
        channelMap = (map_void_t*)malloc(sizeof(map_void_t));
        map_init(channelMap);
    }
}

void cleanLibrary() {
    if (channelMap != NULL) {
        map_deinit(channelMap);
        free(channelMap);
        channelMap = NULL;
    }
}

int createChannel(String channelID, int memSZ) {
    // check params
    if (channelID == NULL || channelID->len == 0) {
        logError("Channel ID must be not either null nor empty string.");
        return ERR_INVALID_PARAM;
    }
    if (map_get(channelMap, channelID->val) != NULL) {
        logError("Channel ID has been used.");
        return ERR_INVALID_PARAM;
    }
    if (memSZ < 128) {
        logError("Mem size must be at least 128Bytes.");
        return ERR_INVALID_PARAM;
    }

    // no: physical file handle, lpAttributes
    HANDLE hShareMem = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, memSZ, channelID->val);

    // check error
    if (hShareMem == INVALID_HANDLE_VALUE || hShareMem == NULL) {
        logError("Failed to create shared memory.");
        return OP_FAILED;
    }
    DWORD err = GetLastError();
    if (err == ERROR_FILE_INVALID) {
        logError("Duplicate channel ID, which has been used by another object (shared memory, mutex, semaphore or critical area).");
        return OP_FAILED;
    } else if (err == ERROR_ALREADY_EXISTS) {
        logError("Duplicate channel ID, which has been used to create a shared memory.");
        return OP_FAILED;
    } else {
        printf("[DEBUG] Channel was created with channel ID %s\n", channelID->val);
    }

    char *szShareMem = (char*) MapViewOfFile(hShareMem,FILE_MAP_WRITE|FILE_MAP_READ,
                                             0,0,0);

    // create channel
    Channel channel = (Channel)malloc(sizeof(struct channel));
    channel->hShareMem = hShareMem;
    channel->memSz = memSZ;
    channel->buf = szShareMem;
    map_set(channelMap, channelID->val, channel);

    return OP_SUCCEED;
}

int openChannel(String channelID, Channel *pChannel) {
    *pChannel = (Channel)map_get(channelMap, channelID->val);
    if (*pChannel != NULL) {
        return OP_SUCCEED;
    }

    HANDLE hShareMem = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, channelID->val);
    if (hShareMem == NULL || hShareMem == INVALID_HANDLE_VALUE) {
        logError("Failed to open channel.");
        return OP_FAILED;
    }

    char *szShareMem = (char*) MapViewOfFile(hShareMem,FILE_MAP_WRITE|FILE_MAP_READ,
                                             0,0,0);

    Channel channel = (Channel)malloc(sizeof(struct channel));
    channel->hShareMem = hShareMem;
    channel->buf = szShareMem;
    *pChannel = channel;
    map_set(channelMap, channelID->val, channel);

    return OP_SUCCEED;
}

int readChannel(String channelID) {
    // check params
    if (channelID == NULL || channelID->len == 0) {
        logError("Channel ID must be not either null nor empty string.");
        return ERR_INVALID_PARAM;
    }

}

int writeChannel(String channelID) {

}

int closeChannel(String channelID) {

}
