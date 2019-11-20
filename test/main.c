#include <stdio.h>
#include <util.h>
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

}

void writer() {

}

int main() {
    if (openChannel(cid, CHAN_W, 128) != OP_SUCCEED) {
        return -1;
    }

    closeChannel(cid);
    return 0;
}