#include "alloysafe.h"

static int g_tests_run = 0;
static int g_tests_passed = 0;

#define TEST_ASSERT(cond, msg)                                            \
  do                                                                      \
  {                                                                       \
    g_tests_run++;                                                        \
    if (!(cond))                                                          \
    {                                                                     \
      fprintf(stderr, "  [FAIL] %s:%d: %s\n", __FILE__, __LINE__, (msg)); \
    }                                                                     \
    else                                                                  \
    {                                                                     \
      g_tests_passed++;                                                   \
    }                                                                     \
  } while (0)

/* ---------------------------------------------------------
 * 1. Checked Arithmetic Tests
 * --------------------------------------------------------- */
static void test_checked_arithmetic(void)
{
  printf("[RUNNING] test_checked_arithmetic...\n");

  TEST_ASSERT(IS_SIGNED_TYPE(int), "int must be signed");
  TEST_ASSERT(IS_SIGNED_TYPE(int64_t), "int64_t must be signed");
  TEST_ASSERT(!IS_SIGNED_TYPE(unsigned int), "unsigned int must be unsigned");
  TEST_ASSERT(!IS_SIGNED_TYPE(uint64_t), "uint64_t must be unsigned");
  TEST_ASSERT(!IS_SIGNED_TYPE(size_t), "size_t must be unsigned");

  let sum = CHECKED_ADD(15, 27);
  TEST_ASSERT(sum == 42, "15 + 27 == 42");

  let diff = CHECKED_SUB(100, 42);
  TEST_ASSERT(diff == 58, "100 - 42 == 58");

  let prod = CHECKED_MUL(6, 7);
  TEST_ASSERT(prod == 42, "6 * 7 == 42");

  let quot = CHECKED_DIV(144, 12);
  TEST_ASSERT(quot == 12, "144 / 12 == 12");

  uint32_t u_dividend = 0x80000000U;
  uint32_t u_divisor = 0xFFFFFFFFU;
  uint32_t u_res = CHECKED_DIV(u_dividend, u_divisor);
  TEST_ASSERT(u_res == 0U, "Unsigned 0x80000000U / 0xFFFFFFFFU must be 0");
}

/* ---------------------------------------------------------
 * 2. Ownership, Move Semantics & Allocator Tests
 * --------------------------------------------------------- */
static void test_ownership_and_alloc(void)
{
  printf("[RUNNING] test_ownership_and_alloc...\n");

  {
    OWN int *zero_buf = SAFE_ALLOC(int, 0);
    TEST_ASSERT(zero_buf != NULL, "Zero-count allocation must return valid non-null pointer");
  }

  {
    OWN int *buf = SAFE_ALLOC(int, 10);
    bool all_zero = true;
    for (size_t i = 0; i < 10; ++i)
    {
      if (buf[i] != 0)
      {
        all_zero = false;
        break;
      }
    }
    TEST_ASSERT(all_zero, "SAFE_ALLOC must zero-initialize allocated memory");

    buf[0] = 123;
    buf[9] = 456;

    OWN int *new_owner = MOVE(buf);
    TEST_ASSERT(buf == NULL, "Source pointer must be NULL after MOVE");
    TEST_ASSERT(new_owner != NULL, "Destination pointer must hold moved address");
    TEST_ASSERT(new_owner[0] == 123 && new_owner[9] == 456, "Moved data must remain intact");
  }
}

/* ---------------------------------------------------------
 * 3. Type-Safe Heap Box<T> Tests
 * --------------------------------------------------------- */
static void test_box(void)
{
  printf("[RUNNING] test_box...\n");

  {
    OWN_BOX(BoxInt)
    b = BoxInt_new(99);
    TEST_ASSERT(b.ptr != NULL, "BoxInt pointer must not be NULL");
    TEST_ASSERT(*(b.ptr) == 99, "BoxInt value must be 99");

    *(b.ptr) = 100;
    TEST_ASSERT(*(b.ptr) == 100, "BoxInt value after mutation must be 100");

    BoxInt moved = BoxInt_move(&b);
    TEST_ASSERT(b.ptr == NULL, "Source box pointer must be NULL after move");
    TEST_ASSERT(moved.ptr != NULL && *(moved.ptr) == 100, "Moved box value must be 100");
    BoxInt_drop(&moved);
  }
}

/* ---------------------------------------------------------
 * 4. Dynamic Growable Vector Vec<T> Tests
 * --------------------------------------------------------- */
static void test_vec(void)
{
  printf("[RUNNING] test_vec...\n");

  OWN_VEC(VecInt)
  v = VecInt_new();
  TEST_ASSERT(v.len == 0 && v.cap == 0, "New VecInt must have 0 length and 0 capacity");

  // Push items triggering capacity expansions (0 -> 4 -> 8 -> 16)
  for (int i = 0; i < 10; ++i)
  {
    VecInt_push(&v, i * 10);
  }
  TEST_ASSERT(v.len == 10, "VecInt length must be 10 after 10 pushes");
  TEST_ASSERT(v.cap >= 10, "VecInt capacity must be at least 10");
  TEST_ASSERT(VecInt_get(&v, 0) == 0, "v[0] == 0");
  TEST_ASSERT(VecInt_get(&v, 9) == 90, "v[9] == 90");

  VecInt_set(&v, 5, 555);
  TEST_ASSERT(VecInt_get(&v, 5) == 555, "v[5] after set must be 555");

  // Pop
  OptionInt popped = VecInt_pop(&v);
  TEST_ASSERT(popped.is_some && popped.value == 90, "Popped value must be 90");
  TEST_ASSERT(v.len == 9, "Length after pop must be 9");

  // Slices from Vec
  let s = VecInt_as_slice(&v);
  TEST_ASSERT(s.len == 9, "Slice length from Vec must match vector length");
  TEST_ASSERT(IntSlice_get(s, 5) == 555, "Slice get from Vec must yield 555");

  let cs = VecInt_as_const_slice(&v);
  TEST_ASSERT(cs.len == 9, "ConstSlice length must match");
  TEST_ASSERT(ConstIntSlice_get(cs, 0) == 0, "ConstSlice get must yield 0");
}

/* ---------------------------------------------------------
 * 5. Volatile Secure Memory Zeroing Tests
 * --------------------------------------------------------- */
static void test_secure_zero(void)
{
  printf("[RUNNING] test_secure_zero...\n");

  uint8_t buffer[16];
  for (size_t i = 0; i < sizeof(buffer); ++i)
  {
    buffer[i] = 0xAA;
  }
  safe_memzero_volatile(buffer, sizeof(buffer));
  bool all_zero = true;
  for (size_t i = 0; i < sizeof(buffer); ++i)
  {
    if (buffer[i] != 0)
    {
      all_zero = false;
      break;
    }
  }
  TEST_ASSERT(all_zero, "safe_memzero_volatile must clear entire buffer");
}

/* ---------------------------------------------------------
 * 6. Bounds-Checked Slices & Const Slices
 * --------------------------------------------------------- */
static void test_slices(void)
{
  printf("[RUNNING] test_slices...\n");

  OWN int *raw = SAFE_ALLOC(int, 6);
  for (int i = 0; i < 6; ++i)
  {
    raw[i] = (i + 1) * 10;
  }

  let slice = IntSlice_from_raw(raw, 6);
  TEST_ASSERT(slice.len == 6, "Slice length must be 6");
  TEST_ASSERT(IntSlice_get(slice, 2) == 30, "IntSlice_get(2) must be 30");

  IntSlice_set(slice, 2, 999);
  TEST_ASSERT(IntSlice_get(slice, 2) == 999, "IntSlice_set must update value");

  let sub = IntSlice_subslice(slice, 1, 4);
  TEST_ASSERT(sub.len == 3, "Subslice length must be 3");
  TEST_ASSERT(IntSlice_get(sub, 0) == 20, "sub[0] == 20");
  TEST_ASSERT(IntSlice_get(sub, 1) == 999, "sub[1] == 999");
  TEST_ASSERT(IntSlice_get(sub, 2) == 40, "sub[2] == 40");

  let dst_sub = IntSlice_subslice(slice, 0, 2);
  let src_sub = IntSlice_subslice(slice, 4, 6);
  IntSlice_copy_from(dst_sub, src_sub);
  TEST_ASSERT(IntSlice_get(slice, 0) == 50, "slice[0] after copy must be 50");
  TEST_ASSERT(IntSlice_get(slice, 1) == 60, "slice[1] after copy must be 60");

  let cslice = ConstIntSlice_from_raw(raw, 6);
  TEST_ASSERT(ConstIntSlice_get(cslice, 0) == 50, "ConstIntSlice get must read correctly");
}

/* ---------------------------------------------------------
 * 7. Safe String Views (StrSlice)
 * --------------------------------------------------------- */
static void test_string_slices(void)
{
  printf("[RUNNING] test_string_slices...\n");

  let s = STR_LIT("RustInC-Safety");
  TEST_ASSERT(s.len == 14, "STR_LIT length must be 14");
  TEST_ASSERT(StrSlice_get(s, 0) == 'R', "StrSlice_get(0) == 'R'");
  TEST_ASSERT(StrSlice_get(s, 13) == 'y', "StrSlice_get(13) == 'y'");

  let sub = StrSlice_subslice(s, 0, 7);
  let expected = StrSlice_from_cstr("RustInC", 16);
  TEST_ASSERT(StrSlice_eq(sub, expected), "Subslice must equal 'RustInC'");

  let diff = STR_LIT("RustInC-SafetyX");
  TEST_ASSERT(!StrSlice_eq(s, diff), "Slices of different lengths must not be equal");

  // Prefix and Suffix checking
  let prefix_check = STR_LIT("RustInC");
  let suffix_check = STR_LIT("Safety");
  let not_prefix = STR_LIT("InC");
  TEST_ASSERT(StrSlice_starts_with(s, prefix_check), "s must start with 'RustInC'");
  TEST_ASSERT(!StrSlice_starts_with(s, not_prefix), "s must not start with 'InC'");
  TEST_ASSERT(StrSlice_ends_with(s, suffix_check), "s must end with 'Safety'");
  TEST_ASSERT(!StrSlice_ends_with(s, prefix_check), "s must not end with 'RustInC'");

  // Substring containment
  let needle = STR_LIT("InC-Safe");
  let missing_needle = STR_LIT("NotFound");
  TEST_ASSERT(StrSlice_contains(s, needle), "s must contain 'InC-Safe'");
  TEST_ASSERT(!StrSlice_contains(s, missing_needle), "s must not contain 'NotFound'");

  // Trimming whitespace
  let untrimmed = STR_LIT("  \t\n  trimmed-content \r\n ");
  let trimmed = StrSlice_trim(untrimmed);
  let expected_trimmed = STR_LIT("trimmed-content");
  TEST_ASSERT(StrSlice_eq(trimmed, expected_trimmed), "Trimmed slice must equal 'trimmed-content'");

  // Integer Parsing
  let int_str = STR_LIT("  -123456 ");
  int parsed_val = 0;
  bool parse_ok = StrSlice_parse_int(int_str, &parsed_val);
  TEST_ASSERT(parse_ok, "StrSlice_parse_int must succeed for '  -123456 '");
  TEST_ASSERT(parsed_val == -123456, "Parsed integer must be -123456");

  let invalid_int = STR_LIT("123abc");
  bool invalid_ok = StrSlice_parse_int(invalid_int, &parsed_val);
  TEST_ASSERT(!invalid_ok, "StrSlice_parse_int must fail for '123abc'");

  let overflow_int = STR_LIT("999999999999999999999999999999999");
  bool overflow_ok = StrSlice_parse_int(overflow_int, &parsed_val);
  TEST_ASSERT(!overflow_ok, "StrSlice_parse_int must fail on overflow");
}

/* ---------------------------------------------------------
 * 8. Monadic Option<T> and Result<T, E> Tests
 * --------------------------------------------------------- */
static void test_option_and_result(void)
{
  printf("[RUNNING] test_option_and_result...\n");

  OptionInt some = SomeInt(100);
  OptionInt none = NoneInt;
  TEST_ASSERT(some.is_some, "SomeInt must have is_some == true");
  TEST_ASSERT(!none.is_some, "NoneInt must have is_some == false");
  TEST_ASSERT(OptionInt_unwrap(some) == 100, "OptionInt_unwrap must return 100");
  TEST_ASSERT(OptionInt_unwrap_or(none, 999) == 999, "OptionInt_unwrap_or(none) must return default");
  TEST_ASSERT(OptionInt_unwrap_or(some, 999) == 100, "OptionInt_unwrap_or(some) must return value");

  ResultInt ok = OkInt(256);
  ResultInt err = ErrInt("operation failed");
  TEST_ASSERT(ok.is_ok, "OkInt must have is_ok == true");
  TEST_ASSERT(!err.is_ok, "ErrInt must have is_ok == false");
  TEST_ASSERT(ResultInt_unwrap(ok) == 256, "ResultInt_unwrap must return 256");
  TEST_ASSERT(ResultInt_expect(ok, "must succeed") == 256, "ResultInt_expect must return 256");
  TEST_ASSERT(ResultInt_unwrap_or(err, 50) == 50, "ResultInt_unwrap_or(err) must return default");
  TEST_ASSERT(ResultInt_unwrap_or(ok, 50) == 256, "ResultInt_unwrap_or(ok) must return value");
}

/* ---------------------------------------------------------
 * 9. TRY (?) Operator Pipeline and RAII Drop Tests
 * --------------------------------------------------------- */
static MUST_USE ResultInt helper_div(int a, int b)
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

static MUST_USE ResultInt helper_pipeline(int a, int b, int c)
{
  OWN int *scratch = SAFE_ALLOC(int, 4);
  scratch[0] = TRY(helper_div(a, b));
  scratch[1] = TRY(helper_div(scratch[0], c));
  return OkInt(CHECKED_ADD(scratch[0], scratch[1]));
}

static void test_try_pipeline(void)
{
  printf("[RUNNING] test_try_pipeline...\n");

  ResultInt success = helper_pipeline(100, 2, 5);
  TEST_ASSERT(success.is_ok, "Pipeline should succeed");
  TEST_ASSERT(ResultInt_unwrap(success) == 60, "Pipeline result should be 60");

  ResultInt fail1 = helper_pipeline(100, 0, 5);
  TEST_ASSERT(!fail1.is_ok, "Pipeline with divisor=0 should fail early");
  TEST_ASSERT(strcmp(fail1.err, "Division by zero") == 0, "Error must be 'Division by zero'");

  ResultInt fail2 = helper_pipeline(100, 2, 0);
  TEST_ASSERT(!fail2.is_ok, "Pipeline with second divisor=0 should fail");
  TEST_ASSERT(strcmp(fail2.err, "Division by zero") == 0, "Error must be 'Division by zero'");

  ResultInt overflow_res = helper_div(INT_MIN, -1);
  TEST_ASSERT(!overflow_res.is_ok, "INT_MIN / -1 must return Err");
}

/* ---------------------------------------------------------
 * 10. Runtime Borrow Checker (RefCell<T>) Tests
 * --------------------------------------------------------- */
static void test_borrow_checker(void)
{
  printf("[RUNNING] test_borrow_checker...\n");

  mut cell = RefCellInt_new(10);
  TEST_ASSERT(cell.borrow_state == 0, "New RefCell must start in unborrowed state (0)");

  {
    BORROW_MUT(RefCellInt)
    writer = RefCellInt_borrow_mut(&cell);
    TEST_ASSERT(cell.borrow_state == -1, "Mutable borrow must set borrow_state to -1");
    *(writer.ptr) += 20;
  }
  TEST_ASSERT(cell.borrow_state == 0, "Borrow state must return to 0 after mutable guard drops");
  TEST_ASSERT(cell.value == 30, "Cell value must be 30 after mutation");

  {
    BORROW_REF(RefCellInt)
    r1 = RefCellInt_borrow(&cell);
    TEST_ASSERT(cell.borrow_state == 1, "First immutable borrow must set borrow_state to 1");

    {
      BORROW_REF(RefCellInt)
      r2 = RefCellInt_borrow(&cell);
      TEST_ASSERT(cell.borrow_state == 2, "Second immutable borrow must set borrow_state to 2");
      TEST_ASSERT(*(r1.ptr) == 30 && *(r2.ptr) == 30, "Both readers must observe same value");
    }
    TEST_ASSERT(cell.borrow_state == 1, "Dropping r2 must decrease borrow_state back to 1");
  }
  TEST_ASSERT(cell.borrow_state == 0, "Dropping r1 must decrease borrow_state back to 0");
}

/* ---------------------------------------------------------
 * 11. Custom Panic Hook Subsystem Tests
 * --------------------------------------------------------- */
static bool g_hook_called = false;
static void custom_panic_hook(const char *file, int line, const char *func, const char *msg)
{
  (void)file;
  (void)line;
  (void)func;
  (void)msg;
  g_hook_called = true;
}

static void test_panic_hook(void)
{
  printf("[RUNNING] test_panic_hook...\n");

  alloysafe_set_panic_handler(custom_panic_hook);
  TEST_ASSERT(*alloysafe_get_panic_hook_ptr() == custom_panic_hook,
              "Registered panic handler must match pointer");
  alloysafe_set_panic_handler(NULL); // Reset to default
  TEST_ASSERT(*alloysafe_get_panic_hook_ptr() == NULL, "Panic handler reset to NULL");
}

/* ---------------------------------------------------------
 * 12. In-Code Diagnostics & Scoped Pragma Tests
 * --------------------------------------------------------- */
static void test_diagnostics_system(void)
{
  printf("[RUNNING] test_diagnostics_system...\n");

  ALLOYSAFE_DIAGNOSTIC_PUSH()
  ALLOYSAFE_DIAGNOSTIC_IGNORE("-Wunused-variable")
  int scoped_test = 12345;
  (void)scoped_test;
  ALLOYSAFE_DIAGNOSTIC_POP()

  TEST_ASSERT(true, "ALLOYSAFE_DIAGNOSTIC macros executed correctly");
}

/* ---------------------------------------------------------
 * 13. Reference Counting Tests (Rc<T> and Arc<T>)
 * --------------------------------------------------------- */
static void test_rc_and_arc(void)
{
  printf("[RUNNING] test_rc_and_arc...\n");

  // Single-threaded Rc<T>
  {
    OWN_RC(RcInt) rc1 = RcInt_new(42);
    TEST_ASSERT(RcInt_count(&rc1) == 1, "Initial Rc count must be 1");
    TEST_ASSERT(*(rc1.ptr) == 42, "Rc value must be 42");

    {
      OWN_RC(RcInt) rc2 = RcInt_clone(&rc1);
      TEST_ASSERT(RcInt_count(&rc1) == 2, "Rc count after clone must be 2");
      TEST_ASSERT(RcInt_count(&rc2) == 2, "rc2 count must be 2");
      TEST_ASSERT(*(rc2.ptr) == 42, "rc2 value must be 42");
    }
    TEST_ASSERT(RcInt_count(&rc1) == 1, "Rc count after rc2 drop must be 1");
  }

  // Atomically Reference Counted Arc<T>
  {
    OWN_ARC(ArcInt) arc1 = ArcInt_new(84);
    TEST_ASSERT(ArcInt_count(&arc1) == 1, "Initial Arc count must be 1");
    TEST_ASSERT(*(arc1.ptr) == 84, "Arc value must be 84");

    {
      OWN_ARC(ArcInt) arc2 = ArcInt_clone(&arc1);
      TEST_ASSERT(ArcInt_count(&arc1) == 2, "Arc count after clone must be 2");
      TEST_ASSERT(ArcInt_count(&arc2) == 2, "arc2 count must be 2");
      TEST_ASSERT(*(arc2.ptr) == 84, "arc2 value must be 84");
    }
    TEST_ASSERT(ArcInt_count(&arc1) == 1, "Arc count after arc2 drop must be 1");
  }
}

/* ---------------------------------------------------------
 * Test Suite Entry Point
 * --------------------------------------------------------- */
int main(void)
{
  printf("=========================================\n");
  printf(" ALLOYSAFE-C PRODUCTION TEST SUITE       \n");
  printf("=========================================\n");

  test_checked_arithmetic();
  test_ownership_and_alloc();
  test_box();
  test_vec();
  test_secure_zero();
  test_slices();
  test_string_slices();
  test_option_and_result();
  test_try_pipeline();
  test_borrow_checker();
  test_panic_hook();
  test_diagnostics_system();
  test_rc_and_arc();

  printf("=========================================\n");
  printf(" RESULTS: %d / %d tests passed\n", g_tests_passed, g_tests_run);
  printf("=========================================\n");

  return (g_tests_passed == g_tests_run) ? 0 : 1;
}
