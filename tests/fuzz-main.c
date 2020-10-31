#include <unittest.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// The module under test defines the fuzz main function which we call with
// the filename of the fuzzer input
extern int fuzz_main(const char *filename);


int main(int argc, char *argv[])
{
	if (argc < 2)
		return -1;

	return fuzz_main(argv[1]);
}

// Need a mock panic function because the module under test doesn't expect panic() to return
void _panic(const char *function, const char *str, ...)
{
	va_list ap;

	printf("panic: %s: ", function);

	va_start(ap, str);
	vprintf(str, ap);
	va_end(ap);

	exit(1);
}

// And we need a mock test assert fail in case the module under test uses any of
// the modules from tests/mocks that make test assertions
void test_assert_fail(const char *file, const char *func, unsigned line, const char *fmt, ...)
{
	va_list ap;

	printf("%s:%s:%u: assertion failed: ", file, func, line);
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);

	exit(1);
}
