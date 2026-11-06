/* ==============================================================================
 * Alloysafe-C: Hardening Legacy C Code with an Ironclad Alloy of Rust Safety
 * ------------------------------------------------------------------------------
 * Copyright (c) 2026 Subrata Roy. All rights reserved.
 * SPDX-License-Identifier: MIT
 * ==============================================================================
 */

#ifndef ALLOYSAFE_ARC_H
#define ALLOYSAFE_ARC_H

#include "mem.h"

#if ALLOYSAFE_HAS_MALLOC

/* =========================================================
 * THREAD-SAFE ATOMICALLY REFERENCE COUNTED POINTER (Arc<T>)
 * Uses lock-free GCC/Clang atomic builtins.
 * ========================================================= */
#define DEFINE_ARC(T, Name)                                                     \
  typedef struct                                                                \
  {                                                                             \
    T *ptr;                                                                     \
    size_t *strong_count;                                                       \
  } Name;                                                                       \
                                                                                \
  MUST_USE static inline Name Name##_new(T val)                                 \
  {                                                                             \
    T *p = SAFE_ALLOC(T, 1);                                                    \
    size_t *cnt = SAFE_ALLOC(size_t, 1);                                        \
    *p = val;                                                                   \
    *cnt = 1;                                                                   \
    return (Name){.ptr = p, .strong_count = cnt};                               \
  }                                                                             \
                                                                                \
  MUST_USE static inline Name Name##_clone(const Name *arc)                     \
  {                                                                             \
    if (arc != NULL && arc->strong_count != NULL)                               \
    {                                                                           \
      __atomic_add_fetch(arc->strong_count, 1U, __ATOMIC_SEQ_CST);              \
      return *arc;                                                              \
    }                                                                           \
    return (Name){.ptr = NULL, .strong_count = NULL};                           \
  }                                                                             \
                                                                                \
  MUST_USE static inline size_t Name##_count(const Name *arc)                   \
  {                                                                             \
    if (arc == NULL || arc->strong_count == NULL)                               \
      return 0;                                                                 \
    return __atomic_load_n(arc->strong_count, __ATOMIC_SEQ_CST);                \
  }                                                                             \
                                                                                \
  static inline void Name##_drop(Name *arc)                                     \
  {                                                                             \
    if (arc != NULL && arc->strong_count != NULL)                               \
    {                                                                           \
      size_t rem = __atomic_sub_fetch(arc->strong_count, 1U, __ATOMIC_SEQ_CST); \
      if (rem == 0)                                                             \
      {                                                                         \
        free(arc->ptr);                                                         \
        free(arc->strong_count);                                                \
      }                                                                         \
      arc->ptr = NULL;                                                          \
      arc->strong_count = NULL;                                                 \
    }                                                                           \
  }

#define OWN_ARC(ArcName) __attribute__((cleanup(ArcName##_drop))) ArcName

DEFINE_ARC(int, ArcInt)

#endif // ALLOYSAFE_HAS_MALLOC

#endif // ALLOYSAFE_ARC_H
