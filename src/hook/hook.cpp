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
		method* native_method_ptr; // here for jmethodID to reference it
		jvm_type_copy<method> native_method;
		void* orig_interp_entry;
		void* orig_i2i_entry;
		method::access_flags orig_acc_flags;
		std::uint8_t orig_flags;
	};

	// key is original method
	std::shared_mutex hooks_mtx;
	std::unordered_map<method*, std::unique_ptr<hook>> hooks;

	// unhook never frees a hook: a thread can already be inside jaba_hk_stub with
	// native_method_ptr loaded, past the point jaba_hook_c_handler takes hooks_mtx, so there is
	// no safe point at which to reclaim it. retired hooks are kept alive for the process
	// lifetime instead of risking a use-after-free on that in-flight call
	std::vector<std::unique_ptr<hook>> retired_hooks;

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
		const std::uint8_t orig_flags = orig_method->flags();

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

		auto hk = std::make_unique<hook>(hook{
			.native_method_ptr = native_method.data(),
			.native_method = std::move(native_method),
			.orig_interp_entry = orig_method->from_interp_entry(),
			.orig_i2i_entry = orig_method->i2i_entry(),
			.orig_acc_flags = orig_acc_flags,
			.orig_flags = orig_flags
		});

		// heap-allocated and never freed (see retired_hooks), so this address outlives unhook
		auto& stored = *hooks.emplace(orig_method, std::move(hk)).first->second;

		orig_method->set_from_interp_entry(reinterpret_cast<void*>(jaba_hk_stub));
		orig_method->set_i2i_entry(reinterpret_cast<void*>(jaba_hk_stub));

		return reinterpret_cast<jmethodID>(&stored.native_method_ptr);
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

	std::unique_lock lock(hooks_mtx);

	const auto it = hooks.find(orig_method);

	if (it == hooks.end())
		return false;

	auto& hk = *it->second;

	orig_method->set_from_interp_entry(hk.orig_interp_entry);
	orig_method->set_i2i_entry(hk.orig_i2i_entry);
	orig_method->set_acc_flags(hk.orig_acc_flags);
	orig_method->set_flags(hk.orig_flags);

	// move out rather than erase-and-destroy: a thread can still be mid-flight through
	// jaba_hk_stub using native_method, so the hook outlives its entry in "hooks"
	retired_hooks.push_back(std::move(it->second));
	hooks.erase(it);

	return true;
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
