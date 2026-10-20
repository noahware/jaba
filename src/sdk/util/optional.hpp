#pragma once
#include "sdk/util/collections.hpp"

namespace jaba::util::raw
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

namespace jaba::util
{
	template <class T>
	class optional : public raw::optional
	{
	public:
		using raw::optional::optional;

		optional() = default;

		explicit optional(const raw::optional& raw)
			: raw::optional(raw) { }

		[[nodiscard]] static optional of(const T& value)
		{
			return detail::retype<optional>(raw::optional::of(detail::erase(value)));
		}

		[[nodiscard]] static optional of_nullable(const T& value)
		{
			return detail::retype<optional>(raw::optional::of_nullable(detail::erase(value)));
		}

		[[nodiscard]] static optional empty()
		{
			return detail::retype<optional>(raw::optional::empty());
		}

		[[nodiscard]] T get() const
		{
			return detail::retype<T>(raw::optional::get());
		}

		[[nodiscard]] T or_else(const T& other) const
		{
			return detail::retype<T>(raw::optional::or_else(detail::erase(other)));
		}
	};
}
