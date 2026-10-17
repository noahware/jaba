#pragma once
#include <type_traits>
#include <jni/jni.h>
#include "../util/fixed_string.hpp"
#include "object.hpp"
#include "wrap.hpp"

#if defined(_MSC_VER)
#define JABA_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
#define JABA_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif

namespace jaba
{
	struct static_loc { };

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

	template <class ValT, class ClassT, fixed_string Mapping>
	using static_method = method<ValT, ClassT, Mapping, static_loc>;
}

#include "method.inl"
