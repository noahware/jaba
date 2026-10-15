#pragma once
#include "vm_struct.hpp"
#include <format>
#include <stdexcept>
#include <string_view>

namespace jaba
{
    template<size_t N>
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

    template <fixed_string Name>
    class jvm_type
    {
    public:
        static constexpr auto name = Name;

        [[nodiscard]] std::size_t size() const
        {
            const vm_type_entry* const vmt = vm_type_entry::find(name);
        	
        	if (!vmt)
				throw std::runtime_error(std::format("unable to find {}", name));

            return vmt->size();
        }
    };
}
