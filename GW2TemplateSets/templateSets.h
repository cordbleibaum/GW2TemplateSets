#pragma once

#include <string>
#include <filesystem>

bool save(std::filesystem::path exePath, std::string folder);
void remove(std::filesystem::path exePath, std::string folder);
void overwrite(std::filesystem::path exePath, std::string folder);
void load(std::filesystem::path exePath, std::string folder);