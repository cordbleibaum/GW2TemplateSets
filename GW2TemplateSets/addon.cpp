#include "addon.h"
#include "imgui_internal.h"
#include <regex>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "templateSets.h"

std::filesystem::path exePath;

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ulReasonForCall, LPVOID lpReserved) 
{
	wchar_t path[2048];
	switch(ulReasonForCall) {
	case DLL_PROCESS_ATTACH: 		
		GetModuleFileName(nullptr, &path[0], 2048);
		exePath = std::wstring(path);
		break;
	case DLL_THREAD_ATTACH:  
		GetModuleFileName(nullptr, &path[0], 2048);
		exePath = std::wstring(path);
		break;
	case DLL_THREAD_DETACH:  break;
	case DLL_PROCESS_DETACH:  break;
	}
	return 1;
}

extern "C" __declspec(dllexport) void* get_init_addr(char* arcversionstr, void* imguicontext, IDirect3DDevice9*) 
{
	ImGui::SetCurrentContext(static_cast<ImGuiContext*>(imguicontext));
	return reinterpret_cast<void*>(mod_init);
}

extern "C" __declspec(dllexport) void* get_release_addr() 
{
	return reinterpret_cast<void*>(mod_release);
}


//
// Main mod code
//

bool windowVisible = false;

std::vector<std::string> directoryStrings;
const char** directories;
size_t count;
int selected;

char* setNameBuf;
size_t setNameBufSize;

bool modifier;
bool modifierLShift;

std::string currentName;

boost::property_tree::ptree properties;

std::string configPath()
{
	std::filesystem::path buildPath = exePath.parent_path();
	buildPath /= L"addons";
	buildPath /= L"templatesets";
	buildPath /= L"settings.json";
	return buildPath.string();
}

std::wstring buildPath(std::wstring path)
{
	std::filesystem::path buildPath = exePath.parent_path();
	buildPath /= path;
	return buildPath.wstring();
}

void rebuildSets()
{
	directoryStrings = get_directories(buildPath(L"addons\\templatesets"));
	count = directoryStrings.size();
	directories = new const char*[count];
	for (size_t i = 0; i < count; ++i)
	{
		directories[i] = directoryStrings[i].c_str();
	}

	if (static_cast<size_t>(selected) >= count)
	{
		selected = 0;
	}
}

void selectByName(const std::string name) {
	const auto iterator = std::find(directoryStrings.begin(), directoryStrings.end(), name);
	if(iterator != directoryStrings.end()) {
		selected = std::distance(directoryStrings.begin(), iterator);
	}
}

arcdps_exports* mod_init() 
{
	windowVisible = false;

	selected = 0;

	if(!std::filesystem::exists(buildPath(L"addons"))) {
		std::filesystem::create_directory(buildPath(L"addons\\templatesets\\"));
	}

	if (!std::filesystem::exists(buildPath(L"addons\\templatesets\\")))
	{
		std::filesystem::create_directory(buildPath(L"addons\\templatesets\\"));
	}

	rebuildSets();

	if (!std::filesystem::exists(configPath()))
	{
		std::fstream configFile;
		configFile.open(configPath(), std::fstream::out);
		configFile << "{}";
		configFile.close();
	}


	boost::property_tree::read_json(configPath(), properties);
	
	auto currentSet = properties.get<std::string>("currentSet", ".");
	selectByName(currentSet);

	setNameBufSize = 32;
	setNameBuf = new char[setNameBufSize];
	memset(&setNameBuf[0], 0, sizeof setNameBufSize);

	memset(&arc_exports, 0, sizeof(arcdps_exports));
	arc_exports.sig = 0xC0AA;
	arc_exports.size = sizeof(arcdps_exports);
	arc_exports.out_name = "templatesets";
	arc_exports.out_build = "0.7";
	arc_exports.wnd_filter = reinterpret_cast<void*>(mod_wnd_filter);
	arc_exports.wnd_nofilter = reinterpret_cast<void*>(mod_wnd_nofilter);
	arc_exports.imgui = reinterpret_cast<void*>(mod_imgui);
	return &arc_exports;
}

uintptr_t mod_release() 
{
	return 0;
}

uintptr_t mod_wnd_filter(HWND, UINT uMsg, WPARAM wParam, LPARAM) 
{
	if (uMsg == WM_SYSKEYDOWN && wParam == 0x51)
	{
		windowVisible = !windowVisible;
		return 0;
	}

	return uMsg;
}

uintptr_t mod_wnd_nofilter(HWND, UINT uMsg, WPARAM wParam, LPARAM) {
	if (uMsg == WM_KEYDOWN && wParam == VK_ESCAPE) 
	{
		windowVisible = false;
	}

	return uMsg;
}

uintptr_t mod_imgui(uint32_t)
{
	if (windowVisible)
	{
		ImGui::Begin("Template Sets", &windowVisible, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse );

		if (!directoryStrings.empty()) {
			ImGui::ListBox("", &selected, directories, directoryStrings.size());
		}

		if(ImGui::Button("Load") && !directoryStrings.empty()) {
			const auto folder = std::string(directories[selected]);
			load(exePath, folder);
			properties.put<std::string>("currentSet", folder);
			boost::property_tree::write_json(configPath(), properties);
		}
		ImGui::SameLine();
		if(ImGui::Button("Overwrite") && !directoryStrings.empty()) {
			const auto folder = std::string(directories[selected]);
			overwrite(exePath, folder);
			rebuildSets();
			selectByName(folder);
			properties.put<std::string>("currentSet", folder);
			boost::property_tree::write_json(configPath(), properties);
		}
		ImGui::SameLine();
		if(ImGui::Button("Delete") && !directoryStrings.empty()) {
			const auto folder = std::string(directories[selected]);
			remove(exePath, folder);
			rebuildSets();
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::Text("Name");
		ImGui::InputText("", setNameBuf, setNameBufSize);

		if (ImGui::Button("Save"))
		{
			const auto folder = std::string(setNameBuf);
			if (save(exePath, folder))
			{
				memset(&setNameBuf[0], 0, sizeof setNameBufSize);
				currentName = folder;
				rebuildSets();
				selectByName(folder);
				properties.put<std::string>("currentSet", folder);
				boost::property_tree::write_json(configPath(), properties);
			}
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		if(ImGui::Button("Clear")) {
			ImGui::OpenPopup("Confirm");
		}

		if(ImGui::BeginPopupModal("Confirm", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("Are you sure you want to clear ArcDPS buildtemplate folder?");
			if(ImGui::Button("Yes")) {
				std::filesystem::remove_all(buildPath(L"addons\\arcdps\\arcdps.templates\\"));
				std::filesystem::create_directory(buildPath(L"addons\\arcdps\\arcdps.templates\\"));
				ImGui::CloseCurrentPopup();
			}
			if(ImGui::Button("No")) {
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		ImGui::End();
	}
	return 0;
}