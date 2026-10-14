#pragma once
#include <jni/jni.h>

namespace jaba
{
	using method_detour_t = void*;

	void hook_method(jmethodID method, method_detour_t detour);
	void unhook_method(jmethodID method);
}
