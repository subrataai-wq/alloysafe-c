/* ==============================================================================
 * Alloysafe-C: Scientific Performance Benchmarking Suite
 * Measures overhead of safety abstractions against raw, unchecked C primitives.
 * ==============================================================================
 */

#include "alloysafe.h"
#include <time.h>

#define N_ITER 10000000 // 10 million iterations

static double get_time_sec(void)
{
  return (double)clock() / CLOCKS_PER_SEC;
}

/* ---------------------------------------------------------
 * Benchmark 1: Raw vs Checked Addition
 * --------------------------------------------------------- */
static void bench_math(void)
{
  printf("| Arithmetic (10M ops)           | ");

  // Raw C
  volatile int raw_sum = 0;
  double t0 = get_time_sec();
  for (int i = 0; i < N_ITER; ++i)
  {
    raw_sum += (i & 1);
  }
  double t_raw = get_time_sec() - t0;
  (void)raw_sum;

  // Checked Add
  volatile int safe_sum = 0;
  double t1 = get_time_sec();
  for (int i = 0; i < N_ITER; ++i)
  {
    safe_sum = CHECKED_ADD(safe_sum, (i & 1));
  }
  double t_safe = get_time_sec() - t1;
  (void)safe_sum;

  printf("Raw: %.3fs (%.1f ns/op) | Safe: %.3fs (%.1f ns/op) |\n",
         t_raw, (t_raw * 1e9) / N_ITER,
         t_safe, (t_safe * 1e9) / N_ITER);
}

/* ---------------------------------------------------------
 * Benchmark 2: Raw Pointer vs Bounds-Checked Slice Access
 * --------------------------------------------------------- */
static void bench_slice(void)
{
  printf("| Slice Indexing (10M reads)     | ");

  OWN int *buf = SAFE_ALLOC(int, 1024);
  for (int i = 0; i < 1024; ++i)
    buf[i] = i;

  // Raw C Pointer
  volatile int raw_acc = 0;
  double t0 = get_time_sec();
  for (int i = 0; i < N_ITER; ++i)
  {
    raw_acc += buf[i & 1023];
  }
  double t_raw = get_time_sec() - t0;
  (void)raw_acc;

  // Bounds-Checked Slice
  let s = IntSlice_from_raw(buf, 1024);
  volatile int safe_acc = 0;
  double t1 = get_time_sec();
  for (int i = 0; i < N_ITER; ++i)
  {
    safe_acc += IntSlice_get(s, i & 1023);
  }
  double t_safe = get_time_sec() - t1;
  (void)safe_acc;

  printf("Raw: %.3fs (%.1f ns/op) | Safe: %.3fs (%.1f ns/op) |\n",
         t_raw, (t_raw * 1e9) / N_ITER,
         t_safe, (t_safe * 1e9) / N_ITER);
}

/* ---------------------------------------------------------
 * Benchmark 3: RefCell Dynamic Borrowing & Scope Release
 * --------------------------------------------------------- */
static void bench_borrow(void)
{
  printf("| RefCell Borrow Guard (5M)      | ");

  mut cell = RefCellInt_new(10);
  const int n = 5000000;

  double t0 = get_time_sec();
  for (int i = 0; i < n; ++i)
  {
    BORROW_REF(RefCellInt)
    r = RefCellInt_borrow(&cell);
    (void)r;
  }
  double t_elapsed = get_time_sec() - t0;

  printf("N/A (C has no borrow)  | Safe: %.3fs (%.1f ns/op) |\n",
         t_elapsed, (t_elapsed * 1e9) / n);
}

/* ---------------------------------------------------------
 * Benchmark 4: Atomic Arc Clone & Drop Throughput
 * --------------------------------------------------------- */
static void bench_arc(void)
{
  printf("| Arc<T> Atomic Clone/Drop (5M)  | ");

  OWN_ARC(ArcInt)
  root = ArcInt_new(100);
  const int n = 5000000;

  double t0 = get_time_sec();
  for (int i = 0; i < n; ++i)
  {
    OWN_ARC(ArcInt)
    handle = ArcInt_clone(&root);
    (void)handle;
  }
  double t_elapsed = get_time_sec() - t0;

  printf("N/A (Raw C unchecked)  | Safe: %.3fs (%.1f ns/op) |\n",
         t_elapsed, (t_elapsed * 1e9) / n);
}

/* ---------------------------------------------------------
 * Benchmark 5: Vec<T> Sequential Pushes
 * --------------------------------------------------------- */
static void bench_vec(void)
{
  printf("| Vec<T> Pushes (1M items)       | ");

  const int n = 1000000;

  double t0 = get_time_sec();
  {
    OWN_VEC(VecInt)
    v = VecInt_new();
    for (int i = 0; i < n; ++i)
    {
      VecInt_push(&v, i);
    }
  }
  double t_elapsed = get_time_sec() - t0;

  printf("N/A                    | Safe: %.3fs (%.1f ns/push) |\n",
         t_elapsed, (t_elapsed * 1e9) / n);
}

int main(void)
{
  printf("=========================================================================\n");
  printf(" ALLOYSAFE-C SCIENTIFIC PERFORMANCE BENCHMARK                           \n");
  printf(" Compiled with GCC -O2 optimization on modern x86_64                     \n");
  printf("=========================================================================\n");
  printf("| Benchmark Operation            | Performance Metrics                  |\n");
  printf("|--------------------------------|--------------------------------------|\n");

  bench_math();
  bench_slice();
  bench_borrow();
  bench_arc();
  bench_vec();

  printf("=========================================================================\n");
  return 0;
}
