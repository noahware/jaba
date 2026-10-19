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
}
