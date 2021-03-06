//
// Created by I507145 on 7/17/2020.
//

#ifndef LIPC_ORG_LUNCERT_SMIPC_SMIPC_H
#define LIPC_ORG_LUNCERT_SMIPC_SMIPC_H


/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class org_luncert_smipc_SmipcLib */

#ifndef _Included_org_luncert_smipc_SmipcLib
#define _Included_org_luncert_smipc_SmipcLib
#ifdef __cplusplus
extern "C" {
#endif
#undef org_luncert_smipc_SmipcLib_OP_SUCCEED
#define org_luncert_smipc_SmipcLib_OP_SUCCEED 0L
#undef org_luncert_smipc_SmipcLib_OP_FAILED
#define org_luncert_smipc_SmipcLib_OP_FAILED -1L
#undef org_luncert_smipc_SmipcLib_OPPOSITE_END_CLOSED
#define org_luncert_smipc_SmipcLib_OPPOSITE_END_CLOSED -2L
#undef org_luncert_smipc_SmipcLib_LOG_DISABLE
#define org_luncert_smipc_SmipcLib_LOG_DISABLE 0L
#undef org_luncert_smipc_SmipcLib_LOG_BASIC
#define org_luncert_smipc_SmipcLib_LOG_BASIC 1L
#undef org_luncert_smipc_SmipcLib_LOG_ALL
#define org_luncert_smipc_SmipcLib_LOG_ALL 2L
#undef org_luncert_smipc_SmipcLib_CHAN_READ
#define org_luncert_smipc_SmipcLib_CHAN_READ 0L
#undef org_luncert_smipc_SmipcLib_CHAN_WRITE
#define org_luncert_smipc_SmipcLib_CHAN_WRITE 1L
/*
 * Class:     org_luncert_smipc_SmipcLib
 * Method:    initLibrary
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_org_luncert_smipc_SmipcLib_initLibrary
        (JNIEnv *, jclass, jint);

/*
 * Class:     org_luncert_smipc_SmipcLib
 * Method:    cleanLibrary
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_luncert_smipc_SmipcLib_cleanLibrary
        (JNIEnv *, jclass);

/*
 * Class:     org_luncert_smipc_SmipcLib
 * Method:    openChannel
 * Signature: ([CII)I
 */
JNIEXPORT jint JNICALL Java_org_luncert_smipc_SmipcLib_openChannel
        (JNIEnv *, jclass, jcharArray, jint, jint);

/*
 * Class:     org_luncert_smipc_SmipcLib
 * Method:    writeChannel
 * Signature: ([C[BII)I
 */
JNIEXPORT jint JNICALL Java_org_luncert_smipc_SmipcLib_writeChannel
        (JNIEnv *, jclass, jcharArray, jbyteArray, jint, jint);

/*
 * Class:     org_luncert_smipc_SmipcLib
 * Method:    readChannel
 * Signature: ([C[BIIZ)I
 */
JNIEXPORT jint JNICALL Java_org_luncert_smipc_SmipcLib_readChannel
        (JNIEnv *, jclass, jcharArray, jbyteArray, jint, jint, jboolean);

/*
 * Class:     org_luncert_smipc_SmipcLib
 * Method:    printChannelStatus
 * Signature: ([C)I
 */
JNIEXPORT jint JNICALL Java_org_luncert_smipc_SmipcLib_printChannelStatus
        (JNIEnv *, jclass, jcharArray);

/*
 * Class:     org_luncert_smipc_SmipcLib
 * Method:    closeChannel
 * Signature: ([C)I
 */
JNIEXPORT void JNICALL Java_org_luncert_smipc_SmipcLib_closeChannel
        (JNIEnv *, jclass, jcharArray);

#ifdef __cplusplus
}
#endif
#endif


#endif //LIPC_ORG_LUNCERT_SMIPC_SMIPC_H
