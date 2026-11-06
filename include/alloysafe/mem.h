/* ==============================================================================
 * Alloysafe-C: Hardening Legacy C Code with an Ironclad Alloy of Rust Safety
 * ------------------------------------------------------------------------------
 * Copyright (c) 2026 Subrata Roy. All rights reserved.
 * SPDX-License-Identifier: MIT
 * ==============================================================================
 */

#ifndef ALLOYSAFE_MEM_H
#define ALLOYSAFE_MEM_H

#include "core.h"
#include "types.h"
#include "math.h"

/* =========================================================
 * LOW-LEVEL MEMORY SECURITY & VOLATILE WIPING
 * ========================================================= */
// Securely wipes memory, guaranteed not to be eliminated by compiler dead-store optimization
static inline void safe_memzero_volatile(void *ptr, size_t n)
{
  if (ptr == NULL || n == 0)
  {
    return;
  }
  volatile uint8_t *p = (volatile uint8_t *)ptr;
  while (n--)
  {
    *p++ = 0;
  }
}

/* =========================================================
 * MOVE SEMANTICS
 * ========================================================= */
// Transfers ownership by stealing the pointer and nulling the source lvalue
#define MOVE(src) ({        \
  __auto_type _tmp = (src); \
  (src) = NULL;             \
  _tmp;                     \
})

#if ALLOYSAFE_HAS_MALLOC

/* =========================================================
 * SINGLE OWNERSHIP & AUTOMATIC SCOPE CLEANUP
 * ========================================================= */
static inline void auto_free_ptr(void *p)
{
  void **ptr = (void **)p;
  if (ptr != NULL && *ptr != NULL)
  {
    free(*ptr);
    *ptr = NULL;
  }
}

#define OWN __attribute__((cleanup(auto_free_ptr)))

#if ALLOYSAFE_HAS_STDIO
static inline void auto_close_file(FILE **fp)
{
  if (fp != NULL && *fp != NULL)
  {
    fclose(*fp);
    *fp = NULL;
  }
}
#define OWN_FILE __attribute__((cleanup(auto_close_file)))
#endif

// Overflow-checked, zero-initialized heap allocation
MUST_USE static inline void *safe_calloc_checked(size_t count, size_t elem_size)
{
  size_t total_bytes = 0;
  if (__builtin_mul_overflow(count, elem_size, &total_bytes))
  {
    PANIC("Heap allocation size overflow (count * sizeof(T))!");
  }
  // Ensure count >= 1 and elem_size >= 1 so calloc never returns NULL on 0-sized requests
  size_t alloc_count = (count == 0) ? 1U : count;
  size_t alloc_size = (elem_size == 0) ? 1U : elem_size;
  void *ptr = calloc(alloc_count, alloc_size);
  if (ptr == NULL)
  {
    PANIC("Out of memory: safe_calloc_checked failed!");
  }
  return ptr;
}

#define SAFE_ALLOC(T, count) ((T *)safe_calloc_checked((size_t)(count), sizeof(T)))

#endif // ALLOYSAFE_HAS_MALLOC

#endif // ALLOYSAFE_MEM_H

