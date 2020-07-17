//
// Created by I507145 on 7/17/2020.
//

#include "org_luncert_smipc_Smipc.h"
#include "sm_channel.h"

char* parseJCharArrayToCharPointer(JNIEnv *jniEnv, jcharArray raw) {
    // TODO: usage of isCopy
    jboolean isCopy;
    return (char*) (*jniEnv)->GetCharArrayElements(jniEnv, raw, &isCopy);
}

// main

void JNICALL Java_org_luncert_smipc_SmipcLib_initLibrary(JNIEnv *jniEnv, jclass clazz, jint logMode) {
    initLibrary(logMode);
}

void JNICALL Java_org_luncert_smipc_SmipcLib_cleanLibrary(JNIEnv *jniEnv, jclass clazz) {
    cleanLibrary();
}

JNIEXPORT jint JNICALL Java_org_luncert_smipc_SmipcLib_openChannel(JNIEnv *jniEnv, jclass clazz,
                                                                   jcharArray channelId, jint mode, jint chanSz) {
    return openChannel(parseJCharArrayToCharPointer(jniEnv, channelId), mode, chanSz);
}

jint JNICALL Java_org_luncert_smipc_SmipcLib_writeChannel(JNIEnv *jniEnv, jclass clazz,
        jcharArray channelId, jbyteArray data, jint start, jint end) {
    jboolean isCopy;
    jbyte* byteData = (*jniEnv)->GetByteArrayElements(jniEnv, data, &isCopy);
    return writeChannel(parseJCharArrayToCharPointer(jniEnv, channelId),
                        byteData, start, end);
}

jint JNICALL Java_org_luncert_smipc_SmipcLib_readChannel(JNIEnv *jniEnv, jclass clazz,
        jcharArray channelId, jbyteArray buffer, jint start, jint end, jboolean blocking) {
    jboolean isCopy;
    jbyte* byteBuffer = (*jniEnv)->GetByteArrayElements(jniEnv, buffer, &isCopy);
    return readChannel(parseJCharArrayToCharPointer(jniEnv, channelId),
            byteBuffer, start, end, blocking);
}

jint JNICALL Java_org_luncert_smipc_SmipcLib_printChannelStatus(JNIEnv *jniEnv, jclass clazz,
        jcharArray channelId) {
    return printChannelStatus(parseJCharArrayToCharPointer(jniEnv, channelId));
}

void JNICALL Java_org_luncert_smipc_SmipcLib_closeChannel(JNIEnv *jniEnv, jclass clazz,
        jcharArray channelId) {
    closeChannel(parseJCharArrayToCharPointer(jniEnv, channelId));
}