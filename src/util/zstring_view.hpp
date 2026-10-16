#pragma once
#include <string_view>
#include <string>

namespace jaba
{
	class zstring_view : public std::string_view
	{
	public:
		constexpr zstring_view() noexcept = default;

		constexpr zstring_view(const char* const c) noexcept
			:	std::string_view(c) { }

		constexpr zstring_view(const std::string& s) noexcept
			:	std::string_view(s) {}
	};
}
