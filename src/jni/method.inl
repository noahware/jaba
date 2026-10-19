#pragma once
#include <format>
#include <stdexcept>
#include "method.hpp"
#include "clazz.hpp"
#include "env.hpp"
#include "exception.hpp"
#include "signature.hpp"
#include "types.hpp"

namespace jaba::jni
{
	template <class ClassT, fixed_string Mapping, class Loc, class RetT, class... ArgsT>
	jmethodID method<ClassT, Mapping, Loc, RetT, ArgsT...>::native()
	{
		// a failed lookup throws out of the initializer, so it's retried next call
		static const jmethodID id = []
		{
			const zstring_view name = mapping();
			// a constructor's RetT is the class it builds, but its descriptor always ends in V
			const std::string desc = is_constructor()
				? sig::form_method<void, ArgsT...>()
				: sig::form_method<RetT, ArgsT...>();

			JNIEnv* const env = jni::env();

			// <init> is looked up with GetMethodID even though the wrapper is static
			const jmethodID mid = is_static && !is_constructor()
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
	RetT method<ClassT, Mapping, Loc, RetT, ArgsT...>::invoke(JNIEnv* const env, const jmethodID id, const jobject target, const ArgsT&... args)
	{
		// has to run right after the call, before the result is wrapped. What java threw is kept
		// in the exception (see java_exception)
		const auto check = [env]
		{
			if (env->ExceptionCheck())
				throw java_exception::take(env, std::format("thrown by method {} of {}", Mapping.view(), std::string_view{ ClassT::mapping() }));
		};

		if constexpr (is_constructor())
		{
			const jobject created = env->NewObject(static_cast<jclass>(target), id, detail::unwrap(args)...);
			check();
			return detail::wrap<RetT>(created);
		}
		else if constexpr (std::is_void_v<RetT>)
		{
			if constexpr (is_static)
				env->CallStaticVoidMethod(static_cast<jclass>(target), id, detail::unwrap(args)...);
			else
				env->CallVoidMethod(target, id, detail::unwrap(args)...);

			check();
		}
		else
		{
#define JABA_CALL_MID(type, name) \
			if constexpr (is_jni_type<RetT, type>) \
			{ \
				const auto raw = [&] \
				{ \
					if constexpr (is_static) \
						return env->CallStatic##name##Method(static_cast<jclass>(target), id, detail::unwrap(args)...); \
					else \
						return env->Call##name##Method(target, id, detail::unwrap(args)...); \
				}(); \
				check(); \
				return detail::wrap<RetT>(raw); \
			}
			JABA_FOREACH_JNI_TYPE_EX(JABA_CALL_MID)
#undef JABA_CALL_MID
		}
	}

	template <class ClassT, fixed_string Mapping, class Loc, class RetT, class... ArgsT>
	RetT method<ClassT, Mapping, Loc, RetT, ArgsT...>::operator()(const ArgsT&... args) const
	{
		const jobject target = [&]
		{
			if constexpr (is_static)
				return static_cast<jobject>(cls());
			else
				return self(this);
		}();

		return invoke(jni::env(), native(), target, args...);
	}
}
