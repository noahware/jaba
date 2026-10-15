#pragma once
#include "jvm/compiler.hpp"
#include "hook/hook.hpp"

namespace jaba
{
	[[nodiscard]] inline bool init(JNIEnv* const env)
	{
		return abstract_interpreter::init(env);
	}
}
