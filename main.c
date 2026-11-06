#include "alloysafe.h"

MUST_USE ResultInt safe_divide(int a, int b)
{
  if (b == 0)
  {
    return ErrInt("Division by zero");
  }
  if (a == INT_MIN && b == -1)
  {
    return ErrInt("Signed integer division overflow");
  }
  return OkInt(CHECKED_DIV(a, b));
}

// Demonstrates Rust's '?' operator via TRY() with automatic scope cleanup
MUST_USE ResultInt compute_pipeline(int base, int divisor)
{
  OWN int *scratch = SAFE_ALLOC(int, 2);
  scratch[0] = TRY(safe_divide(base, divisor));
  scratch[1] = TRY(safe_divide(scratch[0], 2));
  return OkInt(CHECKED_ADD(scratch[0], scratch[1]));
} // 'scratch' is automatically freed whether TRY() returns early or succeeds!

int main(void)
{
  printf("=========================================\n");
  printf(" ALLOYSAFE-C PRODUCTION SHOWCASE         \n");
  printf("=========================================\n");

  // 1. Single Ownership + Overflow-Checked Zeroed Allocation
  OWN int *buf = SAFE_ALLOC(int, 4);
  buf[0] = 10;
  buf[1] = 20;

  // 2. Move Semantics: transfers pointer to 'new_owner' and nulls 'buf'
  OWN int *new_owner = MOVE(buf);
  ASSERT_SAFE(buf == NULL, "Moved pointer must be NULL");

  // 3. Bounds-Checked Slice, Subslice, and Safe Copy
  let slice = IntSlice_from_raw(new_owner, 4);
  IntSlice_set(slice, 2, 30);
  IntSlice_set(slice, 3, 40);

  let lower_half = IntSlice_subslice(slice, 0, 2);
  let upper_half = IntSlice_subslice(slice, 2, 4);
  IntSlice_copy_from(lower_half, upper_half); // Copies [30, 40] into [0..2]

  let first = IntSlice_get(slice, 0); // Now 30

  // 3b. Read-only Slice View (&[T])
  let const_view = ConstIntSlice_from_raw(slice.data, slice.len);
  ASSERT_SAFE(ConstIntSlice_get(const_view, 0) == 30, "ConstIntSlice mismatch");

  // 4. Checked Overflow Math (Single-Evaluation Safe)
  let sum = CHECKED_ADD(first, 50); // 80

  // 5. Forced Result Handling + TRY() Pipeline with Automatic Unwinding
  ResultInt failed_res = compute_pipeline(sum, 0);
  ASSERT_SAFE(!failed_res.is_ok, "Pipeline with divisor=0 must return Err");

  ResultInt res = compute_pipeline(sum, 2); // 80 / 2 = 40, 40 / 2 = 20 -> 40 + 20 = 60
  let final_val = ResultInt_expect(res, "Math pipeline failed");

  // 6. Type-Safe Heap Box (Box<T>)
  {
    OWN_BOX(BoxInt)
    my_box = BoxInt_new(final_val);
    ASSERT_SAFE(*(my_box.ptr) == 60, "BoxInt value mismatch");
    *(my_box.ptr) += 5; // 65
    ASSERT_SAFE(*(my_box.ptr) == 65, "BoxInt mutation mismatch");
  } // 'my_box' is automatically dropped here!

  // 7. Dynamic Vector (Vec<T> with exponential growth & RAII drop)
  {
    OWN_VEC(VecInt)
    vec = VecInt_new();
    VecInt_push(&vec, 100);
    VecInt_push(&vec, 200);
    VecInt_push(&vec, 300);
    ASSERT_SAFE(vec.len == 3, "VecInt length must be 3");

    OptionInt popped = VecInt_pop(&vec);
    ASSERT_SAFE(popped.is_some && popped.value == 300, "Popped value must be 300");
    ASSERT_SAFE(vec.len == 2, "VecInt length after pop must be 2");

    let vec_slice = VecInt_as_const_slice(&vec);
    ASSERT_SAFE(ConstIntSlice_get(vec_slice, 0) == 100, "Vec slice[0] == 100");
  } // 'vec' is automatically freed here!

  // 8. Low-Level Memory Security: Volatile Zeroing
  uint8_t secret_key[8] = {1, 2, 3, 4, 5, 6, 7, 8};
  safe_memzero_volatile(secret_key, sizeof(secret_key));
  ASSERT_SAFE(secret_key[0] == 0 && secret_key[7] == 0, "Secret key must be zeroed");

  // 9. Safe String Views (StrSlice / &str)
  let tag = STR_LIT("alloysafe-c");
  let prefix = StrSlice_subslice(tag, 0, 9);
  let expected = StrSlice_from_cstr("alloysafe", 32);
  ASSERT_SAFE(StrSlice_eq(prefix, expected), "String slice mismatch");

  // 10. Runtime Borrow Checker (RefCell with automatic scope release)
  mut cell = RefCellInt_new(final_val);
  {
    BORROW_MUT(RefCellInt)
    writer = RefCellInt_borrow_mut(&cell);
    *(writer.ptr) = CHECKED_ADD(*(writer.ptr), 10); // 60 + 10 = 70
  } // 'writer' automatically drops its mutable borrow here!

  BORROW_REF(RefCellInt)
  reader1 = RefCellInt_borrow(&cell);
  BORROW_REF(RefCellInt)
  reader2 = RefCellInt_borrow(&cell);

  printf("[%.*s] Success! Shared read values: %d and %d\n",
         (int)prefix.len, prefix.data, *(reader1.ptr), *(reader2.ptr));
  return 0;
}
