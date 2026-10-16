#include "jaba.hpp"
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

	jaba::jni::set_vm(vm);

	JNIEnv* const env = jaba::jni::env();

	if (!env)
		return false;

#ifdef _WIN32
	const auto jvm = reinterpret_cast<const jaba::util::image*>(GetModuleHandleA("jvm.dll"));
#else
	const jaba::util::image* const jvm = jaba::util::image::find("libjvm.so");
#endif

	return jvm && jaba::jvm::abstract_interpreter::init(env) && jaba::jvm::deopt::init(env, jvm);
}
