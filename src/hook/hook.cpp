#include "hook.hpp"
#include "jvm/compiler.hpp"
#include "jvm/deopt.hpp"
#include "jvm/method.hpp"
#include <unordered_map>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <vector>
#include <memory>

using namespace jaba;
using namespace jaba::jvm;

extern "C" void jaba_hk_stub();

namespace
{
	using recompile_method_fn = std::function<bool(method*)>;

	struct hook
	{
		jvm_type_copy<method> native_method;
		method* callable_method_ptr; // jmethodID of the callable original references this
		jvm_type_copy<method> callable_method;
		void* orig_interp_entry;
		void* orig_i2i_entry;
		method::access_flags orig_acc_flags;
		std::uint32_t orig_flags;
	};

	// key is original method
	std::shared_mutex hooks_mtx;
	std::unordered_map<method*, std::unique_ptr<hook>> hooks;

	// unhook never frees a hook: a thread can already be inside jaba_hk_stub with
	// native_method loaded into rbx, past the point jaba_hook_c_handler takes hooks_mtx, so there is
	// no safe point at which to reclaim it. retired hooks are kept alive for the process
	// lifetime instead of risking a use-after-free on that in-flight call.
	// the vector itself is heap-allocated and intentionally leaked: a namespace-scope vector would be destroyed
	// at DLL unload and free every hook buffer while the jvm (nmethods, jmethodIDs) still points at them
	auto& retired_hooks = *new std::vector<std::unique_ptr<hook>>;

	jvm_type_copy<method> make_native_handler(const method* const m, const method_detour_t detour)
	{
		jvm_type_copy<method> native_method = m->copy<method>();

		native_method->make_native();
		native_method->set_vtable_idx(-2);

		auto& buf = native_method.buf();

		const auto detour_ref = reinterpret_cast<const std::uint8_t*>(&detour);

		buf.insert(buf.end(), detour_ref, detour_ref + sizeof(std::uintptr_t));
		buf.insert(buf.end(), sizeof(std::uintptr_t), 0);

		return native_method;
	}

	// the original the user calls: a copy that keeps the method's own compile flags, so the jit is free to compile it.
	// the hooked method itself has to stay interpreted, its entries are the stub
	jvm_type_copy<method> make_callable_original(const method* const m, const method::access_flags acc_flags,
	                                             const std::uint32_t flags)
	{
		jvm_type_copy<method> callable_method = m->copy<method>();

		callable_method->set_acc_flags(acc_flags);
		callable_method->set_flag_bits(flags);

		// the copy is not in any vtable, so it has to be invoked directly or a virtual call lands back in the hook
		callable_method->set_vtable_idx(-2);

		return callable_method;
	}

	std::optional<jmethodID> raw_hook_method(const jmethodID jmethod, const method_detour_t detour,
	                                         const recompile_method_fn& recompile_method)
	{
		method* orig_method = method::from(jmethod);

		{
			std::shared_lock lock(hooks_mtx);

			if (hooks.contains(orig_method))
				return { };
		}

		// snapshot the flags before recompile_method can touch them
		const method::access_flags orig_acc_flags = orig_method->acc_flags();
		const std::uint32_t orig_flags = orig_method->flag_bits();

		if (!recompile_method(orig_method))
			return { };

		// the recompile can replace the method object, so the old pointer is stale
		orig_method = method::from(jmethod);

		orig_method->make_no_inline();
		orig_method->make_interpreted();

		std::unique_lock lock(hooks_mtx);

		// under the lock
		if (hooks.contains(orig_method))
			return { };

		jvm_type_copy<method> native_method = make_native_handler(orig_method, detour);
		jvm_type_copy<method> callable_method = make_callable_original(orig_method, orig_acc_flags, orig_flags);

		auto hk = std::make_unique<hook>(hook{
			.native_method = std::move(native_method),
			.callable_method_ptr = callable_method.data(),
			.callable_method = std::move(callable_method),
			.orig_interp_entry = orig_method->from_interp_entry(),
			.orig_i2i_entry = orig_method->i2i_entry(),
			.orig_acc_flags = orig_acc_flags,
			.orig_flags = orig_flags
		});

		// heap-allocated and never freed (see retired_hooks), so this address outlives unhook
		auto& stored = *hooks.emplace(orig_method, std::move(hk)).first->second;

		orig_method->set_from_interp_entry(reinterpret_cast<void*>(jaba_hk_stub));
		orig_method->set_i2i_entry(reinterpret_cast<void*>(jaba_hk_stub));

		return reinterpret_cast<jmethodID>(&stored.callable_method_ptr);
	}

	// hooks_mtx has to be held exclusively
	bool unhook_locked(method* const orig_method)
	{
		const auto it = hooks.find(orig_method);

		if (it == hooks.end())
			return false;

		auto& hk = *it->second;

		orig_method->set_from_interp_entry(hk.orig_interp_entry);
		orig_method->set_i2i_entry(hk.orig_i2i_entry);
		orig_method->set_acc_flags(hk.orig_acc_flags);
		orig_method->set_flag_bits(hk.orig_flags);

		// move out rather than erase-and-destroy: a thread can still be mid-flight through
		// jaba_hk_stub using native_method, so the hook outlives its entry in "hooks"
		retired_hooks.push_back(std::move(it->second));
		hooks.erase(it);

		return true;
	}

	bool raw_unhook_method(method* const orig_method)
	{
		std::unique_lock lock(hooks_mtx);

		return unhook_locked(orig_method);
	}
}

std::optional<jmethodID> jaba::hook_method(const jmethodID jmethod, const method_detour_t detour)
{
	return raw_hook_method(jmethod, detour,
		[](method* const m) -> bool
		{
			m->make_no_inline();
			m->make_interpreted();

			if (m->code())
				deopt::run();

			return true;
		}
	);
}

bool jaba::unhook_method(const jmethodID jmethod)
{
	method* const orig_method = method::from(jmethod);

	return raw_unhook_method(orig_method);
}

void jaba::remove_all_hooks()
{
	std::unique_lock lock(hooks_mtx);

	// unhooking erases the entry, so take the first one each time instead of iterating
	while (!hooks.empty())
		unhook_locked(hooks.begin()->first);
}

extern "C" void jaba_hook_c_handler(method** method)
{
	std::shared_lock lock(hooks_mtx);

	const auto it = hooks.find(*method);

	if (it != hooks.end())
	{
		auto& hk = *it->second;

		*method = hk.native_method.data();
	}
}
