#include <Windows.h>
#include "java/java.hpp"
#include "util/image.hpp"
#include "util/log.hpp"

namespace
{
	[[nodiscard]] jmethodID set_angles_method()
	{
        return java::find_method("pk", "c", "(FF)V");
	}

	[[noreturn]] void thread_entry(const HMODULE mod)
	{
        util::log::init();
        java::init();

        pe::image* const jvm = util::find_img("jvm.dll");
        LOG("jvm {}, size {}", jvm->as<std::uintptr_t>(), jvm->size());

        const auto set_angles = set_angles_method();

        Sleep(3000);

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

