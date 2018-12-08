#include "addon.h"
#include "imgui_internal.h"
#include <regex>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "templateSets.h"
#include "paths.h"

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

TemplateSetList templates;
int selected;

char* setNameBuf;
size_t setNameBufSize;

bool modifier;
bool modifierLShift;

boost::property_tree::ptree properties;

arcdps_exports* mod_init() 
{
	windowVisible = false;
	selected = 5;

	initPaths(exePath);

	templates = TemplateSetList(exePath);
	boost::property_tree::read_json(configPath(exePath), properties);
	
	auto currentSet = properties.get<std::string>("currentSet", ".");
	selected = templates.find(templates[selected]);

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

		if (!templates.empty()) {
			ImGui::ListBox("", &selected, templates.directories(), templates.count());

			if(ImGui::Button("Load")) {
				templates.load(selected);
				properties.put<std::string>("currentSet", templates[selected]);
				boost::property_tree::write_json(configPath(exePath), properties);
			}
			ImGui::SameLine();
			if(ImGui::Button("Overwrite")) {
				templates.overwrite(selected);
				properties.put<std::string>("currentSet", templates[selected]);
				boost::property_tree::write_json(configPath(exePath), properties);
			}
			ImGui::SameLine();
			if(ImGui::Button("Delete")) {
				templates.remove(selected);
			}
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::Text("Name");
		ImGui::InputText("", setNameBuf, setNameBufSize);

		if (ImGui::Button("Save"))
		{
			const auto folder = std::string(setNameBuf);
			if (templates.save(folder))
			{
				memset(&setNameBuf[0], 0, sizeof setNameBufSize);
				selected = templates.find(folder);
				properties.put<std::string>("currentSet", folder);
				boost::property_tree::write_json(configPath(exePath), properties);
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
				std::filesystem::remove_all(buildPath(exePath, L"addons\\arcdps\\arcdps.templates\\"));
				std::filesystem::create_directory(buildPath(exePath, L"addons\\arcdps\\arcdps.templates\\"));
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