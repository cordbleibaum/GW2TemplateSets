#include "templateSets.h"

#include <regex>

#include "paths.h"

using namespace std;

vector<string> get_directories(const wstring& s) {
	vector<std::string> r;
	for(auto& p : filesystem::directory_iterator(s))
		if(p.status().type() == filesystem::file_type::directory)
			r.push_back(p.path().stem().string());
	return r;
}

TemplateSetList::TemplateSetList(filesystem::path exePath)
{
	vector<string> directoryStrings = get_directories(buildPath(exePath, L"addons\\templatesets"));
	_count = directoryStrings.size();
	_directories = new char*[_count];
	for(size_t i = 0; i < _count; ++i) {
		const auto directory = new char[directoryStrings[i].length() + 1];
		strncpy_s(directory, directoryStrings[i].length() + 1, directoryStrings[i].c_str(), directoryStrings[i].length());
		directory[directoryStrings[i].length()] = '\0';
		_directories[i] = directory;
	}
	this->exePath = exePath;
	isValid = true;
}

TemplateSetList& TemplateSetList::operator=(const TemplateSetList& other)
{
	if (this != &other)
	{
		clearData();
	}

	if (other.isValid) {
		_count = other._count;
		_directories = new char*[other._count];
		for(size_t i = 0; i < _count; ++i) {
			_directories[i] = new char[strlen(other._directories[i])+1];
			strncpy_s(_directories[i], strlen(other._directories[i]) + 1, other._directories[i], strlen(other._directories[i]) + 1);
			_directories[i][strlen(other._directories[i])] = '\0';
		}
		isValid = true;
	}

	return *this;
}

size_t TemplateSetList::count() const
{
	return _count;
}

char** TemplateSetList::directories() const
{
	return _directories;
}

const std::string TemplateSetList::operator[](const size_t position) const {
	return std::string(_directories[position]);
}

bool TemplateSetList::empty() const
{
	return _count == 0;
}

int TemplateSetList::find(string name) const
{
	for (size_t i = 0; i < _count; i++)
	{
		if (string((*this)[i]) == name) return i;
	}
	return -1;
}

bool TemplateSetList::save(string folder)
{
	if(regex_match(folder, regex(R"(^[^ ./:<>*?\\][^/:<>^*?\\]+$)"))) {
		const auto folderWS = wstring(folder.begin(), folder.end());

		if(filesystem::exists(buildPath(exePath, L"addons\\templatesets\\") + wstring(folder.begin(), folder.end()))) {
			filesystem::remove_all(buildPath(exePath, L"addons\\templatesets\\") + wstring(folder.begin(), folder.end()));
		}
		filesystem::copy(buildPath(exePath, L"addons\\arcdps\\arcdps.templates"), buildPath(exePath, L"addons\\templatesets\\") + folderWS, filesystem::copy_options::recursive);

		reload();

		return true;
	}
	return false;
}

void TemplateSetList::remove(string folder)
{
	const auto folderWS = wstring(folder.begin(), folder.end());
	filesystem::remove_all(buildPath(exePath, L"addons\\templatesets\\") + folderWS);
	reload();
}

void TemplateSetList::remove(int position)
{
	remove((*this)[position]);
}

void TemplateSetList::overwrite(int position) const
{
	overwrite((*this)[position]);
}

void TemplateSetList::overwrite(string folder) const
{
	const auto folderWS = wstring(folder.begin(), folder.end());
	filesystem::remove_all(buildPath(exePath, L"addons\\templatesets\\") + folderWS);
	filesystem::copy(buildPath(exePath, L"addons\\arcdps\\arcdps.templates"), buildPath(exePath, L"addons\\templatesets\\") + folderWS, filesystem::copy_options::recursive);
}

void TemplateSetList::load(int position) const
{
	load((*this)[position]);
}

void TemplateSetList::load(std::string folder) const
{
	const auto folderWS = wstring(folder.begin(), folder.end());
	filesystem::remove_all(buildPath(exePath, L"addons\\arcdps\\arcdps.templates\\"));
	filesystem::copy(buildPath(exePath, L"addons\\templatesets\\") + folderWS, buildPath(exePath, L"addons\\arcdps\\arcdps.templates"), filesystem::copy_options::recursive);
}

void TemplateSetList::reload()
{
	clearData();

	vector<string> directoryStrings = get_directories(buildPath(exePath, L"addons\\templatesets"));
	_count = directoryStrings.size();
	_directories = new char*[_count];
	for(size_t i = 0; i < _count; ++i) {
		const auto directory = new char[directoryStrings[i].length() + 1];
		strncpy_s(directory, directoryStrings[i].length() + 1, directoryStrings[i].c_str(), directoryStrings[i].length());
		directory[directoryStrings[i].length()] = '\0';
		_directories[i] = directory;
	}
	this->exePath = exePath;
	isValid = true;
}

void TemplateSetList::clearData()
{
	if(isValid) {
		for(int i = 0; i < _count; i++) {
			delete[] _directories[i];
		}
		delete[] _directories;
		isValid = false;
	}
}

TemplateSetList::~TemplateSetList()
{
	if (isValid) {
		for (int i = 0; i < _count; i++)
		{
			delete[] _directories[i];
		}
		delete[] _directories;
	}
}
