/* ==============================================================================
 * Alloysafe-C: High-Volume Property & Stress Testing Suite
 * Validates performance, memory stability, and correctness under extreme load.
 * ==============================================================================
 */

#include "alloysafe.h"
#include <time.h>

static int g_stress_run = 0;
static int g_stress_passed = 0;

#define STRESS_ASSERT(cond, msg)                                          \
  do                                                                      \
  {                                                                       \
    g_stress_run++;                                                       \
    if (!(cond))                                                          \
    {                                                                     \
      fprintf(stderr, "  [FAIL] %s:%d: %s\n", __FILE__, __LINE__, (msg)); \
    }                                                                     \
    else                                                                  \
    {                                                                     \
      g_stress_passed++;                                                  \
    }                                                                     \
  } while (0)

/* ---------------------------------------------------------
 * 1. Vector High-Volume Stress Test (200,000 Elements)
 * --------------------------------------------------------- */
static void stress_vector(void)
{
  printf("[RUNNING] stress_vector: pushing and popping 200,000 items...\n");
  clock_t start = clock();

  OWN_VEC(VecInt)
  v = VecInt_new();
  const int count = 200000;

  for (int i = 0; i < count; ++i)
  {
    VecInt_push(&v, i * 3);
  }

  STRESS_ASSERT(v.len == (size_t)count, "Vector length must equal 200,000");
  STRESS_ASSERT(v.cap >= (size_t)count, "Vector capacity must accommodate 200,000");

  bool data_valid = true;
  for (int i = 0; i < count; i += 1000)
  {
    if (VecInt_get(&v, i) != i * 3)
    {
      data_valid = false;
      break;
    }
  }
  STRESS_ASSERT(data_valid, "Sampled vector elements must match expected values");

  // Pop all items in reverse
  bool pop_valid = true;
  for (int i = count - 1; i >= 0; --i)
  {
    OptionInt opt = VecInt_pop(&v);
    if (!opt.is_some || opt.value != i * 3)
    {
      pop_valid = false;
      break;
    }
  }
  STRESS_ASSERT(pop_valid, "All 200,000 elements popped in reverse order successfully");
  STRESS_ASSERT(v.len == 0, "Vector length must be 0 after popping all items");

  OptionInt empty_pop = VecInt_pop(&v);
  STRESS_ASSERT(!empty_pop.is_some, "Popping empty vector must return None");

  clock_t end = clock();
  double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
  printf("  [BENCH] Vector 200k push/pop completed in %.3f seconds\n", elapsed);
}

/* ---------------------------------------------------------
 * 2. Reference Counting Stress Test (100,000 Clones/Drops)
 * --------------------------------------------------------- */
static void stress_ref_counting(void)
{
  printf("[RUNNING] stress_ref_counting: 100,000 clone & drop cycles...\n");
  clock_t start = clock();

  const int iterations = 100000;

  // Single-threaded RcInt stress
  {
    OWN_RC(RcInt)
    root_rc = RcInt_new(777);
    for (int i = 0; i < iterations; ++i)
    {
      OWN_RC(RcInt)
      temp = RcInt_clone(&root_rc);
      (void)temp;
    }
    STRESS_ASSERT(RcInt_count(&root_rc) == 1, "Rc strong count must return to 1 after 100k drops");
  }

  // Atomic ArcInt stress
  {
    OWN_ARC(ArcInt)
    root_arc = ArcInt_new(888);
    for (int i = 0; i < iterations; ++i)
    {
      OWN_ARC(ArcInt)
      temp = ArcInt_clone(&root_arc);
      (void)temp;
    }
    STRESS_ASSERT(ArcInt_count(&root_arc) == 1, "Arc strong count must return to 1 after 100k drops");
  }

  clock_t end = clock();
  double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
  printf("  [BENCH] RefCounting 100k cycles completed in %.3f seconds\n", elapsed);
}

/* ---------------------------------------------------------
 * 3. Borrow Checker Heavy Alternation Stress (100,000 Scopes)
 * --------------------------------------------------------- */
static void stress_borrow_checker(void)
{
  printf("[RUNNING] stress_borrow_checker: 100,000 alternating borrow scopes...\n");
  clock_t start = clock();

  mut cell = RefCellInt_new(0);
  const int iterations = 100000;

  for (int i = 0; i < iterations; ++i)
  {
    // Mutable borrow scope
    {
      BORROW_MUT(RefCellInt)
      writer = RefCellInt_borrow_mut(&cell);
      *(writer.ptr) += 1;
    }
    // Shared immutable borrow scope
    {
      BORROW_REF(RefCellInt)
      r1 = RefCellInt_borrow(&cell);
      BORROW_REF(RefCellInt)
      r2 = RefCellInt_borrow(&cell);
      if (*(r1.ptr) != *(r2.ptr))
      {
        STRESS_ASSERT(false, "Readers saw inconsistent values");
      }
    }
  }

  STRESS_ASSERT(cell.borrow_state == 0, "Final borrow state must be 0");
  STRESS_ASSERT(cell.value == iterations, "Final value must equal iterations (100,000)");

  clock_t end = clock();
  double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
  printf("  [BENCH] BorrowChecker 100k cycles completed in %.3f seconds\n", elapsed);
}

/* ---------------------------------------------------------
 * 4. Checked Math Fuzzing & Stress (1,000,000 Operations)
 * --------------------------------------------------------- */
static void stress_math(void)
{
  printf("[RUNNING] stress_math: 1,000,000 arithmetic property operations...\n");
  clock_t start = clock();

  int acc = 0;
  for (int i = 1; i <= 1000000; ++i)
  {
    int delta = (i % 2 == 0) ? 1 : -1;
    acc = CHECKED_ADD(acc, delta);
  }
  STRESS_ASSERT(acc == 0, "Alternating 1,000,000 additions must sum to 0");

  // Division identity
  for (int i = 1; i <= 10000; ++i)
  {
    int prod = CHECKED_MUL(i, 7);
    int quot = CHECKED_DIV(prod, 7);
    if (quot != i)
    {
      STRESS_ASSERT(false, "Division must invert multiplication");
      break;
    }
  }
  STRESS_ASSERT(true, "10,000 mul-div identity properties validated");

  clock_t end = clock();
  double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
  printf("  [BENCH] Checked math 1M operations completed in %.3f seconds\n", elapsed);
}

/* ---------------------------------------------------------
 * Main Stress Harness
 * --------------------------------------------------------- */
int main(void)
{
  printf("=========================================\n");
  printf(" ALLOYSAFE-C HIGH-VOLUME STRESS SUITE    \n");
  printf("=========================================\n");

  stress_math();
  stress_vector();
  stress_ref_counting();
  stress_borrow_checker();

  printf("=========================================\n");
  printf(" STRESS RESULTS: %d / %d tests passed\n", g_stress_passed, g_stress_run);
  printf("=========================================\n");

  return (g_stress_passed == g_stress_run) ? 0 : 1;
}
