#pragma once
#include "jvm/compiler.hpp"
#include "hook/hook.hpp"

namespace jaba
{
	inline void init(const pe::image* const jvm)
	{
		abstract_interpreter::init(jvm);
	}
}
