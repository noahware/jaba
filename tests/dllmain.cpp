#include <Windows.h>

#include "jaba.hpp"
#include "hook/hook.hpp"
#include "java/java.hpp"
#include "util/image.hpp"
#include "util/log.hpp"

namespace
{
    jmethodID orig_set_angles = nullptr;

	[[nodiscard]] jmethodID set_angles_method()
	{
        return java::find_method("pk", "c", "(FF)V");
	}

    void set_angles_detour(JNIEnv* const env, const jobject obj, const jfloat yaw, const jfloat pitch)
    {
        env->CallVoidMethod(obj, orig_set_angles, yaw, pitch);

        LOG("set angles called {} {}", yaw, pitch);
    }

	[[noreturn]] void thread_entry(const HMODULE mod)
	{
        util::log::init();
        java::init();

        pe::image* const jvm = util::find_img("jvm.dll");
        LOG("jvm {}, size {}", jvm->as<std::uintptr_t>(), jvm->size());

        if (!jaba::init(java::env(), jvm))
        {
            LOG("failed to init jaba");

            util::log::reset();

            FreeLibraryAndExitThread(mod, 0);
        }

        jvmtiEnv* const jvmti = java::tinv();

        if (!jvmti)
        {
            LOG("failed to create jvmti");

            util::log::reset();

            FreeLibraryAndExitThread(mod, 0);
        }

        const auto set_angles = set_angles_method();

        orig_set_angles = jaba::hook_method(set_angles, set_angles_detour).value_or(nullptr);

        if (!orig_set_angles)
            LOG("failed to hook set angles");

        Sleep(3000);

        jaba::unhook_method(set_angles);

        util::log::reset();

        FreeLibraryAndExitThread(mod, 0);
	}
}

BOOL APIENTRY DllMain(const HMODULE mod, const DWORD reason, [[maybe_unused]] const LPVOID reserved)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(mod);

        CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(thread_entry), mod, 0, nullptr);
    }

    return TRUE;
}

