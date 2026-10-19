#pragma once
#include <type_traits>
#include <jni/jni.h>
#include "../util/fixed_string.hpp"
#include "object.hpp"
#include "member.hpp"
#include "wrap.hpp"

namespace jaba::jni
{
	template <class ValT, class ClassT, fixed_string Mapping, class Loc>
	class field : public clazz_member<ClassT, Mapping, Loc>
	{
		using member = clazz_member<ClassT, Mapping, Loc>;

	public:
		using member::is_static;
		using member::mapping;

		[[nodiscard]] static jfieldID native();

		[[nodiscard]] ValT get() const;
		void set(const ValT& val);

		[[nodiscard]] operator ValT() const
		{
			return get();
		}

		field& operator=(const ValT& val)
		{
			set(val);
			return *this;
		}

		struct val_holder
		{
			ValT val;

			[[nodiscard]] ValT* operator->() noexcept
			{
				return &val;
			}
		};

		[[nodiscard]] val_holder operator->() const
		{
			return { get() };
		}

	private:
		using member::cls;
		using member::self;
	};

	template <class ValT, class ClassT, fixed_string Mapping>
	using static_field = field<ValT, ClassT, Mapping, static_loc>;
}

#include "field.inl"
