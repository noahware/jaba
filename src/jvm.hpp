#pragma once
#include <cstdint>

struct const_method_t
{
	
};

class method_t
{
public:
	std::uint8_t* i2i_entry() const
	{
		return i2i_entry_;
	}

	void set_i2i_entry(std::uint8_t* const i2i_entry)
	{
		i2i_entry_ = i2i_entry;
	}

	std::uint8_t* compiled_entry() const
	{
		return compiled_entry_;
	}

	void set_compiled_entry(std::uint8_t* const compiled_entry)
	{
		compiled_entry_ = compiled_entry;
	}

	std::uint8_t* interpreted_entry() const
	{
		return interpreted_entry_;
	}

	void set_interpreted_entry(std::uint8_t* const interpreted_entry)
	{
		interpreted_entry_ = interpreted_entry;
	}

protected:
	std::uint8_t pad_one_[8] = { };
	const_method_t* const_method_ = nullptr;
	std::uint8_t pad_two_[16] = { };
	std::uint32_t flags_ = 0;
	std::uint8_t pad_three_[12] = { };
	std::uint8_t* i2i_entry_ = nullptr;
	std::uint8_t pad_four_[8] = { };
	std::uint8_t* compiled_entry_ = nullptr;
	std::uint8_t pad_five_[8] = { };
	std::uint8_t* interpreted_entry_ = nullptr;
	std::uint8_t pad_six_[40] = { };
};
