#pragma once
#include <jni/jni.h>

namespace jaba::jni
{
	void set_vm(JavaVM* vm);
	[[nodiscard]] JNIEnv* env();
}
