#include "CBaseUI.h"
#include <Windows.h>
#include <cstdint>
#include <queue>
#include "../../XxharCs/Engine/keydefs.h"
#include "../../XxharCs/Engine/interface.h"
#include "../../XxharCs/Engine/IGameUI.h"
#include "../../XxharCs/clientdll.h"
#include "../../XxharCs/Misc/vmt.h"
#include "imgui_impl_gl.h"
#include "../../XxharCs/IBaseUI.h"

#define MOUSE_BUTTON_COUNT 3
int	g_iMouseOldButtonState = 0;
bool g_bHasInitialized = false;
extern cl_enginefuncs_s gEngfuncs;
extern cl_clientfunc_t g_Client;
extern IBaseUI* g_pGameUI;

void __fastcall Hooked_Initialize(IGameUI*, LPVOID, CreateInterfaceFn*, int);
void __fastcall Hooked_Shutdown(IGameUI*, LPVOID);
int __fastcall Hooked_KeyEvent(IGameUI*, LPVOID, int, int, const char*);
void __fastcall Hooked_CallEngineSurfaceProc(IGameUI*, LPVOID, HWND, uint32_t, WPARAM, LPARAM);
void __fastcall Hooked_Paint(IGameUI*, LPVOID, int, int, int, int);

using FnInitialize = void(__thiscall*)(IGameUI*, CreateInterfaceFn*, int);
FnInitialize g_pfnInitialize = nullptr;

using FnShutdown = void(__thiscall*)(IGameUI*);
FnShutdown g_pfnShutdown = nullptr;

using FnKeyEvent = int(__thiscall*)(IGameUI*, int, int, const char*);
FnKeyEvent g_pfnKeyEvent = nullptr;

using FnCallEngineSurfaceProc = void(__thiscall*)(IGameUI*, HWND, uint32_t, WPARAM, LPARAM);
FnCallEngineSurfaceProc g_pfnCallEngineSurfaceProc = nullptr;

using FnPaint = void(__thiscall*)(IGameUI*, int, int, int, int);
FnPaint g_pfnPaint = nullptr;

CVmtHook g_hookBaseUI;

void BaseUI_InstallHook(void)
{
	if (g_pGameUI == nullptr)
		return;

	// VMT Hook
	PDWORD vmt = *(PDWORD*)g_pGameUI;
	g_hookBaseUI.Init(g_pGameUI);
	g_pfnInitialize = (FnInitialize)g_hookBaseUI.HookFunction(1, Hooked_Initialize);
	g_pfnShutdown = (FnShutdown)g_hookBaseUI.HookFunction(3, Hooked_Shutdown);
	g_pfnKeyEvent = (FnKeyEvent)g_hookBaseUI.HookFunction(4, Hooked_KeyEvent);
	g_pfnCallEngineSurfaceProc = (FnCallEngineSurfaceProc)g_hookBaseUI.HookFunction(5, Hooked_CallEngineSurfaceProc);
	g_pfnPaint = (FnPaint)g_hookBaseUI.HookFunction(6, Hooked_Paint);
	g_hookBaseUI.InstallHook();
}

void IN_MouseEvent(int mstate)
{
	// IMGUI mouse input
	for (int i = 0; i < MOUSE_BUTTON_COUNT; ++i)
	{
		if ((mstate & (1 << i)) && !(g_iMouseOldButtonState & (1 << i)))
		{
			ImGui_ImplGL_MouseButtonCallback(i, 1);
		}
		if (!(mstate & (1 << i)) && (g_iMouseOldButtonState & (1 << i)))
		{
			ImGui_ImplGL_MouseButtonCallback(i, 0);
		}
	}

	g_iMouseOldButtonState = mstate;
	g_Client.IN_MouseEvent(mstate);
}

void __fastcall Hooked_Initialize(IGameUI* _ecx, LPVOID _edx, CreateInterfaceFn* factory, int code)
{
	g_pfnInitialize(_ecx, factory, code);

	ImGui_ImplGL_Init();
	g_bHasInitialized = true;
}

void __fastcall Hooked_Shutdown(IGameUI* _ecx, LPVOID _edx)
{
	ImGui_ImplGL_Shutdown();
	g_pfnShutdown(_ecx);
}

int __fastcall Hooked_KeyEvent(IGameUI* _ecx, LPVOID _edx, int down, int keynum, const char* pszCurrentBinding)
{
	int result = g_pfnKeyEvent(_ecx, down, keynum, pszCurrentBinding);

	// IMGUI input
	switch (keynum)
	{
	case K_MWHEELDOWN:
		ImGui_ImplGL_ScrollCallback(0, -1.0);
		break;
	case K_MWHEELUP:
		ImGui_ImplGL_ScrollCallback(0, 1.0);
		break;
	default:
		ImGui_ImplGL_KeyCallback(keynum, down);
		break;
	}

	return result;
}

void __fastcall Hooked_CallEngineSurfaceProc(IGameUI* _ecx, LPVOID _edx, HWND hwnd, uint32_t msg, WPARAM lParam, LPARAM wParam)
{
	g_pfnCallEngineSurfaceProc(_ecx, hwnd, msg, lParam, wParam);

	static std::queue<BYTE> gCharQueue;
	static BYTE gCharHiByte = 0;
	static BYTE gCharLoByte = 0;

	// PREMISE: 
	// THE GAME WINDOW IS NON-UNICODE

	// 
	// The character input message with IME
	// 
	// WM_IME_CHAR(wParam:C4E3)
	// WM_IME_CHAR(wParam:BAC3)
	// WM_CHAR(wParam:C4)
	// WM_CHAR(wParam:E3)
	// WM_CHAR(wParam:BA)
	// WM_CHAR(wParam:C3)
	// 

	// IMGUI character input
	switch (msg)
	{
		case WM_IME_CHAR:
		{
			// character from IME
			gCharQueue.push(wParam > 0x7f ? 2 : 0);
			break;
		}
		case WM_CHAR:
		{
			if (!gCharQueue.empty()) {
				// This character is from IME
				BYTE& count = gCharQueue.front();
				if (count == 2) {
					// Hold the high byte of the multi byte character
					count--;
					gCharHiByte = (BYTE)wParam;
				}
				else if (count == 1) {
					gCharQueue.pop();
					// Hold the low byte of the multi byte character
					gCharLoByte = (BYTE)wParam;
					// Get a complete multi byte character
					CHAR szMultiByteChar[4] = { (CHAR)gCharHiByte, (CHAR)gCharLoByte, NULL, NULL };
					WCHAR szWideChar[4] = { NULL, NULL, NULL, NULL };
					// Convert to unicode character
					int nLen = MultiByteToWideChar(CP_ACP, 0, szMultiByteChar, -1, szWideChar, 4);
					if (nLen > 1) {		// Included NULL terminated character
						ImGui_ImplGL_CharCallback(szWideChar[0]);
					}
				}
				else {
					// This character is from IME, but don't need convertsion.
					gCharQueue.pop();
					ImGui_ImplGL_CharCallback(wParam);
				}
			}
			else {
				// This character is from keyboard
				ImGui_ImplGL_CharCallback(wParam);
			}

			break;
		}
	}
}

void __fastcall Hooked_Paint(IGameUI* _ecx, LPVOID _edx, int x, int y, int right, int bottom)
{
	g_pfnPaint(_ecx, x, y, right, bottom);

	if (!g_bHasInitialized)
		ImGui_ImplGL_Init();

	ImGui_ImplGL_NewFrame();

	// IMGUI test
	static float f = 0.0f;
	static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	static bool show_demo_window = true;
	static bool show_another_window = false;
	ImGui::Text("Hello, world!");
	ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
	ImGui::ColorEdit3("clear color", (float*)&clear_color);
	if (ImGui::Button("Demo Window"))
		show_demo_window ^= 1;
	if (ImGui::Button("Another Window"))
		show_another_window ^= 1;
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	if (show_another_window)
	{
		ImGui::Begin("Another Window", &show_another_window);
		ImGui::Text("Hello from another window!");
		ImGui::End();
	}
	if (show_demo_window)
	{
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
		ImGui::ShowDemoWindow(&show_demo_window);
	}

	ImGui_ImplGL_Render();
}
