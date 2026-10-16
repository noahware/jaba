#pragma once
#include "vm_struct.hpp"
#include <cstring>
#include <format>
#include <optional>
#include <vector>
#include <stdexcept>
#include <string_view>

namespace jaba::jvm
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

        [[nodiscard]] T* operator->() noexcept
        {
            return reinterpret_cast<T*>(buf_.data());
        }

        [[nodiscard]] const T* operator->() const noexcept
        {
            return reinterpret_cast<const T*>(buf_.data());
        }

        [[nodiscard]] T* data() noexcept
        {
            return reinterpret_cast<T*>(buf_.data());
        }

        [[nodiscard]] const T* data() const noexcept
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

        [[nodiscard]] static std::optional<std::ptrdiff_t> field_offset_opt(const std::string_view field_name) noexcept
        {
            const auto vmf = vm_struct_entry::find(name, field_name);

            if (!vmf)
                return std::nullopt;

            return static_cast<std::ptrdiff_t>(vmf->offset());
        }

        [[nodiscard]] static std::ptrdiff_t field_offset(const std::string_view field_name)
        {
            const auto opt_off = field_offset_opt(field_name);

            if (!opt_off)
                throw std::runtime_error(std::format("unable to find {}!{}", name.view(), field_name));

            return *opt_off;
        }

        template <class FallbackT>
        [[nodiscard]] static std::ptrdiff_t field_offset_or(const std::string_view field_name, FallbackT&& fallback)
        {
            if (const auto vmf = vm_struct_entry::find(name, field_name))
                return static_cast<std::ptrdiff_t>(vmf->offset());

            return fallback();
        }

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
