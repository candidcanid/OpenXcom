#pragma once

#if defined(__x86_64__) || defined(_M_X64)
	#define __PLATFORM_CPU_X64 1
#elif defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
	#define __PLATFORM_CPU_X86 1
#elif defined(__aarch64__) || defined(_M_ARM64)
	#define __PLATFORM_CPU_ARM64 1
#endif


#define CPU(FEATURE) (defined __PLATFORM_CPU_##FEATURE  && __PLATFORM_CPU_##FEATURE)
