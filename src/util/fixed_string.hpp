#pragma once
#include <cstddef>
#include <string_view>

namespace jaba
{
	template <std::size_t N>
	struct fixed_string
	{
		char data[N]{};

		constexpr fixed_string(const char(&str)[N])
		{
			for (std::size_t i = 0; i < N; ++i)
				data[i] = str[i];
		}

		[[nodiscard]] constexpr std::string_view view() const
		{
			return { data, N - 1 };
		}

		[[nodiscard]] constexpr operator std::string_view() const
		{
			return view();
		}
	};
}
