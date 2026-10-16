#pragma once
#include <cstdint>
#include <string_view>

namespace jaba::jvm
{
	class vm_struct_entry
	{
	public:
		static vm_struct_entry* find(std::string_view type_name, std::string_view field_name) noexcept;

		[[nodiscard]] std::string_view type_name() const noexcept
		{
			if (!type_name_)
				return { };

			return type_name_;
		}

		[[nodiscard]] std::string_view field_name() const noexcept
		{
			if (!field_name_)
				return { };

			return field_name_;
		}

		[[nodiscard]] std::string_view type_string() const noexcept
		{
			if (!type_string_)
				return { };

			return type_string_;
		}

		[[nodiscard]] bool is_static() const noexcept
		{
			return is_static_;
		}

		[[nodiscard]] std::uint64_t offset() const noexcept
		{
			return offset_;
		}

		[[nodiscard]] std::uint8_t* address() const noexcept
		{
			return address_;
		}

	protected:
		const char* type_name_ = nullptr;
		const char* field_name_ = nullptr;
		const char* type_string_ = nullptr;
		std::int32_t is_static_ = 0;
		std::uint64_t offset_ = 0;
		std::uint8_t* address_ = nullptr;
	};

	class vm_type_entry
	{
	public:
		static vm_type_entry* find(std::string_view type_name) noexcept;

		[[nodiscard]] std::string_view type_name() const noexcept
		{
			if (!type_name_)
				return { };

			return type_name_;
		}

		[[nodiscard]] std::string_view class_name() const noexcept
		{
			if (!super_class_name_)
				return { };

			return super_class_name_;
		}

		[[nodiscard]] bool is_oop_type() const noexcept
		{
			return is_oop_type_;
		}

		[[nodiscard]] bool is_integer_type() const noexcept
		{
			return is_integer_type_;
		}

		[[nodiscard]] bool is_unsigned() const noexcept
		{
			return is_unsigned_;
		}

		[[nodiscard]] std::size_t size() const noexcept
		{
			return size_;
		}

	protected:
		const char* type_name_ = nullptr;
		const char* super_class_name_ = nullptr;
		std::int32_t is_oop_type_ = 0;
		std::int32_t is_integer_type_ = 0;
		std::int32_t is_unsigned_ = 0;
		std::uint64_t size_ = 0;
	};
}
