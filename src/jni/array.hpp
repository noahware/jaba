#pragma once
#include "object.hpp"
#include "env.hpp"

namespace jaba
{
	template <class T>
	class array : public object
	{
	public:
		using object::object;

		[[nodiscard]] std::size_t size() const noexcept
		{
			return jni::env()->GetArrayLength(native());
		}

	protected:
		[[nodiscard]] jarray native() const noexcept
		{
			return reinterpret_cast<jarray>(obj_);
		}
	};
}
