#include <sys.h>

extern "C" void
__cxa_pure_virtual(void)
{
	panic("Pure virtual function called");
}

extern "C" int
__cxa_atexit(void (*func)(void*), void* obj, void* dso)
{
	// do nothing
	return 0;
}
