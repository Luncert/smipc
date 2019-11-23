#include <stdio.h>
#include <util.h>
#include <string.h>
#include <library.h>

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

void reader() {
    logInfo("Start as reader.");
    initLibrary();
    if (openChannel(cid, CHAN_R, 128) != OP_SUCCEED) {
        return;
    }

    char buf[128] = {0};
    printChannelStatus(cid);

    for (int i = 0; i < 3; i++) {
        readChannel(cid, buf, 9, TRUE);
        printf("READER:%s\n", buf);
        printChannelStatus(cid);
    }

    closeChannel(cid);
    cleanLibrary();
}

void writer() {
    logInfo("Start as writer.");
    initLibrary();
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
        printf("WRITER:done.\n");
        printChannelStatus(cid);
        Sleep(1000);
    }

    closeChannel(cid);
    cleanLibrary();
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        return -1;
    }
    if (strcmp("-w", argv[1]) == 0) {
        writer();
    } else if (strcmp("-r", argv[1]) == 0) {
        reader();
    } else {
        logError("Invalid parameter.");
        return -1;
    }
    return 0;
}