#pragma once
#include <jni/jni.h>
#include "wrap.hpp"

namespace jaba
{
	class object : public jni_wrap<jobject>
	{
	public:
		using jni_wrap::jni_wrap;
	};

	class obj_user
	{
	public:
		obj_user() noexcept = default;
		obj_user(object obj) noexcept
			:	obj_(std::move(obj)) { }

		[[nodiscard]] object obj() const noexcept
		{
			return obj_;
		}

	protected:
		object obj_;
	};
}
