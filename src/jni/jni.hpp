#pragma once
#include <cstddef>
#include "clazz.hpp"
#include "field.hpp"
#include "method.hpp"
#include "../hook/pending.hpp"

namespace jaba::jni
{
	// the classes whose fields a class inherits, none is fine:
	//   JABA_CLASS_BEGIN(entity, "...")
	//   JABA_CLASS_BEGIN(living, "...", entity)
	//   JABA_CLASS_BEGIN(player, "...", living, other)
	template <class... Ts>
	struct bases
	{
		template <class Self>
		struct apply : Ts::template members<Self>... { };
	};
}

#define JABA_CLASS_BEGIN(name, mapping, ...) \
	class name; \
	template <class Self> \
	struct JABA_EMPTY_BASES name##_members : jaba::jni::bases<__VA_ARGS__>::apply<Self> \
	{ \
		using jaba_members = name##_members; \
		using jaba_self = Self; \
		using jaba_decl = name; \
		static constexpr jaba::fixed_string jaba_owner{ mapping };

// walks back from a member's address to the instance its members struct lives in
#define JABA_MEMBER_LOC(name, loc) \
		struct loc \
		{ \
			static const jaba::jni::object& owner(const void* const member) noexcept \
			{ \
				const auto* const members = reinterpret_cast<const jaba_members*>( \
					static_cast<const char*>(member) - offsetof(jaba_members, name)); \
				return static_cast<const jaba_self&>(*members); \
			} \
		};

#define JABA_FIELD(type, name, mapping) \
		JABA_MEMBER_LOC(name, name##_loc) \
		JABA_NO_UNIQUE_ADDRESS jaba::jni::field<type, jaba_self, mapping, name##_loc> name;

#define JABA_STATIC_FIELD(type, name, mapping) \
		inline static jaba::jni::static_field<type, jaba_self, mapping> name;

// argument types follow the mapping, none is fine:
//   JABA_METHOD(jint, size, "size")
//   JABA_METHOD(void, set_pos, "setPosition", jdouble, jdouble, jdouble)
#define JABA_METHOD(ret, name, mapping, ...) \
		JABA_MEMBER_LOC(name, name##_loc_m) \
		JABA_NO_UNIQUE_ADDRESS jaba::jni::method<jaba_self, mapping, name##_loc_m, ret __VA_OPT__(,) __VA_ARGS__> name;

#define JABA_STATIC_METHOD(ret, name, mapping, ...) \
		inline static jaba::jni::static_method<jaba_self, mapping, ret __VA_OPT__(,) __VA_ARGS__> name;

// argument types follow the name, none is fine, calling it gives back the new instance:
//   JABA_CONSTRUCTOR(create, jint, jint)
#define JABA_CONSTRUCTOR(name, ...) \
		JABA_STATIC_METHOD(jaba_self, name, "<init>" __VA_OPT__(,) __VA_ARGS__)

// hooks a method declared above it, the detour is a function declared before the class
// (qualify it if it's in another namespace):
//   JABA_METHOD(void, set_pos, "setPosition", jdouble, jdouble, jdouble)
//   JABA_HOOK(set_pos, on_set_pos)
#define JABA_HOOK(method, detour) \
		using method##_mhk_original = typename jaba::jni::method_in_t<decltype(method), jaba_decl>::sig_t*; \
		inline static jaba::jni::jni_hook<jaba::jni::method_in_t<decltype(method), jaba_decl>, &detour> method##_hook;

// the explicit instantiation constructs the members' inline statics (the hooks), which would
// otherwise never be instantiated and so never queued, it needs name to be complete
#define JABA_CLASS_END(name) \
	}; \
	class name : public jaba::jni::clazz<name##_members<name>::jaba_owner, name##_members<name>> \
	{ \
	public: \
		using clazz::clazz; \
		template <class Self> \
		using members = name##_members<Self>; \
		[[nodiscard]] name as_global() const noexcept\
		{\
			jaba::jni::object global = as_global_obj();\
			const ref_t ref = global.ref();\
			return name{ global.release(), ref };\
		}\
	}; \
	template struct name##_members<name>;
