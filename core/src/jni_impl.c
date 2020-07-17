//
// Created by I507145 on 7/17/2020.
//

#include "org_luncert_smipc_Smipc.h"
#include "sm_channel.h"

// main

void JNICALL Java_org_luncert_smipc_SmipcLib_initLibrary(JNIEnv *jniEnv, jclass clazz, jint logMode) {
    initLibrary(logMode);
}

void JNICALL Java_org_luncert_smipc_SmipcLib_cleanLibrary(JNIEnv *jniEnv, jclass clazz) {
    cleanLibrary();
}

JNIEXPORT jint JNICALL Java_org_luncert_smipc_SmipcLib_openChannel(JNIEnv *jniEnv, jclass clazz,
                                                                   jbyteArray channelId, jint mode, jint chanSz) {
    jboolean isCopy;
    const char* cid = (*jniEnv)->GetStringUTFChars(jniEnv, channelId, &isCopy);

    int ret = openChannel(cid, mode, chanSz);

    (*jniEnv)->ReleaseStringUTFChars(jniEnv, channelId, cid);
    return ret;
}

jint JNICALL Java_org_luncert_smipc_SmipcLib_writeChannel(JNIEnv *jniEnv, jclass clazz,
        jbyteArray channelId, jbyteArray data, jint start, jint end) {
    jboolean isCopy;
    jbyte* byteData = (*jniEnv)->GetByteArrayElements(jniEnv, data, &isCopy);
    const char* cid = (*jniEnv)->GetStringUTFChars(jniEnv, channelId, &isCopy);

    int ret = writeChannel(cid, byteData, start, end);

    (*jniEnv)->ReleaseStringUTFChars(jniEnv, channelId, cid);
    (*jniEnv)->ReleaseByteArrayElements(jniEnv, data, byteData, JNI_COMMIT);
    return ret;
}

jint JNICALL Java_org_luncert_smipc_SmipcLib_readChannel(JNIEnv *jniEnv, jclass clazz,
        jstring channelId, jbyteArray buffer, jint start, jint end, jboolean blocking) {
    jboolean isCopy;
    jbyte* byteBuffer = (*jniEnv)->GetByteArrayElements(jniEnv, buffer, &isCopy);
    const char* cid = (*jniEnv)->GetStringUTFChars(jniEnv, channelId, &isCopy);

    int ret = readChannel(cid, byteBuffer, start, end, blocking);

    (*jniEnv)->ReleaseStringUTFChars(jniEnv, channelId, cid);
    (*jniEnv)->ReleaseByteArrayElements(jniEnv, buffer, byteBuffer, JNI_COMMIT);
    return ret;
}

jint JNICALL Java_org_luncert_smipc_SmipcLib_printChannelStatus(JNIEnv *jniEnv, jclass clazz,
        jbyteArray channelId) {
    jboolean isCopy;
    const char* cid = (*jniEnv)->GetStringUTFChars(jniEnv, channelId, &isCopy);

    int ret = printChannelStatus(cid);

    (*jniEnv)->ReleaseStringUTFChars(jniEnv, channelId, cid);
    return ret;
}

void JNICALL Java_org_luncert_smipc_SmipcLib_closeChannel(JNIEnv *jniEnv, jclass clazz,
        jbyteArray channelId) {
    jboolean isCopy;
    const char* cid = (*jniEnv)->GetStringUTFChars(jniEnv, channelId, &isCopy);

    closeChannel(cid);

    (*jniEnv)->ReleaseStringUTFChars(jniEnv, channelId, cid);
}