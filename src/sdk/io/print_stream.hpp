#pragma once
#include "sdk/lang/core.hpp"

namespace jaba::io
{
	JABA_CLASS_BEGIN(print_stream, "java/io/PrintStream", object)
		JABA_METHOD(void, print, "print", string)
		JABA_METHOD(void, println, "println", string)
		JABA_METHOD(void, flush, "flush")
	JABA_CLASS_END(print_stream)
}
