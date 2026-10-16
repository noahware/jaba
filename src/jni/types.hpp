#pragma once
#include <jni/jni.h>

#define JABA_FOREACH_JNI_TYPE(X) \
	X(jboolean, Boolean) \
	X(jbyte, Byte) \
	X(jchar, Char) \
	X(jshort, Short) \
	X(jint, Int) \
	X(jlong, Long) \
	X(jfloat, Float) \
	X(jdouble, Double) \
	X(jobject, Object)
