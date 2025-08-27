#include <Windows.h>
#include <unordered_map>
#include <thread>
#include <print>

#include "jni/jni.h"
#include "jni/jvmti.h"

#include "../src/jvm.hpp"
#include "../src/trap_frame.hpp"

extern "C" std::uint8_t* original_interpreted_method_handler = nullptr;
extern "C" std::uint8_t* original_i2i_method_handler = nullptr;

extern "C" void interpreted_method_hook_handler();
extern "C" void i2i_method_hook_handler();

namespace java
{
    JavaVM* get_virtual_machine()
    {
        JavaVM* virtual_machine = nullptr;

        JNI_GetCreatedJavaVMs(&virtual_machine, 1, nullptr);

        return virtual_machine;
    }

    JNIEnv* get_env()
    {
        JavaVM* virtual_machine = get_virtual_machine();

        JNIEnv* env = nullptr;

        if (virtual_machine->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_8) == JNI_EDETACHED)
        {
            virtual_machine->AttachCurrentThread(reinterpret_cast<void**>(&env), nullptr);
        }

        return env;
    }

    jvmtiEnv* get_jvmti_env()
    {
        JavaVM* virtual_machine = get_virtual_machine();

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

    jmethodID get_sample_method()
    {
        JNIEnv* const env = get_env();

        // 1.8.9 mappings
        const jclass entity = env->FindClass("pk");
        const jmethodID set_angles_method = env->GetMethodID(entity, "c", "(FF)V");

        return set_angles_method;
    }
}

// <thread_id, should_skip>
std::unordered_map<std::thread::id, std::uint8_t> thread_list;

std::uint8_t should_skip_call()
{
    return thread_list[std::this_thread::get_id()];
}

void skip_next_call()
{
    thread_list[std::this_thread::get_id()] = 1;
}

void do_call_skip()
{
    thread_list[std::this_thread::get_id()] = 0;
}

void call_method_original(const jobject current_object, const jfloat yaw, const jfloat pitch)
{
    skip_next_call();

    JNIEnv* const env = java::get_env();

    const jmethodID sample_method = java::get_sample_method();

    env->CallVoidMethod(current_object, sample_method, yaw, pitch);
}

class argument_parser_t
{
public:
    template <class t>
    t get_argument(const std::uint64_t i)
    {
        return reinterpret_cast<t>(reinterpret_cast<void**>(this) + i);
    }
};

// returns 1 == should jump to original method handler
extern "C" std::uint8_t method_detour(argument_parser_t* const arguments)
{
    if (should_skip_call())
    {
        do_call_skip();

        return 1;
    }

    jobject const current_object = arguments->get_argument<jobject>(2);
    jfloat const yaw = arguments->get_argument<jvalue*>(1)->f;
    jfloat const pitch = arguments->get_argument<jvalue*>(0)->f;

    call_method_original(current_object, yaw, pitch);

    std::println("entity_t::set_angles was called ({},{})", yaw, pitch);

    return 0;
}

void recompile_method_class(const jmethodID method)
{
    jvmtiEnv* const env = java::get_jvmti_env();

    jclass declaring_class = nullptr;

    env->GetMethodDeclaringClass(method, &declaring_class);
    env->RetransformClasses(1, &declaring_class);
}

DWORD __stdcall thread_entry(void* const module)
{
    AllocConsole();

    FILE* console_stream = nullptr;
    freopen_s(&console_stream, "CONOUT$", "w", stdout);

    const jmethodID sample_method = java::get_sample_method();

    recompile_method_class(sample_method);

    method_t* const method = *reinterpret_cast<method_t**>(sample_method);

    original_interpreted_method_handler = method->interpreted_entry();
    original_i2i_method_handler = method->i2i_entry();

    std::uint8_t* const interpreted_detour_handler = reinterpret_cast<std::uint8_t*>(interpreted_method_hook_handler);
    std::uint8_t* const i2i_detour_handler = reinterpret_cast<std::uint8_t*>(i2i_method_hook_handler);

    method->set_interpreted_entry(interpreted_detour_handler);
    method->set_i2i_entry(i2i_detour_handler);

    method->set_no_inline();
    method->set_access_flags(method->access_flags() | 0xE000000);

    while (!GetAsyncKeyState(VK_DELETE))
    {
        std::this_thread::sleep_for(std::chrono::microseconds(25));
    }

    method->set_interpreted_entry(original_interpreted_method_handler);
    method->set_i2i_entry(original_i2i_method_handler);

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

