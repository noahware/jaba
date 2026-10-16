#include "env.hpp"

namespace
{
	JavaVM* java_vm = nullptr;

	// detaches the calling thread from java_vm, but only if env() attached it itself: a thread
	// the jvm already owned must stay attached since we did not attach it
	struct thread_attach_guard
	{
		~thread_attach_guard()
		{
			if (attached && java_vm)
				java_vm->DetachCurrentThread();
		}

		bool attached = false;
	};
}

void jaba::jni::set_vm(JavaVM* const vm)
{
	java_vm = vm;
}

JNIEnv* jaba::jni::env()
{
	thread_local JNIEnv* cached = nullptr;
	thread_local thread_attach_guard guard;

	if (cached)
		return cached;

	if (!java_vm)
		return nullptr;

	const jint status = java_vm->GetEnv(reinterpret_cast<void**>(&cached), JNI_VERSION_1_6);

	if (status == JNI_EDETACHED)
	{
		if (java_vm->AttachCurrentThread(reinterpret_cast<void**>(&cached), nullptr) != JNI_OK)
		{
			cached = nullptr;
			return nullptr;
		}

		guard.attached = true;
	}
	else if (status != JNI_OK)
	{
		cached = nullptr;
		return nullptr;
	}

	return cached;
}
