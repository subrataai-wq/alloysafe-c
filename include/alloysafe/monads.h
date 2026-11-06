/* ==============================================================================
 * Alloysafe-C: Hardening Legacy C Code with an Ironclad Alloy of Rust Safety
 * ------------------------------------------------------------------------------
 * Copyright (c) 2026 Subrata Roy. All rights reserved.
 * SPDX-License-Identifier: MIT
 * ==============================================================================
 */

#ifndef ALLOYSAFE_MONADS_H
#define ALLOYSAFE_MONADS_H

#include "core.h"

/* =========================================================
 * GENERIC OPTION<T>, RESULT<T, E> & TRY (?) OPERATOR
 * ========================================================= */
#define DEFINE_OPTION(T, Name)                                       \
  typedef struct                                                     \
  {                                                                  \
    bool is_some;                                                    \
    T value;                                                         \
  } Name;                                                            \
                                                                     \
  MUST_USE static inline Name Name##_Some(T v)                       \
  {                                                                  \
    return (Name){.is_some = true, .value = v};                      \
  }                                                                  \
                                                                     \
  MUST_USE static inline Name Name##_None(void)                      \
  {                                                                  \
    Name _none = {.is_some = false};                                 \
    return _none;                                                    \
  }                                                                  \
                                                                     \
  MUST_USE static inline T Name##_unwrap(Name opt)                   \
  {                                                                  \
    if (!opt.is_some)                                                \
    {                                                                \
      PANIC("Called " #Name "_unwrap() on a None value!");           \
    }                                                                \
    return opt.value;                                                \
  }                                                                  \
                                                                     \
  MUST_USE static inline T Name##_unwrap_or(Name opt, T default_val) \
  {                                                                  \
    return opt.is_some ? opt.value : default_val;                    \
  }

DEFINE_OPTION(int, OptionInt)
#define SomeInt(v) OptionInt_Some(v)
#define NoneInt OptionInt_None()

#define DEFINE_RESULT(OkT, ErrT, Name)                                   \
  typedef struct                                                         \
  {                                                                      \
    bool is_ok;                                                          \
    union                                                                \
    {                                                                    \
      OkT ok;                                                            \
      ErrT err;                                                          \
    };                                                                   \
  } Name;                                                                \
                                                                         \
  MUST_USE static inline Name Name##_Ok(OkT v)                           \
  {                                                                      \
    return (Name){.is_ok = true, .ok = v};                               \
  }                                                                      \
                                                                         \
  MUST_USE static inline Name Name##_Err(ErrT e)                         \
  {                                                                      \
    return (Name){.is_ok = false, .err = e};                             \
  }                                                                      \
                                                                         \
  MUST_USE static inline OkT Name##_unwrap(Name res)                     \
  {                                                                      \
    if (!res.is_ok)                                                      \
    {                                                                    \
      PANIC("Called " #Name "_unwrap() on an Err value!");               \
    }                                                                    \
    return res.ok;                                                       \
  }                                                                      \
                                                                         \
  MUST_USE static inline OkT Name##_expect(Name res, const char *msg)    \
  {                                                                      \
    if (!res.is_ok)                                                      \
    {                                                                    \
      PANIC(msg);                                                        \
    }                                                                    \
    return res.ok;                                                       \
  }                                                                      \
                                                                         \
  MUST_USE static inline OkT Name##_unwrap_or(Name res, OkT default_val) \
  {                                                                      \
    return res.is_ok ? res.ok : default_val;                             \
  }

DEFINE_RESULT(int, const char *, ResultInt)
#define OkInt(v) ResultInt_Ok(v)
#define ErrInt(e) ResultInt_Err(e)

// Rust's '?' operator: unwraps Ok(v) or returns Err(e) from enclosing function
// while automatically running all OWN scope cleanups.
#define TRY(result_expr) ({         \
  __auto_type _res = (result_expr); \
  if (UNLIKELY(!_res.is_ok))        \
  {                                 \
    return _res;                    \
  }                                 \
  _res.ok;                          \
})

#endif // ALLOYSAFE_MONADS_H
