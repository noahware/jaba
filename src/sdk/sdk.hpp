#pragma once

// the java standard library, as jaba classes. Java 8 and up: only what every version has is
// declared, the rest of a version's api can be declared the same way with JABA_CLASS_BEGIN
//
//   using namespace jaba;
//
//   string s = "hello";                                 // java.lang.String
//   util::array_list<string> list = util::array_list<string>::create(); // ArrayList<String>
//   list.add(s);
//
//   for (const string& str : list)                      // a java.lang.Iterable
//       LOG("{}", str.to_std());
//
// java.lang is inline (java imports it implicitly), other packages keep their name minus the
// java prefix: java.util is jaba::util. The names are the java ones in snake_case, and a
// trailing underscore where that's a c++ keyword (long_, float_, double_, class_)
#include "sdk/fwd.hpp"

#include "sdk/lang/core.hpp"
#include "sdk/lang/throwable.hpp"
#include "sdk/lang/boxed.hpp"
#include "sdk/io/print_stream.hpp"
#include "sdk/lang/system.hpp"

#include "sdk/util/collections.hpp"
#include "sdk/util/map.hpp"
#include "sdk/util/optional.hpp"
