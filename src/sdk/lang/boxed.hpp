#pragma once
#include "sdk/lang/core.hpp"

// The boxed primitives, built from a plain value (integer i = 5) and read back through
// the explicit conversion or the *_value methods. Long, Float and Double have a trailing
// underscore, the plain names are c++ keywords
namespace jaba::inline lang
{
	JABA_CLASS_BEGIN(number, "java/lang/Number", object)
		JABA_METHOD(jbyte, byte_value, "byteValue")
		JABA_METHOD(jshort, short_value, "shortValue")
		JABA_METHOD(jint, int_value, "intValue")
		JABA_METHOD(jlong, long_value, "longValue")
		JABA_METHOD(jfloat, float_value, "floatValue")
		JABA_METHOD(jdouble, double_value, "doubleValue")
	JABA_CLASS_END(number)

	JABA_CLASS_BEGIN(integer, "java/lang/Integer", number)
		JABA_STATIC_METHOD(integer, value_of, "valueOf", jint)
		JABA_STATIC_METHOD(jint, parse_int, "parseInt", string)
		JABA_STATIC_METHOD(jint, compare, "compare", jint, jint)
		JABA_STATIC_FIELD(jint, max_value, "MAX_VALUE")
		JABA_STATIC_FIELD(jint, min_value, "MIN_VALUE")

		static jaba_self jaba_make(const jint value)
		{
			return value_of(value);
		}

		[[nodiscard]] explicit operator jint() const
		{
			return this->int_value();
		}

		JABA_METHOD(jint, compare_to, "compareTo", integer)

		// a < b ... by compareTo, == is equals
		[[nodiscard]] friend std::strong_ordering operator<=>(const jaba_self& a, const jaba_self& b)
		{
			return detail::compare_objects(a, b);
		}
	JABA_CLASS_END(integer)

	JABA_CLASS_BEGIN(long_, "java/lang/Long", number)
		JABA_STATIC_METHOD(long_, value_of, "valueOf", jlong)
		JABA_STATIC_METHOD(jlong, parse_long, "parseLong", string)
		JABA_STATIC_METHOD(jint, compare, "compare", jlong, jlong)
		JABA_STATIC_FIELD(jlong, max_value, "MAX_VALUE")
		JABA_STATIC_FIELD(jlong, min_value, "MIN_VALUE")

		static jaba_self jaba_make(const jlong value)
		{
			return value_of(value);
		}

		[[nodiscard]] explicit operator jlong() const
		{
			return this->long_value();
		}

		JABA_METHOD(jint, compare_to, "compareTo", long_)

		[[nodiscard]] friend std::strong_ordering operator<=>(const jaba_self& a, const jaba_self& b)
		{
			return detail::compare_objects(a, b);
		}
	JABA_CLASS_END(long_)

	JABA_CLASS_BEGIN(float_, "java/lang/Float", number)
		JABA_STATIC_METHOD(float_, value_of, "valueOf", jfloat)
		JABA_STATIC_METHOD(jfloat, parse_float, "parseFloat", string)
		JABA_METHOD(jboolean, is_nan, "isNaN")

		static jaba_self jaba_make(const jfloat value)
		{
			return value_of(value);
		}

		[[nodiscard]] explicit operator jfloat() const
		{
			return this->float_value();
		}

		JABA_METHOD(jint, compare_to, "compareTo", float_)

		[[nodiscard]] friend std::strong_ordering operator<=>(const jaba_self& a, const jaba_self& b)
		{
			return detail::compare_objects(a, b);
		}
	JABA_CLASS_END(float_)

	JABA_CLASS_BEGIN(double_, "java/lang/Double", number)
		JABA_STATIC_METHOD(double_, value_of, "valueOf", jdouble)
		JABA_STATIC_METHOD(jdouble, parse_double, "parseDouble", string)
		JABA_METHOD(jboolean, is_nan, "isNaN")

		static jaba_self jaba_make(const jdouble value)
		{
			return value_of(value);
		}

		[[nodiscard]] explicit operator jdouble() const
		{
			return this->double_value();
		}

		JABA_METHOD(jint, compare_to, "compareTo", double_)

		[[nodiscard]] friend std::strong_ordering operator<=>(const jaba_self& a, const jaba_self& b)
		{
			return detail::compare_objects(a, b);
		}
	JABA_CLASS_END(double_)

	JABA_CLASS_BEGIN(boolean, "java/lang/Boolean", object)
		JABA_STATIC_METHOD(boolean, value_of, "valueOf", jboolean)
		JABA_STATIC_METHOD(jboolean, parse_boolean, "parseBoolean", string)
		JABA_METHOD(jboolean, boolean_value, "booleanValue")

		static jaba_self jaba_make(const jboolean value)
		{
			return value_of(value);
		}

		[[nodiscard]] explicit operator jboolean() const
		{
			return this->boolean_value();
		}

		JABA_METHOD(jint, compare_to, "compareTo", boolean)

		[[nodiscard]] friend std::strong_ordering operator<=>(const jaba_self& a, const jaba_self& b)
		{
			return detail::compare_objects(a, b);
		}
	JABA_CLASS_END(boolean)

	JABA_CLASS_BEGIN(character, "java/lang/Character", object)
		JABA_STATIC_METHOD(character, value_of, "valueOf", jchar)
		JABA_METHOD(jchar, char_value, "charValue")

		static jaba_self jaba_make(const jchar value)
		{
			return value_of(value);
		}

		[[nodiscard]] explicit operator jchar() const
		{
			return this->char_value();
		}

		JABA_METHOD(jint, compare_to, "compareTo", character)

		[[nodiscard]] friend std::strong_ordering operator<=>(const jaba_self& a, const jaba_self& b)
		{
			return detail::compare_objects(a, b);
		}
	JABA_CLASS_END(character)

	// object's side of the boxing, declared in core.hpp
	template <class Self>
	object object_members<Self>::jaba_make(const jboolean value)
	{
		return boolean::value_of(value);
	}

	template <class Self>
	object object_members<Self>::jaba_make(const jchar value)
	{
		return character::value_of(value);
	}

	template <class Self>
	object object_members<Self>::jaba_make(const jint value)
	{
		return integer::value_of(value);
	}

	template <class Self>
	object object_members<Self>::jaba_make(const jlong value)
	{
		return long_::value_of(value);
	}

	template <class Self>
	object object_members<Self>::jaba_make(const jfloat value)
	{
		return float_::value_of(value);
	}

	template <class Self>
	object object_members<Self>::jaba_make(const jdouble value)
	{
		return double_::value_of(value);
	}
}
