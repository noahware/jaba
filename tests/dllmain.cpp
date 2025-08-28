#include <Windows.h>
#include <thread>
#include <print>

#include "jni/jni.h"
#include "jni/jvmti.h"

#include "../src/hook.hpp"

namespace java
{
    JavaVM* virtual_machine = nullptr;

    void set_up()
    {
        JNI_GetCreatedJavaVMs(&virtual_machine, 1, nullptr);
    }

    JNIEnv* get_env()
    {
        JNIEnv* env = nullptr;

        if (virtual_machine->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) == JNI_EDETACHED)
        {
            virtual_machine->AttachCurrentThread(reinterpret_cast<void**>(&env), nullptr);
        }

        return env;
    }

    jvmtiEnv* get_jvmti_env()
    {
        jvmtiEnv* env = nullptr;

        if (virtual_machine->GetEnv(reinterpret_cast<void**>(&env), JVMTI_VERSION_1_2) != JNI_OK)
        {
            return nullptr;
        }

        constexpr jvmtiCapabilities capabilities = { .can_retransform_classes = JVMTI_ENABLE };

        if (env->AddCapabilities(&capabilities) != JVMTI_ERROR_NONE)
        {
            return nullptr;
        }

        return env;
    }

    jmethodID get_sample_method(JNIEnv* const env)
    {
		const jclass entity_living_base = env->FindClass("pr");

        const jmethodID jump_method = env->GetMethodID(entity_living_base, "bF", "()V");

        return jump_method;
    }
}

std::uint64_t jump_handler(argument_parser_t* arguments, std::uint8_t* should_call_original)
{
    *should_call_original = 1;

    std::println("entity_living_base_t::jump was called");

    return 0;
}

DWORD __stdcall thread_entry(void* const module)
{
    java::set_up();

    AllocConsole();

    FILE* console_stream = nullptr;
    freopen_s(&console_stream, "CONOUT$", "w", stdout);

    JNIEnv* const env = java::get_env();
    jvmtiEnv* const ti = java::get_jvmti_env();

    const jmethodID sample_method = java::get_sample_method(env);

    hook::add_hook(env, ti, sample_method, jump_handler);

    while (!GetAsyncKeyState(VK_DELETE))
    {
        std::this_thread::sleep_for(std::chrono::microseconds(25));
    }

    hook::remove_hook(sample_method);

    FreeLibraryAndExitThread(static_cast<HMODULE>(module), 0);
}

BOOL APIENTRY DllMain( HMODULE const module,
                       const DWORD reason_for_call,
                       LPVOID const reserved
                     )
{
    switch (reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        DisableThreadLibraryCalls(module);

        CreateThread(nullptr, 0, thread_entry, module, 0, nullptr);

        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

