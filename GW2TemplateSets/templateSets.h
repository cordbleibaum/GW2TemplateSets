#pragma once

#include <string>
#include <filesystem>

class TemplateSetList
{
public:
	TemplateSetList() = default;
	explicit TemplateSetList(std::filesystem::path exePath);
	TemplateSetList& operator=(const TemplateSetList& other);
	~TemplateSetList();

	size_t count() const;
	char** directories() const;
	const std::string operator[](const size_t position) const;
	bool empty() const;
	int find(std::string name) const;

	bool save(std::string folder);
	void remove(std::string folder);
	void remove(int position);
	void overwrite(int position) const;
	void overwrite( std::string folder) const;
	void load(int position) const;
	void load(std::string folder) const;
	void reload();
private:
	void clearData();
	std::filesystem::path exePath;
	bool isValid = false;
	char** _directories;
	size_t _count;
};