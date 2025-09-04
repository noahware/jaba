#pragma once
#include <cstdint>

struct const_method_t
{
	
};

class nmethod_t
{
public:
	enum state_t : std::uint8_t
	{
		in_use = 0,
		not_entrant = 1,
		zombie = 2,
		unloaded = 3
	};

	[[nodiscard]] std::uint8_t marked_for_decompilation() const
	{
		return marked_for_decompilation_;
	}

	void set_marked_for_decompilation(const std::uint8_t marked_for_decompilation)
	{
		marked_for_decompilation_ = marked_for_decompilation;
	}

	[[nodiscard]] state_t state() const
	{
		return state_;
	}

	void set_state(const state_t state)
	{
		state_ = state;
	}

protected:
	std::uint8_t pad_one_[0xD6] = { };
	std::uint8_t marked_for_decompilation_;
	state_t state_ = { };
};

class method_t
{
public:
	[[nodiscard]] std::uint8_t* i2i_entry() const
	{
		return i2i_entry_;
	}

	void set_i2i_entry(std::uint8_t* const i2i_entry)
	{
		i2i_entry_ = i2i_entry;
	}

	[[nodiscard]] std::uint8_t* compiled_entry() const
	{
		return compiled_entry_;
	}

	void set_compiled_entry(std::uint8_t* const compiled_entry)
	{
		compiled_entry_ = compiled_entry;
	}

	[[nodiscard]] std::uint8_t* interpreted_entry() const
	{
		return interpreted_entry_;
	}

	void set_interpreted_entry(std::uint8_t* const interpreted_entry)
	{
		interpreted_entry_ = interpreted_entry;
	}

	[[nodiscard]] std::int32_t access_flags() const
	{
		return access_flags_;
	}

	void set_access_flags(const std::int32_t flags)
	{
		access_flags_ = flags;
	}

	void set_no_inline()
	{
		dont_inline_ = 1;
		force_inline_ = 0;
	}

	[[nodiscard]] std::int32_t vtable_index() const
	{
		return vtable_index_;
	}

	void set_vtable_index(std::int32_t const vtable_index)
	{
		vtable_index_ = vtable_index;
	}

	[[nodiscard]] const_method_t* const_method() const
	{
		return const_method_;
	}

	[[nodiscard]] nmethod_t* code() const
	{
		return code_;
	}

	void set_code(nmethod_t* const code)
	{
		code_ = code;
	}

	void decompile()
	{
		nmethod_t* const nmethod = code();

		if (nmethod == nullptr)
		{
			return;
		}

		nmethod->set_marked_for_decompilation(1);
		nmethod->set_state(nmethod_t::state_t::not_entrant);

		set_code(nullptr);
	}

protected:
	std::uint8_t pad_one_[8] = { };
	const_method_t* const_method_ = nullptr;
	std::uint8_t pad_two_[16] = { };
	std::int32_t access_flags_ = 0;
	std::int32_t vtable_index_ = 0;
	std::int32_t result_index_ = 0;
	std::uint16_t method_size_ = 0;
	std::uint8_t intrinsic_id_ = 0;
	std::uint8_t jfr_towrite_ : 1,
		caller_sensitive_ : 1,
		force_inline_ : 1,
		hidden_ : 1,
		dont_inline_ : 1,
		: 3;
	std::uint8_t* i2i_entry_ = nullptr;
	std::uint8_t pad_four_[8] = { };
	std::uint8_t* compiled_entry_ = nullptr;
	nmethod_t* code_ = nullptr;
	std::uint8_t* interpreted_entry_ = nullptr;
	std::uint8_t pad_five_[40] = { };
};

class argument_parser_t
{
public:
	template <class t>
	t get_argument(const std::uint64_t i)
	{
		return reinterpret_cast<t>(reinterpret_cast<void**>(this) + i);
	}
};
