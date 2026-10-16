#pragma once

namespace jaba
{
	template <class T>
	class jni_wrap
	{
	public:
		using value_type = T;

		jni_wrap() noexcept = default;

		jni_wrap(const value_type val)
			: val_(val) { }

	protected:
		value_type val_;
	};
}
