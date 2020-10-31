/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

/* The ARM backend of the gcc compiler generates calls to out-of-line
 * divide and modulo functions for non-trivial cases. This is due to
 * ARM lacking a hardware integer divide instruction in most
 * implementations. These magic function names may be different in
 * other compilers.
 */

#if defined(TEST) || (defined(__GNUC__) && defined(__arm__))

#include <stdint.h>

#ifdef TEST
static void panic(const char *reason); // Logs an error rather than exit.
#else
#include <sys.h>
#endif

typedef uint64_t (*func_u64)(uint64_t num, uint64_t den);

uint64_t __udivdi3(uint64_t num, uint64_t den)
{
  /* Algorithm:
   *
   * Remove factors of 2, then use binary long multiplication to find
   * the greatest integer that can be multiplied by the denonimator
   * and have a result less than or equal to the numerator. The answer
   * bits are shifted in from the right. This is optimized to reduce
   * the number of whole-64-bit operations, with the aim of reducing
   * code size, not improving performance.
   */
  uint64_t answer = 0;
  uint64_t product = 0;
  if (den == 0) {
    panic("Divide by zero");
    return 0;
  }
  // Remove factors of 2.
  while ((den & 1) == 0) {
    den >>= 1;
    num >>= 1;
  }
  // Shift left 'den' all the way to the MSB.
  while ((den & (1ULL << 63)) == 0) den <<= 1;
  for (;;) {
    // Long multiplication step. Try adding 'den' at its current shift.
    uint64_t new_product = product + den;
    answer <<= 1;
    if (new_product > product && new_product <= num) {
      // No overflow and product is less than 'num'. This bit must be set.
      product = new_product; // Keep track of the running product.
      answer |= 1;
    }
    if (den & 1) break; // Not calculating fractions, so that was the last step.
    den >>= 1;
  }
  return answer;
}

uint64_t __udivmoddi4(uint64_t num, uint64_t den, uint64_t *rem)
{
  uint64_t answer = __udivdi3(num, den);
  *rem = num - answer * den;
  return answer;
}

uint64_t __umoddi3(uint64_t num, uint64_t den)
{
  uint64_t rem;
  (void) __udivmoddi4(num, den, &rem);
  return rem;
}

static int64_t apply_signed(int64_t num, int64_t den, func_u64 func, int mod)
{
  int negate = 0;
  uint64_t answer;
  if (num < 0) {
    num = -num;
    negate ^= 1;
  }
  if (den < 0) {
    den = -den;
    if (!mod) negate ^= 1; // Modulo denominator sign does not affect result.
  }
  answer = func((uint64_t) num, (uint64_t) den);
  if (negate) answer = -answer;
  return answer;  
}

int64_t __divdi3(int64_t num, int64_t den)
{
  if (num == INT64_MIN && den == -1) return INT64_MIN; // overflow
  else return apply_signed(num, den, __udivdi3, 0);
}

int64_t __moddi3(int64_t num, int64_t den)
{
  return apply_signed(num, den, __umoddi3, 1);
}

int64_t __divmoddi4(int64_t num, int64_t den, int64_t *rem)
{
  *rem = __moddi3(num, den);
  return __divdi3(num, den);
}

uint32_t __udivsi3(uint32_t num, uint32_t den)
{
  return (uint32_t) __udivdi3((uint64_t) num, (uint64_t) den);
}

uint32_t __umodsi3(uint32_t num, uint32_t den)
{
  return (uint32_t) __umoddi3((uint64_t) num, (uint64_t) den);
}

uint32_t __udivmodsi4(uint32_t num, uint32_t den, uint32_t *rem)
{
  *rem = __umodsi3(num, den);
  return __udivsi3(num, den);
}

int32_t __divsi3(int32_t num, int32_t den)
{
  if (num == INT32_MIN && den == -1) return INT32_MIN; // overflow
  else return (int32_t) apply_signed((int64_t) num, (int64_t) den, __udivdi3, 0);
}

int32_t __modsi3(int32_t num, int32_t den)
{
  return (int32_t) apply_signed((int64_t) num, (int64_t) den, __umoddi3, 1);
}

int32_t __divmodsi4(int32_t num, int32_t den, int32_t *rem)
{
  *rem = __modsi3(num, den);
  return __divsi3(num, den);
}

#endif // defined(TEST) || (defined(__GNUC__) && defined(__arm__))

////////////////////////////////////////////////////////////////////////

#ifdef TEST
/*
 * Tests:
gcc -arch i386 -g -Wall -o builtin_divide builtin_divide.c -DTEST && ./builtin_divide
gcc -arch x86_64 -g -Wall -o builtin_divide builtin_divide.c -DTEST && ./builtin_divide
*/

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

static unsigned tested = 0;
static unsigned divide_error_count = 0;

// func_u64 defined in common section above.
typedef int64_t (*func_s64)(int64_t num, int64_t den);
typedef uint64_t (*func_udm64)(uint64_t num, uint64_t den, uint64_t *rem);
typedef int64_t (*func_sdm64)(int64_t num, int64_t den, int64_t *rem);

typedef uint32_t (*func_u32)(uint32_t num, uint32_t den);
typedef int32_t (*func_s32)(int32_t num, int32_t den);
typedef uint32_t (*func_udm32)(uint32_t num, uint32_t den, uint32_t *rem);
typedef int32_t (*func_sdm32)(int32_t num, int32_t den, int32_t *rem);

static void check_result(uint64_t num, uint64_t den, const char *op,
			 char signed_calc, unsigned bits,
			 uint64_t my_result, uint64_t compiler_result)
{
  ++tested;
  if (my_result == compiler_result) return;
  printf("Bad result for %c%u %lld (%llu) %s %lld (%llu)\n",
	 signed_calc, bits, (int64_t) num, num, op, (int64_t) den, den);
  printf("Custom divide result: %lld (%llu)\n",
	 (int64_t) my_result, my_result);
  printf("Compiler result: %lld (%llu)\n",
	 (int64_t) compiler_result, compiler_result);
  exit(1);
}

static const char *op_to_string(char op)
{
  switch (op) {
  case '/': return "divide";
  case '%': return "modulo";
  default:
    printf("Bad op '%c'\n", op);
    exit(1);
  }
}

static void do_test_64(uint64_t num, uint64_t den,
		       char op,
		       func_u64 func_u, func_s64 func_s)
{
  // Test an individual 64 bit function.
  uint64_t my_result, compiler_result;
  if (den == 0) return;
  if (func_s && op == '/' && num == INT64_MIN && den == -1) return; // overflow
  if (func_u) {
    my_result = func_u(num, den);
    if (op == '/') compiler_result = num / den;
    else compiler_result = num % den;
  } else {
    my_result = (uint64_t) func_s((int64_t) num, (int64_t) den);
    // Test for overflow. The host may trigger an exception.
    if (op == '/') {
      if (num == INT64_MIN && den == -1) compiler_result = (uint64_t) INT64_MIN;
      else compiler_result = (uint64_t) (((int64_t) num) / ((int64_t) den));
    } else {
      if (num == INT64_MIN && den == -1) compiler_result = 0;
      else compiler_result = ((int64_t) num) % ((int64_t) den);
    }
  }
  check_result(num, den, op_to_string(op), func_u ? 'u' : 's', 64,
	       my_result, compiler_result);
}

static void do_test_32(uint32_t num, uint32_t den,
		       char op, func_u32 func_u, func_s32 func_s)
{
  // Test an individual 32 bit function.
  uint32_t my_result, compiler_result;
  if (den == 0) return;
  if (func_s && op == '/' && num == INT32_MIN && den == -1) return; // overflow
  if (func_u) {
    my_result = func_u(num, den);
    if (op == '/') compiler_result = num / den;
    else compiler_result = num % den;
  } else {
    my_result = (uint32_t) func_s((int32_t) num, (int32_t) den);
    // Test for overflow. The host may trigger an exception.
    if (op == '/') {
      if (num == INT32_MIN && den == -1) compiler_result = (uint32_t) INT32_MIN;
      else compiler_result = (uint32_t) (((int32_t) num) / ((int32_t) den));
    } else {
      if (num == INT32_MIN && den == -1) compiler_result = 0;
      else compiler_result = ((int32_t) num) % ((int32_t) den);
    }
  }
  check_result(num, den, op_to_string(op), func_u ? 'u' : 's', 32,
	       my_result, compiler_result);
}

static void do_test_divmod_64(uint64_t num, uint64_t den,
			      func_udm64 func_u, func_sdm64 func_s)
{
  // Test an individual 64 bit divmod function matches separate results.
  uint64_t my_result, my_remainder, compiler_result, compiler_remainder;
  if (den == 0) return;
  if (func_s && num == INT64_MIN && den == -1) return; // overflow
  if (func_u) {
    my_result = func_u(num, den, &my_remainder);
    compiler_result = num / den;
    compiler_remainder = num % den;
  } else {
    int64_t my_remainder_s;
    my_result = (uint64_t) func_s((int64_t) num, (int64_t) den,
				  &my_remainder_s);
    my_remainder = (int64_t) my_remainder_s;
    // Test for overflow. The host may trigger an exception.
    if (num == INT64_MIN && den == -1) {
      compiler_result = (uint64_t) INT64_MIN;
      compiler_remainder = 0;
    } else {
      compiler_result = (uint64_t) (((int64_t) num) / ((int64_t) den));
      compiler_remainder = (uint64_t) (((int64_t) num) % ((int64_t) den));
    }
  }
  check_result(num, den, "divmod-result", func_u ? 'u' : 's', 64,
	       my_result, compiler_result);
  check_result(num, den, "divmod-remainder", func_u ? 'u' : 's', 64,
	       my_remainder, compiler_remainder);
}

static void do_test_divmod_32(uint32_t num, uint32_t den,
			      func_udm32 func_u, func_sdm32 func_s)
{
  // Test an individual 32 bit divmod function matches separate results.
  uint32_t my_result, my_remainder, compiler_result, compiler_remainder;
  if (den == 0) return;
  if (func_s && num == INT32_MIN && den == -1) return; // overflow
  if (func_u) {
    my_result = func_u(num, den, &my_remainder);
    compiler_result = num / den;
    compiler_remainder = num % den;
  } else {
    int32_t my_remainder_s;
    my_result = (uint32_t) func_s((int32_t) num, (int32_t) den,
				  &my_remainder_s);
    my_remainder = (int32_t) my_remainder_s;
    // Test for overflow. The host may trigger an exception.
    if (num == INT32_MIN && den == -1) {
      compiler_result = (uint32_t) INT64_MIN;
      compiler_remainder = 0;
    } else {
      compiler_result = (uint32_t) (((int32_t) num) / ((int32_t) den));
      compiler_remainder = (uint32_t) (((int32_t) num) % ((int32_t) den));
    }
  }
  check_result(num, den, "divmod-result", func_u ? 'u' : 's', 32,
	       my_result, compiler_result);
  check_result(num, den, "divmod-remainder", func_u ? 'u' : 's', 32,
	       my_remainder, compiler_remainder);
}

static void do_test_all_ops_64(uint64_t num_u64, uint64_t den_u64,
			       func_u64 func_u_div, func_s64 func_s_div,
			       func_u64 func_u_mod, func_s64 func_s_mod,
			       func_udm64 func_u_divmod,
			       func_sdm64 func_s_divmod)
{
  // Test all combinations of unsigned/signed divide/modulo.
  do_test_64(num_u64, den_u64, '/', func_u_div, NULL);
  do_test_64(num_u64, den_u64, '%', func_u_mod, NULL);
  do_test_64(num_u64, den_u64, '/', NULL, func_s_div);
  do_test_64(num_u64, den_u64, '%', NULL, func_s_mod);
  do_test_divmod_64(num_u64, den_u64, func_u_divmod, NULL);
  do_test_divmod_64(num_u64, den_u64, NULL, func_s_divmod);
}

static void do_test_all_ops_32(uint32_t num_u32, uint32_t den_u32,
			       func_u32 func_u_div, func_s32 func_s_div,
			       func_u32 func_u_mod, func_s32 func_s_mod,
			       func_udm32 func_u_divmod,
			       func_sdm32 func_s_divmod)
{
  // Test all combinations of unsigned/signed divide/modulo.
  do_test_32(num_u32, den_u32, '/', func_u_div, NULL);
  do_test_32(num_u32, den_u32, '%', func_u_mod, NULL);
  do_test_32(num_u32, den_u32, '/', NULL, func_s_div);
  do_test_32(num_u32, den_u32, '%', NULL, func_s_mod);
  do_test_divmod_32(num_u32, den_u32, func_u_divmod, NULL);
  do_test_divmod_32(num_u32, den_u32, NULL, func_s_divmod);
}

static void do_test_all_types(uint64_t num_u64, uint64_t den_u64)
{
  // Test 32 and 64 bit types.
  uint32_t num_u32 = (uint32_t) num_u64;
  uint32_t den_u32 = (uint32_t) den_u64;
  do_test_all_ops_64(num_u64, den_u64,
		     __udivdi3, __divdi3, __umoddi3, __moddi3,
		     __udivmoddi4, __divmoddi4);
  do_test_all_ops_32(num_u32, den_u32,
		     __udivsi3, __divsi3, __umodsi3, __modsi3,
		     __udivmodsi4, __divmodsi4);
}

static void do_test_all_signs(uint64_t num, uint64_t den)
{
  // Test all combinations of sign.
  do_test_all_types(num, den);
  do_test_all_types(-num, den);
  do_test_all_types(num, -den);
  do_test_all_types(-num, -den);
}

static void do_test_ways(uint64_t num, uint64_t den)
{
  // Test both ways around.
  do_test_all_signs(num, den);
  do_test_all_signs(den, num);
}

static void do_test_variants(uint64_t num, uint64_t den)
{
  // Test +/-5 from each num, den.
  int i, j;
  for (i = -5; i <= 5; ++i) {
    uint64_t new_num = num + i;
    for (j = -5; j <= 5; ++j) {
      uint64_t new_den = den + j;
      do_test_ways(new_num, new_den);
    }
  }
}

static void panic(const char *reason __attribute__((unused)))
{
  ++divide_error_count;
}

int main()
{
  static const uint64_t tests[][2] = {
    { 0, 1 },
    { 13, 7 },
    { 29, 13 },
    { 13, 29 },
    { 999, 111 },
    { 999, 110 },
    { 999, 112 },
    { 9999, 112 },
    { 65536, 65536 },
    { UINT32_MAX, UINT32_MAX },
    { UINT32_MAX >> 1, UINT32_MAX },
    { UINT32_MAX >> 1, UINT32_MAX >> 1},
    { UINT64_MAX, 1 },
    { UINT64_MAX, 96 },
    { UINT64_MAX, 65536 },
    { UINT64_MAX, UINT32_MAX + 65536 + 71 },
    { UINT64_MAX, UINT32_MAX },
    { UINT64_MAX, UINT64_MAX >> 1 },
    { UINT64_MAX >> 1, UINT64_MAX >> 1 },
    { UINT64_MAX, UINT64_MAX },
    { UINT64_MAX >> 1, UINT64_MAX },
  };

  size_t t;
  for (t = 0; t < sizeof(tests) / sizeof(tests[0]); ++t) {
    // Start a HUGE tree of tests for this pair.
    do_test_variants(tests[t][0], tests[t][1]);
  }

  if (divide_error_count > 0) {
    printf("Got %u errors in %u cases, should have had none\n",
	   divide_error_count, tested);
    return 1;
  }
  
  // Test error cases work.
  __divdi3(1, 0);
  assert(divide_error_count == 1);
  __divdi3(INT64_MIN, 0);
  assert(divide_error_count == 2);
  __divdi3(INT64_MAX, 0);
  assert(divide_error_count == 3);
  __modsi3(INT32_MIN, 0);
  assert(divide_error_count == 4);

  printf("Tested %u cases ok\n", tested);
  return 0;
}

#endif // TEST
