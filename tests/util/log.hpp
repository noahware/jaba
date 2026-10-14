#pragma once
#include <cstdio>
#include <format>

#define LOG(...) std::printf("%s\n", std::format(__VA_ARGS__).c_str())

namespace util::log
{
	void init();
	void reset();
}
