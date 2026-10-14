#include <Windows.h>

namespace
{
	void thread_entry(const HMODULE mod)
	{
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

