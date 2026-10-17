#pragma once
#include <type_traits>
#include <jni/jni.h>
#include "../util/fixed_string.hpp"
#include "member.hpp"
#include "object.hpp"
#include "wrap.hpp"

namespace jaba
{
	template <class ClassT, fixed_string Mapping, class Loc, class RetT, class... ArgsT>
	class method : public clazz_member<ClassT, Mapping, Loc>
	{
		using member = clazz_member<ClassT, Mapping, Loc>;

	public:
		using member::is_static;
		using member::mapping;

		[[nodiscard]] static jmethodID native();

		RetT operator()(ArgsT... args) const;

	private:
		using member::cls;
		using member::self;
	};

	template <class ClassT, fixed_string Mapping, class RetT, class... ArgsT>
	using static_method = method<ClassT, Mapping, static_loc, RetT, ArgsT...>;
}

#include "method.inl"
