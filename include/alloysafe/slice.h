/* ==============================================================================
 * Alloysafe-C: Hardening Legacy C Code with an Ironclad Alloy of Rust Safety
 * ------------------------------------------------------------------------------
 * Copyright (c) 2026 Subrata Roy. All rights reserved.
 * SPDX-License-Identifier: MIT
 * ==============================================================================
 */

#ifndef ALLOYSAFE_SLICE_H
#define ALLOYSAFE_SLICE_H

#include "core.h"
#include "math.h"

/* =========================================================
 * BOUNDS-CHECKED SLICES (Read, Write, Subslice & Safe Copy)
 * ========================================================= */
#define DEFINE_SLICE(T, Name)                                                   \
  typedef struct                                                                \
  {                                                                             \
    T *data;                                                                    \
    size_t len;                                                                 \
  } Name;                                                                       \
                                                                                \
  MUST_USE static inline Name Name##_from_raw(T *data, size_t len)              \
  {                                                                             \
    if (len > 0 && data == NULL)                                                \
    {                                                                           \
      PANIC(#Name "_from_raw: NULL pointer with non-zero length!");             \
    }                                                                           \
    return (Name){.data = data, .len = len};                                    \
  }                                                                             \
                                                                                \
  MUST_USE static inline T Name##_get(Name s, size_t idx)                       \
  {                                                                             \
    if (idx >= s.len || s.data == NULL)                                         \
    {                                                                           \
      PANIC(#Name "_get: slice index out of bounds!");                          \
    }                                                                           \
    return s.data[idx];                                                         \
  }                                                                             \
                                                                                \
  MUST_USE static inline T *Name##_get_mut(Name s, size_t idx)                  \
  {                                                                             \
    if (idx >= s.len || s.data == NULL)                                         \
    {                                                                           \
      PANIC(#Name "_get_mut: slice index out of bounds!");                      \
    }                                                                           \
    return &s.data[idx];                                                        \
  }                                                                             \
                                                                                \
  static inline void Name##_set(Name s, size_t idx, T val)                      \
  {                                                                             \
    if (idx >= s.len || s.data == NULL)                                         \
    {                                                                           \
      PANIC(#Name "_set: slice index out of bounds!");                          \
    }                                                                           \
    s.data[idx] = val;                                                          \
  }                                                                             \
                                                                                \
  MUST_USE static inline Name Name##_subslice(Name s, size_t start, size_t end) \
  {                                                                             \
    if (start > end || end > s.len)                                             \
    {                                                                           \
      PANIC(#Name "_subslice: invalid range!");                                 \
    }                                                                           \
    return (Name){.data = s.data + start, .len = end - start};                  \
  }                                                                             \
                                                                                \
  static inline void Name##_copy_from(Name dst, Name src)                       \
  {                                                                             \
    if (dst.len != src.len)                                                     \
    {                                                                           \
      PANIC(#Name "_copy_from: source and destination slice lengths differ!");  \
    }                                                                           \
    if (dst.len > 0)                                                            \
    {                                                                           \
      if (dst.data == NULL || src.data == NULL)                                 \
      {                                                                         \
        PANIC(#Name "_copy_from: NULL slice data pointer!");                    \
      }                                                                         \
      size_t bytes = CHECKED_MUL(dst.len, sizeof(T));                           \
      __builtin_memmove(dst.data, src.data, bytes);                             \
    }                                                                           \
  }

#define DEFINE_CONST_SLICE(T, Name)                                             \
  typedef struct                                                                \
  {                                                                             \
    const T *data;                                                              \
    size_t len;                                                                 \
  } Name;                                                                       \
                                                                                \
  MUST_USE static inline Name Name##_from_raw(const T *data, size_t len)        \
  {                                                                             \
    if (len > 0 && data == NULL)                                                \
    {                                                                           \
      PANIC(#Name "_from_raw: NULL pointer with non-zero length!");             \
    }                                                                           \
    return (Name){.data = data, .len = len};                                    \
  }                                                                             \
                                                                                \
  MUST_USE static inline T Name##_get(Name s, size_t idx)                       \
  {                                                                             \
    if (idx >= s.len || s.data == NULL)                                         \
    {                                                                           \
      PANIC(#Name "_get: slice index out of bounds!");                          \
    }                                                                           \
    return s.data[idx];                                                         \
  }                                                                             \
                                                                                \
  MUST_USE static inline Name Name##_subslice(Name s, size_t start, size_t end) \
  {                                                                             \
    if (start > end || end > s.len)                                             \
    {                                                                           \
      PANIC(#Name "_subslice: invalid range!");                                 \
    }                                                                           \
    return (Name){.data = s.data + start, .len = end - start};                  \
  }

DEFINE_SLICE(int, IntSlice)
DEFINE_SLICE(uint8_t, ByteSlice)
DEFINE_CONST_SLICE(int, ConstIntSlice)
DEFINE_CONST_SLICE(uint8_t, ConstByteSlice)

#endif // ALLOYSAFE_SLICE_H

