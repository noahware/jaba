#pragma once
#include "method.hpp"
#include "clazz.hpp"
#include "env.hpp"
#include "signature.hpp"
#include "types.hpp"

namespace jaba
{
	template <class ClassT, fixed_string Mapping, class Loc, class RetT, class... ArgsT>
	jmethodID method<ClassT, Mapping, Loc, RetT, ArgsT...>::native()
	{
		// a failed lookup throws out of the initializer, so it's retried next call
		static const jmethodID id = []
		{
			const zstring_view name = mapping();
			const std::string desc = sig::form_method<RetT, ArgsT...>();

			JNIEnv* const env = jni::env();

			const jmethodID mid = is_static
				? env->GetStaticMethodID(ClassT::native(), name.data(), desc.c_str())
				: env->GetMethodID(ClassT::native(), name.data(), desc.c_str());

			if (!mid)
			{
				env->ExceptionClear();
				throw std::runtime_error(std::format("unable to find method '{}' in '{}'", Mapping.view(), std::string_view{ ClassT::mapping() }));
			}

			return mid;
		}();

		return id;
	}

	template <class ClassT, fixed_string Mapping, class Loc, class RetT, class... ArgsT>
	RetT method<ClassT, Mapping, Loc, RetT, ArgsT...>::operator()(ArgsT... args) const
	{
		const jmethodID id = native();
		JNIEnv* const env = jni::env();

#define JABA_CALL_MID(type, name) \
		if constexpr (is_jni_type<RetT, type>) \
		{ \
			if constexpr (is_static) \
				return detail::wrap<RetT>(env->CallStatic##name##Method(cls(), id, args...)); \
			else \
				return detail::wrap<RetT>(env->Call##name##Method(self(this), id, args...)); \
		}
		JABA_FOREACH_JNI_METHOD_TYPE_EX(JABA_CALL_MID)
#undef JABA_CALL_MID
	}
}
