/* ==============================================================================
 * Alloysafe-C: Hardening Legacy C Code with an Ironclad Alloy of Rust Safety
 * ------------------------------------------------------------------------------
 * Copyright (c) 2026 Subrata Roy. All rights reserved.
 * SPDX-License-Identifier: MIT
 * ==============================================================================
 */

#ifndef ALLOYSAFE_RC_H
#define ALLOYSAFE_RC_H

#include "mem.h"

#if ALLOYSAFE_HAS_MALLOC

/* =========================================================
 * SINGLE-THREADED REFERENCE COUNTED POINTER (Rc<T>)
 * ========================================================= */
#define DEFINE_RC(T, Name)                                   \
  typedef struct                                             \
  {                                                          \
    T *ptr;                                                  \
    size_t *strong_count;                                    \
  } Name;                                                    \
                                                             \
  MUST_USE static inline Name Name##_new(T val)              \
  {                                                          \
    T *p = SAFE_ALLOC(T, 1);                                 \
    size_t *cnt = SAFE_ALLOC(size_t, 1);                     \
    *p = val;                                                \
    *cnt = 1;                                                \
    return (Name){.ptr = p, .strong_count = cnt};            \
  }                                                          \
                                                             \
  MUST_USE static inline Name Name##_clone(const Name *rc)   \
  {                                                          \
    if (rc != NULL && rc->strong_count != NULL)              \
    {                                                        \
      *(rc->strong_count) += 1;                              \
      return *rc;                                            \
    }                                                        \
    return (Name){.ptr = NULL, .strong_count = NULL};        \
  }                                                          \
                                                             \
  MUST_USE static inline size_t Name##_count(const Name *rc) \
  {                                                          \
    if (rc == NULL || rc->strong_count == NULL)              \
      return 0;                                              \
    return *(rc->strong_count);                              \
  }                                                          \
                                                             \
  static inline void Name##_drop(Name *rc)                   \
  {                                                          \
    if (rc != NULL && rc->strong_count != NULL)              \
    {                                                        \
      *(rc->strong_count) -= 1;                              \
      if (*(rc->strong_count) == 0)                          \
      {                                                      \
        free(rc->ptr);                                       \
        free(rc->strong_count);                              \
      }                                                      \
      rc->ptr = NULL;                                        \
      rc->strong_count = NULL;                               \
    }                                                        \
  }

#define OWN_RC(RcName) __attribute__((cleanup(RcName##_drop))) RcName

DEFINE_RC(int, RcInt)

#endif // ALLOYSAFE_HAS_MALLOC

#endif // ALLOYSAFE_RC_H
