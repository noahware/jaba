#pragma once
#include "jvm/compiler.hpp"
#include "jvm/deopt.hpp"
#include "jvm/method.hpp"
#include "hook/hook.hpp"

namespace jaba
{
	[[nodiscard]] inline bool init(JNIEnv* const env, const pe::image* const jvm)
	{
		if (!abstract_interpreter::init(env))
			return false;

		if (!deopt::init(env, jvm))
			return false;

		return true;
	}
}
