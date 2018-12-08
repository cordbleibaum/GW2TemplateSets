#include "paths.h"

#include <fstream>

std::string configPath(std::filesystem::path exePath) {
	std::filesystem::path buildPath = exePath.parent_path();
	buildPath /= L"addons";
	buildPath /= L"templatesets";
	buildPath /= L"settings.json";
	return buildPath.string();
}

std::wstring buildPath(std::filesystem::path exePath, std::wstring path) {
	std::filesystem::path buildPath = exePath.parent_path();
	buildPath /= path;
	return buildPath.wstring();
}

void initPaths(std::filesystem::path exePath)
{
	if(!std::filesystem::exists(buildPath(exePath, L"addons"))) {
		std::filesystem::create_directory(buildPath(exePath, L"addons\\templatesets\\"));
	}

	if(!std::filesystem::exists(buildPath(exePath, L"addons\\templatesets\\"))) {
		std::filesystem::create_directory(buildPath(exePath, L"addons\\templatesets\\"));
	}

	if(!std::filesystem::exists(configPath(exePath))) {
		std::fstream configFile;
		configFile.open(configPath(exePath), std::fstream::out);
		configFile << "{}";
		configFile.close();
	}
}
