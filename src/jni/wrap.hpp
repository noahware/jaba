#pragma once

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

		[[nodiscard]] value_type obj() const noexcept
		{
			return obj_;
		}

	protected:
		value_type obj_;
	};
}
