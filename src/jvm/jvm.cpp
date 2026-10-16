#include "version.hpp"
#include "compiler.hpp"
#include "method.hpp"

std::uint8_t* jaba::jvm::abstract_interpreter::native_entry = nullptr;

bool jaba::jvm::pre_jdk9()
{
	static auto marked_for_deopt_off = nmethod::field_offset_opt("_marked_for_deoptimization");

	return marked_for_deopt_off.has_value();
}
