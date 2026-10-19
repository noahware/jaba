#pragma once
#include <cstdint>
#include <cstring>
#include <format>
#include <string>
#include <string_view>
#include <jni/jni.h>
#include "jni/env.hpp"
#include "util/image.hpp"

// deoptimizes every compiled method through the whitebox native the jvm already ships
// the natives are never registered without -XX:+WhiteBoxAPI, but the JNINativeMethod table is always in the binary
namespace jaba::jvm::deopt
{
	namespace detail
	{
		using run_t = void(*)(JNIEnv*, jobject);

		inline run_t run_fn = nullptr;

		[[nodiscard]] inline const std::uint8_t* scan(const jaba::mem::image* const jvm, const std::string_view bytes)
		{
			std::string sig;

			for (const char c : bytes)
				sig += std::format("{:02X} ", static_cast<std::uint8_t>(c));

			return jvm->sig_scan(sig, false, false);
		}

		// the name needs its terminator so deoptimizeMethod does not match deoptimizeMethod0
		[[nodiscard]] inline void* find_native(const jaba::mem::image* const jvm, const std::string_view name, const char* const sig)
		{
			const std::uint8_t* const str = scan(jvm, name);

			if (!str)
				return nullptr;

			const auto str_addr = reinterpret_cast<std::uintptr_t>(str);
			const std::uint8_t* const entry = scan(jvm, { reinterpret_cast<const char*>(&str_addr), sizeof(str_addr) });

			if (!entry || std::strcmp(*reinterpret_cast<const char* const*>(entry + sizeof(void*)), sig))
				return nullptr;

			return *reinterpret_cast<void* const*>(entry + 2 * sizeof(void*));
		}
	}

	[[nodiscard]] inline bool init(JNIEnv* const env, const jaba::mem::image* const jvm)
	{
		detail::run_fn = reinterpret_cast<detail::run_t>(detail::find_native(jvm, std::string_view("deoptimizeAll\0", 14), "()V"));

		return detail::run_fn;
	}

	// blocks until the vm operation is done, so never call it while holding a lock a java thread can wait on
	inline void run()
	{
		JNIEnv* const env = jni::env();

		if (!env)
			return;

		detail::run_fn(env, nullptr);
	}
}
