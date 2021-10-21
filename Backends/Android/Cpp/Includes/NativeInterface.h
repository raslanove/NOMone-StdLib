#pragma once

#include <jni.h>

#define NATIVES_CLASS "com/nomone/std_lib/StdLibNatives"
#define FORMATTED_NATIVES_CLASS Java_com_nomone_std_1lib_StdLibNatives_

#define APPEND_AGAIN(a, b) a ## b
#define APPEND(a, b) APPEND_AGAIN(a, b)

#define JNI_CALL_PREFIX 								extern "C" JNIEXPORT
#define JNI_FUNCTION_NAME_PREFIX(functionName) 			APPEND(FORMATTED_NATIVES_CLASS, functionName)
#define JNI_FUNCTION_SIGNATURE_PREFIX					JNIEnv *env, jclass clazz
