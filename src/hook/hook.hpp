#pragma once
#include <optional>
#include <jni/jni.h>

namespace jaba
{
	using method_detour_t = void*;

	// returns callable original method if success
	std::optional<jmethodID> hook_method(jmethodID jmethod, method_detour_t detour);
	bool unhook_method(jmethodID jmethod);
}
