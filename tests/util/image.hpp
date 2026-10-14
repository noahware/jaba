#pragma once
#include <pe/image.hpp>
#include <Windows.h>
#include <string>

namespace util
{
	[[nodiscard]] inline pe::image* find_img(const std::string& name)
	{
		return reinterpret_cast<pe::image*>(GetModuleHandleA(name.c_str()));
	}
}
