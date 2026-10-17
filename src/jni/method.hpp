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
	class method : public mapped<Mapping>
	{
	public:
		static constexpr bool is_static = std::is_same_v<Loc, static_loc>;

		using mapped<Mapping>::mapping;
		
		[[nodiscard]] static jmethodID native();

	private:
		[[nodiscard]] jobject self() const noexcept
		{
			return Loc::owner(this).obj();
		}
	};

	template <class ClassT, fixed_string Mapping, class RetT, class... ArgsT>
	using static_method = method<ClassT, Mapping, static_loc, RetT, ArgsT...>;
}

#include "method.inl"
