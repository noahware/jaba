#include "hook.hpp"
#include <unordered_map>

#include "jvm/method.hpp"

using namespace jaba;

namespace
{
	/*struct hook
	{
		std::unordered_map<method_t*>
	};*/

	jvm_type_copy<method> make_native_handler(const method* const m, const method_detour_t detour)
	{
		jvm_type_copy<method> native_method = m->copy<method>();

		native_method->make_native();
		native_method->set_vtable_idx(-2);

		auto& buf = native_method.buf();

		const auto detour_ref = reinterpret_cast<const std::uint8_t*>(&detour);
		const auto ssh_ref = reinterpret_cast<const std::uint8_t*>(&detour);

		buf.insert(buf.end(), detour_ref, detour_ref + sizeof(std::uintptr_t));
		buf.insert(buf.end(), ssh_ref, ssh_ref + sizeof(std::uintptr_t));

		return native_method;
	}
}

void jaba::hook_method(const jmethodID jmethod, const method_detour_t detour)
{
	method* const orig_method = method::from(jmethod);


}

void jaba::unhook_method(jmethodID jmethod)
{

}
