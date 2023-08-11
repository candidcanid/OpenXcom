#pragma once

#include "cpu.h"
#include "platform.h"

#if defined(__APPLE__)

#include <os/log.h>

#endif

#if defined(MSVC)
#define BreakpointTrap()  __debugbreak()
#elif CPU(X64) || CPU(X86)
#define BreakpointTrap()  asm volatile ("int3")
#elif CPU(ARM64)
#define BreakpointTrap()  asm volatile ("brk #0xc471")
#else
#define BreakpointTrap() Crash() // no platform equiv
#endif

#define ABORT() std::abort()

#if defined (NDEBUG)
#define ASSERT_ENABLED 0
#else
#define ASSERT_ENABLED 1
#endif

void Crash();

#if !ASSERT_ENABLED

#define ASSERT(assertion, ...) ((void)0)
#define ASSERT_NOT_REACHED(...) ((void)0)
#define ASSERT_IMPLIES(condition, assertion) ((void)0)

#else

#define ASSERT(assertion, ...) do { \
    if (!(assertion)) { \
    	BreakpointTrap(); \
        Crash(); \
    } \
} while (0)

#define ASSERT_NOT_REACHED(...) do { \
	BreakpointTrap(); \
    Crash(); \
} while (0)


#define ASSERT_IMPLIES(condition, assertion) do { \
    if ((condition) && !(assertion)) { \
    	BreakpointTrap(); \
        Crash(); \
    } \
} while (0)

#endif

#define RELEASE_ASSERT(assertion, ...) do { \
    if (!(assertion)) { \
    	BreakpointTrap(); \
        Crash(); \
    } \
} while (0)

#if defined(MSVC) // MSVC
	#define UNREACHABLE() __assume(false);
#else  // GCC, Clang
	#define UNREACHABLE() __builtin_unreachable();
#endif
