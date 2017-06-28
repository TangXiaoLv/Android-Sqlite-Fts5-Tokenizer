//
// Created by tang on 2017/4/22.
//

#ifndef SQLITE_ANDROID_BINDINGS_TOKEN_H
#define SQLITE_ANDROID_BINDINGS_TOKEN_H

#include "nativehelper/jni.h"

#ifdef __cplusplus
#define EXTERN_C_BEGIN extern "C" {
#define EXTERN_C_END }
#else
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#endif

EXTERN_C_BEGIN

JNIEXPORT jstring JNICALL
        Java_org_sqlite_database_extra_Enhance_initFtsTokenizer(JNIEnv *env, jobject jobj,
                                                                jstring jpath);

JNIEXPORT jstring JNICALL
        Java_org_sqlite_database_extra_Enhance_insert(JNIEnv *env, jobject jobj, jstring jtext);

JNIEXPORT jstring JNICALL
        Java_org_sqlite_database_extra_Enhance_query(JNIEnv *env, jobject jobj, jstring jtext);

EXTERN_C_END
#endif //SQLITE_ANDROID_BINDINGS_TOKEN_H
