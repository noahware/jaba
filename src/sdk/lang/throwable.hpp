#pragma once
#include "sdk/lang/core.hpp"

// The constructors are declared once, on throwable, and every exception below inherits them:
// create() and create(message) look the constructor up on the class they're called on, and
// all of these declare the same two
namespace jaba::inline lang
{
	JABA_CLASS_BEGIN(throwable, "java/lang/Throwable", object)
		JABA_CONSTRUCTOR(create)
		JABA_CONSTRUCTOR(create, string)
		JABA_METHOD(string, get_message, "getMessage")
		JABA_METHOD(string, get_localized_message, "getLocalizedMessage")
		JABA_METHOD(throwable, get_cause, "getCause")
		JABA_METHOD(void, print_stack_trace, "printStackTrace")
	JABA_CLASS_END(throwable)

	JABA_CLASS_BEGIN(exception, "java/lang/Exception", throwable)
	JABA_CLASS_END(exception)

	JABA_CLASS_BEGIN(runtime_exception, "java/lang/RuntimeException", exception)
	JABA_CLASS_END(runtime_exception)

	JABA_CLASS_BEGIN(illegal_argument_exception, "java/lang/IllegalArgumentException", runtime_exception)
	JABA_CLASS_END(illegal_argument_exception)

	JABA_CLASS_BEGIN(number_format_exception, "java/lang/NumberFormatException", illegal_argument_exception)
	JABA_CLASS_END(number_format_exception)

	JABA_CLASS_BEGIN(illegal_state_exception, "java/lang/IllegalStateException", runtime_exception)
	JABA_CLASS_END(illegal_state_exception)

	JABA_CLASS_BEGIN(null_pointer_exception, "java/lang/NullPointerException", runtime_exception)
	JABA_CLASS_END(null_pointer_exception)

	JABA_CLASS_BEGIN(index_out_of_bounds_exception, "java/lang/IndexOutOfBoundsException", runtime_exception)
	JABA_CLASS_END(index_out_of_bounds_exception)

	JABA_CLASS_BEGIN(unsupported_operation_exception, "java/lang/UnsupportedOperationException", runtime_exception)
	JABA_CLASS_END(unsupported_operation_exception)
}
