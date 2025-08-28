#pragma once
#include "jvm.hpp"

#include "../tests/jni/jni.h"
#include "../tests/jni/jvmti.h"

namespace hook
{
	typedef std::uint64_t(*handler_t)(argument_parser_t* arguments, std::uint8_t* should_call_original);

	// jvmti_env must have ability to retransform classes
	void add_hook(JNIEnv* env, jvmtiEnv* jvmti_env, jmethodID method_id, handler_t handler);

	void remove_hook(jmethodID method_id);
}
