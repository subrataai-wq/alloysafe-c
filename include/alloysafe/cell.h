/* ==============================================================================
 * Alloysafe-C: Hardening Legacy C Code with an Ironclad Alloy of Rust Safety
 * ------------------------------------------------------------------------------
 * Copyright (c) 2026 Subrata Roy. All rights reserved.
 * SPDX-License-Identifier: MIT
 * ==============================================================================
 */

#ifndef ALLOYSAFE_CELL_H
#define ALLOYSAFE_CELL_H

#include "core.h"

/* =========================================================
 * RUNTIME BORROW CHECKER (RefCell<T> + RAII Guards)
 *    state == 0 : Unborrowed
 *    state >  0 : Count of active immutable borrows (&T)
 *    state == -1: Active exclusive mutable borrow (&mut T)
 * ========================================================= */
#define DEFINE_REFCELL(T, Name)                                                                          \
  typedef struct                                                                                         \
  {                                                                                                      \
    T value;                                                                                             \
    ptrdiff_t borrow_state;                                                                              \
  } Name;                                                                                                \
                                                                                                         \
  typedef struct                                                                                         \
  {                                                                                                      \
    const T *ptr;                                                                                        \
    ptrdiff_t *_state;                                                                                   \
  } Name##_Ref;                                                                                          \
                                                                                                         \
  typedef struct                                                                                         \
  {                                                                                                      \
    T *ptr;                                                                                              \
    ptrdiff_t *_state;                                                                                   \
  } Name##_RefMut;                                                                                       \
                                                                                                         \
  MUST_USE static inline Name Name##_new(T initial)                                                      \
  {                                                                                                      \
    return (Name){.value = initial, .borrow_state = 0};                                                  \
  }                                                                                                      \
                                                                                                         \
  static inline void Name##_release_ref(Name##_Ref *r)                                                   \
  {                                                                                                      \
    if (r != NULL && r->_state != NULL)                                                                  \
    {                                                                                                    \
      if (*(r->_state) <= 0)                                                                             \
      {                                                                                                  \
        PANIC("RefCell underflow on drop: invalid borrow state!");                                       \
      }                                                                                                  \
      *(r->_state) -= 1;                                                                                 \
      r->_state = NULL;                                                                                  \
      r->ptr = NULL;                                                                                     \
    }                                                                                                    \
  }                                                                                                      \
                                                                                                         \
  static inline void Name##_release_ref_mut(Name##_RefMut *r)                                            \
  {                                                                                                      \
    if (r != NULL && r->_state != NULL)                                                                  \
    {                                                                                                    \
      if (*(r->_state) != -1)                                                                            \
      {                                                                                                  \
        PANIC("RefCell underflow on drop: mutable borrow was not active!");                              \
      }                                                                                                  \
      *(r->_state) = 0;                                                                                  \
      r->_state = NULL;                                                                                  \
      r->ptr = NULL;                                                                                     \
    }                                                                                                    \
  }                                                                                                      \
                                                                                                         \
  MUST_USE NONNULL(1) static inline Name##_Ref Name##_borrow(Name *cell)                                 \
  {                                                                                                      \
    if (cell->borrow_state < 0)                                                                          \
    {                                                                                                    \
      PANIC("BorrowChecker violation in " #Name ": cannot borrow as immutable while mutably borrowed!"); \
    }                                                                                                    \
    cell->borrow_state += 1;                                                                             \
    return (Name##_Ref){.ptr = &cell->value, ._state = &cell->borrow_state};                             \
  }                                                                                                      \
                                                                                                         \
  MUST_USE NONNULL(1) static inline Name##_RefMut Name##_borrow_mut(Name *cell)                          \
  {                                                                                                      \
    if (cell->borrow_state != 0)                                                                         \
    {                                                                                                    \
      PANIC("BorrowChecker violation in " #Name ": cannot borrow as mutable while already borrowed!");   \
    }                                                                                                    \
    cell->borrow_state = -1;                                                                             \
    return (Name##_RefMut){.ptr = &cell->value, ._state = &cell->borrow_state};                          \
  }

DEFINE_REFCELL(int, RefCellInt)

#define BORROW_REF(CellName) __attribute__((cleanup(CellName##_release_ref))) CellName##_Ref
#define BORROW_MUT(CellName) __attribute__((cleanup(CellName##_release_ref_mut))) CellName##_RefMut

#endif // ALLOYSAFE_CELL_H
