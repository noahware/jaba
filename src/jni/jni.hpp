#pragma once
#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>
#include "array.hpp"
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

	// Members has a static jaba_make(A) that builds its own class (see JABA_CLASS_END). A concept
	// and not an inline requires, a member that doesn't exist is only a soft failure when
	// the class it's looked up in is dependent.
	// Never for a jobject or an object, they convert to jobject and that goes on to bool,
	// which would silently turn one into a boxed boolean. Those use the upcast instead
	template <class Members, class A>
	concept makes_from = !std::is_convertible_v<A, jobject> && requires(A&& arg)
	{
		{ Members::jaba_make(std::forward<A>(arg)) } -> std::same_as<typename Members::jaba_self>;
	};

	template <class Members, class A>
	[[nodiscard]] typename Members::jaba_self make_from(A&& arg)
	{
		return Members::jaba_make(std::forward<A>(arg));
	}
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

// argument types follow the name, none is fine, calling it gives back the new instance. Like
// in java, constructors overload on their arguments and all share one name:
//   JABA_CONSTRUCTOR(create)
//   JABA_CONSTRUCTOR(create, jint, jint)
//   JABA_CONSTRUCTOR(create, string)
// Each one is a hidden constructor behind an overload of name that only takes what it can be
// called with. (__COUNTER__ for the hidden one, each in its own macro: pasting a macro
// argument keeps it from being expanded first)
#define JABA_CONSTRUCTOR(name, ...) \
		JABA_CONSTRUCTOR_ID(name, __COUNTER__ __VA_OPT__(,) __VA_ARGS__)

#define JABA_CONSTRUCTOR_ID(name, id, ...) \
		JABA_CONSTRUCTOR_IMPL(name, id __VA_OPT__(,) __VA_ARGS__)

#define JABA_CONSTRUCTOR_IMPL(name, id, ...) \
		JABA_STATIC_METHOD(jaba_self, jaba_ctor_##id, "<init>" __VA_OPT__(,) __VA_ARGS__) \
		template <class... A> \
			requires std::is_invocable_v<decltype(jaba_ctor_##id)&, A&&...> \
		static jaba_self name(A&&... args) \
		{ \
			return jaba_ctor_##id(std::forward<A>(args)...); \
		}

// hooks a method declared above it, the detour is a function declared before the class
// (qualify it if it's in another namespace):
//   JABA_METHOD(void, set_pos, "setPosition", jdouble, jdouble, jdouble)
//   JABA_HOOK(set_pos, on_set_pos)
#define JABA_HOOK(method, detour) \
		using method##_mhk_original = typename jaba::jni::method_in_t<decltype(method), jaba_decl>::sig_t*; \
		inline static jaba::jni::jni_hook<jaba::jni::method_in_t<decltype(method), jaba_decl>, &detour> method##_hook;

// JABA_HOOK from outside the class, for a class that can't be edited (the sdk generator's). cls
// is the class that declares the method, and it goes at namespace scope:
//   void on_jump(const player& self, JABA_HOOK_ORIGINAL(player, jump) original);
//   JABA_HOOK_EX(player, jump, on_jump)
// JABA_HOOK_ORIGINAL is the type of the original, what <method>_mhk_original is in the class.
// The hook is a variable of this file (static), named by __COUNTER__ since a class has many
#define JABA_HOOK_ORIGINAL(cls, method) 	jaba::jni::method_in_t<decltype(cls::method), cls>::sig_t*

#define JABA_HOOK_EX(cls, method, detour) 	JABA_HOOK_EX_ID(cls, method, detour, __COUNTER__)

#define JABA_HOOK_EX_ID(cls, method, detour, id) 	JABA_HOOK_EX_IMPL(cls, method, detour, id)

#define JABA_HOOK_EX_IMPL(cls, method, detour, id) 	[[maybe_unused]] static jaba::jni::jni_hook<jaba::jni::method_in_t<decltype(cls::method), cls>, &detour> jaba_hook_##id;

// On top of the constructors it inherits, the class gets two conversions:
//  - from whatever its members build through a static jaba_make(arg) that returns the class
//    itself, e.g. a string from a literal. Members may define it, none of them have to
//  - to every class whose members it inherits (an upcast, always valid): the class only mixes
//    those members in, it is not derived from the class itself
//
// The explicit instantiation constructs the members' inline statics (the hooks), which would
// otherwise never be instantiated and so never queued, it needs name to be complete
#define JABA_CLASS_END(name) \
	}; \
	class name : public jaba::jni::clazz<name##_members<name>::jaba_owner, name##_members<name>> \
	{ \
	public: \
		using clazz::clazz; \
		name() = default; \
		template <class A> \
			requires jaba::jni::makes_from<name##_members<name>, A> \
		name(A&& arg) : name(jaba::jni::make_from<name##_members<name>>(std::forward<A>(arg))) { } \
		template <std::derived_from<jaba::jni::object> T> \
			requires (!std::is_same_v<T, name> && std::is_base_of_v<typename T::template members<name>, name>) \
		[[nodiscard]] operator T() const { return as<T>(); } \
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
