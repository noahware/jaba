#include <Windows.h>
#include <thread>
#include <print>

#include "jni/jni.h"

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

    jmethodID get_set_angles_method(JNIEnv* const env)
    {
        const jclass entity = env->FindClass("pk");
        const jmethodID set_angles_method = env->GetMethodID(entity, "c", "(FF)V");

        return set_angles_method;
    }
}

void set_angles_handler(JNIEnv* const env, const jobject current_object, const jfloat yaw, const jfloat pitch)
{
    const jmethodID set_angles_method = java::get_set_angles_method(env);

    method_t* callable_method = hook::jni::get_callable_method(set_angles_method);
    const auto callable_method_id = reinterpret_cast<jmethodID>(&callable_method);

    env->CallVoidMethod(current_object, callable_method_id, yaw, pitch);

    std::println("set angles");
}

// <base of code offset, size of code>
std::pair<std::uint32_t, std::uint32_t> get_module_code_info(std::uint8_t* const module_base)
{
    const auto dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(module_base);
    const auto nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS>(module_base + dos_header->e_lfanew);

    return std::make_pair(nt_headers->OptionalHeader.BaseOfCode, nt_headers->OptionalHeader.SizeOfCode);
}

DWORD __stdcall thread_entry(void* const module)
{
    AllocConsole();

    FILE* console_stream = nullptr;
    freopen_s(&console_stream, "CONOUT$", "w", stdout);

    std::uint8_t* const jvm_base = reinterpret_cast<std::uint8_t*>(GetModuleHandleA("jvm.dll"));

    const auto module_code_info = get_module_code_info(jvm_base);

    java::set_up();
    hook::set_up(jvm_base + module_code_info.first, module_code_info.second);

    JNIEnv* const env = java::get_env();

    const jmethodID set_angles_method = java::get_set_angles_method(env);

    hook::jni::add_hook(set_angles_method, reinterpret_cast<void*>(&set_angles_handler));

	while (!GetAsyncKeyState(VK_DELETE))
    {
        std::this_thread::sleep_for(std::chrono::microseconds(25));
    }

    hook::jni::remove_hook(set_angles_method);

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

