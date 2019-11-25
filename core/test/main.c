#include <stdio.h>
#include <stdlib.h>
#include <util.h>
#include <string.h>
#include <library.h>
#include <time.h>
#include "crc.h"

void testString() {
    printf("[test String]\n");
    String s1 = parseConstToString("Hi");
    if (s1->len != 2 || s1->cap != 2 || memcmp(s1->val, "Hi", 2) != 0) {
        logFatal("Test failed 1: s1.");
    }
    appendString(s1, ", ", 2);
    if (s1->len != 4 || s1->cap != 7 || memcmp(s1->val, "Hi, ", 4) != 0) {
        logFatal("Test failed 2: s1.");
    }
    String s2 = parseToString("Luncert", 3);
    if (s2->len != 3 || s2->cap != 3 || memcmp(s2->val, "Lun", 3) != 0) {
        logFatal("Test failed 3: s2.");
    }
    appendString(s2, "!", 1);
    if (s2->len != 4 || s2->cap != 7 || memcmp(s2->val, "Lun!", 4) != 0) {
        logFatal("Test failed 4: s2.");
    }
    String s3 = concatString(s1, s2);
    if (s3->len != 8 || s3->cap != 8 || memcmp(s3->val, "Hi, Lun!", 8) != 0) {
        logFatal("Test failed 5: s3.");
    }
    releaseString(s1);
    releaseString(s2);
    releaseString(s3);
    printf("All passed.\n");
}

char *cid = "test-chan";

void simple_reader() {
    logInfo("Start as reader.");
    if (openChannel(cid, CHAN_R, 128) != OP_SUCCEED) {
        return;
    }

    char buf[128] = {0};
    printChannelStatus(cid);

    for (int i = 0; i < 3; i++) {
        int ret = readChannel(cid, buf, 9, TRUE);
        printf("READ:%s RET:%d\n", buf, ret);
        printChannelStatus(cid);
    }

    closeChannel(cid);
}

void simple_writer() {
    if (openChannel(cid, CHAN_W, 128) != OP_SUCCEED) {
        return;
    }

    printChannelStatus(cid);
    char *data[] = {
            "test/id=1",
            "test/id=2",
            "test/id=3"
    };
    for (int i = 0; i < 3; i++) {
        writeChannel(cid, data[i], 9);
        printChannelStatus(cid);
        Sleep(1000);
    }

    closeChannel(cid);
}

void benchmark_reader() {
    struct timeb st, et;

    int frameSz = 1920 * 1080;
    int chanSz = frameSz * 3;

    // prepare test buf
    char *buf = (char*)malloc(frameSz);
    buf[0] = 0;
    buf[frameSz - 1] = 0;

    if (openChannel(cid, CHAN_R, chanSz) != OP_SUCCEED) {
        return;
    }
    printChannelStatus(cid);
    for (int i = 0; i < 3; i++) {
        ftime(&st);
        int ret = readChannel(cid, buf, frameSz, TRUE);
        ftime(&et);
        printf("RET:%d buf[0]:%d buf[-1]:%d rt: %dms\n", ret, buf[0], buf[frameSz - 1], et.millitm - st.millitm);
        printChannelStatus(cid);
        // reset
        buf[0] = 0;
        buf[frameSz - 1] = 0;
    }
    closeChannel(cid);

    free(buf);
}

void benchmark_writer() {
    struct timeb st, et;

    int frameSz = 1920 * 1080;
    int chanSz = frameSz * 3;

    // prepare test data
    char *data = (char*)malloc(frameSz);
    data[0] = 0x8;
    data[frameSz - 1] = 0xf;

    if (openChannel(cid, CHAN_W, chanSz) != OP_SUCCEED) {
        return;
    }
    printChannelStatus(cid);
    for (int i = 0; i < 3; i++) {
        ftime(&st);
        writeChannel(cid, data, frameSz);
        ftime(&et);
        printf("wt: %dms\n", et.millitm - st.millitm);
        printChannelStatus(cid);
    }
    closeChannel(cid);

    free(data);
}

const int chanSz = 64;
const int dataSz = 10247;
const int totalSz = dataSz + 2;

void printBuf(char *buf, int sz) {
    printf("[");
    if (sz > 0) {
        for (int i = 0; i < sz - 1; i++) {
            printf("%d ", buf[i]);
        }
        printf("%d", buf[sz - 1]);
    }
    printf("]\n");
}

char *buildTestData() {
    char *buf = (char*)malloc(totalSz);

    srand((unsigned)time(NULL));
    for (int i = 0; i < dataSz; i++) {
        buf[i] = (char)(rand() % 256 - 128);
    }
    // calc crc
    crcInit();
    u_short crc = crcCompute((u_char*)buf, dataSz);
    buf[dataSz] = (char)(crc >> 8);
    buf[dataSz + 1] = (char)crc;

    return buf;
}

int isDataValid(char *data) {
    crcInit();
    return crcCompute((u_char*)data, totalSz) == 0;
}

// test case 1 (build data bigger than channel, send it at one time)

void test1_reader() {
    if (openChannel(cid, CHAN_R, chanSz) != OP_SUCCEED) {
        return;
    }
    printChannelStatus(cid);

    char *buf = (char*)malloc(totalSz);
    int n = readChannel(cid, buf, totalSz, TRUE);
    if (n < 0 || n != totalSz) {
        printf("[ERROR] Test failed, read failed, ret=%d\n", n);
        goto clean;
    }

    // check data
    if (isDataValid(buf) == FALSE) {
        printf("[ERROR] Test failed, data incorrect.\n");
        goto clean;
    }

    logInfo("Test passed.");

    clean:
    printChannelStatus(cid);
    closeChannel(cid);
    free(buf);
}

void test1_writer() {
    if (openChannel(cid, CHAN_W, chanSz) != OP_SUCCEED) {
        return;
    }
    printChannelStatus(cid);

    char *buf = buildTestData();
    if (writeChannel(cid, buf, totalSz) != OP_SUCCEED) {
        logError("Test failed, write failed.");
        goto clean;
    }

    logInfo("Test passed.");

    clean:
    printChannelStatus(cid);
    closeChannel(cid);
    free(buf);
}

// test case 2 (build data bigger than channel, break into many parts and send to receiver)

void test2_reader() {
    const int blockSz = 9;

    if (openChannel(cid, CHAN_R, chanSz) != OP_SUCCEED) {
        return;
    }
    printChannelStatus(cid);

    char *buf = (char*)malloc(totalSz);
    // read
    for (int offset = 0, rest = totalSz, n; rest > 0; offset += n, rest -= n) {
        n = readChannel(cid, buf + offset, rest < blockSz ? rest : blockSz, TRUE);
        if (n < 0) {
            printf("[ERROR] Test failed, read failed, ret=%d\n", n);
            goto clean;
        }
        printChannelStatus(cid);
    }

    // check data
    if (!isDataValid(buf)) {
        printf("[ERROR] Test failed, data incorrect.\n");
        goto clean;
    }

    logInfo("Test passed.");

    clean:
    closeChannel(cid);
    free(buf);
}

void test2_writer() {
    const int blockSz = 7;

    if (openChannel(cid, CHAN_W, chanSz) != OP_SUCCEED) {
        return;
    }
    printChannelStatus(cid);

    char *buf = buildTestData();

    // send
    for (int offset = 0, rest = totalSz, n; rest > 0;  offset += n, rest -= n) {
        n = rest < blockSz ? rest : blockSz;
        if (writeChannel(cid, buf + offset, n) != OP_SUCCEED) {
            logError("Test failed, write failed.");
            goto clean;
        }
        printChannelStatus(cid);
    }

    logInfo("Test passed.");

    clean:
    closeChannel(cid);
    free(buf);
}

void testCrc() {
    crcInit();
    char a[] = "asd123\0\0";
    u_short crc = crcCompute((u_char*)a, 6);
    a[6] = (char)(crc >> 8);
    a[7] = (char) crc;
    printf("crc: %d\n", crc);
    crc = crcCompute((u_char*)a, 8);
    printf("crc: %d\n", crc);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        testCrc();
        testString();
        return 0;
    }

    char *p = argv[1];

    initLibrary(TRUE);
    if (strcmp("-w", p) == 0) {
        simple_writer();
    } else if (strcmp("-r", p) == 0) {
        simple_reader();
    } else if (strcmp("-bw", p) == 0) {
        benchmark_writer();
    } else if (strcmp("-br", p) == 0) {
        benchmark_reader();
    } else if (strcmp("-t1r", p) == 0) {
        test1_reader();
    } else if (strcmp("-t1w", p) == 0) {
        test1_writer();
    } else if (strcmp("-t2r", p) == 0) {
        test2_reader();
    } else if (strcmp("-t2w", p) == 0) {
        test2_writer();
    } else {
        logError("Invalid parameter.");
    }
    cleanLibrary();

    return 0;
}