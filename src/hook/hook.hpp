#pragma once
#include <optional>
#include <jni/jni.h>

#define JABA_MHK_USE_JVMTI

#ifdef JABA_MHK_USE_JVMTI
#include <jni/jvmti.h>
#endif

namespace jaba
{
	using method_detour_t = void*;

#ifdef JABA_MHK_USE_JVMTI
	// returns callable original method if success
	std::optional<jmethodID> hook_method(jmethodID jmethod, method_detour_t detour, jvmtiEnv* tinv);
#else
	// returns callable original method if success
	std::optional<jmethodID> hook_method(jmethodID jmethod, method_detour_t detour);
#endif

	bool unhook_method(jmethodID jmethod);
}
