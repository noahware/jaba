#include "hook.hpp"
#include "jvm/compiler.hpp"
#include "jvm/method.hpp"
#include <unordered_map>

using namespace jaba;

extern "C" void jaba_hk_stub();

namespace
{
	struct hook
	{
		jvm_type_copy<method> native_method;
		void* orig_interp_entry;
		void* orig_i2i_entry;
	};

	jvm_type_copy<method> make_native_handler(const method* const m, const method_detour_t detour)
	{
		jvm_type_copy<method> native_method = m->copy<method>();

		native_method->make_native();
		native_method->set_vtable_idx(-2);

		auto& buf = native_method.buf();

		const auto detour_ref = reinterpret_cast<const std::uint8_t*>(&detour);
		const auto ssh_ref = reinterpret_cast<const std::uint8_t*>(&abstract_interpreter::slow_sig_handler);

		buf.insert(buf.end(), detour_ref, detour_ref + sizeof(std::uintptr_t));
		buf.insert(buf.end(), ssh_ref, ssh_ref + sizeof(std::uintptr_t));

		return native_method;
	}

	// key is original method
	std::unordered_map<method*, hook> hooks;
}

bool jaba::hook_method(const jmethodID jmethod, const method_detour_t detour)
{
	method* const orig_method = method::from(jmethod);

	if (hooks.contains(orig_method))
		return false;

	jvm_type_copy<method> native_method = make_native_handler(orig_method, detour);

	hook hk = {
		.native_method = std::move(native_method),
		.orig_interp_entry = orig_method->from_interp_entry(),
		.orig_i2i_entry = orig_method->i2i_entry()
	};

	hooks[orig_method] = std::move(hk);

	orig_method->set_from_interp_entry(jaba_hk_stub);
	orig_method->set_i2i_entry(jaba_hk_stub);

	return true;
}

bool jaba::unhook_method(const jmethodID jmethod)
{
	method* const orig_method = method::from(jmethod);

	const auto it = hooks.find(orig_method);

	if (it == hooks.end())
		return false;

	const auto& hk = it->second;

	orig_method->set_from_interp_entry(hk.orig_interp_entry);
	orig_method->set_i2i_entry(hk.orig_i2i_entry);

	hooks.erase(it);

	return true;
}

extern "C" void jaba_hook_c_handler(method** method)
{
	const auto it = hooks.find(*method);

	if (it != hooks.end())
	{
		auto& hk = it->second;

		*method = hk.native_method.data();
	}
}
