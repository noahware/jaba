#pragma once
#include "vm_struct.hpp"
#include <format>
#include <vector>
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

    template <class T>
    class jvm_type_copy
    {
    public:
        jvm_type_copy() noexcept = default;

        explicit jvm_type_copy(std::vector<std::uint8_t> buf)
	        :   buf_(std::move(buf)) { }

        T* operator->() noexcept
        {
            return reinterpret_cast<T*>(buf_.data());
        }

        const T* operator->() const noexcept
        {
            return reinterpret_cast<const T*>(buf_.data());
        }

        [[nodiscard]] std::vector<std::uint8_t>& buf() noexcept
        {
            return buf_;
        }

        [[nodiscard]] const std::vector<std::uint8_t>& buf() const noexcept
        {
            return buf_;
        }

    protected:
        std::vector<std::uint8_t> buf_;
    };

    template <fixed_string Name>
    class jvm_type
    {
    public:
        static constexpr fixed_string name = Name;

        [[nodiscard]] std::size_t size() const
        {
            const vm_type_entry* const vmt = vm_type_entry::find(name);
        	
        	if (!vmt)
				throw std::runtime_error(std::format("unable to find {}", name.view()));

            return vmt->size();
        }

        template <class T = void>
		[[nodiscard]] jvm_type_copy<T> copy() const
        {
            const auto s = size();

            std::vector<std::uint8_t> c;
            c.resize(s);

            std::memcpy(c.data(), this, s);

            return jvm_type_copy<T>{ std::move(c) };
        }
    };
}
