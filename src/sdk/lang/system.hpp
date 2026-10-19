#pragma once
#include "sdk/io/print_stream.hpp"
#include "sdk/lang/core.hpp"

namespace jaba::inline lang
{
	JABA_CLASS_BEGIN(system, "java/lang/System", object)
		JABA_STATIC_FIELD(io::print_stream, out, "out")
		JABA_STATIC_FIELD(io::print_stream, err, "err")
		JABA_STATIC_METHOD(jlong, current_time_millis, "currentTimeMillis")
		JABA_STATIC_METHOD(jlong, nano_time, "nanoTime")
		JABA_STATIC_METHOD(string, get_property, "getProperty", string)
		JABA_STATIC_METHOD(string, get_env, "getenv", string)
		JABA_STATIC_METHOD(string, line_separator, "lineSeparator")
		JABA_STATIC_METHOD(jint, identity_hash_code, "identityHashCode", object)
		JABA_STATIC_METHOD(void, gc, "gc")
	JABA_CLASS_END(system)
}
