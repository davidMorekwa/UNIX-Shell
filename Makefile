# Makefile for the mysh project
#
# Everyday use (run these from the project folder):
#   make test1        build + run the Milestone 1 tests
#   make test2        build + run the Milestone 2 tests
#   make test3        build + run the Milestone 3 tests
#   make test         all three
#   make lint         check the milestone-specific coding rules
#   make asan2        run the Milestone 2 tests with AddressSanitizer + UBSan
#   make valgrind2    run the Milestone 2 tests under valgrind
#   make              build the shell itself (build/mysh)
#   make clean
#
# Instructors can run the same targets against the reference solution:
#   make SRCDIR=solution test

CC      ?= gcc
SRCDIR  ?= src

# SAN=1 builds everything with AddressSanitizer + UndefinedBehaviorSanitizer
# into a separate directory (build-san) so the two builds never mix.
SAN ?= 0
ifeq ($(SAN),1)
BUILD    := build-san
SANFLAGS := -fsanitize=address,undefined -fno-omit-frame-pointer
else
BUILD    := build
SANFLAGS :=
endif

CFLAGS  := -std=gnu11 -Wall -Wextra -Werror -g -O0 -Iinclude $(SANFLAGS)
LDFLAGS := $(SANFLAGS)

# Lets the tests make malloc fail on purpose (GNU ld feature; Linux/WSL only).
WRAP := -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=realloc

M1_MODS := command shell_state builtin input
M2_MODS := mystring strvec lexer parser joblist
M3_MODS := iohelp executor
ALL_MODS := $(M1_MODS) $(M2_MODS) $(M3_MODS)

obj = $(addprefix $(BUILD)/,$(addsuffix .o,$(1)))
HEADERS := $(wildcard include/*.h) tests/testlib.h

.PHONY: all test test1 test2 test3 myTests lint clean \
        asan1 asan2 asan3 valgrind1 valgrind2 valgrind3

all: $(BUILD)/mysh

# ---- compiling -------------------------------------------------------------
$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: $(SRCDIR)/%.c $(HEADERS) | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: tests/%.c $(HEADERS) | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

# ---- programs --------------------------------------------------------------
$(BUILD)/mysh: $(call obj,main $(ALL_MODS))
	$(CC) $(LDFLAGS) $^ -o $@

# Milestone 1 only needs the Milestone 1 modules (so Week 1 builds even
# while the later modules are still empty stubs).
$(BUILD)/mysh_m1: $(call obj,main $(M1_MODS))
	$(CC) $(LDFLAGS) $^ -o $@

$(BUILD)/test_m1: $(call obj,test_m1 alloc_fail $(M1_MODS))
	$(CC) $(LDFLAGS) $(WRAP) $^ -o $@

$(BUILD)/test_m2: $(call obj,test_m2 alloc_fail $(M1_MODS) $(M2_MODS))
	$(CC) $(LDFLAGS) $(WRAP) $^ -o $@

$(BUILD)/test_m3: $(call obj,test_m3 alloc_fail $(M1_MODS) $(M3_MODS))
	$(CC) $(LDFLAGS) $(WRAP) $^ -o $@

# ---- running the tests -----------------------------------------------------
test1: $(BUILD)/test_m1 $(BUILD)/mysh_m1
	./$(BUILD)/test_m1
	sh tests/integration_m1.sh ./$(BUILD)/mysh_m1

myTests: $(BUILD)/test_m2
	TEST=$(TEST) ./$(BUILD)/test_m2

test2: $(BUILD)/test_m2
	./$(BUILD)/test_m2

test3: $(BUILD)/test_m3
	./$(BUILD)/test_m3

test: lint test1 test2 test3

lint:
	sh tests/lint.sh $(SRCDIR)



# ---- memory checking -------------------------------------------------------
asan1:
	$(MAKE) SAN=1 test1
asan2:
	$(MAKE) SAN=1 test2
asan3:
	$(MAKE) SAN=1 test3

VG = valgrind -q --leak-check=full --errors-for-leak-kinds=definite --error-exitcode=99

valgrind1: $(BUILD)/test_m1
	TEST_TIMEOUT=120 $(VG) ./$(BUILD)/test_m1
valgrind2: $(BUILD)/test_m2
	TEST_TIMEOUT=120 $(VG) ./$(BUILD)/test_m2
valgrind3: $(BUILD)/test_m3
	TEST_TIMEOUT=120 $(VG) ./$(BUILD)/test_m3

clean:
	rm -rf build build-san
