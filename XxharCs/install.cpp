#include <windows.h>
#include <unordered_map>
#include "install.h"
#include "gamehooking.h"
#include "misc/engine.h"
#include "misc/splice.h"
#include "opengl.h"
#include "./Misc/offsetscan.h"
#include "./Misc/xorstr.h"
#include "./detours/detourxs.h"
#include "detours.h"
#include "clientdll.h"
#include "xEngine.h"
#include "peb.h"
#include "eventhook.h"
#include "Engine/exporttable.h"
#include "drawing/drawing.h"
#include "Misc/vmt.h"

#include "Engine/ISurface.h"
#include "Engine/IEngineVGui.h"
#include "Engine/IRunGameEngine.h"
#include "Engine/IGameUI.h"
#include "Engine/IGameConsole.h"
#include "../imgui/goldsrc/CBaseUI.h"
#include "IBaseUI.h"

extern COffsets g_offsetScanner;
extern cl_clientfunc_t *g_pClient;
extern cl_enginefunc_t *g_pEngine;
extern engine_studio_api_t *g_pStudio;
extern cl_clientslots_s* g_pSlots;
extern PDWORD g_pDynamicSound;
extern PDWORD g_pSpeedBooster;
extern PDWORD g_pInitPoint;
extern FnVGuiPaint g_pfnVGuiPaint;
extern bool* g_pSendPacket;
extern FnGetWeaponByID g_pfnGetWeaponByID;
extern PSPLICE_ENTRY g_pSplicVGuiPaint;

extern vgui::IPanel* g_pPanel;
extern vgui::ISurface* g_pSurface;
extern vgui::IEngineVGui* g_pEngineVGui;
extern IRunGameEngine* g_pRunGameEngine;
extern IBaseUI* g_pGameUI;
extern IGameConsole* g_pGameConsole;

extern int	HookUserMsg(char *szMsgName, pfnUserMsgHook pfn);
extern void StudioEntityLight(struct alight_s *plight);
extern void PreS_DynamicSound(int, DWORD, const char*, float[3], float, float, int, int);
extern int __cdecl Hooked_VGuiPaint();
extern PreS_DynamicSound_t g_oDynamicSound;
extern decltype(g_pStudio->StudioEntityLight) g_oStudioEntityLight;
DetourXS g_hookDynamicSound, g_hookFireBullets, g_hookSendPacket;

CVmtHook g_hookPanel;
FnPaintTraverse g_pfnPaintTraverse;

//////////////////////////////////////////////////////////////////////////
// global offsets
extern cl_enginefuncs_s* pEngfuncs;
extern engine_studio_api_s* pStudio;
extern _CLIENT_* pClient;
extern double* globalTime;
extern globalvars_t* g_pGlobals;
extern export_t* g_pExport;

extern cl_enginefunc_t g_Engine;
extern cl_clientfunc_t g_Client;
extern engine_studio_api_t g_Studio;
extern PUserMgsList g_pUserMsgList;
extern std::unordered_map<std::string, FnUserMsgHook> g_userMsgFunc;

//////////////////////////////////////////////////////////////////////////

int XREAD_LONG( unsigned char* gpBuf )
{
	return gpBuf[0] + (gpBuf[1] << 8) + (gpBuf[2] << 16) + (gpBuf[3] << 24);
}

DWORD Base, Size = 0;

void InitOffsets()
{
	DWORD HlBase = 0;
	DWORD HwBase = 0;

	HlBase = (DWORD)GetModuleHandleA(NULL);
	HwBase = (DWORD)GetModuleHandleA((LPCSTR)"hw.dll");

	if(HwBase == 0) 
	{	// 47 protocol
		Base = HlBase;
		Size = 0x02116000;
	} 
	else
	{
		Base = HwBase;
		Size = 0x0122A000;
		//Protocol 48
		//Size = 0x119000;
	}
	

	DWORD enginefunctions = utilsFindPattern(Base, Size, (BYTE*)"invalid sound %i\x0A", "xxxxxxxxxxxxxxxxx")+24;
	pEngfuncs = (cl_enginefuncs_s*)enginefunctions;

	if( IsBadReadPtr((LPCVOID)pEngfuncs->pfnClientCmd, sizeof DWORD) )
	{
		MessageBoxA(0, "BadReadPtr: pEngfuncs->pfnClientCmd", "Fatal Error", MB_OK|MB_ICONERROR);
		ExitProcess(0);
	}

	unsigned char* PushEngstudioPointer = (unsigned char*)utilsFindPattern(Base, Size, 
		(BYTE*)"\x68\x00\x00\x00\x00\x68\x00\x00\x00\x00\x6A\x01\xFF\xD0\x83\xC4\x0C\x85\xC0\x75\x12\x68\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x83\xC4\x04\xE9\x00\x00\x00\x00\xC3",
		"x????x????xxxxxxxxxxxx????x????xxxx????x");
	/* Protocol 48
	unsigned char* PushEngstudioPointer = (unsigned char*)utilsFindPattern(Base, Size, 
		(BYTE*)"\x68\x00\x00\x00\x00\x68\x00\x00\x00\x00\x6A\x01\xFF\xD0\x83\xC4\x0C\x85\xC0\x75\x12\x68\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x83\xC4\x04\xE8\x00\x00\x00\x00\x5D\xC3",
		"x????x????xxxxxxxxxxxx????x????xxxx????xx");*/
	if( !IsBadReadPtr((LPCVOID)PushEngstudioPointer, sizeof DWORD) )
	{
		pStudio = (engine_studio_api_s*)XREAD_LONG(PushEngstudioPointer+1);
		if( IsBadReadPtr((LPCVOID)pStudio, sizeof DWORD) ) 
		{
			MessageBoxA(0, "BadReadPtr: pStudio", "Fatal Error", MB_OK|MB_ICONWARNING);
			ExitProcess(0);
		}
	} else 
	{
		MessageBoxA(0, "BadReadPtr: PushEngstudioPointer", "Fatal Error", MB_OK|MB_ICONWARNING);
		ExitProcess(0);
	}

	__try
	{
		pClient = (_CLIENT_*)(*(DWORD*)(utilsFindPattern((DWORD)Base, Size,
			(BYTE*)"\x8B\x44\x24\x04\x6A\x00\x68\x00\x00\x00\x00\x68\x00\x00\x00\x00\x50\xC7\x05",
			"xxxxxxx????x????xxx")+7));
	}
	__except ( EXCEPTION_CONTINUE_EXECUTION )
	{
		pClient = 0;
	} 
	if ( IsBadReadPtr((void*)pClient, sizeof(_CLIENT_)) )
	{
		MessageBoxA(0, "BadReadPtr: pClient", "Fatal Error", MB_OK|MB_ICONWARNING);
		ExitProcess(0);
	}
}

//////////////////////////////////////////////////////////////////////////
PSPLICE_ENTRY xQPC;
extern BOOL bClientActive;
extern BOOL bEngineActive;
BOOL WINAPI xQueryPerformanceCounter(LARGE_INTEGER* pLI)
{
	return (((BOOL(WINAPI*)(LARGE_INTEGER*))xQPC->trampoline)(pLI));
}

//////////////////////////////////////////////////////////////////////////
DWORD WINAPI InstallCheat(LPVOID params)
{
	// xQPC = SpliceHookFunction(QueryPerformanceCounter, xQueryPerformanceCounter); 
#ifndef _DEBUG
	HideDll((HINSTANCE)params);
#endif

	while (!g_offsetScanner.Initialize())
		Sleep(100);

	DWORD clientBase = g_offsetScanner.ClientBase();
	g_pSlots = *(decltype(g_pSlots)*)g_offsetScanner.Slots();
	g_pClient = (cl_clientfunc_t*)g_offsetScanner.ClientFuncs();
	g_pEngine = (cl_enginefunc_t*)g_offsetScanner.EngineFuncs();
	g_pStudio = (engine_studio_api_t*)g_offsetScanner.EngineStudio();
	g_pDynamicSound = (PDWORD)g_offsetScanner.Sound();
	g_pSpeedBooster = (PDWORD)g_offsetScanner.SpeedHackPtr();
	GetCrossHairTeam = (FnGetCrossHairTeam)g_offsetScanner.GetCurosrTeam();
	g_pCrossHairTeam = (DWORD*)(g_offsetScanner.HwBase + 0x61B82C);
	g_oFireBullets = (FnFireBullets)g_offsetScanner.FireBullets();
	g_pInitPoint = (PDWORD)g_offsetScanner.InitPoint();
	g_pSendPacket = (bool*)g_offsetScanner.SendPacket();
	g_pfnVGuiPaint = (FnVGuiPaint)g_offsetScanner.VGuiPaint();
	g_pfnGetWeaponByID = (FnGetWeaponByID)g_offsetScanner.GetWeaponByID();
	g_pGlobals = (globalvars_t*)g_offsetScanner.GlobalVars();
	g_pExport = (export_t*)g_offsetScanner.ExportPointer();
	g_offsetScanner.GameInfo();

	// 调试用
	g_pEngine->Con_Printf(XorStr("clientBase = 0x%X\n"), (DWORD)clientBase);
	g_pEngine->Con_Printf(XorStr("g_pClient = 0x%X\n"), (DWORD)g_pClient);
	g_pEngine->Con_Printf(XorStr("g_pEngine = 0x%X\n"), (DWORD)g_pEngine);
	g_pEngine->Con_Printf(XorStr("g_pStudio = 0x%X\n"), (DWORD)g_pStudio);
	g_pEngine->Con_Printf(XorStr("g_pDynamicSound = 0x%X\n"), (DWORD)g_pDynamicSound);
	g_pEngine->Con_Printf(XorStr("g_pSpeedBooster = 0x%X\n"), (DWORD)g_pSpeedBooster);
	g_pEngine->Con_Printf(XorStr("g_pSlots = 0x%X\n"), (DWORD)g_pSlots);
	g_pEngine->Con_Printf(XorStr("GetCrossHairTeam = 0x%X\n"), (DWORD)GetCrossHairTeam);
	g_pEngine->Con_Printf(XorStr("g_pFireBullets = 0x%X\n"), (DWORD)g_oFireBullets);
	g_pEngine->Con_Printf(XorStr("g_pInitPoint = 0x%X\n"), (DWORD)g_pInitPoint);
	g_pEngine->Con_Printf(XorStr("g_pSendPacket = 0x%X\n"), (DWORD)g_pSendPacket);
	g_pEngine->Con_Printf(XorStr("g_pfnVGuiPaint = 0x%X\n"), (DWORD)g_pfnVGuiPaint);
	g_pEngine->Con_Printf(XorStr("g_pfnGetWeaponByID = 0x%X\n"), (DWORD)g_pfnGetWeaponByID);
	g_pEngine->Con_Printf(XorStr("g_pGlobals = 0x%X\n"), (DWORD)g_pGlobals);
	g_pEngine->Con_Printf(XorStr("g_pExport = 0x%X\n"), (DWORD)g_pExport);
	g_pEngine->Con_Printf(XorStr("g_pPanel = 0x%X\n"), (DWORD)g_pPanel);
	g_pEngine->Con_Printf(XorStr("g_pEngineVGui = 0x%X\n"), (DWORD)g_pEngineVGui);
	g_pEngine->Con_Printf(XorStr("g_pGameConsole = 0x%X\n"), (DWORD)g_pGameConsole);
	g_pEngine->Con_Printf(XorStr("g_pSurface = 0x%X\n"), (DWORD)g_pSurface);
	g_pEngine->Con_Printf(XorStr("g_pRunGameEngine = 0x%X\n"), (DWORD)g_pRunGameEngine);
	g_pEngine->Con_Printf(XorStr("g_pGameUI = 0x%X\n"), (DWORD)g_pGameUI);

	InitOffsets();
	if (g_pClient == nullptr || g_pEngine == nullptr || g_pStudio == nullptr)
	{
		g_pEngine = pEngfuncs;
		g_pClient = (decltype(g_pClient))pClient;
		g_pStudio = pStudio;

		g_pEngine->pfnConsolePrint(XorStr("MetaHook Plus Detected!\n"));
	}

	// 备份，一会要还原的
	RtlCopyMemory(&g_Client, g_pClient, sizeof(cl_clientfunc_t));
	RtlCopyMemory(&g_Engine, g_pEngine, sizeof(cl_enginefunc_t));
	RtlCopyMemory(&g_Studio, g_pStudio, sizeof(engine_studio_api_t));

	if (g_pDynamicSound != nullptr)
	{
		// 根据声音更新位置 (虽然不是很准确) 的 Hook
		g_hookDynamicSound.Create(g_pDynamicSound, PreS_DynamicSound);
		g_oDynamicSound = (PreS_DynamicSound_t)g_hookDynamicSound.GetTrampoline();
	}

	if (g_oFireBullets != nullptr)
	{
		/*
		// FIXME: 修复 call 地址
		g_hookFireBullets.Create(g_oFireBullets, Hooked_FireBullets);
		g_oFireBullets = (FnFireBullets)g_hookFireBullets.GetTrampoline();
		*/

		// g_oFireBullets = (FnFireBullets)DetourFunction((PBYTE)g_oFireBullets, (PBYTE)Hooked_FireBullets);
	}
	
	if (g_pStudio->StudioEntityLight != nullptr)
	{
		// 检查 Hitbox 或 Bone 用的 Hook
		g_oStudioEntityLight = g_pStudio->StudioEntityLight;
		g_pStudio->StudioEntityLight = StudioEntityLight;
	}

	if (g_pEngine->pfnHookUserMsg != nullptr)
	{
		PBYTE address = (PBYTE)g_pEngine->pfnHookUserMsg;
		g_pUserMsgList = *(PUserMgsList*)(*(PDWORD)(((address += 0x1A) + *(PDWORD)(address + 1) + 5) + 0x0D));

		size_t iterIndex = 0;
		PUserMgsList iter = g_pUserMsgList;
		while (iter != nullptr)
		{
			// 防止陷入无限循环
			if (iterIndex >= 200)
				break;

			// 遍历整个消息 Hook 列表，获取原函数
			g_userMsgFunc[iter->msgname] = iter->address;
			iter = iter->next;
			++iterIndex;
		}

		g_pEngine->Con_Printf(XorStr("UserMsgHook Total: %d\n"), iterIndex);
	}

	/*
	if (g_pfnSendPacket != nullptr)
	{
		g_hookSendPacket.Create(g_pfnSendPacket, Hooked_SendPacket);
		g_pfnSendPacket = (FnSendPacket)g_hookSendPacket.GetTrampoline();
	}
	*/

	/*
	if (g_pfnVGuiPaint != nullptr)
	{
		// g_hookVGuiPaint.Create(g_pfnVGuiPaint, Hooked_VGuiPaint);
		// g_pfnVGuiPaint = (FnVGuiPaint)g_hookVGuiPaint.GetTrampoline();
		// g_pfnVGuiPaint = (FnVGuiPaint)DetourFunction((PBYTE)g_pfnVGuiPaint, (PBYTE)Hooked_VGuiPaint);
		g_pSplicVGuiPaint = SpliceHookFunction(g_pfnVGuiPaint, Hooked_VGuiPaint);
		g_pfnVGuiPaint = (FnVGuiPaint)g_pSplicVGuiPaint->trampoline;
	}
	*/

	if(ActivateClient())
		g_pEngine->Con_Printf(XorStr("Client Active\n"));
	if(ActivateEngine())
		g_pEngine->Con_Printf(XorStr("Engine Active\n"));

	// 注册消息获取 Hook
	g_pEngine->pfnHookUserMsg = HookUserMsg;
	g_pEngine->pfnAddCommand = AddCommand;
	g_pEngine->pfnRegisterVariable = RegisterVariable;
	g_pEngine->pfnHookEvent = HookEvent;
	g_pEngine->pfnPlaybackEvent = PlaybackEvent;

	// 启动消息获取
	g_Client.Initialize(g_pEngine, CLDLL_INTERFACE_VERSION);
	g_Client.HUD_Init();

	// 消息获取完成，还原 Hook
	g_pEngine->pfnHookEvent = g_Engine.pfnHookEvent;
	g_pEngine->pfnHookUserMsg = g_Engine.pfnHookUserMsg;
	g_pEngine->pfnAddCommand = g_Engine.pfnAddCommand;
	g_pEngine->pfnRegisterVariable = g_Engine.pfnRegisterVariable;
	// g_pEngine->pfnPlaybackEvent = g_Engine.pfnPlaybackEvent;

	enginePatchEngine();
	InstallGL();
	HookUserMsg2();
	// BaseUI_InstallHook();

	if (g_pSurface != nullptr)
	{
		drawing::SetupFonts();
		if (g_pPanel != nullptr)
		{
			g_hookPanel.Init(g_pPanel);
			g_pfnPaintTraverse = (FnPaintTraverse)g_hookPanel.HookFunction(41, Hooked_PaintTraverse);
			g_hookPanel.InstallHook();
		}
	}

	pEngfuncs = &g_Engine;
	pClient = (decltype(pClient))&g_Client;
	pStudio = &g_Studio;

	dwClientCmd = (DWORD)pEngfuncs->pfnClientCmd;
	dwCenterPrint = (DWORD)pEngfuncs->pfnCenterPrint;
	dwCreateVisibleEntity = (DWORD)pEngfuncs->CL_CreateVisibleEntity;
	dwGetEntityByIndex = (DWORD)pEngfuncs->GetEntityByIndex;
	dwConsolePrint = (DWORD)pEngfuncs->pfnConsolePrint;
	dwSetScreenFade = (DWORD)pEngfuncs->pfnSetScreenFade;
	dwGetScreenFade = (DWORD)pEngfuncs->pfnGetScreenFade;
	dwSetViewAngles = (DWORD)pEngfuncs->SetViewAngles;
	dwDrawConsoleString = (DWORD)pEngfuncs->pfnDrawConsoleString;
	dwDrawSetTextColor = (DWORD)pEngfuncs->pfnDrawSetTextColor;
	dwGetViewModel = (DWORD)pEngfuncs->GetViewModel;
	dwDrawCharacter = (DWORD)pEngfuncs->pfnDrawCharacter;
	dwGetScreenInfo = (DWORD)pEngfuncs->pfnGetScreenInfo;
	dwDrawConsoleStringLen = (DWORD)pEngfuncs->pfnDrawConsoleStringLen;

	return 0;
}

//////////////////////////////////////////////////////////////////////////
void UninstallCheat()
{
	UninstallGL();
}

