#pragma once
#include "jvm/compiler.hpp"
#include "jvm/deopt.hpp"
#include "jvm/method.hpp"
#include "hook/hook.hpp"

namespace jaba
{
	[[nodiscard]] inline bool init(JNIEnv* const env, [[maybe_unused]] const pe::image* const jvm)
	{
		if (!abstract_interpreter::init(env))
			return false;

#ifndef JABA_MHK_USE_JVMTI
		if (!deopt::init(env, jvm))
			return false;
#endif

		return true;
	}
}
