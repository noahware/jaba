#pragma once
#include <type_traits>
#include <jni/jni.h>
#include "../util/fixed_string.hpp"
#include "member.hpp"
#include "object.hpp"
#include "wrap.hpp"

namespace jaba
{
	namespace detail
	{
		// ValT as it arrives in a native method: primitives as they are, clazz types as a jobject
		template <class T>
		struct raw { using type = jobject; };

		template <class T> requires (std::is_arithmetic_v<T> || std::is_void_v<T>)
		struct raw<T> { using type = T; };

		template <class T>
		using raw_t = typename raw<T>::type;
	}

	template <class ClassT, fixed_string Mapping, class Loc, class RetT, class... ArgsT>
	class method : public clazz_member<ClassT, Mapping, Loc>
	{
		using member = clazz_member<ClassT, Mapping, Loc>;

	public:
		using member::is_static;
		using member::mapping;

		using class_t = ClassT;
		using ret_t = RetT;

		// the method itself, what a detour calls to run the real one: self comes first unless static
		using sig_t = std::conditional_t<is_static, RetT(const ArgsT&...), RetT(const ClassT&, const ArgsT&...)>;

		// the signature a hook of this method has to have, the original goes after self
		using detour_t = std::conditional_t<is_static,
			RetT (*)(sig_t*, const ArgsT&...),
			RetT (*)(const ClassT&, sig_t*, const ArgsT&...)>;

		[[nodiscard]] static jmethodID native();

		RetT operator()(const ArgsT&... args) const;

		// calls id on target (the instance, or the class when static), throws if java does
		static RetT invoke(JNIEnv* env, jmethodID id, jobject target, const ArgsT&... args);

	private:
		using member::cls;
		using member::self;

		static constexpr bool is_constructor() noexcept
		{
			return Mapping.view() == "<init>";
		}

		// what jaba_hk_stub calls: the raw native entry, it wraps everything up for Detour.
		// orig_id is what hook_method returned, and call_original runs it
		template <bool Static, detour_t Detour>
		struct hooked;

		template <detour_t Detour>
		struct hooked<false, Detour>
		{
			static inline jmethodID orig_id = nullptr;

			static RetT call_original(const ClassT& self, const ArgsT&... args)
			{
				return invoke(jni::env(), orig_id, self.obj(), args...);
			}

			static detail::raw_t<RetT> entry(JNIEnv* env, jobject self, detail::raw_t<ArgsT>... args)
			{
				// self and the args are the jvm frame's refs, so the objects get copies of their own
				const ClassT self_obj{ env->NewLocalRef(self) };

				if constexpr (std::is_void_v<RetT>)
					Detour(self_obj, &call_original, detail::wrap_copy<ArgsT>(args)...);
				else
					return detail::unwrap_ret(Detour(self_obj, &call_original, detail::wrap_copy<ArgsT>(args)...));
			}
		};

		template <detour_t Detour>
		struct hooked<true, Detour>
		{
			static inline jmethodID orig_id = nullptr;

			static RetT call_original(const ArgsT&... args)
			{
				return invoke(jni::env(), orig_id, ClassT::native(), args...);
			}

			static detail::raw_t<RetT> entry(JNIEnv*, jobject, detail::raw_t<ArgsT>... args)
			{
				if constexpr (std::is_void_v<RetT>)
					Detour(&call_original, detail::wrap_copy<ArgsT>(args)...);
				else
					return detail::unwrap_ret(Detour(&call_original, detail::wrap_copy<ArgsT>(args)...));
			}
		};

	public:
		template <detour_t Detour>
		using hooked_t = hooked<is_static, Detour>;
	};

	// the same method looked up on another class. Members are templated on the most derived
	// class, a hook has to be of the class that declares the method and not of whoever inherits it
	template <class M, class NewClassT>
	struct method_in;

	template <class ClassT, fixed_string Mapping, class Loc, class RetT, class... ArgsT, class NewClassT>
	struct method_in<method<ClassT, Mapping, Loc, RetT, ArgsT...>, NewClassT>
	{
		using type = method<NewClassT, Mapping, Loc, RetT, ArgsT...>;
	};

	template <class M, class NewClassT>
	using method_in_t = typename method_in<M, NewClassT>::type;

	template <class ClassT, fixed_string Mapping, class RetT, class... ArgsT>
	using static_method = method<ClassT, Mapping, static_loc, RetT, ArgsT...>;
}

#include "method.inl"
