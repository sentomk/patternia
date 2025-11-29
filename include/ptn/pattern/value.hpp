#pragma once

/**
 * @file value.hpp
 * @brief Aggregated public API for all value patterns.
 *
 * This header provides a single entry point to all value pattern factories
 * (`lit`, `pred`, `lt`, `eq`, etc.), simplifying user includes.
 *
 * @namespace ptn::pat
 */

#include "ptn/pattern/value/literal.hpp"
#include "ptn/pattern/value/predicate.hpp"
#include "ptn/pattern/value/relational.hpp"
