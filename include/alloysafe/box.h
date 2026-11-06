/* ==============================================================================
 * Alloysafe-C: Hardening Legacy C Code with an Ironclad Alloy of Rust Safety
 * ------------------------------------------------------------------------------
 * Copyright (c) 2026 Subrata Roy. All rights reserved.
 * SPDX-License-Identifier: MIT
 * ==============================================================================
 */

#ifndef ALLOYSAFE_BOX_H
#define ALLOYSAFE_BOX_H

#include "mem.h"

#if ALLOYSAFE_HAS_MALLOC

/* =========================================================
 * TYPE-SAFE HEAP BOX (Box<T> with RAII Drop)
 * ========================================================= */
#define DEFINE_BOX(T, BoxName)                                \
  typedef struct                                              \
  {                                                           \
    T *ptr;                                                   \
  } BoxName;                                                  \
                                                              \
  MUST_USE static inline BoxName BoxName##_new(T val)         \
  {                                                           \
    T *p = SAFE_ALLOC(T, 1);                                  \
    *p = val;                                                 \
    return (BoxName){.ptr = p};                               \
  }                                                           \
                                                              \
  static inline void BoxName##_drop(BoxName *b)               \
  {                                                           \
    if (b != NULL && b->ptr != NULL)                          \
    {                                                         \
      free(b->ptr);                                           \
      b->ptr = NULL;                                          \
    }                                                         \
  }                                                           \
                                                              \
  MUST_USE static inline BoxName BoxName##_move(BoxName *src) \
  {                                                           \
    if (src == NULL)                                          \
    {                                                         \
      PANIC(#BoxName "_move: NULL source pointer!");          \
    }                                                         \
    BoxName tmp = *src;                                       \
    src->ptr = NULL;                                          \
    return tmp;                                               \
  }

#define OWN_BOX(BoxName) __attribute__((cleanup(BoxName##_drop))) BoxName

DEFINE_BOX(int, BoxInt)

#endif // ALLOYSAFE_HAS_MALLOC

#endif // ALLOYSAFE_BOX_H
