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
        const jclass entity = env->FindClass("pk");
        const jmethodID set_angles_method = env->GetMethodID(entity, "c", "(FF)V");

        return set_angles_method;
    }
}

std::uint64_t set_angles_handler(argument_parser_t* const arguments, JNIEnv* const env, std::uint8_t* const should_call_original)
{
    *should_call_original = 1;

    const jfloat yaw = arguments->get_argument<jvalue*>(1)->f;
    const jfloat pitch = arguments->get_argument<jvalue*>(0)->f;

    std::println("entity_t::set_angles was called ({},{})", yaw, pitch);

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

    hook::add_hook(env, ti, sample_method, set_angles_handler);

	while (!GetAsyncKeyState(VK_DELETE))
    {
        std::this_thread::sleep_for(std::chrono::microseconds(25));
    }

    hook::remove_hook(sample_method);

    const HWND console_window = GetConsoleWindow();

    ShowWindow(console_window, 0);

    FreeConsole();

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

