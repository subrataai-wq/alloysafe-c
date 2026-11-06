/* ==============================================================================
 * Alloysafe-C: Hardening Legacy C Code with an Ironclad Alloy of Rust Safety
 * ------------------------------------------------------------------------------
 * Copyright (c) 2026 Subrata Roy. All rights reserved.
 * SPDX-License-Identifier: MIT
 * ==============================================================================
 */

#ifndef ALLOYSAFE_CORE_H
#define ALLOYSAFE_CORE_H

#include "env.h"

/* =========================================================
 * COMPILER ATTRIBUTES & OPTIMIZATION HINTS
 * ========================================================= */
#define MUST_USE __attribute__((warn_unused_result))
#define NONNULL(...) __attribute__((nonnull(__VA_ARGS__)))
#define NORETURN _Noreturn
#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

/* =========================================================
 * CONFIGURABLE PANIC HOOK & ERROR DISPATCH
 * ========================================================= */
typedef void (*alloysafe_panic_hook_t)(const char *file, int line, const char *func, const char *msg);

static inline alloysafe_panic_hook_t *alloysafe_get_panic_hook_ptr(void)
{
  static alloysafe_panic_hook_t hook = NULL;
  return &hook;
}

static inline void alloysafe_set_panic_handler(alloysafe_panic_hook_t handler)
{
  *alloysafe_get_panic_hook_ptr() = handler;
}

NORETURN static inline void alloysafe_panic(const char *file, int line, const char *func, const char *msg)
{
  alloysafe_panic_hook_t hook = *alloysafe_get_panic_hook_ptr();
  if (hook != NULL)
  {
    hook(file, line, func, msg);
  }

#if ALLOYSAFE_HAS_STDIO
  fprintf(stderr, "\n[ALLOYSAFE PANIC] at %s:%d in %s(): %s\n", file, line, func, msg);
#endif

#if ALLOYSAFE_HOSTED
  abort();
#else
  __builtin_trap();
  while (1)
  {
    // Halt in baremetal freestanding targets
  }
#endif
}

#define PANIC(msg) alloysafe_panic(__FILE__, __LINE__, __func__, (msg))

#define ASSERT_SAFE(cond, msg) \
  do                           \
  {                            \
    if (UNLIKELY(!(cond)))     \
    {                          \
      PANIC(msg);              \
    }                          \
  } while (0)

#endif // ALLOYSAFE_CORE_H
