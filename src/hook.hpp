#pragma once
#include "jvm.hpp"

namespace hook
{
	void set_up();

#ifdef _JAVAHOOK_USE_JNI
	typedef std::uint64_t(*handler_t)(argument_parser_t* arguments, JNIEnv* env, jmethodID method_copy, std::uint8_t* should_call_original);

	namespace jni
	{
		void add_hook(jmethodID method_id, handler_t handler);

		void remove_hook(jmethodID method_id);
	}
#else
	typedef std::uint64_t(*handler_t)(argument_parser_t* arguments, method_t* method_copy, std::uint8_t* should_call_original);
#endif

	void add_hook(method_t* method, handler_t handler);

	void remove_hook(method_t* method);
}
