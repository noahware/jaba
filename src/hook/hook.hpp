#pragma once
#include <jni/jni.h>

namespace jaba
{
	using method_detour_t = void*;

	bool hook_method(jmethodID jmethod, method_detour_t detour);
	void unhook_method(jmethodID jmethod);
}
