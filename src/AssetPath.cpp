// The platform headers come first on purpose: <windows.h> declares its own
// global `byte`, which clashes with std::byte (C2872) if the `using namespace
// std;` from AssetPath.h is already in scope when it is parsed.
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "AssetPath.h"


using namespace std;


static string executableDir()
{
#if defined(_WIN32)
	char buffer[MAX_PATH];
	DWORD length = GetModuleFileNameA(NULL, buffer, MAX_PATH);
	if(length == 0 || length == MAX_PATH)
		return ".";
	string path(buffer, length);
	size_t separator = path.find_last_of("\\/");
#else
	char buffer[4096];
	ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
	if(length <= 0)
		return ".";
	string path(buffer, length);
	size_t separator = path.find_last_of('/');
#endif
	if(separator == string::npos)
		return ".";

	return path.substr(0, separator);
}

string assetPath(const string &relativePath)
{
	static const string base = executableDir() + "/assets/";

	return base + relativePath;
}
