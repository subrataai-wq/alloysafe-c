/* ==============================================================================
 * Alloysafe-C: Hardening Legacy C Code with an Ironclad Alloy of Rust Safety
 * ------------------------------------------------------------------------------
 * Copyright (c) 2026 Subrata Roy. All rights reserved.
 * SPDX-License-Identifier: MIT
 * ==============================================================================
 */

#ifndef ALLOYSAFE_STR_H
#define ALLOYSAFE_STR_H

#include "core.h"

/* =========================================================
 * SAFE STRING VIEWS (StrSlice / &str)
 * ========================================================= */
typedef struct
{
  const char *data;
  size_t len;
} StrSlice;

// Compile-time string literal wrapper (strictly enforces string literals via "" concatenation)
#define STR_LIT(lit) ((StrSlice){.data = ("" lit), .len = sizeof("" lit) - 1U})

// Bounded construction from a legacy C string (scans at most max_capacity bytes)
MUST_USE static inline StrSlice StrSlice_from_cstr(const char *cstr, size_t max_capacity)
{
  if (cstr == NULL)
  {
    PANIC("StrSlice_from_cstr: NULL pointer!");
  }
  size_t len = 0;
  while (len < max_capacity && cstr[len] != '\0')
  {
    len += 1U;
  }
  if (len == max_capacity)
  {
    PANIC("StrSlice_from_cstr: unterminated C string exceeds max_capacity!");
  }
  return (StrSlice){.data = cstr, .len = len};
}

MUST_USE static inline bool StrSlice_eq(StrSlice a, StrSlice b)
{
  if (a.len != b.len)
  {
    return false;
  }
  if (a.len == 0)
  {
    return true;
  }
  if (a.data == NULL || b.data == NULL)
  {
    PANIC("StrSlice_eq: NULL data pointer with non-zero length!");
  }
  return __builtin_memcmp(a.data, b.data, a.len) == 0;
}

MUST_USE static inline char StrSlice_get(StrSlice s, size_t idx)
{
  if (idx >= s.len || s.data == NULL)
  {
    PANIC("StrSlice_get: byte index out of bounds!");
  }
  return s.data[idx];
}

MUST_USE static inline StrSlice StrSlice_subslice(StrSlice s, size_t start, size_t end)
{
  if (start > end || end > s.len)
  {
    PANIC("StrSlice_subslice: invalid range!");
  }
  return (StrSlice){.data = s.data + start, .len = end - start};
}

MUST_USE static inline bool StrSlice_starts_with(StrSlice s, StrSlice prefix)
{
  if (prefix.len > s.len) return false;
  if (prefix.len == 0) return true;
  if (s.data == NULL || prefix.data == NULL) return false;
  return __builtin_memcmp(s.data, prefix.data, prefix.len) == 0;
}

MUST_USE static inline bool StrSlice_ends_with(StrSlice s, StrSlice suffix)
{
  if (suffix.len > s.len) return false;
  if (suffix.len == 0) return true;
  if (s.data == NULL || suffix.data == NULL) return false;
  return __builtin_memcmp(s.data + (s.len - suffix.len), suffix.data, suffix.len) == 0;
}

MUST_USE static inline bool StrSlice_contains(StrSlice s, StrSlice needle)
{
  if (needle.len > s.len) return false;
  if (needle.len == 0) return true;
  if (s.data == NULL || needle.data == NULL) return false;
  for (size_t i = 0; i <= s.len - needle.len; ++i)
  {
    if (__builtin_memcmp(s.data + i, needle.data, needle.len) == 0)
    {
      return true;
    }
  }
  return false;
}

MUST_USE static inline StrSlice StrSlice_trim(StrSlice s)
{
  if (s.len == 0 || s.data == NULL) return s;
  size_t start = 0;
  while (start < s.len && (s.data[start] == ' ' || s.data[start] == '\t' ||
                           s.data[start] == '\r' || s.data[start] == '\n'))
  {
    start++;
  }
  size_t end = s.len;
  while (end > start && (s.data[end - 1] == ' ' || s.data[end - 1] == '\t' ||
                         s.data[end - 1] == '\r' || s.data[end - 1] == '\n'))
  {
    end--;
  }
  return (StrSlice){.data = s.data + start, .len = end - start};
}

MUST_USE static inline bool StrSlice_parse_int(StrSlice s, int *out_val)
{
  if (out_val == NULL) return false;
  StrSlice trimmed = StrSlice_trim(s);
  if (trimmed.len == 0 || trimmed.data == NULL) return false;

  size_t idx = 0;
  bool negative = false;
  if (trimmed.data[0] == '-')
  {
    negative = true;
    idx = 1;
  }
  else if (trimmed.data[0] == '+')
  {
    idx = 1;
  }
  if (idx >= trimmed.len) return false;

  int acc = 0;
  while (idx < trimmed.len)
  {
    char c = trimmed.data[idx];
    if (c < '0' || c > '9') return false;
    int digit = c - '0';
    if (__builtin_mul_overflow(acc, 10, &acc)) return false;
    if (negative)
    {
      if (__builtin_sub_overflow(acc, digit, &acc)) return false;
    }
    else
    {
      if (__builtin_add_overflow(acc, digit, &acc)) return false;
    }
    idx++;
  }
  *out_val = acc;
  return true;
}

#endif // ALLOYSAFE_STR_H

