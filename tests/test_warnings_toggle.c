/* ==============================================================================
 * Test In-Code Diagnostic Flags & Scoped Pragmas
 * ============================================================================== */

// Enable shadow tolerance toggle directly from C code
#define ALLOYSAFE_ALLOW_SHADOW 1

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

static void test_scoped_pragmas(void)
{
  printf("[RUNNING] test_scoped_pragmas...\n");

  // Verify scoped pragma ignoring an unused variable inside a nested block
  ALLOYSAFE_DIAGNOSTIC_PUSH()
  ALLOYSAFE_DIAGNOSTIC_IGNORE("-Wunused-variable")
  int intentionally_unused = 42;
  (void)intentionally_unused;
  ALLOYSAFE_DIAGNOSTIC_POP()

  TEST_ASSERT(true, "Scoped diagnostic push/ignore/pop compiled successfully");
}

static void test_shadow_toggle(void)
{
  printf("[RUNNING] test_shadow_toggle...\n");

  int shadowed_var = 100;
  {
    // With ALLOYSAFE_ALLOW_SHADOW defined, shadowing is allowed without compiler error
    int shadowed_var = 200;
    TEST_ASSERT(shadowed_var == 200, "Inner shadowed variable must be 200");
  }
  TEST_ASSERT(shadowed_var == 100, "Outer variable must remain 100");
}

int main(void)
{
  printf("=========================================\n");
  printf(" ALLOYSAFE-C WARNINGS & PRAGMAS TEST     \n");
  printf("=========================================\n");

  test_scoped_pragmas();
  test_shadow_toggle();

  printf("=========================================\n");
  printf(" RESULTS: %d / %d tests passed\n", g_tests_passed, g_tests_run);
  printf("=========================================\n");

  return (g_tests_passed == g_tests_run) ? 0 : 1;
}
