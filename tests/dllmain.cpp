#include <thread>
#include <Windows.h>
#include "jni/jni.h"

#include "../src/jvm.hpp"
#include "../src/trap_frame.hpp"

extern "C" std::uint8_t* original_compiled_method_handler = nullptr;
extern "C" std::uint8_t* original_interpreted_method_handler = nullptr;

extern "C" void compiled_method_hook_handler();
extern "C" void interpreted_method_hook_handler();

namespace java
{
    JavaVM* virtual_machine = nullptr;
    JNIEnv* env = nullptr;

    void set_up()
    {
        JNI_GetCreatedJavaVMs(&virtual_machine, 1, nullptr);

        if (virtual_machine->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_8) == JNI_EDETACHED)
        {
            virtual_machine->AttachCurrentThread(reinterpret_cast<void**>(&env), nullptr);
        }
    }

    jmethodID get_sample_method()
    {
        // 1.8.9 mappings
        const jclass entity_living_base = env->FindClass("pr");

        const jmethodID jump_method = env->GetMethodID(entity_living_base, "bF", "()V");

        return jump_method;
    }
}

extern "C" void method_detour(trap_frame_t* const trap_frame, void* const original_method_handler)
{
	
}

DWORD __stdcall thread_entry(void* const module)
{
    java::set_up();

    const jmethodID sample_method = java::get_sample_method();

    method_t* const method = *reinterpret_cast<method_t**>(sample_method);

    original_compiled_method_handler = method->compiled_entry();
    original_interpreted_method_handler = method->interpreted_entry();

    method->set_compiled_entry(reinterpret_cast<std::uint8_t*>(compiled_method_hook_handler));
    method->set_interpreted_entry(reinterpret_cast<std::uint8_t*>(interpreted_method_hook_handler));

    while (!GetAsyncKeyState(VK_DELETE))
    {
        std::this_thread::sleep_for(std::chrono::microseconds(25));
    }

    method->set_compiled_entry(original_compiled_method_handler);
    method->set_interpreted_entry(original_interpreted_method_handler);

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

