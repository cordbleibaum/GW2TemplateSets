#include "addon.h"
#include "imgui_internal.h"

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ulReasonForCall, LPVOID lpReserved) 
{
	switch(ulReasonForCall) 
	{
	case DLL_PROCESS_ATTACH: break;
	case DLL_PROCESS_DETACH: break;
	case DLL_THREAD_ATTACH:  break;
	case DLL_THREAD_DETACH:  break;
	}
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

	rebuildSets();

	setNameBufSize = 32;
	setNameBuf = new char[setNameBufSize];
	memset(&setNameBuf[0], 0, sizeof(setNameBufSize));

	memset(&arc_exports, 0, sizeof(arcdps_exports));
	arc_exports.sig = 0xC0AA;
	arc_exports.size = sizeof(arcdps_exports);
	arc_exports.out_name = "templatesets";
	arc_exports.out_build = "0.5";
	arc_exports.wnd_filter = mod_wnd_filter;
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

uintptr_t mod_imgui(uint32_t not_charsel_or_loading)
{
	if (windowVisible)
	{
		ImGui::Begin("Template Sets", &windowVisible, ImGuiWindowFlags_AlwaysAutoResize);

		if (!directoryStrings.empty()) {
			ImGui::ListBox("", &selected, directories, IM_ARRAYSIZE(directories));
		}

		ImGui::Separator();

		ImGui::InputText("", setNameBuf, setNameBufSize);

		if (ImGui::Button("Save"))
		{
			std::string folder = std::string(setNameBuf);
			std::filesystem::copy("addons/arcdps/arcdps.templates", "addons/templatesets/" + folder, std::filesystem::copy_options::recursive);
			memset(&setNameBuf[0], 0, sizeof(setNameBufSize));
			rebuildSets();
		}
		if (ImGui::Button("Delete")) {
			std::string folder = std::string(directories[selected]);
			std::filesystem::remove_all("addons/templatesets/"+folder);
			rebuildSets();
		}
		if (ImGui::Button("Load")) {
			std::string folder = std::string(directories[selected]);
			std::filesystem::remove_all("addons/arcdps/arcdps.templates/");
			std::filesystem::copy("addons/templatesets/" + folder, "addons/arcdps/arcdps.templates", std::filesystem::copy_options::recursive);
		}

		ImGui::End();
	}
	return 0;
}