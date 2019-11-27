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

char *buildCrcTestData(int sz) {
    int dataSz = sz - 2;
    char *buf = (char*)malloc(sz);

    srand((unsigned)time(NULL));
    for (int i = 0; i < dataSz; i++) {
        buf[i] = (char)(rand() % 256 - 128);
    }
    // calc crc
    u_short crc = crcCompute((u_char*)buf, dataSz);
    buf[sz - 2] = (char)(crc >> 8);
    buf[sz - 1] = (char)crc;

    return buf;
}

int isDataValid(char *data, int sz) {
    return crcCompute((u_char*)data, sz) == 0;
}

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

char *cid = "test-chan";

void simple_reader() {
    if (openChannel(cid, CHAN_R, 3) != OP_SUCCEED) {
        return;
    }

    char buf[10] = {0};
    printChannelStatus(cid);

    for (int i = 0; i < 3; i++) {
        int ret = readChannel(cid, buf, 9, TRUE);
        printf("READ:%s RET:%d\n", buf, ret);
        printChannelStatus(cid);
    }

    closeChannel(cid);
}

void simple_writer() {
    crcInit();
    if (openChannel(cid, CHAN_W, 3) != OP_SUCCEED) {
        return;
    }

    char *data[] = {
            "test/id=1",
            "test/id=2",
            "test/id=3"
    };
    printChannelStatus(cid);
    for (int i = 0; i < 3; i++) {
        writeChannel(cid, data[i], 9);
        printChannelStatus(cid);
        Sleep(1000);
    }

    closeChannel(cid);
}

#define A_CHAN_SZ 77
#define A_FRAME_SZ 512
#define A_FRAME_NUM 100

char ar_buf[A_FRAME_SZ];
int ar_buf_offset = 0;
int frame_count = 1;

void on_data(char *data, int sz) {
    int tmp = ar_buf_offset + sz;
    if (tmp > A_FRAME_SZ) {
        // 把data前半部分拷贝进ar_buf，剩下的在最后的条件判断里拷贝
        tmp = A_FRAME_SZ - ar_buf_offset;
        memcpy(ar_buf + ar_buf_offset, data, tmp);
        ar_buf_offset += tmp;
        sz -= tmp;
    } else {
        memcpy(ar_buf + ar_buf_offset, data, sz);
        ar_buf_offset += sz;
        sz = 0;
    }
    if (ar_buf_offset == A_FRAME_SZ) {
        if (!isDataValid(ar_buf, A_FRAME_SZ)) {
            printf("[ERROR] Test failed, data incorrect. frame id=%d\n", frame_count);
        } else {
            printf("[INFO] Data validation pass, frame id=%d\n", frame_count);
        }
        frame_count++;
        ar_buf_offset = 0;
    }
    if (sz > 0) {
        memcpy(ar_buf + ar_buf_offset, data + tmp, sz);
        ar_buf_offset += sz;
    }
}

void async_reader() {
    crcInit();

    if (openChannel(cid, CHAN_R, A_CHAN_SZ) != OP_SUCCEED) {
        return;
    }

    onChannelData(cid, on_data);
    Sleep(10000);
    removeListener(cid);

    closeChannel(cid);
}

void async_writer() {
    crcInit();

    if (openChannel(cid, CHAN_W, A_CHAN_SZ) != OP_SUCCEED) {
        return;
    }

    char *data;
    printChannelStatus(cid);
    for (int i = 0; i < A_FRAME_NUM; i++) {
        data = buildCrcTestData(A_FRAME_SZ);
        writeChannel(cid, data, A_FRAME_SZ);
        free(data);
    }

    closeChannel(cid);
}

#define B_CHAN_SZ 512
#define B_FRAME_SZ 1111111

void benchmark_reader() {
    struct timeb st, et;

    // prepare test buf
    char stub = 0xa;
    char *buf = (char*)malloc(B_FRAME_SZ);
    buf[100] = 0;

    if (openChannel(cid, CHAN_R, B_CHAN_SZ) != OP_SUCCEED) {
        return;
    }
    printChannelStatus(cid);
    for (int i = 0; i < 3; i++) {
        ftime(&st);
        int ret = readChannel(cid, buf, B_FRAME_SZ, TRUE);
        ftime(&et);
        printf("RET:%d buf[100]:%d stub:%d rt: %dms\n", ret, buf[100], stub++, et.millitm - st.millitm);
        printChannelStatus(cid);
        // reset
        buf[0] = 0;
        buf[B_FRAME_SZ - 1] = 0;
    }
    closeChannel(cid);

    free(buf);
}

void benchmark_writer() {
    struct timeb st, et;

    // prepare test data
    char *data = (char*)malloc(B_FRAME_SZ);
    data[100] = 0xa;

    if (openChannel(cid, CHAN_W, B_CHAN_SZ) != OP_SUCCEED) {
        return;
    }
    printChannelStatus(cid);
    for (int i = 0; i < 3; i++, data[100]++) {
        ftime(&st);
        writeChannel(cid, data, B_FRAME_SZ);
        ftime(&et);
        printf("wt: %dms\n", et.millitm - st.millitm);
        printChannelStatus(cid);
    }
    closeChannel(cid);

    free(data);
}

#define T_CHAN_SZ 64
#define T_DATA_SZ 10247
#define T_TOTAL_SZ (T_DATA_SZ + 2)

// test case 1 (build data bigger than channel, send it at one time)

void test1_reader() {
    crcInit();

    if (openChannel(cid, CHAN_R, T_CHAN_SZ) != OP_SUCCEED) {
        return;
    }
    printChannelStatus(cid);

    char *buf = (char*)malloc(T_TOTAL_SZ);
    int n = readChannel(cid, buf, T_TOTAL_SZ, TRUE);
    if (n < 0 || n != T_TOTAL_SZ) {
        printf("[ERROR] Test failed, read failed, ret=%d\n", n);
        goto clean;
    }

    // check data
    if (isDataValid(buf, T_TOTAL_SZ) == FALSE) {
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
    crcInit();

    if (openChannel(cid, CHAN_W, T_CHAN_SZ) != OP_SUCCEED) {
        return;
    }
    printChannelStatus(cid);

    char *buf = buildCrcTestData(T_TOTAL_SZ);
    if (writeChannel(cid, buf, T_TOTAL_SZ) != OP_SUCCEED) {
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
    crcInit();

    const int blockSz = 9;

    if (openChannel(cid, CHAN_R, T_CHAN_SZ) != OP_SUCCEED) {
        return;
    }
    printChannelStatus(cid);

    char *buf = (char*)malloc(T_TOTAL_SZ);
    // read
    for (int offset = 0, rest = T_TOTAL_SZ, n; rest > 0; offset += n, rest -= n) {
        n = readChannel(cid, buf + offset, rest < blockSz ? rest : blockSz, TRUE);
        if (n < 0) {
            printf("[ERROR] Test failed, read failed, ret=%d\n", n);
            goto clean;
        }
        printChannelStatus(cid);
    }

    // check data
    if (!isDataValid(buf, T_TOTAL_SZ)) {
        logError("Test failed, data incorrect.");
        goto clean;
    }

    logInfo("Test passed.");

    clean:
    closeChannel(cid);
    free(buf);
}

void test2_writer() {
    crcInit();

    const int blockSz = 7;

    if (openChannel(cid, CHAN_W, T_CHAN_SZ) != OP_SUCCEED) {
        return;
    }
    printChannelStatus(cid);

    char *buf = buildCrcTestData(T_TOTAL_SZ);

    // send
    for (int offset = 0, rest = T_TOTAL_SZ, n; rest > 0; offset += n, rest -= n) {
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

int main(int argc, char *argv[]) {
    if (argc != 2) {
        testCrc();
        testString();
        return 0;
    }

    char *p = argv[1];

    initLibrary(LOG_ALL);
    if (strcmp("-w", p) == 0) {
        simple_writer();
    } else if (strcmp("-r", p) == 0) {
        simple_reader();
    } else if (strcmp("-ar", p) == 0) {
        async_reader();
    } else if (strcmp("-aw", p) == 0) {
        async_writer();
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