#pragma once
/**
 * @file meta.hpp
 * @brief Core compile-time utilities forming the foundation of the
 *        Patternia meta layer (type-list, indexing, folds, traits).
 *
 * This header is NOT part of the Pattern DSL. It is internal meta
 * infrastructure used by type-patterns, variant-patterns, and template
 * introspection layers.
 *
 * It aggregates the main meta components:
 *   - type_list.hpp       : type_list and basic list combinators
 *   - index.hpp           : contains<T>, index_of<T>
 *   - algorithms.hpp      : all/any/none, map, filter
 *   - traits.hpp          : remove_cvref_t, is_spec_of, is_tmpl
 *   - template_info.hpp   : template_info<T>
 */

#include "ptn/meta/type_list.hpp"
#include "ptn/meta/index.hpp"
#include "ptn/meta/algorithms.hpp"
#include "ptn/meta/traits.hpp"
#include "ptn/meta/template_info.hpp"
