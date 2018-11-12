#pragma once

#include <string>
#include <vector>
#include <filesystem>


std::vector<std::string> get_directories(const std::string& s) {
	std::vector<std::string> r;
	for(auto& p : std::filesystem::directory_iterator(s))
		if(p.status().type() == std::filesystem::file_type::directory)
			r.push_back(p.path().stem().string());
	return r;
}