
#include "MacOS.h"

#if defined(__APPLE__)

#include <unistd.h>
#include <CoreFoundation/CoreFoundation.h>

namespace OpenXcom
{
namespace MacOS
{

std::string getBundlePath()
{
	CFBundleRef bundle = CFBundleGetMainBundle();
	CFURLRef bundleURL = CFBundleCopyBundleURL(bundle);
	char path[PATH_MAX];
	Boolean success = CFURLGetFileSystemRepresentation(bundleURL, TRUE, (UInt8 *)path, PATH_MAX);
	assert(success);
	CFRelease(bundleURL);
	return std::string(path); // assuming null-terminated
}

}
}

#endif