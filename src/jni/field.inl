#pragma once
#include "field.hpp"
#include "clazz.hpp"
#include "env.hpp"
#include "signature.hpp"
#include "types.hpp"

namespace jaba
{
	namespace detail
	{
		// raw jni value -> ValT (clazz types get wrapped in an object)
		template <class V, class R>
		[[nodiscard]] V wrap(const R raw) noexcept
		{
			if constexpr (std::is_same_v<V, R>)
				return raw;
			else
				return V{ object{ raw } };
		}

		// ValT -> raw jni value
		template <class V>
		[[nodiscard]] auto unwrap(const V& val) noexcept
		{
			if constexpr (std::is_arithmetic_v<V>)
				return val;
			else
				return val.obj().obj();
		}
	}

	template <class ClassT, fixed_string Mapping, class ValT, bool Static>
	jfieldID field<ClassT, Mapping, ValT, Static>::native()
	{
		static const jfieldID id = [&]
		{
			const jclass cls = ClassT::native();
			const zstring_view name = mapping();
			const std::string desc = sig::form<ValT>();

			if constexpr (Static)
				return jni::env()->GetStaticFieldID(cls, name.data(), desc.c_str());
			else
				return jni::env()->GetFieldID(cls, name.data(), desc.c_str());
		}();
		return id;
	}

	template <class ClassT, fixed_string Mapping, class ValT, bool Static>
	ValT field<ClassT, Mapping, ValT, Static>::get() const noexcept
	{
		JNIEnv* const env = jni::env();
		const jfieldID id = native();

#define JABA_GET(type, name) \
		if constexpr (is_jni_type<ValT, type>) \
		{ \
			if constexpr (Static) \
				return detail::wrap<ValT>(env->GetStatic##name##Field(ClassT::native(), id)); \
			else \
				return detail::wrap<ValT>(env->Get##name##Field(obj_.obj(), id)); \
		}
		JABA_FOREACH_JNI_TYPE_EX(JABA_GET)
#undef JABA_GET
	}

	template <class ClassT, fixed_string Mapping, class ValT, bool Static>
	void field<ClassT, Mapping, ValT, Static>::set(ValT val) noexcept
	{
		JNIEnv* const env = jni::env();
		const jfieldID id = native();

#define JABA_SET(type, name) \
		if constexpr (is_jni_type<ValT, type>) \
		{ \
			if constexpr (Static) \
				env->SetStatic##name##Field(ClassT::native(), id, detail::unwrap(val)); \
			else \
				env->Set##name##Field(obj_.obj(), id, detail::unwrap(val)); \
		}
		JABA_FOREACH_JNI_TYPE_EX(JABA_SET)
#undef JABA_SET
	}
}
