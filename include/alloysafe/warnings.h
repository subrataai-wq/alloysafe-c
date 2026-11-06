/* ==============================================================================
 * Alloysafe-C: Hardening Legacy C Code with an Ironclad Alloy of Rust Safety
 * ------------------------------------------------------------------------------
 * Copyright (c) 2026 Subrata Roy. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * ==============================================================================
 */

#ifndef ALLOYSAFE_WARNINGS_H
#define ALLOYSAFE_WARNINGS_H

/* ==============================================================================
 * SCOPED DIAGNOSTIC PRAGMA MACROS
 * Enables fine-grained diagnostic control in user and library code.
 * ============================================================================== */
#if defined(__GNUC__) || defined(__clang__)

#define ALLOYSAFE_PRAGMA(x) _Pragma(#x)
#define ALLOYSAFE_DIAGNOSTIC_PUSH() ALLOYSAFE_PRAGMA(GCC diagnostic push)
#define ALLOYSAFE_DIAGNOSTIC_POP() ALLOYSAFE_PRAGMA(GCC diagnostic pop)
#define ALLOYSAFE_DIAGNOSTIC_IGNORE(flag) ALLOYSAFE_PRAGMA(GCC diagnostic ignored flag)
#define ALLOYSAFE_DIAGNOSTIC_ERROR(flag) ALLOYSAFE_PRAGMA(GCC diagnostic error flag)
#define ALLOYSAFE_DIAGNOSTIC_WARN(flag) ALLOYSAFE_PRAGMA(GCC diagnostic warning flag)

#else

#define ALLOYSAFE_DIAGNOSTIC_PUSH()
#define ALLOYSAFE_DIAGNOSTIC_POP()
#define ALLOYSAFE_DIAGNOSTIC_IGNORE(flag)
#define ALLOYSAFE_DIAGNOSTIC_ERROR(flag)
#define ALLOYSAFE_DIAGNOSTIC_WARN(flag)

#endif

/* ==============================================================================
 * IN-CODE COMPILER DIAGNOSTIC ENFORCEMENT & TOGGLES
 *
 * Configuration defines:
 *   #define ALLOYSAFE_DISABLE_DIAGNOSTICS 1  // Completely disable in-code pragmas
 *   #define ALLOYSAFE_WARN_ONLY 1            // Emit warnings instead of errors
 *   #define ALLOYSAFE_ALLOW_SHADOW 1         // Disable -Wshadow checks
 *   #define ALLOYSAFE_ALLOW_UNUSED_RESULT 1  // Disable -Wunused-result checks
 * ============================================================================== */
#if !defined(ALLOYSAFE_DISABLE_DIAGNOSTICS) && (defined(__GNUC__) || defined(__clang__))

  // Ignore unknown pragmas to allow cross-compiler warning options
  #pragma GCC diagnostic ignored "-Wpragmas"

  #if defined(__clang__)
    #pragma clang diagnostic ignored "-Wgnu-statement-expression"
    #pragma clang diagnostic ignored "-Wgnu-auto-type"
  #elif defined(__GNUC__)
    #pragma GCC diagnostic ignored "-Wpedantic"
  #endif

  #if defined(ALLOYSAFE_WARN_ONLY)
    #pragma GCC diagnostic warning "-Wpointer-arith"
    #pragma GCC diagnostic warning "-Wuninitialized"
    #pragma GCC diagnostic warning "-Wformat-security"
    #pragma GCC diagnostic warning "-Wformat=2"
    #pragma GCC diagnostic warning "-Wimplicit-function-declaration"
  #else
    #pragma GCC diagnostic error "-Wpointer-arith"
    #pragma GCC diagnostic error "-Wuninitialized"
    #pragma GCC diagnostic error "-Wformat-security"
    #pragma GCC diagnostic error "-Wformat=2"
    #pragma GCC diagnostic error "-Wimplicit-function-declaration"
  #endif

  #if defined(ALLOYSAFE_ALLOW_SHADOW)
    #pragma GCC diagnostic ignored "-Wshadow"
  #else
    #if defined(ALLOYSAFE_WARN_ONLY)
      #pragma GCC diagnostic warning "-Wshadow"
    #else
      #pragma GCC diagnostic error "-Wshadow"
    #endif
  #endif

  #if defined(ALLOYSAFE_ALLOW_UNUSED_RESULT)
    #pragma GCC diagnostic ignored "-Wunused-result"
  #else
    #if defined(ALLOYSAFE_WARN_ONLY)
      #pragma GCC diagnostic warning "-Wunused-result"
    #else
      #pragma GCC diagnostic error "-Wunused-result"
    #endif
  #endif

#endif

#endif // ALLOYSAFE_WARNINGS_H
