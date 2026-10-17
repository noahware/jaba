#include <Windows.h>

#include "jaba.hpp"
#include "hook/hook.hpp"
#include "java/java.hpp"
#include "jni/jni.hpp"
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

    JABA_CLASS_BEGIN(local_player, "net/minecraft/client/entity/EntityPlayerSP")
        JABA_FIELD(jboolean, on_ground, "onGround")
    JABA_CLASS_END(local_player)

    JABA_CLASS_BEGIN(minecraft, "net/minecraft/client/Minecraft")
        JABA_STATIC_FIELD(minecraft, mc, "theMinecraft")
        JABA_FIELD(local_player, player, "thePlayer")
    JABA_CLASS_END(minecraft)

	[[noreturn]] void thread_entry(const HMODULE mod)
	{
        util::log::init();
        java::init();

        //const jaba::clazz mc = jaba::find_class("net/le/minecraft");
        //const jaba::field field = mc.find_static_field<jint>("", "");

        if (!jaba::init())
        {
            LOG("failed to init jaba");

            util::log::reset();

            FreeLibraryAndExitThread(mod, 0);
        }

        jaba::load_mappings({
    { "net/minecraft/client/Minecraft", "ave" },
    { "net/minecraft/client/entity/EntityPlayerSP", "bew" },
    { "thePlayer", "h" },
    { "onGround", "C" },
    { "theMinecraft", "S" },
            });

        while (!GetAsyncKeyState(VK_DELETE))
        {
            try
            {
                const bool on_ground = minecraft::mc->player->on_ground;

                LOG("on ground {}", on_ground);
            }
            catch (const std::exception& e)
            {
                LOG("caught excp: {}", e.what());
            }

            Sleep(15);
        }

        /*const auto set_angles = set_angles_method();

        orig_set_angles = jaba::hook_method(set_angles, set_angles_detour).value_or(nullptr);

        if (!orig_set_angles)
            LOG("failed to hook set angles");

        Sleep(3000);

        jaba::unhook_method(set_angles);*/

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

