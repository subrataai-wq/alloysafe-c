/* ==============================================================================
 * Alloysafe-C: Hardening Legacy C Code with an Ironclad Alloy of Rust Safety
 * ------------------------------------------------------------------------------
 * Copyright (c) 2026 Subrata Roy. All rights reserved.
 * SPDX-License-Identifier: MIT
 * ==============================================================================
 */

#ifndef ALLOYSAFE_TYPES_H
#define ALLOYSAFE_TYPES_H

#include "core.h"

/* =========================================================
 * IMMUTABILITY BY DEFAULT & TYPE INTROSPECTION
 * ========================================================= */
#define let const __auto_type
#define mut __auto_type

// Type signedness query (evaluates to true for signed integers, false for unsigned)
#define IS_SIGNED_TYPE(val) (((__typeof__(val))-1) < (__typeof__(val))0)

#endif // ALLOYSAFE_TYPES_H
