#include "jaba.hpp"
#include <Windows.h>

bool jaba::init()
{
	JavaVM* vm = nullptr;
	jsize vm_count = 0;

	if (JNI_GetCreatedJavaVMs(&vm, 1, &vm_count) != JNI_OK || !vm_count)
		return false;

	JNIEnv* env = nullptr;

	const jint status = vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);

	// only detach a thread this call attached, an already attached caller must stay attached
	const bool attached = status == JNI_EDETACHED && vm->AttachCurrentThread(reinterpret_cast<void**>(&env), nullptr) == JNI_OK;

	if (!env)
		return false;

	const auto jvm = reinterpret_cast<const pe::image*>(GetModuleHandleA("jvm.dll"));

	const bool ok = jvm && abstract_interpreter::init(env) && deopt::init(env, jvm);

	if (attached)
		vm->DetachCurrentThread();

	return ok;
}
