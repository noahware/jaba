#pragma once
#include "../util/scan.hpp"

namespace jaba
{
	class abstract_interpreter
	{
	public:
		static bool init(const pe::image* const jvm)
		{
			constexpr std::size_t native_method_kind = 2;

			const auto slow_sig_slot = util::scan_rel<std::uint8_t**>(jvm, "48 8B 15 ? ? ? ? E9 ? ? ? ? 48 8B 15 ? ? ? ? E8", 3, 7);
			const auto method_entry_table = util::scan_rel<std::uint8_t**>(jvm, "48 8D 0D ? ? ? ? 48 89 14 C1", 3, 7);

			if (!slow_sig_slot || !method_entry_table)
				return false;

			slow_sig_handler = *slow_sig_slot;
			native_entry = method_entry_table[native_method_kind];

			return slow_sig_handler && native_entry;
		}

		inline static std::uint8_t* slow_sig_handler = nullptr;
		static std::uint8_t* native_entry;
	};
}
