#include "jaba.hpp"
#include "hook/pending.hpp"
#include "jni/env.hpp"

#ifdef _WIN32
#include <Windows.h>
#endif

bool jaba::init()
{
	JavaVM* vm = nullptr;
	jsize vm_count = 0;

	if (JNI_GetCreatedJavaVMs(&vm, 1, &vm_count) != JNI_OK || !vm_count)
		return false;

	jni::set_vm(vm);

	JNIEnv* const env = jni::env();

	if (!env)
		return false;

#ifdef _WIN32
	const auto jvm = reinterpret_cast<const util::image*>(GetModuleHandleA("jvm.dll"));
#else
	const util::image* const jvm = util::image::find("libjvm.so");
#endif

	if (!jvm || !jvm::abstract_interpreter::init(env) || !jvm::deopt::init(env, jvm))
		return false;

	return true;
}

void jaba::uninit()
{
	remove_all_hooks();
}
