# ==============================================================================
# ALLOYSAFE-C PRODUCTION MAKEFILE
# ==============================================================================

# Compiler & Standard
CC ?= gcc
STD ?= -std=gnu11

# ------------------------------------------------------------------------------
# Configurable Warning Flags
# Users can toggle WARN_ERROR=0 or override WARNING_FLAGS / EXTRA_FLAGS:
#   e.g.: make WARN_ERROR=0
#   e.g.: make EXTRA_FLAGS="-fsanitize=address"
# ------------------------------------------------------------------------------
WARN_ERROR ?= 1

# Modular Warning Flags: easily add or remove specific checks
BASE_WARNINGS ?= -Wall -Wextra -Wshadow -Wpointer-arith
GNU_EXT_WARNINGS ?= -Wno-pedantic

WARNING_FLAGS ?= $(BASE_WARNINGS) $(GNU_EXT_WARNINGS)
ifeq ($(WARN_ERROR), 1)
  WARNING_FLAGS += -Werror
endif

# User Extra Flags (e.g. EXTRA_FLAGS="-g")
EXTRA_FLAGS ?=

# Optimization & Debug Flags
OPT_FLAGS ?= -O2

# Include Directories
INCLUDES ?= -Iinclude -I.

# Aggregate CFLAGS
CFLAGS = $(STD) $(OPT_FLAGS) $(WARNING_FLAGS) $(INCLUDES) $(EXTRA_FLAGS)

# ------------------------------------------------------------------------------
# Build Directories & Artifacts
# ------------------------------------------------------------------------------
BUILD_DIR ?= build
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin

TARGET_MAIN = $(BIN_DIR)/main.exe
TARGET_TEST = $(BIN_DIR)/test_runner.exe
TARGET_WARN = $(BIN_DIR)/test_warnings.exe
TARGET_PANIC = $(BIN_DIR)/test_panics.exe
TARGET_STRESS = $(BIN_DIR)/test_stress.exe
TARGET_BENCH = $(BIN_DIR)/bench_alloysafe.exe

SRC_MAIN = main.c
SRC_TEST = tests/test_alloysafe.c
SRC_WARN = tests/test_warnings_toggle.c
SRC_PANIC = tests/test_panics.c
SRC_STRESS = tests/test_stress.c
SRC_BENCH = benchmarks/bench_alloysafe.c

OBJ_MAIN = $(OBJ_DIR)/main.o
OBJ_TEST = $(OBJ_DIR)/test_alloysafe.o
OBJ_WARN = $(OBJ_DIR)/test_warnings_toggle.o
OBJ_PANIC = $(OBJ_DIR)/test_panics.o
OBJ_STRESS = $(OBJ_DIR)/test_stress.o
OBJ_BENCH = $(OBJ_DIR)/bench_alloysafe.o

HEADERS = include/alloysafe.h \
          include/alloysafe/warnings.h \
          include/alloysafe/env.h \
          include/alloysafe/core.h \
          include/alloysafe/types.h \
          include/alloysafe/math.h \
          include/alloysafe/mem.h \
          include/alloysafe/box.h \
          include/alloysafe/slice.h \
          include/alloysafe/str.h \
          include/alloysafe/monads.h \
          include/alloysafe/cell.h \
          include/alloysafe/rc.h \
          include/alloysafe/arc.h \
          include/alloysafe/vec.h \
          rust_in_c.h

# ------------------------------------------------------------------------------
# Build Targets
# ------------------------------------------------------------------------------
.PHONY: all clean run test debug release baremetal-check help

all: directories $(TARGET_MAIN) $(TARGET_TEST) $(TARGET_WARN) $(TARGET_PANIC) $(TARGET_STRESS)

directories:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR)

# Compilation Rules
$(OBJ_DIR)/%.o: %.c $(HEADERS) | directories
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/test_alloysafe.o: tests/test_alloysafe.c $(HEADERS) | directories
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/test_warnings_toggle.o: tests/test_warnings_toggle.c $(HEADERS) | directories
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/test_panics.o: tests/test_panics.c $(HEADERS) | directories
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/test_stress.o: tests/test_stress.c $(HEADERS) | directories
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/bench_alloysafe.o: benchmarks/bench_alloysafe.c $(HEADERS) | directories
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Linking Rules
$(TARGET_MAIN): $(OBJ_MAIN)
	$(CC) $(CFLAGS) $^ -o $@

$(TARGET_TEST): $(OBJ_TEST)
	$(CC) $(CFLAGS) $^ -o $@

$(TARGET_WARN): $(OBJ_WARN)
	$(CC) $(CFLAGS) $^ -o $@

$(TARGET_PANIC): $(OBJ_PANIC)
	$(CC) $(CFLAGS) $^ -o $@

$(TARGET_STRESS): $(OBJ_STRESS)
	$(CC) $(CFLAGS) $^ -o $@

$(TARGET_BENCH): $(OBJ_BENCH)
	$(CC) $(CFLAGS) $^ -o $@

# Execution Rules
run: $(TARGET_MAIN)
	./$(TARGET_MAIN)

bench: $(TARGET_BENCH)
	./$(TARGET_BENCH)

test: $(TARGET_TEST) $(TARGET_WARN) $(TARGET_PANIC) $(TARGET_STRESS)
	./$(TARGET_TEST)
	./$(TARGET_WARN)
	./$(TARGET_PANIC)
	./$(TARGET_STRESS)

test-stress: $(TARGET_STRESS)
	./$(TARGET_STRESS)

# Debug Build
debug:
	$(MAKE) OPT_FLAGS="-O0 -g -DDEBUG" WARN_ERROR=1 all

# Release Build (Aggressive optimization)
release:
	$(MAKE) OPT_FLAGS="-O3 -DNDEBUG" WARN_ERROR=1 all

# Baremetal / Freestanding Verification Check
baremetal-check: directories
	$(CC) -std=gnu11 -ffreestanding -Wall -Wextra -Werror -Iinclude -DALLOYSAFE_BAREMETAL -c tests/test_baremetal.c -o $(OBJ_DIR)/baremetal_check.o
	@echo "[SUCCESS] Alloysafe-C compiles cleanly in baremetal/freestanding mode!"

# Clean Build Directory
clean:
	rm -rf $(BUILD_DIR) *.o *.exe

help:
	@echo "Available Targets:"
	@echo "  make               - Build main and test runner in build/bin/"
	@echo "  make run           - Run main application"
	@echo "  make test          - Run test runner"
	@echo "  make debug         - Build with debug symbols and no optimization"
	@echo "  make release       - Build with -O3 optimization"
	@echo "  make baremetal-check - Verify baremetal mode compilation"
	@echo "  make clean         - Remove build directory"
	@echo "Options:"
	@echo "  WARN_ERROR=0       - Disable treating warnings as errors"
	@echo "  EXTRA_FLAGS='...'  - Pass additional compiler flags"
