#include "clazz.hpp"

using namespace jaba;

namespace
{
	clazz default_find_class(const std::string_view name)
	{
		return {};
	}
}

find_class_fn jaba::find_class = default_find_class;

void jaba::set_find_class_fn(const find_class_fn& fn)
{
	find_class = fn;
}
