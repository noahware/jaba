#pragma once
#include "jvm.hpp"

namespace hook
{
	void set_up(std::uint8_t* jvm_code_base, std::uint64_t jvm_code_size);

#ifdef _JAVAHOOK_USE_JNI
	namespace jni
	{
		void add_hook(jmethodID method_id, void* handler);

		void remove_hook(jmethodID method_id);

		method_t* get_callable_method(jmethodID original_method_id);
	}
#else
	typedef std::uint64_t(*handler_t)(argument_parser_t* arguments, method_t* method_copy, std::uint8_t* should_call_original);
#endif

	void add_hook(method_t* method, void* handler);
	void remove_hook(method_t* method);

	method_t* get_callable_method(method_t* original_method);
}
