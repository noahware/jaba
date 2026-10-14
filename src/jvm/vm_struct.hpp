#pragma once
#include <cstdint>
#include <string_view>

class vm_struct_entry
{
public:
	[[nodiscard]] std::string_view type_name() const noexcept
	{
		return type_name_;
	}

	[[nodiscard]] std::string_view field_name() const noexcept
	{
		return field_name_;
	}

	[[nodiscard]] std::string_view type_string() const noexcept
	{
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
	[[nodiscard]] std::string_view type_name() const
	{
		return type_name_;
	}

	[[nodiscard]] std::string_view class_name() const
	{
		return super_class_name_;
	}

	[[nodiscard]] bool is_oop_type() const
	{
		return is_oop_type_;
	}

	[[nodiscard]] bool is_integer_type() const
	{
		return is_integer_type_;
	}

	[[nodiscard]] bool is_unsigned() const
	{
		return is_unsigned_;
	}

	[[nodiscard]] std::uint64_t size() const
	{
		return size_;
	}

protected:
	const char* type_name_;
	const char* super_class_name_;
	std::int32_t is_oop_type_ = 0;
	std::int32_t is_integer_type_ = 0;
	std::int32_t is_unsigned_ = 0;
	std::uint64_t size_ = 0;
};
