// Baremetal / Freestanding test file (NO stdlib.h or stdio.h available)
#ifndef ALLOYSAFE_BAREMETAL
#define ALLOYSAFE_BAREMETAL 1
#endif

#include "alloysafe.h"

// Custom baremetal panic hook counter
static int g_baremetal_panics = 0;
static void baremetal_panic_handler(const char *file, int line, const char *func, const char *msg)
{
  (void)file;
  (void)line;
  (void)func;
  (void)msg;
  g_baremetal_panics++;
}

// Baremetal arithmetic and monadic return
static MUST_USE ResultInt baremetal_calculate(int sensor_a, int sensor_b)
{
  if (sensor_b == 0)
  {
    return ErrInt("Sensor B reading invalid: zero divisor");
  }
  let sum = CHECKED_ADD(sensor_a, 10);
  let quot = CHECKED_DIV(sum, sensor_b);
  return OkInt(quot);
}

int baremetal_entry(void)
{
  alloysafe_set_panic_handler(baremetal_panic_handler);

  // 1. Immutability & Checked Math
  let reading1 = 50;
  let reading2 = 5;
  ResultInt res = baremetal_calculate(reading1, reading2);
  if (!res.is_ok)
  {
    return -1;
  }
  let val = ResultInt_unwrap(res); // (50 + 10) / 5 = 12

  // 2. Static / Stack Memory Bounds-Checked Slices
  int stack_telemetry[4] = {val, 24, 36, 48};
  let telemetry_slice = IntSlice_from_raw(stack_telemetry, 4);
  IntSlice_set(telemetry_slice, 0, 99);
  let first_reading = IntSlice_get(telemetry_slice, 0);

  // 3. Const Slices
  let const_view = ConstIntSlice_from_raw(stack_telemetry, 4);
  if (ConstIntSlice_get(const_view, 0) != 99)
  {
    return -2;
  }

  // 4. Safe String Views
  let hardware_tag = STR_LIT("CORTEX-M4-FREESTANDING");
  let prefix = StrSlice_subslice(hardware_tag, 0, 9); // "CORTEX-M4"
  (void)prefix;

  // 5. Secure Volatile Hardware Register/Buffer Zeroing
  uint8_t crypto_key[8] = {0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x02, 0x03, 0x04};
  safe_memzero_volatile(crypto_key, sizeof(crypto_key));

  // 6. RefCell Interior Mutability in Baremetal
  mut cell = RefCellInt_new(first_reading);
  {
    BORROW_MUT(RefCellInt)
    writer = RefCellInt_borrow_mut(&cell);
    *(writer.ptr) += 1; // 99 + 1 = 100
  }
  BORROW_REF(RefCellInt)
  reader = RefCellInt_borrow(&cell);

  return *(reader.ptr) == 100 ? 0 : -3;
}
