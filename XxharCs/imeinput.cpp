#include "imeinput.h"
#include <windows.h>
#include "clientdll.h"
#include "./Misc/memoryscan.h"
#include "./Misc/xorstr.h"
#include "./detours/detourxs.h"
#include "./Engine/keydefs.h"
#include "detours.h"

extern cl_enginefuncs_s gEngfuncs;
extern cl_enginefuncs_s* g_pEngine;
DetourXS g_detourKeyEvent, g_detourDrawConsoleString;

char *g_pszInputCommand;
int(*g_pfnIsConsoleVisible)(void) = nullptr;
void(*g_pfnKeyMessage)(int key) = nullptr;
int(*g_pfnDrawConsoleString)(int x, int y, char *string) = nullptr;

char g_szInputBuffer[4096];
int* g_pEngineKeyDest = nullptr;

WNDPROC g_pfnOldWindProc = nullptr;
LRESULT CALLBACK Hooked_WndProc(HWND, UINT, WPARAM, LPARAM);

void PatchNonASCIICheck(void);
void Hooked_KeyEvent(int key);
int Hooked_DrawConsoleString(int x, int y, char *string);

typedef struct cmd_function_s
{
	struct cmd_function_s *next;
	char *name;
	void(*function)(void);
	int flags;
}cmd_function_t;

void IME_InstallHook()
{
	for (cmd_function_t* cmd = (cmd_function_t*)gEngfuncs.GetFirstCmdFunctionHandle(); cmd != nullptr; cmd = cmd->next)
	{
		if (stricmp(cmd->name, "toggleconsole") == 0)
		{
			DWORD addr = (DWORD)cmd->function + 0x1;
			g_pfnIsConsoleVisible = (int(*)(void))(addr + *(DWORD *)addr + 0x4);
		}
		else if (stricmp(cmd->name, "escape") == 0)
		{
			DWORD addr = FindPattern((DWORD)cmd->function + 0x40, (DWORD)cmd->function + 0x40 + 0x20,
				XorStr("6A 1B E8"));

			if (addr == 0)
				return;

			addr += 0x3;
			g_pfnKeyMessage = (void(*)(int))(addr + *(DWORD *)addr + 0x4);
		}
		else if (stricmp(cmd->name, "messagemode") == 0)
		{
			DWORD addr = FindPattern((DWORD)cmd->function + 0x10, (DWORD)cmd->function + 0x10 + 0x50,
				XorStr("83 C4 04 50 68"));

			if (addr == 0)
				return;

			addr += 0x5;
			g_pszInputCommand = *(char **)addr;
		}
	}

	HWND window = FindWindowA(XorStr("Valve001"), NULL);
	if (window == NULL)
		return;

	g_pfnOldWindProc = (WNDPROC)GetWindowLongA(window, GWL_WNDPROC);
	SetWindowLongA(window, GWL_WNDPROC, (LONG)Hooked_WndProc);

	PatchNonASCIICheck();

	DWORD addr = FindPattern((DWORD)gEngfuncs.VGui_ViewportPaintBackground + 0x10,
		(DWORD)gEngfuncs.VGui_ViewportPaintBackground + 0x10 + 0x200,
		XorStr("85 C0 75 ? 83 3D ? ? ? ? 01"));

	if (addr == 0)
		return;

	addr += 0xE;
	g_pEngineKeyDest = *(int**)addr;
	g_szInputBuffer[0] = '\0';

	gEngfuncs.pfnConsolePrint(XorStr("IME Input Fix Install\n"));
	gEngfuncs.Con_Printf(XorStr("g_pfnIsConsoleVisible = 0x%X\n"), (DWORD)g_pfnIsConsoleVisible);
	gEngfuncs.Con_Printf(XorStr("g_pszInputCommand = 0x%X\n"), (DWORD)g_pszInputCommand);
	gEngfuncs.Con_Printf(XorStr("g_pEngineKeyDest = 0x%X\n"), (DWORD)g_pEngineKeyDest);
	gEngfuncs.Con_Printf(XorStr("g_pfnKeyMessage = 0x%X\n"), (DWORD)g_pfnKeyMessage);
	gEngfuncs.Con_Printf(XorStr("gEngfuncs.pfnDrawConsoleString = 0x%X\n"), (DWORD)gEngfuncs.pfnDrawConsoleString);

	g_detourKeyEvent.Create(g_pfnKeyMessage, Hooked_KeyEvent);
	g_pfnKeyMessage = (decltype(g_pfnKeyMessage))g_detourKeyEvent.GetTrampoline();

	/*
	// FIXME: 修正相对偏移
	g_detourDrawConsoleString.Create(gEngfuncs.pfnDrawConsoleString, Hooked_DrawConsoleString);
	g_pfnDrawConsoleString = (decltype(g_pfnDrawConsoleString))g_detourDrawConsoleString.GetTrampoline();
	*/

	g_pfnDrawConsoleString = (decltype(g_pfnDrawConsoleString))DetourFunction((PBYTE)gEngfuncs.pfnDrawConsoleString, (PBYTE)Hooked_DrawConsoleString);
}

LRESULT CALLBACK Hooked_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_CHAR)
	{
		if (!g_pfnIsConsoleVisible() && *g_pEngineKeyDest == 1)
		{
			int len = strlen(g_szInputBuffer);

			if (wParam == VK_BACK)
			{
				if (g_szInputBuffer[len - 1] < 0)
					g_szInputBuffer[len - 3] = '\0';
				else
					g_szInputBuffer[len - 1] = '\0';
			}
			else
			{
				g_szInputBuffer[len] = wParam;
				g_szInputBuffer[len + 1] = '\0';
			}

			return 1;
		}
	}
	else if (message == WM_IME_CHAR)
	{
		int len = strlen(g_szInputBuffer);

		if (!g_pfnIsConsoleVisible() && *g_pEngineKeyDest == 1)
		{
			char buf[3];
			buf[0] = wParam >> 8;
			buf[1] = (byte)wParam;
			buf[2] = '\0';
			strcat_s(g_szInputBuffer, UnicodeToUTF8(ANSIToUnicode(buf)));
			return 1;
		}
	}
	
	return CallWindowProcA(g_pfnOldWindProc, hWnd, message, wParam, lParam);
}

void Hooked_KeyEvent(int key)
{
	if (key == K_ENTER || key == K_KP_ENTER)
	{
		static char command[276];
		sprintf_s(command, "%s \"%s\"", g_pszInputCommand, g_szInputBuffer);
		gEngfuncs.pfnClientCmd(command);
		g_szInputBuffer[0] = '\0';
		*g_pEngineKeyDest = 0;
	}
	else if (key == K_ESCAPE)
	{
		g_szInputBuffer[0] = '\0';
		*g_pEngineKeyDest = 0;
	}
}

int Hooked_DrawConsoleString(int x, int y, char * string)
{
	int result = 0;

	if (*g_pEngineKeyDest == 1)
	{
		static int need = 0;

		if (string == g_pszInputCommand)
		{
			need = 3;
			result = g_pfnDrawConsoleString(x, y, string);
		}
		else if (need == 1)
			result = g_pfnDrawConsoleString(x, y, g_szInputBuffer);
		else
			result = g_pfnDrawConsoleString(x, y, string);

		need--;
	}
	else
		result = g_pfnDrawConsoleString(x, y, string);

	return result;
}

wchar_t *UTF8ToUnicode(const char *str)
{
	static wchar_t result[1024];
	int len = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
	MultiByteToWideChar(CP_UTF8, 0, str, -1, result, len);
	result[len] = L'\0';
	return result;
}

wchar_t *ANSIToUnicode(const char *str)
{
	static wchar_t result[1024];
	int len = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
	MultiByteToWideChar(CP_ACP, 0, str, -1, result, len);
	result[len] = '\0';
	return result;
}

char *UnicodeToUTF8(const wchar_t *str)
{
	static char result[1024];
	int len = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_UTF8, 0, str, -1, result, len, NULL, NULL);
	result[len] = '\0';
	return result;
}

char *UnicodeToANSI(const wchar_t *str)
{
	static char result[1024];
	int len = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, str, -1, result, len, NULL, NULL);
	result[len] = '\0';
	return result;
}

void PatchNonASCIICheck(void)
{
	DWORD addr = FindPattern((DWORD)gEngfuncs.pNetAPI->SetValueForKey + 0x1B0,
		(DWORD)gEngfuncs.pNetAPI->SetValueForKey + 0x1B0 + 0xFF,
		XorStr("83 FB 20 7C ? 83 FB 7E"));

	if (addr != 0)
	{
		DWORD oldProtect;
		if (VirtualProtect((LPVOID)addr, 10, PAGE_EXECUTE_READWRITE, &oldProtect))
		{
			// 写 nop
			for (DWORD i = 0; i < 10; i++)
				*(BYTE *)((DWORD)addr + i) = 0x90;

			VirtualProtect((LPVOID)addr, 10, oldProtect, &oldProtect);
		}
	}

	cvar_t *cl_name = gEngfuncs.pfnGetCvarPointer("name");
	if (cl_name != nullptr)
		cl_name->flags &= ~FCVAR_PRINTABLEONLY;
}
