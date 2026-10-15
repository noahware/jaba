#pragma once
#include <optional>
#include <jni/jni.h>

#ifdef JABA_MHK_USE_JVMTI
#include <jni/jvmti.h>
#endif

namespace jaba
{
	using method_detour_t = void*;

#ifdef JABA_MHK_USE_JVMTI
	// called once by jaba::init, nothing else needs to touch jvmti directly
	[[nodiscard]] bool init_jvmti(JNIEnv* env);
#endif

	// returns callable original method if success
	std::optional<jmethodID> hook_method(jmethodID jmethod, method_detour_t detour);

	bool unhook_method(jmethodID jmethod);
}
