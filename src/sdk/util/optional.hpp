#pragma once
#include "sdk/lang/core.hpp"

namespace jaba::util
{
	JABA_CLASS_BEGIN(optional, "java/util/Optional", object)
		JABA_STATIC_METHOD(optional, of, "of", object)
		JABA_STATIC_METHOD(optional, of_nullable, "ofNullable", object)
		JABA_STATIC_METHOD(optional, empty, "empty")
		JABA_METHOD(jboolean, is_present, "isPresent")
		JABA_METHOD(object, get, "get")
		JABA_METHOD(object, or_else, "orElse", object)
	JABA_CLASS_END(optional)
}
