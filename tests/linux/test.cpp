#include <cstdio>
#include <format>

#include "jaba.hpp"

#define LOG(...) std::printf("%s\n", std::format(__VA_ARGS__).c_str())

namespace
{
	jmethodID target_add = nullptr;
	jmethodID orig_add = nullptr;

	jint add_detour(JNIEnv* const env, const jobject obj, const jint a, const jint b)
	{
		return env->CallIntMethod(obj, orig_add, a, b) + 100;
	}
}

extern "C" JNIEXPORT jboolean JNICALL Java_Target_install(JNIEnv* const env, const jclass clazz)
{
	if (!jaba::init())
	{
		LOG("failed to init jaba");
		return JNI_FALSE;
	}

	target_add = env->GetMethodID(clazz, "add", "(II)I");

	if (!target_add)
		return JNI_FALSE;

	orig_add = jaba::hook_method(target_add, reinterpret_cast<jaba::method_detour_t>(add_detour)).value_or(nullptr);

	if (!orig_add)
		LOG("failed to hook add");

	return orig_add != nullptr;
}

extern "C" JNIEXPORT jboolean JNICALL Java_Target_uninstall([[maybe_unused]] JNIEnv* const env, [[maybe_unused]] const jclass clazz)
{
	return jaba::unhook_method(target_add);
}
