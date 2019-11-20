#include <stdio.h>
#include <util.h>
#include <windows.h>

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

SyncBuf syncBuf;

int writeRoutine(LPVOID lpParam) {
    writeSyncBuf(syncBuf, "test.", 5);
    printf("write 1 done.\n");

    writeSyncBuf(syncBuf, "test.test.te", 12);
    printf("write 2 done.\n");

    writeSyncBuf(syncBuf, "test.test.test.test.", 20);
    printf("write 3 done.\n");
}

int readRoutine(LPVOID lpParam) {
    char buf[21];

    // 等第1次写完成
//    Sleep(1000);
    int n = readSyncBuf(syncBuf, buf, 5);
    buf[n] = 0;
    printf("read 1 done[count=%d]: %s\n", n, buf);

//    Sleep(1000);
    n = readSyncBuf(syncBuf, buf, 12);
    buf[n] = 0;
    printf("read 2 done[count=%d]: %s\n", n, buf);


    n = readSyncBufB(syncBuf, buf, 20);
    buf[n] = 0;
    printf("read 3 done[count=%d]: %s\n", n, buf);
}

void testSyncBuf() {
    printf("[test SyncBuf]\n");
    char *buf = (char*)malloc(12);
    syncBuf = newSyncBuf(parseConstToString("testSyncBuf"), buf, 12);

    DWORD tid1, tid2;
    HANDLE t1 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)writeRoutine, NULL, 0, &tid1);
    HANDLE t2 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)readRoutine, NULL, 0, &tid1);

    Sleep(3000);
    releaseSyncBuf(syncBuf);
    free(buf);
}

int main() {
    testString();
    testSyncBuf();
    return 0;
}

