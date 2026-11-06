/* ==============================================================================
 * Alloysafe-C: Subprocess Panic & Invariant Violation Verification Suite
 * Verifies that all 22 safety invariant violations properly abort and halt
 * execution rather than proceeding with undefined behavior or memory corruption.
 * ==============================================================================
 */

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#define ALLOYSAFE_ALLOW_UNUSED_RESULT 1
#include "alloysafe.h"

typedef struct
{
  const char *name;
  const char *description;
  void (*trigger_fn)(void);
} PanicCase;

/* ---------------------------------------------------------
 * Individual Panic Trigger Cases
 * --------------------------------------------------------- */
static void case_div_zero(void)
{
  int x = 100;
  int y = 0;
  volatile int res = CHECKED_DIV(x, y);
  (void)res;
}

static void case_div_overflow(void)
{
  int x = INT_MIN;
  int y = -1;
  volatile int res = CHECKED_DIV(x, y);
  (void)res;
}

static void case_add_overflow(void)
{
  int x = INT_MAX;
  int y = 1;
  volatile int res = CHECKED_ADD(x, y);
  (void)res;
}

static void case_sub_overflow(void)
{
  uint32_t x = 0U;
  uint32_t y = 1U;
  volatile uint32_t res = CHECKED_SUB(x, y);
  (void)res;
}

static void case_mul_overflow(void)
{
  int x = INT_MAX;
  int y = 2;
  volatile int res = CHECKED_MUL(x, y);
  (void)res;
}

static void case_slice_oob_get(void)
{
  int raw[2] = {10, 20};
  let s = IntSlice_from_raw(raw, 2);
  volatile int val = IntSlice_get(s, 2); // Index out of bounds (len is 2)
  (void)val;
}

static void case_slice_oob_set(void)
{
  int raw[2] = {10, 20};
  let s = IntSlice_from_raw(raw, 2);
  IntSlice_set(s, 2, 999);
}

static void case_slice_null_with_len(void)
{
  IntSlice_from_raw(NULL, 10);
}

static void case_slice_subslice_invalid(void)
{
  int raw[4] = {1, 2, 3, 4};
  let s = IntSlice_from_raw(raw, 4);
  IntSlice_subslice(s, 3, 2); // start > end
}

static void case_slice_subslice_oob(void)
{
  int raw[4] = {1, 2, 3, 4};
  let s = IntSlice_from_raw(raw, 4);
  IntSlice_subslice(s, 0, 5); // end > len
}

static void case_str_oob(void)
{
  let s = STR_LIT("test");
  volatile char c = StrSlice_get(s, 4); // len is 4
  (void)c;
}

static void case_str_unterminated(void)
{
  StrSlice_from_cstr("unterminated", 3);
}

static void case_str_subslice_invalid(void)
{
  let s = STR_LIT("hello");
  StrSlice_subslice(s, 4, 2); // start > end
}

static void case_option_unwrap_none(void)
{
  OptionInt_unwrap(NoneInt);
}

static void case_result_unwrap_err(void)
{
  ResultInt_unwrap(ErrInt("forced error"));
}

static void case_result_expect_err(void)
{
  ResultInt_expect(ErrInt("forced error"), "custom expect message");
}

static void case_refcell_double_mut(void)
{
  mut c = RefCellInt_new(10);
  BORROW_MUT(RefCellInt) m1 = RefCellInt_borrow_mut(&c);
  BORROW_MUT(RefCellInt) m2 = RefCellInt_borrow_mut(&c);
  (void)m1; (void)m2;
}

static void case_refcell_mut_while_ref(void)
{
  mut c = RefCellInt_new(10);
  BORROW_REF(RefCellInt) r = RefCellInt_borrow(&c);
  BORROW_MUT(RefCellInt) m = RefCellInt_borrow_mut(&c);
  (void)r; (void)m;
}

static void case_refcell_ref_while_mut(void)
{
  mut c = RefCellInt_new(10);
  BORROW_MUT(RefCellInt) m = RefCellInt_borrow_mut(&c);
  BORROW_REF(RefCellInt) r = RefCellInt_borrow(&c);
  (void)m; (void)r;
}

static void case_vec_oob_get(void)
{
  OWN_VEC(VecInt) v = VecInt_new();
  VecInt_push(&v, 10);
  volatile int val = VecInt_get(&v, 5);
  (void)val;
}

static void case_vec_oob_set(void)
{
  OWN_VEC(VecInt) v = VecInt_new();
  VecInt_push(&v, 10);
  VecInt_set(&v, 5, 999);
}

static void case_alloc_size_overflow(void)
{
  safe_calloc_checked(SIZE_MAX, 2);
}

static const PanicCase g_cases[] = {
  {"div_zero", "Division by zero", case_div_zero},
  {"div_overflow", "Signed division overflow (MIN / -1)", case_div_overflow},
  {"add_overflow", "Integer addition overflow", case_add_overflow},
  {"sub_overflow", "Unsigned integer subtraction underflow", case_sub_overflow},
  {"mul_overflow", "Integer multiplication overflow", case_mul_overflow},
  {"slice_oob_get", "Slice get index out of bounds", case_slice_oob_get},
  {"slice_oob_set", "Slice set index out of bounds", case_slice_oob_set},
  {"slice_null_with_len", "Slice from NULL with non-zero length", case_slice_null_with_len},
  {"slice_subslice_invalid", "Slice subslice invalid range (start > end)", case_slice_subslice_invalid},
  {"slice_subslice_oob", "Slice subslice range exceeds length", case_slice_subslice_oob},
  {"str_oob", "StrSlice get byte index out of bounds", case_str_oob},
  {"str_unterminated", "StrSlice unterminated C-string exceeds max_capacity", case_str_unterminated},
  {"str_subslice_invalid", "StrSlice subslice invalid range", case_str_subslice_invalid},
  {"option_unwrap_none", "Option unwrap on None value", case_option_unwrap_none},
  {"result_unwrap_err", "Result unwrap on Err value", case_result_unwrap_err},
  {"result_expect_err", "Result expect on Err value", case_result_expect_err},
  {"refcell_double_mut", "RefCell mutable borrow while mutably borrowed", case_refcell_double_mut},
  {"refcell_mut_while_ref", "RefCell mutable borrow while immutably borrowed", case_refcell_mut_while_ref},
  {"refcell_ref_while_mut", "RefCell immutable borrow while mutably borrowed", case_refcell_ref_while_mut},
  {"vec_oob_get", "Vec get index out of bounds", case_vec_oob_get},
  {"vec_oob_set", "Vec set index out of bounds", case_vec_oob_set},
  {"alloc_size_overflow", "Heap allocation byte size overflow", case_alloc_size_overflow},
};

static const size_t g_num_cases = sizeof(g_cases) / sizeof(g_cases[0]);

/* ---------------------------------------------------------
 * Main Harness: Child Dispatch & Parent Assertion Runner
 * --------------------------------------------------------- */
int main(int argc, char **argv)
{
#ifdef _WIN32
  _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
  SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
#endif

  // Child process execution mode
  if (argc > 1 && strncmp(argv[1], "--case=", 7) == 0)
  {
    const char *target = argv[1] + 7;
    // Silence stderr in child so test logs remain clean
    freopen("NUL", "w", stderr);

    for (size_t i = 0; i < g_num_cases; ++i)
    {
      if (strcmp(g_cases[i].name, target) == 0)
      {
        g_cases[i].trigger_fn();
        // If trigger_fn returns without aborting, exit normally with 0 (which parent treats as failure)
        return 0;
      }
    }
    return 0; // Unknown case
  }

  // Parent test runner mode
  printf("=========================================\n");
  printf(" ALLOYSAFE-C SUBPROCESS PANIC TEST SUITE \n");
  printf(" Verifying abort on invariant violations \n");
  printf("=========================================\n");

  const char *exe_path = argv[0];
  int passed = 0;

  for (size_t i = 0; i < g_num_cases; ++i)
  {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "\"%s\" --case=%s", exe_path, g_cases[i].name);

    int exit_status = system(cmd);

    // Any non-zero exit indicates process aborted as required
    if (exit_status != 0)
    {
      printf("  [PASS] %-25s : %s (aborted safely)\n", g_cases[i].name, g_cases[i].description);
      passed++;
    }
    else
    {
      printf("  [FAIL] %-25s : DID NOT ABORT!\n", g_cases[i].name);
    }
  }

  printf("=========================================\n");
  printf(" RESULTS: %d / %zu panic cases verified\n", passed, g_num_cases);
  printf("=========================================\n");

  return (passed == (int)g_num_cases) ? 0 : 1;
}
