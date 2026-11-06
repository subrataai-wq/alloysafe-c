/* ==============================================================================
 * Alloysafe-C: Hardening Legacy C Code with an Ironclad Alloy of Rust Safety
 * ------------------------------------------------------------------------------
 * Copyright (c) 2026 Subrata Roy. All rights reserved.
 * SPDX-License-Identifier: MIT
 * ==============================================================================
 */

#ifndef ALLOYSAFE_ENV_H
#define ALLOYSAFE_ENV_H

/* =========================================================
 * ALLOYSAFE ENVIRONMENT DETECTION & HOSTED / BAREMETAL GUARDS
 * ========================================================= */

#if defined(ALLOYSAFE_BAREMETAL) || defined(ALLOYSAFE_NO_STD) || (defined(__STDC_HOSTED__) && __STDC_HOSTED__ == 0)
  #ifndef ALLOYSAFE_BAREMETAL
    #define ALLOYSAFE_BAREMETAL 1
  #endif
  #define ALLOYSAFE_HOSTED 0
#else
  #define ALLOYSAFE_HOSTED 1
#endif

// Core freestanding headers (always safe to include in C99/C11 baremetal)
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <limits.h>

#if ALLOYSAFE_HOSTED
  // Hosted OS environment includes full C runtime
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #define ALLOYSAFE_HAS_STDIO 1
  #define ALLOYSAFE_HAS_MALLOC 1
#else
  // Baremetal / Freestanding environment
  #ifndef ALLOYSAFE_HAS_STDIO
    #define ALLOYSAFE_HAS_STDIO 0
  #endif
  #ifndef ALLOYSAFE_HAS_MALLOC
    #define ALLOYSAFE_HAS_MALLOC 0
  #endif
#endif

#endif // ALLOYSAFE_ENV_H

