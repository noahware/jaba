#include "log.hpp"
#include <Windows.h>

namespace
{
	FILE* cstream = nullptr;
}

void util::log::init()
{
	AllocConsole();

	freopen_s(&cstream, "CONOUT$", "w", stdout);
}

void util::log::reset()
{
	if (cstream)
		fclose(cstream);

	const HWND console_window = GetConsoleWindow();

	if (console_window != INVALID_HANDLE_VALUE)
	{
		ShowWindow(console_window, 0);
		FreeConsole();
	}
}
