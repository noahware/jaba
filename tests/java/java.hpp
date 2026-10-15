#pragma once
#include <jni/jni.h>
#include <jni/jvmti.h>
#include <string>

namespace java
{
    inline JavaVM* vm = nullptr;

    inline void init()
    {
        JNI_GetCreatedJavaVMs(&vm, 1, nullptr);
    }

	[[nodiscard]] inline JNIEnv* env()
    {
        JNIEnv* e = nullptr;

        if (vm->GetEnv(reinterpret_cast<void**>(&e), JNI_VERSION_1_6) == JNI_EDETACHED)
        {
            vm->AttachCurrentThread(reinterpret_cast<void**>(&e), nullptr);
        }

        return e;
    }

	[[nodiscard]] inline jvmtiEnv* tinv()
    {
        static jvmtiEnv* const t = []() -> jvmtiEnv*
        {
            static_cast<void>(env());

            jvmtiEnv* e = nullptr;

            if (vm->GetEnv(reinterpret_cast<void**>(&e), JVMTI_VERSION_1_2) != JNI_OK)
                return nullptr;

            jvmtiCapabilities caps = { };
            caps.can_retransform_classes = 1;

            if (e->AddCapabilities(&caps) != JVMTI_ERROR_NONE)
                return nullptr;

            return e;
        }();

        return t;
    }

    // when are we getting null terminated guaranteed string views...
	[[nodiscard]] inline jmethodID find_method(const std::string& class_name, const std::string& method_name,
	                                           const std::string& method_sig)
    {
        const auto e = env();
        const auto clazz = e->FindClass(class_name.c_str());

        if (!clazz)
            return nullptr;

        return e->GetMethodID(clazz, method_name.c_str(), method_sig.c_str());
    }
}
