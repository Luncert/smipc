#include <stdio.h>
#include <util.h>
#include <string.h>
#include <library.h>

void testString() {
    printf("[test String]\n");
    String s1 = parseConstToString("Hi");
    String s2 = parseToString("Luncert", 3);
    appendString(s1, ", ", 2);
    appendString(s2, "!", 1);
    String s3 = concatString(s1, s2);
    printf("s1=%s\ns2=%s\ns3=%s\n", s1->val, s2->val, s3->val);
    releaseString(s1);
    releaseString(s2);
    releaseString(s3);
}

char *cid = "test-channel";

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