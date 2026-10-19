#pragma once
#include <type_traits>
#include <jni/jni.h>
#include "signature.hpp"

#define JABA_FOREACH_JNI_TYPE(X) \
	X(jboolean, Boolean) \
	X(jbyte, Byte) \
	X(jchar, Char) \
	X(jshort, Short) \
	X(jint, Int) \
	X(jlong, Long) \
	X(jfloat, Float) \
	X(jdouble, Double)

// primitives plus the clazz<> case (stored in a field as an object)
#define JABA_FOREACH_JNI_TYPE_EX(X) \
	JABA_FOREACH_JNI_TYPE(X) \
	X(::jaba::jni::clazz_any, Object)

namespace jaba::jni
{
	// stands in for "any clazz<M> or type derived from one, or an array" in the macros above
	struct clazz_any { };

	template <class T, class Match>
	inline constexpr bool is_jni_type = std::is_same_v<T, Match>;

	template <class T>
	inline constexpr bool is_jni_type<T, clazz_any> = sig::detail::clazz_like<T> || sig::detail::array_like<T>;
}
