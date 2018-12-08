#pragma once

#include <string>
#include <filesystem>

std::string configPath(std::filesystem::path exePath);
std::wstring buildPath(std::filesystem::path exePath, std::wstring path);
void initPaths(std::filesystem::path exePath);