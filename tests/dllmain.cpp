#include <Windows.h>

#include "jaba.hpp"
#include "hook/hook.hpp"
#include "hook/pending.hpp"
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

    // the detour has to be declared before JABA_HOOK names it, the class is only forward
// declared here (complete types aren't needed to declare it), defined once the class is
    class entity_living_base;
    void on_jump(const entity_living_base& self, void (*original)(const entity_living_base&));

    // two groups of Entity's fields, each its own members struct
    JABA_CLASS_BEGIN(entity_pos, "net/minecraft/entity/Entity")
        JABA_FIELD(jdouble, x, "posX")
        JABA_FIELD(jdouble, y, "posY")
        JABA_FIELD(jdouble, z, "posZ")
    JABA_CLASS_END(entity_pos)

    JABA_CLASS_BEGIN(entity_rot, "net/minecraft/entity/Entity")
        JABA_FIELD(jfloat, yaw, "rotationYaw")
        JABA_FIELD(jfloat, pitch, "rotationPitch")
    JABA_CLASS_END(entity_rot)

    // multiple inheritance, gets both groups plus its own
    JABA_CLASS_BEGIN(entity_living_base, "net/minecraft/client/entity/EntityPlayerSP", entity_pos, entity_rot)
		JABA_METHOD(void, set_sprinting, "setSprinting", jboolean)

        JABA_METHOD(void, jump, "jump")
		JABA_HOOK(jump, on_jump)
    JABA_CLASS_END(entity_living_base)

    void on_jump(const entity_living_base& self, const entity_living_base::jump_mhk_original original)
    {
        LOG("jump called");

        original(self);
    }

    JABA_CLASS_BEGIN(local_player, "net/minecraft/client/entity/EntityPlayerSP", entity_living_base)
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

        if (!jaba::init())
        {
            LOG("failed to init jaba");

            util::log::reset();

            FreeLibraryAndExitThread(mod, 0);
        }

        jaba::jni::load_mappings({
			    { "net/minecraft/client/Minecraft", "ave" },
			    { "net/minecraft/client/entity/EntityPlayerSP", "bew" },
			    { "net/minecraft/entity/Entity", "pk" },
			    { "posX", "s" },
			    { "posY", "t" },
			    { "posZ", "u" },
			    { "jump", "bF" },
			    { "rotationYaw", "y" },
			    { "rotationPitch", "z" },
			    { "thePlayer", "h" },
			    { "setSprinting", "d" },
			    { "onGround", "C" },
			    { "theMinecraft", "S" },
        });

        // the mappings have to be loaded before a hook can find its method, the flush in
        // jaba::init found none and left the hooks queued
        jaba::jni::flush_queued_hooks();

        while (!GetAsyncKeyState(VK_DELETE))
        {
            try
            {
                const local_player player = minecraft::mc->player;

                player.set_sprinting(true);

                // x/y/z come from entity_pos_members, yaw/pitch from entity_rot_members
                const double x = player.x, y = player.y, z = player.z;
                const float yaw = player.yaw, pitch = player.pitch;
                const bool on_ground = player.on_ground;

                LOG("pos {:.2f} {:.2f} {:.2f} rot {:.1f} {:.1f} on ground {}", x, y, z, yaw, pitch, on_ground);
            }
            catch (const std::exception& e)
            {
                LOG("caught excp: {}", e.what());
            }

            Sleep(150);
        }

        /*const auto set_angles = set_angles_method();

        orig_set_angles = jaba::hook_method(set_angles, set_angles_detour).value_or(nullptr);

        if (!orig_set_angles)
            LOG("failed to hook set angles");

        Sleep(3000);

        jaba::unhook_method(set_angles);*/

        jaba::uninit();
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

