
#include "assertions.h"

void Crash() {
#if defined(__clang__)
	__builtin_trap();
#else
	*(int *)(uintptr_t)0xbbadbeef = 0;
	((void(*)())nullptr)();
#endif
}


// static void printf_stderr_common(const char* format, ...) {
//     va_list args;
//     va_start(args, format);
//     vfprintf(stderr, format, args);
//     va_end(args);
// }
