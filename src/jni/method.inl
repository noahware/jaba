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
}
