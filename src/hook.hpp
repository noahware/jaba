#pragma once
#include "jvm.hpp"

#include "../tests/jni/jni.h"
#include "../tests/jni/jvmti.h"

namespace hook
{
	typedef std::uint64_t(*handler_t)(argument_parser_t* arguments, JNIEnv* env, jmethodID method_copy, std::uint8_t* should_call_original);

	void add_hook(jmethodID method_id, handler_t handler);

	void remove_hook(jmethodID method_id);
}
