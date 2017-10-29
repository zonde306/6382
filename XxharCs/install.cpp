﻿#include <windows.h>
#include <unordered_map>
#include "install.h"
#include "gamehooking.h"
#include "misc/engine.h"
#include "misc/splice.h"
#include "opengl.h"
#include "./Misc/offsetscan.h"
#include "./Misc/xorstr.h"
#include "./detours/detourxs.h"
#include "clientdll.h"

extern COffsets g_offsetScanner;
extern cl_clientfunc_t *g_pClient;
extern cl_enginefunc_t *g_pEngine;
extern engine_studio_api_t *g_pStudio;
extern PDWORD g_pSlots;
extern PDWORD g_pDynamicSound;
extern PDWORD g_pSpeedBooster;

extern int	HookUserMsg(char *szMsgName, pfnUserMsgHook pfn);
extern void StudioEntityLight(struct alight_s *plight);
extern void PreS_DynamicSound(int entid, DWORD u, char *szSoundFile, float *fOrigin, DWORD dont, DWORD know, DWORD ja, DWORD ck);
extern PreS_DynamicSound_t g_oDynamicSound;
extern decltype(g_pStudio->StudioEntityLight) g_oStudioEntityLight;
DetourXS g_hookDynamicSound;

//////////////////////////////////////////////////////////////////////////
// global offsets
extern cl_enginefuncs_s* pEngfuncs;
extern engine_studio_api_s* pStudio;
extern _CLIENT_* pClient;
extern double* globalTime;

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

	__try { pClient = (_CLIENT_*)(*(DWORD*)(utilsFindPattern((DWORD)Base, Size, (BYTE*)"\x8B\x44\x24\x04\x6A\x00\x68\x00\x00\x00\x00\x68\x00\x00\x00\x00\x50\xC7\x05", "xxxxxxx????x????xxx")+7)); } __except ( EXCEPTION_CONTINUE_EXECUTION ) { pClient = 0; } 
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
void InstallCheat()
{
	// xQPC = SpliceHookFunction(QueryPerformanceCounter, xQueryPerformanceCounter); 

	while (!g_offsetScanner.Initialize())
		Sleep(100);

	DWORD clientBase = g_offsetScanner.ClientBase();
	g_pClient = (cl_clientfunc_t*)g_offsetScanner.ClientFuncs();
	g_pEngine = (cl_enginefunc_t*)g_offsetScanner.EngineFuncs();
	g_pStudio = (engine_studio_api_t*)g_offsetScanner.EngineStudio();
	g_pDynamicSound = (PDWORD)g_offsetScanner.Sound();
	g_pSpeedBooster = (PDWORD)g_offsetScanner.SpeedHackPtr();
	g_pSlots = (PDWORD)g_offsetScanner.Slots();
	g_offsetScanner.GameInfo();

	g_pEngine->Con_Printf(XorStr("clientBase = 0x%X\n"), (DWORD)clientBase);
	g_pEngine->Con_Printf(XorStr("g_pClient = 0x%X\n"), (DWORD)g_pClient);
	g_pEngine->Con_Printf(XorStr("g_pEngine = 0x%X\n"), (DWORD)g_pEngine);
	g_pEngine->Con_Printf(XorStr("g_pStudio = 0x%X\n"), (DWORD)g_pStudio);
	g_pEngine->Con_Printf(XorStr("g_pDynamicSound = 0x%X\n"), (DWORD)g_pDynamicSound);
	g_pEngine->Con_Printf(XorStr("g_pSpeedBooster = 0x%X\n"), (DWORD)g_pSpeedBooster);
	g_pEngine->Con_Printf(XorStr("g_pSlots = 0x%X\n"), (DWORD)g_pSlots);

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

	if(ActivateClient())
		g_pEngine->Con_Printf(XorStr("Client Active\n"));
	if(ActivateEngine())
		g_pEngine->Con_Printf(XorStr("Engine Active\n"));

	// 注册消息获取 Hook
	g_pEngine->pfnHookUserMsg = HookUserMsg;
	g_pEngine->pfnAddCommand = AddCommand;
	g_pEngine->pfnRegisterVariable = RegisterVariable;

	// 启动消息获取
	g_Client.Initialize(g_pEngine, CLDLL_INTERFACE_VERSION);
	g_Client.HUD_Init();

	// 消息获取完成，还原 Hook
	g_pEngine->pfnHookEvent = g_Engine.pfnHookEvent;
	g_pEngine->pfnHookUserMsg = g_Engine.pfnHookUserMsg;
	g_pEngine->pfnAddCommand = g_Engine.pfnAddCommand;
	g_pEngine->pfnRegisterVariable = g_Engine.pfnRegisterVariable;

	pEngfuncs = &g_Engine;
	pClient = (decltype(pClient))&g_Client;
	pStudio = &g_Studio;

	enginePatchEngine();
	InstallGL();
	HookUserMsg2();
}

//////////////////////////////////////////////////////////////////////////
void UninstallCheat()
{
	UninstallGL();
}
