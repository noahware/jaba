#pragma once
#include <util/fixed_string.hpp>
#include "mapping.hpp"

namespace jaba
{
	template <class T>
	class jni_wrap
	{
	public:
		using value_type = T;

		jni_wrap() noexcept = default;

		jni_wrap(const value_type obj)
			: obj_(obj) { }

		[[nodiscard]] operator value_type() const noexcept
		{
			return obj_;
		}

		[[nodiscard]] value_type obj() const noexcept
		{
			return obj_;
		}

	protected:
		value_type obj_;
	};

	template <fixed_string Mapping>
	class mapped
	{
	public:
		[[nodiscard]] static zstring_view mapping() noexcept
		{
			return fetch_mapping(Mapping.zview());
		}
	};
}
