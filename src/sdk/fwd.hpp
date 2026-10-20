#pragma once

// every sdk class is declared up front, their signatures refer to each other
//
// java.lang is an inline namespace (java imports it implicitly), so jaba::string is
// jaba::lang::string. Other packages keep their name, minus the java prefix: java.util is
// jaba::util
namespace jaba
{
	inline namespace lang
	{
		class object;
		class class_;
		class char_sequence;
		class string;

		class throwable;
		class exception;
		class runtime_exception;
		class illegal_argument_exception;
		class number_format_exception;
		class illegal_state_exception;
		class null_pointer_exception;
		class index_out_of_bounds_exception;
		class unsupported_operation_exception;

		class number;
		class integer;
		class long_;
		class float_;
		class double_;
		class boolean;
		class character;

		class system;
	}

	namespace io
	{
		class print_stream;
	}

	namespace util
	{
		// the classes as java declares them, the elements are all object. The generic ones are
		// below and wrap these
		namespace raw
		{
			class iterator;
			class iterable;
			class collection;
			class list;
			class array_list;
			class set;
			class hash_set;
			class map;
			class map_entry;
			class hash_map;
			class optional;
		}

		namespace detail
		{
			template <class T, class Raw>
			class iterable_api;

			template <class T, class Raw>
			class collection_api;

			template <class T, class Raw>
			class list_api;

			template <class K, class V, class Raw>
			class map_api;
		}

		// the type arguments default to object, which is what a java raw type is: list<>
		template <class T = object>
		class iterator;

		template <class T = object>
		using iterable = detail::iterable_api<T, raw::iterable>;

		template <class T = object>
		using collection = detail::collection_api<T, raw::collection>;

		template <class T = object>
		using list = detail::list_api<T, raw::list>;

		template <class T = object>
		class array_list;

		template <class T = object>
		using set = detail::collection_api<T, raw::set>;

		template <class T = object>
		class hash_set;

		template <class K = object, class V = object>
		using map = detail::map_api<K, V, raw::map>;

		template <class K = object, class V = object>
		class map_entry;

		template <class K = object, class V = object>
		class hash_map;

		template <class T = object>
		class optional;
	}
}
