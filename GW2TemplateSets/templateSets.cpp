#include "templateSets.h"

#include <regex>

using namespace std;

wstring buildPath(const filesystem::path exePath, wstring path) {
	filesystem::path buildPath = exePath.parent_path();
	buildPath /= path;
	return buildPath.wstring();
}

bool save(const filesystem::path exePath, string folder)
{
	if(regex_match(folder, regex(R"(^[^ ./:<>*?\\][^/:<>^*?\\]+$)"))) {
		const auto folderWS = wstring(folder.begin(), folder.end());

		if(filesystem::exists(buildPath(exePath, L"addons\\templatesets\\") + wstring(folder.begin(), folder.end()))) {
			filesystem::remove_all(buildPath(exePath, L"addons\\templatesets\\") + wstring(folder.begin(), folder.end()));
		}
		filesystem::copy(buildPath(exePath, L"addons\\arcdps\\arcdps.templates"), buildPath(exePath, L"addons\\templatesets\\") + folderWS, filesystem::copy_options::recursive);
		return true;
	}
	return false;
}

void remove(const filesystem::path exePath, string folder)
{
	const auto folderWS = wstring(folder.begin(), folder.end());
	filesystem::remove_all(buildPath(exePath, L"addons\\templatesets\\") + folderWS);
}

void overwrite(const filesystem::path exePath, string folder)
{
	const auto folderWS = wstring(folder.begin(), folder.end());
	filesystem::remove_all(buildPath(exePath, L"addons\\templatesets\\") + folderWS);
	filesystem::copy(buildPath(exePath, L"addons\\arcdps\\arcdps.templates"), buildPath(exePath, L"addons\\templatesets\\") + folderWS, filesystem::copy_options::recursive);
}

void load(const filesystem::path exePath, string folder)
{
	const auto folderWS = wstring(folder.begin(), folder.end());
	filesystem::remove_all(buildPath(exePath, L"addons\\arcdps\\arcdps.templates\\"));
	filesystem::copy(buildPath(exePath, L"addons\\templatesets\\") + folderWS, buildPath(exePath, L"addons\\arcdps\\arcdps.templates"), filesystem::copy_options::recursive);
}
