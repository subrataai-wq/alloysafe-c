/* ==============================================================================
 * Alloysafe-C: Hardening Legacy C Code with an Ironclad Alloy of Rust Safety
 * ------------------------------------------------------------------------------
 * Copyright (c) 2026 Subrata Roy. All rights reserved.
 * SPDX-License-Identifier: MIT
 * ==============================================================================
 */

#ifndef ALLOYSAFE_MATH_H
#define ALLOYSAFE_MATH_H

#include "types.h"

/* =========================================================
 * CHECKED INTEGER ARITHMETIC (Single-Eval & Overflow Safe)
 * ========================================================= */
#define CHECKED_ADD(a, b) ({                 \
  let _a = (a);                              \
  let _b = (b);                              \
  mut _res = _a;                             \
  if (__builtin_add_overflow(_a, _b, &_res)) \
  {                                          \
    PANIC("Integer addition overflow!");     \
  }                                          \
  _res;                                      \
})

#define CHECKED_SUB(a, b) ({                 \
  let _a = (a);                              \
  let _b = (b);                              \
  mut _res = _a;                             \
  if (__builtin_sub_overflow(_a, _b, &_res)) \
  {                                          \
    PANIC("Integer subtraction overflow!");  \
  }                                          \
  _res;                                      \
})

#define CHECKED_MUL(a, b) ({                   \
  let _a = (a);                                \
  let _b = (b);                                \
  mut _res = _a;                               \
  if (__builtin_mul_overflow(_a, _b, &_res))   \
  {                                            \
    PANIC("Integer multiplication overflow!"); \
  }                                            \
  _res;                                        \
})

#define CHECKED_DIV(a, b) ({                               \
  let _a = (a);                                            \
  let _b = (b);                                            \
  if (_b == 0)                                             \
  {                                                        \
    PANIC("Division by zero!");                            \
  }                                                        \
  mut _test = _a;                                          \
  if (IS_SIGNED_TYPE(_a) && IS_SIGNED_TYPE(_b) &&          \
      _b == -1 && __builtin_mul_overflow(_a, _b, &_test))  \
  {                                                        \
    PANIC("Signed integer division overflow (MIN / -1)!"); \
  }                                                        \
  _a / _b;                                                 \
})

#endif // ALLOYSAFE_MATH_H
