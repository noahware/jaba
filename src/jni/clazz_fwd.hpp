#pragma once
#include "../util/fixed_string.hpp"

namespace jaba
{
	template <fixed_string Mapping>
	class clazz;

	// satisfied by any clazz<M> specialization or type derived from one
	template <class T>
	concept class_type = requires(T & t)
	{
		[]<fixed_string M>(clazz<M>&) {}(t);
	};
}
