#include "global.h"
//all gay coding techniques are used in this document
ID3D11Device *g_pdevice = nullptr;
ID3D11DeviceContext *g_pcontext = nullptr;
ID3D11RenderTargetView *g_prendertargetview = nullptr;

HWND g_window = nullptr;
WNDPROC o_wndproc = nullptr;

std::once_flag present;

bool b_showmenu = false;
bool g_initalizated = false;

bool b_espenable = false;
bool b_box = false;
bool b_chams = false;
bool b_health = false;
bool b_name = false;

bool b_aimbotenable = false;

bool b_miscenable = false;
bool b_speedhack = false;
int i_speedhack = 0;


float c_glow[4] = {};
float c_box[4] = {};
float c_name[4] = {};

inline int strcmp_(const char *s1, const char *s2) {
	unsigned char c1, c2;
	while ((c1 = *s1++) == (c2 = *s2++)) {
		if (c1 == '\0')
			return 0;
	}
	return c1 - c2;
}
//credits to https://www.unknowncheats.me/forum/c-and-c-/173907-iat-hooking-class.html
void** find_func(const char* function, HMODULE module = 0) {
	if (!module)
		module = GetModuleHandleA(0);

	PIMAGE_DOS_HEADER img_dos_headers = (PIMAGE_DOS_HEADER)module;
	PIMAGE_NT_HEADERS img_nt_headers = (PIMAGE_NT_HEADERS)((byte*)img_dos_headers + img_dos_headers->e_lfanew);
	PIMAGE_IMPORT_DESCRIPTOR img_import_desc = (PIMAGE_IMPORT_DESCRIPTOR)((byte*)img_dos_headers + img_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

	for (IMAGE_IMPORT_DESCRIPTOR *iid = img_import_desc; iid->Name != 0; iid++) {
		for (int func_idx = 0; *(func_idx + (void**)(iid->FirstThunk + (size_t)module)) != nullptr; func_idx++) {
			char* mod_func_name = (char*)(*(func_idx + (size_t*)(iid->OriginalFirstThunk + (size_t)module)) + (size_t)module + 2);
			const intptr_t nmod_func_name = (intptr_t)mod_func_name;
			if (nmod_func_name >= 0) {
				if (!strcmp_(function, mod_func_name))
					return func_idx + (void**)(iid->FirstThunk + (size_t)module);
			}
		}
	}

	return 0;
}

bool w2s(c_vec source, c_vec &destination) {

	auto get_view_matrix = []() {
		uintptr_t viewRender = *(uintptr_t*)(apex_base + 0xC5EFA10);
		auto viewMatrixPtr = (**(matrix3x4_t**)(viewRender + 0x1a93d0));

		return viewMatrixPtr;
	};


	auto matrix = get_view_matrix();;

	float w = 0.0f;

	destination[0] = matrix[0][0] * source[0] + matrix[0][1] * source[1] + matrix[0][2] * source[2] + matrix[0][3];
	destination[1] = matrix[1][0] * source[0] + matrix[1][1] * source[1] + matrix[1][2] * source[2] + matrix[1][3];
	w = matrix[3][0] * source[0] + matrix[3][1] * source[1] + matrix[3][2] * source[2] + matrix[3][3];

	if (w < 0.01f)
		return false;

	float invw = 1.0f / w;
	destination[0] *= invw;
	destination[1] *= invw;

	int width = 2560;
	int height = 1080;
	
	RECT desktop;
	GetWindowRect(g_window, &desktop);
	width = desktop.right;
	height = desktop.bottom;

	float x = width / 2;
	float y = height / 2;

	x += 0.5 * destination[0] * width + 0.5;
	y -= 0.5 * destination[1] * height + 0.5;

	destination[0] = x;  //rc.left;
	destination[1] = y; //rc.top;
	return true;
}

inline int get_entcount() {
	return *(int*)(apex_base + 0xC008AD0);
}

inline c_entity* get_player(uintptr_t idx) {
	auto e = *(c_entity**)(apex_base + 0x1F96EB8 + (idx << 5));
	if (e) {
		auto h = e->m_shandle();
		if (h && *h) {
			if (!strcmp(h, xorstr_("player"))) {
				return e;
			}
		}
	}
	return nullptr;
}

inline c_entity* get_localentity()
{
	uintptr_t local_entity_id = *(uintptr_t*)(apex_base + 0x24846610);
	for (int i = 0; i < get_entcount(); i++)
	{
		c_entity* ent = get_player(i);
		if (!ent) continue;
		auto h = ent->m_shandle();
		if (h && *h) {
			if (!strcmp(h, xorstr_("player"))) {
				if (ent->m_iindex() == local_entity_id) {
					return ent;
				}
			}
		}
	}
	return nullptr;
}

inline bool friendly(c_entity* e) {
	if (e->m_iteam() == get_localentity()->m_iteam())
		return true;
	return false;
}

inline void render_rect(const ImVec2& from, const ImVec2& to, const ImVec4& color, float rounding, uint32_t roundingCornersFlags, float thickness)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	window->DrawList->AddRect(from, to, ImGui::GetColorU32(color), rounding, roundingCornersFlags, thickness);
}

inline void render_text(const ImVec2& from, const char* text, const ImVec4& color)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	window->DrawList->AddText(from, ImGui::GetColorU32(color), text);
}

__int64 __stdcall hk_wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	if (uMsg == WM_KEYDOWN) {
		if (wParam == VK_INSERT)
			b_showmenu = !b_showmenu;
	}

	if (b_showmenu) {
		ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam);
	}

	return CallWindowProc(o_wndproc, hwnd, uMsg, wParam, lParam);
}

long __stdcall hk_present(IDXGISwapChain* p_swapchain, unsigned int syncintreval, unsigned int flags) {

	std::call_once(present, [&] {
		p_swapchain->GetDevice(__uuidof(g_pdevice), (void**)(&g_pdevice));
		g_pdevice->GetImmediateContext(&g_pcontext);
		ID3D11Texture2D *p_backbuff;
		p_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&p_backbuff);

		
		g_pdevice->CreateRenderTargetView(p_backbuff, NULL, &g_prendertargetview);

		//shit way of doing it
		o_wndproc = (WNDPROC)(SetWindowLongPtrA(g_window, GWLP_WNDPROC, uintptr_t(hk_wndproc)));

		ImGui_ImplDX11_Init(g_window, g_pdevice, g_pcontext);
		ImGui_ImplDX11_CreateDeviceObjects();
		ImGuiIO io = ImGui::GetIO();
		ImFontConfig cfg;
		io.Fonts->AddFontFromFileTTF(xorstr_("C:/windows/fonts/smallest_pixel-7.ttf"), 10.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
		io.Fonts->AddFontDefault();
	});


	g_pcontext->OMSetRenderTargets(1, &g_prendertargetview, NULL);


	ImGuiIO io = ImGui::GetIO();
	ImGui_ImplDX11_NewFrame();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
	ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.0f, 0.0f, 0.0f, 0.0f });
	ImGui::Begin(xorstr_("##esp"), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs);

	ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::SetWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y), ImGuiCond_Always);

	for (int i = 0; i < get_entcount(); i++) {
		auto e = get_player(i);

		if (e && e->m_ihealth() > 0) {
			std::cout << e->m_sname() << std::endl;
		}
	}

	if (b_espenable) {
		for (int i = 0; i < get_entcount(); i++) {
			auto e = get_player(i);

			if (e && e->m_ihealth() > 0) {
				std::cout << e->m_sname() << std::endl;
				if (b_chams)
					e->hl_make_glow();

				c_vec pos, pos3d, top, top3d;
				pos3d = e->m_vorigin();
				top3d = pos3d + c_vec(0, 0, 64);

				if (w2s(pos3d, pos) && w2s(top3d, top) )
				{
					int height = (pos.y - top.y);
					int width = height / 2;
					if(b_box)
						render_rect(ImVec2((pos.x - width / 2), top.y), ImVec2((pos.x - width / 2) + width, top.y + height), ImVec4(c_box[0], c_box[1], c_box[2], c_box[3]), 5, 0, 3);
					if (b_health)
						render_rect(ImVec2((pos.x - width / 2) - 4, top.y), ImVec2((pos.x - width / 2) - 4, top.y + e->m_ihealth() * height / e->m_imaxhealth()), ImVec4(0 / 255.f, 255 / 255.f, 47 / 255.f, 1), 1, 0, 3);
					if (b_name)
						render_text(ImVec2((pos.x - width / 2), top.y - 15), e->m_sname(), ImVec4(c_name[0], c_name[1], c_name[2], c_name[3]));
				}
			}
		}
	}

	if (b_miscenable) {

	}
	
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	window->DrawList->PushClipRectFullScreen();

	ImGui::End();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);
	if (b_showmenu) {
		static short tab = 0;

		ImGui::PushFont(io.Fonts->Fonts[0]);
		if (ImGui::Begin(xorstr_("apexlegends internal"), 0, ImVec2(260, 400), -1.f, ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar)) {
			ImGui::BeginChild(xorstr_("##tabs"), ImVec2(80, 100), true, ImGuiWindowFlags_::ImGuiWindowFlags_NoResize); {
				if (ImGui::Button(xorstr_("aimbot"), ImVec2(50, 20)))
					tab = 0;
				if (ImGui::Button(xorstr_("esp"), ImVec2(50, 20)))
					tab = 1;
				if (ImGui::Button(xorstr_("misc"), ImVec2(50, 20)))
					tab = 2;


				ImGui::EndChild();
			}
			ImGui::SameLine();
			ImGui::BeginChild(xorstr_("##main"), ImVec2(0, 0), true, ImGuiWindowFlags_::ImGuiWindowFlags_NoResize); {

				if (tab == 0) {
					ImGui::Checkbox(xorstr_("enable"), &b_aimbotenable);
				}
				else if (tab == 1) {
					ImGui::Checkbox(xorstr_("enable"), &b_espenable);
					ImGui::Checkbox(xorstr_("glow"), &b_chams); ImGui::SameLine(); ImGui::ColorEdit4("glow", c_glow, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoOptions);
					ImGui::Checkbox(xorstr_("box"), &b_box); ImGui::SameLine(); ImGui::ColorEdit4("box", c_box, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoOptions);
					ImGui::Checkbox(xorstr_("health bar"), &b_health); 
					ImGui::Checkbox(xorstr_("name"), &b_name); ImGui::SameLine(); ImGui::ColorEdit4("name", c_name, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoOptions);
				}
				else if (tab == 2) {
					ImGui::Checkbox(xorstr_("enable"), &b_miscenable);
					ImGui::Checkbox(xorstr_("speedhack"), &b_speedhack); ImGui::SameLine(); ImGui::HotKey("##speedhack", &i_speedhack, ImVec2(50, 20));
				}
				ImGui::EndChild();
			}
			ImGui::End();
		}
		ImGui::PopFont();
	}
	ImGui::Render();
	return o_present(p_swapchain, syncintreval, flags);
}

//EasyAntiCheat_launcher.exe - when running without eac

int __stdcall hk_query(LARGE_INTEGER *unused) {
	if (!g_initalizated) {
		AllocConsole();
		freopen("con", "w", stdout);

		apex_base = (uintptr_t)(iat(GetModuleHandleA).get()(xorstr_("r5apex.exe")));
		discord_base = (uintptr_t)(iat(GetModuleHandleA).get()(xorstr_("DiscordHook64.dll")));
		g_window = iat(FindWindowA).get()(0, xorstr_("Apex Legends"));

		uintptr_t dwpresent = memory::occurence(xorstr_("DiscordHook64.dll"), xorstr_("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 41 8B F8 8B F2"));
		o_getasynckeystate = (getasynckeystate_fn)(memory::occurence(xorstr_("DiscordHook64.dll"), xorstr_("40 53 48 83 EC 20 8B D9 FF 15 ? ? ? ?")));
		createhk = (createhook_fn)(memory::occurence(xorstr_("DiscordHook64.dll"), xorstr_("40 53 55 56 57 41 54 41 56 41 57 48 83 EC 60")));
		enablehk = (enablehook_fn)(memory::occurence(xorstr_("DiscordHook64.dll"), xorstr_("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC 20 33 F6 8B FA")));
		enablequehk = (applyqueued_fn)(memory::occurence(xorstr_("DiscordHook64.dll"), xorstr_("48 89 5C 24 ? 48 89 6C 24 ? 48 89 7C 24 ? 41 57")));

		o_getname = (getname_fn)(memory::occurence(xorstr_("r5apex.exe"), xorstr_("48 83 3D ? ? ? ? ? 74 08 8B 51 30")));
		o_worldtoscreen = (worldtoscreen_fn)(memory::occurence(xorstr_("r5apex.exe"), xorstr_("4C 8B DC 53 56 57 48 83 EC 70 8B 41 08 4D 8D 4B 18 F2 0F 10 01 4D 8D 43 BC 89 44 24 58 48 8B DA 49 8D 43 B8")));

		createhk((void*)dwpresent, (void*)hk_present, (void**)&o_present);
		enablehk((void*)dwpresent, 1);
		enablequehk();
		g_initalizated = true;
	}

	return o_query(unused);
}


int __stdcall DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {

	if (fdwReason == 1) {// allows us inject with face or any other injector wich dont support threadhijacking
		auto&& queryperfomance_ptr = find_func("QueryPerformanceCounter", GetModuleHandleA("xaudio2_6.dll"));// you can use kernel32.dll or any other dll(exe too but apex has protected imports) wich imports QueryPerformanceCounter

		if (queryperfomance_ptr == nullptr || *queryperfomance_ptr == nullptr)
			return 0;

		DWORD old_rights, new_rights = PAGE_READWRITE;
		VirtualProtect(queryperfomance_ptr, sizeof(uintptr_t), new_rights, &old_rights);
		o_query = (query_fn)*queryperfomance_ptr;

		*queryperfomance_ptr = hk_query;

		while (!g_initalizated)
			Sleep(100);

		*queryperfomance_ptr = o_query;

		VirtualProtect(queryperfomance_ptr, sizeof(uintptr_t), old_rights, &new_rights);
	}

	return 1;
}

