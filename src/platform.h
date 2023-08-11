#pragma once

#if defined(__APPLE__)
	#define __PLATFORM_DARWIN 1
#endif


#define PLATFORM(FEATURE) (defined __PLATFORM_TARGET_##FEATURE  && __PLATFORM_TARGET_##FEATURE)
