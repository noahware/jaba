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
		[[nodiscard]] V wrap(const R raw)
		{
			if constexpr (std::is_same_v<V, R>)
				return raw;
			else
			{
				static_assert(std::is_constructible_v<V, jobject>, "add 'using clazz::clazz;' to this class");
				return V{ raw };
			}
		}

		// ValT -> raw jni value
		template <class V>
		[[nodiscard]] auto unwrap(const V& val) noexcept
		{
			if constexpr (std::is_arithmetic_v<V>)
				return val;
			else
				return val.obj();
		}
	}

	template <class ValT, class ClassT, fixed_string Mapping, class Loc>
	jfieldID field<ValT, ClassT, Mapping, Loc>::native()
	{
		// a failed lookup throws out of the initializer, so it's retried next call
		static const jfieldID id = []
		{
			const zstring_view name = mapping();
			const std::string desc = sig::form<ValT>();

			JNIEnv* const env = jni::env();

			const jfieldID fid = is_static
				? env->GetStaticFieldID(ClassT::native(), name.data(), desc.c_str())
				: env->GetFieldID(ClassT::native(), name.data(), desc.c_str());

			if (!fid)
			{
				env->ExceptionClear();
				throw std::runtime_error(std::format("unable to find field '{}' in '{}'", Mapping.view(), std::string_view{ ClassT::mapping() }));
			}

			return fid;
		}();

		return id;
	}

	template <class ValT, class ClassT, fixed_string Mapping, class Loc>
	ValT field<ValT, ClassT, Mapping, Loc>::get() const
	{
		JNIEnv* const env = jni::env();
		const jfieldID id = native();

#define JABA_GET(type, name) \
		if constexpr (is_jni_type<ValT, type>) \
		{ \
			if constexpr (is_static) \
				return detail::wrap<ValT>(env->GetStatic##name##Field(ClassT::native(), id)); \
			else \
				return detail::wrap<ValT>(env->Get##name##Field(self(), id)); \
		}
		JABA_FOREACH_JNI_TYPE_EX(JABA_GET)
#undef JABA_GET
	}

	template <class ValT, class ClassT, fixed_string Mapping, class Loc>
	void field<ValT, ClassT, Mapping, Loc>::set(ValT val)
	{
		JNIEnv* const env = jni::env();
		const jfieldID id = native();

#define JABA_SET(type, name) \
		if constexpr (is_jni_type<ValT, type>) \
		{ \
			if constexpr (is_static) \
				env->SetStatic##name##Field(ClassT::native(), id, detail::unwrap(val)); \
			else \
				env->Set##name##Field(self(), id, detail::unwrap(val)); \
		}
		JABA_FOREACH_JNI_TYPE_EX(JABA_SET)
#undef JABA_SET
	}
}
