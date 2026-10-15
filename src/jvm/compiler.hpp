#pragma once
#include <cstdint>
#include <pe/image.hpp>

namespace jaba
{
	class abstract_interpreter 
	{
	public:
		static void init(const pe::image* const jvm)
		{
			
		}

		inline static std::uint8_t* slow_sig_handler = nullptr;
		inline static std::uint8_t* method_entry_table = nullptr;
	};
}
