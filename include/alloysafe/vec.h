/* ==============================================================================
 * Alloysafe-C: Hardening Legacy C Code with an Ironclad Alloy of Rust Safety
 * ------------------------------------------------------------------------------
 * Copyright (c) 2026 Subrata Roy. All rights reserved.
 * SPDX-License-Identifier: MIT
 * ==============================================================================
 */

#ifndef ALLOYSAFE_VEC_H
#define ALLOYSAFE_VEC_H

#include "mem.h"
#include "slice.h"
#include "monads.h"

#if ALLOYSAFE_HAS_MALLOC

/* =========================================================
 * DYNAMIC EXPONENTIAL VECTOR (Vec<T> with RAII OWN_VEC)
 * ========================================================= */
#define DEFINE_VEC(T, Name, SliceName, ConstSliceName, OptionName)           \
  typedef struct                                                             \
  {                                                                          \
    T *data;                                                                 \
    size_t len;                                                              \
    size_t cap;                                                              \
  } Name;                                                                    \
                                                                             \
  MUST_USE static inline Name Name##_new(void)                               \
  {                                                                          \
    return (Name){.data = NULL, .len = 0, .cap = 0};                         \
  }                                                                          \
                                                                             \
  MUST_USE static inline Name Name##_with_capacity(size_t initial_cap)       \
  {                                                                          \
    if (initial_cap == 0)                                                    \
    {                                                                        \
      return (Name){.data = NULL, .len = 0, .cap = 0};                       \
    }                                                                        \
    T *ptr = (T *)safe_calloc_checked(initial_cap, sizeof(T));               \
    return (Name){.data = ptr, .len = 0, .cap = initial_cap};                \
  }                                                                          \
                                                                             \
  static inline void Name##_drop(Name *v)                                    \
  {                                                                          \
    if (v != NULL && v->data != NULL)                                        \
    {                                                                        \
      free(v->data);                                                         \
      v->data = NULL;                                                        \
      v->len = 0;                                                            \
      v->cap = 0;                                                            \
    }                                                                        \
  }                                                                          \
                                                                             \
  static inline void Name##_reserve(Name *v, size_t additional)              \
  {                                                                          \
    if (v == NULL)                                                           \
      return;                                                                \
    size_t min_cap = 0;                                                      \
    if (__builtin_add_overflow(v->len, additional, &min_cap))                \
    {                                                                        \
      PANIC(#Name "_reserve: capacity calculation overflow!");               \
    }                                                                        \
    if (min_cap <= v->cap)                                                   \
      return;                                                                \
    size_t new_cap = v->cap == 0 ? 4U : v->cap;                              \
    while (new_cap < min_cap)                                                \
    {                                                                        \
      if (__builtin_mul_overflow(new_cap, 2U, &new_cap))                     \
      {                                                                      \
        new_cap = min_cap;                                                   \
        break;                                                               \
      }                                                                      \
    }                                                                        \
    size_t total_bytes = 0;                                                  \
    if (__builtin_mul_overflow(new_cap, sizeof(T), &total_bytes))            \
    {                                                                        \
      PANIC(#Name "_reserve: byte size overflow!");                          \
    }                                                                        \
    T *new_data = (T *)realloc(v->data, total_bytes);                        \
    if (new_data == NULL)                                                    \
    {                                                                        \
      PANIC(#Name "_reserve: out of memory!");                               \
    }                                                                        \
    v->data = new_data;                                                      \
    v->cap = new_cap;                                                        \
  }                                                                          \
                                                                             \
  static inline void Name##_push(Name *v, T item)                            \
  {                                                                          \
    if (v == NULL)                                                           \
      PANIC(#Name "_push: NULL vector pointer!");                            \
    if (v->len >= v->cap)                                                    \
    {                                                                        \
      Name##_reserve(v, 1U);                                                 \
    }                                                                        \
    v->data[v->len] = item;                                                  \
    v->len += 1U;                                                            \
  }                                                                          \
                                                                             \
  MUST_USE static inline OptionName Name##_pop(Name *v)                      \
  {                                                                          \
    if (v == NULL || v->len == 0)                                            \
    {                                                                        \
      return OptionName##_None();                                            \
    }                                                                        \
    v->len -= 1U;                                                            \
    return OptionName##_Some(v->data[v->len]);                               \
  }                                                                          \
                                                                             \
  MUST_USE static inline T Name##_get(const Name *v, size_t idx)             \
  {                                                                          \
    if (v == NULL || idx >= v->len || v->data == NULL)                       \
    {                                                                        \
      PANIC(#Name "_get: index out of bounds!");                             \
    }                                                                        \
    return v->data[idx];                                                     \
  }                                                                          \
                                                                             \
  static inline void Name##_set(Name *v, size_t idx, T val)                  \
  {                                                                          \
    if (v == NULL || idx >= v->len || v->data == NULL)                       \
    {                                                                        \
      PANIC(#Name "_set: index out of bounds!");                             \
    }                                                                        \
    v->data[idx] = val;                                                      \
  }                                                                          \
                                                                             \
  MUST_USE static inline SliceName Name##_as_slice(Name *v)                  \
  {                                                                          \
    if (v == NULL || v->len == 0)                                            \
    {                                                                        \
      return (SliceName){.data = NULL, .len = 0};                            \
    }                                                                        \
    return (SliceName){.data = v->data, .len = v->len};                      \
  }                                                                          \
                                                                             \
  MUST_USE static inline ConstSliceName Name##_as_const_slice(const Name *v) \
  {                                                                          \
    if (v == NULL || v->len == 0)                                            \
    {                                                                        \
      return (ConstSliceName){.data = NULL, .len = 0};                       \
    }                                                                        \
    return (ConstSliceName){.data = v->data, .len = v->len};                 \
  }

#define OWN_VEC(VecName) __attribute__((cleanup(VecName##_drop))) VecName

DEFINE_VEC(int, VecInt, IntSlice, ConstIntSlice, OptionInt)

#endif // ALLOYSAFE_HAS_MALLOC

#endif // ALLOYSAFE_VEC_H
