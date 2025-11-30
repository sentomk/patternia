#pragma once
/**
 * @file type.hpp
 * @brief Public API for type-based patterns in Patternia.
 *
 * This file acts as a facade, including all individual type pattern
 * features. Users can include this file to get access to all type patterns.
 */

// Include all feature files
#include "type/is.hpp"
#include "type/in.hpp"
#include "type/not_in.hpp"
#include "type/from.hpp"

// Legacy alias can stay here, as it's a high-level concept.
namespace ptn::pat::type {
  template <typename T>
  using type_tag = detail::is_pattern<T>;
}
