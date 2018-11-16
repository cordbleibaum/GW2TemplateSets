#include "addon.h"
#include "imgui_internal.h"

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ulReasonForCall, LPVOID lpReserved) 
{
	return 1;
}

extern "C" __declspec(dllexport) void* get_init_addr(char* arcversionstr, void* imguicontext, IDirect3DDevice9* id3dd9) 
{
	arcvers = arcversionstr;
	ImGui::SetCurrentContext((ImGuiContext*)imguicontext);
	return mod_init;
}

extern "C" __declspec(dllexport) void* get_release_addr() 
{
	arcvers = 0;
	return mod_release;
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

void rebuildSets()
{
	directoryStrings = get_directories("addons/templatesets");
	count = directoryStrings.size();
	directories = new const char*[count];
	for (int i = 0; i < count; ++i)
	{
		directories[i] = directoryStrings[i].c_str();
	}
	selected = 0;
}

arcdps_exports* mod_init() 
{
	windowVisible = false;


	if(!std::filesystem::exists("addons/")) {
		std::filesystem::create_directory("addons/templatesets/");
	}

	if (!std::filesystem::exists("addons/templatesets/"))
	{
		std::filesystem::create_directory("addons/templatesets/");
	}

	rebuildSets();

	setNameBufSize = 32;
	setNameBuf = new char[setNameBufSize];
	memset(&setNameBuf[0], 0, sizeof(setNameBufSize));

	memset(&arc_exports, 0, sizeof(arcdps_exports));
	arc_exports.sig = 0xC0AA;
	arc_exports.size = sizeof(arcdps_exports);
	arc_exports.out_name = "templatesets";
	arc_exports.out_build = "0.7";
	arc_exports.wnd_filter = mod_wnd_filter;
	arc_exports.wnd_nofilter = mod_wnd_nofilter;
	arc_exports.imgui = mod_imgui;
	return &arc_exports;
}

uintptr_t mod_release() 
{
	return 0;
}

uintptr_t mod_wnd_filter(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{

	switch (uMsg)
	{
	case WM_SYSKEYDOWN:
		if(wParam == 0x51) {
			windowVisible = !windowVisible;
			return 0;
		}
		break;
	default:
		break;
	}
	return uMsg;
}

uintptr_t mod_wnd_nofilter(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	switch(uMsg) {
	case WM_KEYDOWN:
		if(wParam == VK_ESCAPE) {
			windowVisible = false;
		}
		break;
	default:
		break;
	}
	return uMsg;
}

uintptr_t mod_imgui(uint32_t not_charsel_or_loading)
{
	if (windowVisible)
	{
		ImGui::Begin("Template Sets", &windowVisible, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse );

		if (!directoryStrings.empty()) {
			ImGui::ListBox("", &selected, directories, IM_ARRAYSIZE(directories));
		}

		if(ImGui::Button("Load") && !directoryStrings.empty()) {
			std::string folder = std::string(directories[selected]);
			std::filesystem::remove_all("addons/arcdps/arcdps.templates/");
			std::filesystem::copy("addons/templatesets/" + folder, "addons/arcdps/arcdps.templates", std::filesystem::copy_options::recursive);
		}
		ImGui::SameLine();
		if(ImGui::Button("Overwrite") && !directoryStrings.empty()) {
			std::string folder = std::string(directories[selected]);
			std::filesystem::remove_all("addons/templatesets/" + folder);
			std::filesystem::copy("addons/arcdps/arcdps.templates", "addons/templatesets/" + folder, std::filesystem::copy_options::recursive);
			rebuildSets();
		}
		ImGui::SameLine();
		if(ImGui::Button("Delete") && !directoryStrings.empty()) {
			std::string folder = std::string(directories[selected]);
			std::filesystem::remove_all("addons/templatesets/" + folder);
			rebuildSets();
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::Text("Name");
		ImGui::InputText("", setNameBuf, setNameBufSize);

		if (ImGui::Button("Save"))
		{
			std::string folder = std::string(setNameBuf);
			if (folder.size() > 1) {
				if (std::filesystem::exists("addons/templatesets/" + folder))
				{
					std::filesystem::remove_all("addons/templatesets/" + folder);
				}
				std::filesystem::copy("addons/arcdps/arcdps.templates", "addons/templatesets/" + folder, std::filesystem::copy_options::recursive);
				memset(&setNameBuf[0], 0, sizeof(setNameBufSize));
				rebuildSets();
			}
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		if(ImGui::Button("Clear")) {
			ImGui::OpenPopup("Confirm");
		}

		if(ImGui::BeginPopupModal("Confirm", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("Are you sure you want to clear ArcDPS buildtemplate folder?");
			if(ImGui::Button("Yes")) {
				std::filesystem::remove_all("addons/arcdps/arcdps.templates/");
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