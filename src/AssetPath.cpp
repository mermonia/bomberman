#include "AssetPath.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#endif


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
