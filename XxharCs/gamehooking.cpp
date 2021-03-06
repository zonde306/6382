﻿#include <Windows.h>
#include <unordered_map>
#include "gamehooking.h"
#include "NoSpread.h"
#include "clientdll.h"
#include "install.h"
#include "./Misc/xorstr.h"
#include "./Misc/offsetscan.h"
#include "swfx.h"
#include "cvars.h"
#include "players.h"

#include "Engine/ISurface.h"
#include "Engine/IEngineVGui.h"
#include "Engine/IRunGameEngine.h"
#include "Engine/IGameUI.h"
#include "Engine/IGameConsole.h"
#include "../imgui/goldsrc/CBaseUI.h"
#include "IBaseUI.h"

//////////////////////////////////////////////////////////////////////////
_CLIENT_* pClient;

BOOL bClientActive = FALSE;
BOOL bEngineActive = FALSE;
FnFireBullets g_oFireBullets = nullptr;

extern cl_enginefuncs_s* pEngfuncs;
extern struct cl_enginefuncs_s gEngfuncs;
extern engine_studio_api_s* pStudio;
extern engine_studio_api_t IEngineStudio;
extern cl_clientfunc_t *g_pClient;
extern cl_enginefunc_t *g_pEngine;
extern engine_studio_api_t *g_pStudio;
extern cl_enginefunc_t g_Engine;
extern cl_clientfunc_t g_Client;
extern engine_studio_api_t g_Studio;

IRunGameEngine* g_pRunGameEngine;
vgui::IPanel* g_pPanel;
IGameConsole* g_pGameConsole;
IBaseUI* g_pGameUI;

PUserMgsList g_pUserMsgList;
std::unordered_map<std::string, FnUserMsgHook> g_userMsgFunc;

extern pfnUserMsgHook TeamInfoOrg;
extern pfnUserMsgHook SetFOVOrg;
extern pfnUserMsgHook CurWeaponOrg;
extern pfnUserMsgHook ScoreAttribOrg;
extern pfnUserMsgHook HealthOrg;
extern pfnUserMsgHook BatteryOrg;
extern pfnUserMsgHook ScoreInfoOrg;
extern pfnUserMsgHook DeathMsgOrg;
extern pfnUserMsgHook SayTextOrg;
extern pfnUserMsgHook ResetHUDOrg;
extern pfnUserMsgHook TextMsgOrg;
extern pfnUserMsgHook DamageOrg;
extern pfnUserMsgHook AmmoXOrg;
extern pfnUserMsgHook WeaponListOrg;
extern pfnUserMsgHook MoneyOrg;
extern pfnUserMsgHook RadarOrg;

//////////////////////////////////////////////////////////////////////////
int	HookUserMsg(char *szMsgName, pfnUserMsgHook pfn)
{
#define REDIRECT_MESSAGE(name) \
		else if (!strcmpi(szMsgName,#name)) \
	{ \
	name##Org = pfn; \
	return gEngfuncs.pfnHookUserMsg (szMsgName, ##name ); \
}

	int retval = gEngfuncs.pfnHookUserMsg(szMsgName, pfn);

	if (0) {}
	REDIRECT_MESSAGE(TeamInfo)
		REDIRECT_MESSAGE(CurWeapon)
		REDIRECT_MESSAGE(ScoreAttrib)
		REDIRECT_MESSAGE(SetFOV)
		REDIRECT_MESSAGE(Health)
		REDIRECT_MESSAGE(Battery)
		REDIRECT_MESSAGE(ScoreInfo)
		REDIRECT_MESSAGE(DeathMsg)
		REDIRECT_MESSAGE(SayText)
		REDIRECT_MESSAGE(TextMsg)
		REDIRECT_MESSAGE(ResetHUD)
		REDIRECT_MESSAGE(Damage)
		REDIRECT_MESSAGE(AmmoX)
		REDIRECT_MESSAGE(Money)
	else if (!strcmp(szMsgName, "WeaponList")) // Because the Class is called like the Msg
	{
		WeaponListOrg = pfn;
		retval = gEngfuncs.pfnHookUserMsg(szMsgName, WeaponList);
		g_Engine.pfnConsolePrint("UserMessages Hooked\n");
	}
	return retval;
}

int HookUserMsg2()
{
	if (gEngfuncs.pfnHookUserMsg == nullptr || g_pUserMsgList == nullptr || g_userMsgFunc.empty())
		return -1;

	int hooked = 0;
#define CHECKHOOK_USERMSG(name)	if(name##Org == nullptr && g_userMsgFunc.find(#name) != g_userMsgFunc.end())\
	{\
		name##Org = g_userMsgFunc[#name];\
		gEngfuncs.pfnHookUserMsg(#name, ##name);\
		++hooked;\
	}

	CHECKHOOK_USERMSG(TeamInfo);
	CHECKHOOK_USERMSG(CurWeapon);
	CHECKHOOK_USERMSG(ScoreAttrib);
	CHECKHOOK_USERMSG(SetFOV);
	CHECKHOOK_USERMSG(Health);
	CHECKHOOK_USERMSG(Battery);
	CHECKHOOK_USERMSG(ScoreInfo);
	CHECKHOOK_USERMSG(DeathMsg);
	CHECKHOOK_USERMSG(SayText);
	CHECKHOOK_USERMSG(TextMsg);
	CHECKHOOK_USERMSG(ResetHUD);
	CHECKHOOK_USERMSG(Damage);
	CHECKHOOK_USERMSG(AmmoX);
	CHECKHOOK_USERMSG(Money);
	CHECKHOOK_USERMSG(WeaponList);

	gEngfuncs.Con_Printf(XorStr("UserMsgHook2nd total: %d\n"), hooked);
	return hooked;
}


//==================================================================================
// Client functions jumpgates...
//==================================================================================
DWORD retaddress;

__declspec(naked)void Gateway2_V_CalcRefdef(void)
{
	__asm
	{
		call PostV_CalcRefdef;
		jmp retaddress;
	}
}

DWORD _CalcRefdef = (DWORD)&Gateway2_V_CalcRefdef;
__declspec(naked)void Gateway1_V_CalcRefdef(void)
{

	__asm
	{
		push esi;
		mov esi, dword ptr ss : [esp + 0x10];
		push esi;
		call PreV_CalcRefdef;
		add esp, 4;
		mov esi, dword ptr ss : [esp + 0x0c];
		mov retaddress, esi;
		push _CalcRefdef;
		pop esi;
		mov dword ptr ss : [esp + 0x0c], esi;
		pop esi;
		ret;
	}
}

__declspec(naked)void Gateway2_CL_CreateMove(void)
{
	__asm
	{
		call CL_CreateMove;
		jmp retaddress;
	}
}

DWORD _CreateMove = (DWORD)&Gateway2_CL_CreateMove;
__declspec(naked)void Gateway1_CL_CreateMove(void)
{
	__asm
	{
		push esi;
		mov esi, dword ptr ss : [esp + 0x28];
		mov retaddress, esi;
		push _CreateMove;
		pop esi;
		mov dword ptr ss : [esp + 0x28], esi;
		pop esi;
		ret;
	}
}
__declspec(naked) void Gateway1_CL_CreateMoveDOD(void)
{
	_asm push esi
	_asm mov esi, dword ptr ss : [esp + 0x30]
		_asm mov retaddress, esi
	_asm push _CreateMove
	_asm pop esi
	_asm mov dword ptr ss : [esp + 0x30], esi
	_asm pop esi
	_asm ret
}

int RedrawResult;
__declspec(naked)void Gateway2_HUD_Redraw(void)
{
	__asm
	{
		mov eax, RedrawResult;
		call HUD_Redraw;
		mov eax, RedrawResult;
		jmp retaddress;
	}
}

DWORD _Redraw = (DWORD)&Gateway2_HUD_Redraw;
__declspec(naked)void Gateway1_HUD_Redraw(void)
{
	__asm
	{
		push esi;
		mov esi, dword ptr ss : [esp + 0x10];
		mov retaddress, esi;
		push _Redraw
			pop esi;
		mov dword ptr ss : [esp + 0x10], esi;
		pop esi;
		ret;
	}
}

__declspec(naked)void Gateway2_HUD_PostRunCmd(void)
{
	__asm
	{
		call HUD_PostRunCmd;
		jmp retaddress;
	}
}

DWORD _PostRunCmd = (DWORD)&Gateway2_HUD_PostRunCmd;
__declspec(naked)void Gateway1_HUD_PostRunCmd(void)
{
	__asm
	{
		push esi;
		mov esi, dword ptr ss : [esp + 0x38];
		mov retaddress, esi;
		push _PostRunCmd
			pop esi;
		mov dword ptr ss : [esp + 0x38], esi;
		pop esi;
		ret;
	}
}
__declspec(naked) void Gateway1_HUD_PostRunCmdDOD(void)
{
	_asm nop
	_asm nop
	_asm nop
	_asm push esi
	_asm mov esi, dword ptr ss : [esp + 0x40]
		_asm mov retaddress, esi
	_asm push _PostRunCmd
	_asm pop esi
	_asm mov dword ptr ss : [esp + 0x40], esi
	_asm pop esi
	_asm ret
}

__declspec(naked)void Gateway2_HUD_PlayerMove(void)
{
	__asm
	{
		call HUD_PlayerMove;
		jmp retaddress;
	}
}

DWORD _PlayerMove = (DWORD)&Gateway2_HUD_PlayerMove;
__declspec(naked)void Gateway1_HUD_PlayerMove(void)
{
	__asm
	{
		push esi;
		mov esi, dword ptr ss : [esp + 0x10];
		mov retaddress, esi;
		push _PlayerMove
			pop esi;
		mov dword ptr ss : [esp + 0x10], esi;
		pop esi;
		ret;
	}
}

__declspec(naked)void Gateway2_HUD_Init(void)
{
	__asm
	{
		call HUD_Init;
		jmp retaddress;
	}
}
DWORD _Init = (DWORD)&Gateway2_HUD_Init;
__declspec(naked)void Gateway1_HUD_Init(void)
{
	__asm
	{
		push esi;
		mov esi, dword ptr ss : [esp + 0x08];
		mov retaddress, esi;
		push _Init
			pop esi;
		mov dword ptr ss : [esp + 0x08], esi;
		pop esi;
		ret;
	}
}
int AddEntResult = 1;
__declspec(naked)void Gateway2_HUD_AddEntity(void)
{
	__asm
	{
		mov AddEntResult, eax;
		call HUD_AddEntity;
		mov eax, AddEntResult;
		jmp retaddress;
	}
}

DWORD _AddEnt = (DWORD)&Gateway2_HUD_AddEntity;
__declspec(naked)void Gateway1_HUD_AddEntity(void)
{

	__asm
	{
		push esi;
		mov esi, dword ptr ss : [esp + 0x14];
		mov retaddress, esi;
		push _AddEnt
			pop esi;
		mov dword ptr ss : [esp + 0x14], esi;
		pop esi;
		ret;
	}
}
void HUD_AddEntity_Substitute(int type, struct cl_entity_s **model, const char **modelname)
{
	HUD_AddEntity(type, *model, *modelname);
}

__declspec(naked) void Gateway1_HUD_AddEntityDOD(void)
{
	__asm
	{
		push esi;
		mov esi, dword ptr ss : [esp + 0xE0];
		mov retaddress, esi;
		push _AddEnt
			pop esi;
		mov dword ptr ss : [esp + 0xE0], esi;
		pop esi;
		ret;
	}
}

int KeyEventResult = 1;
__declspec(naked)void Gateway2_HUD_Key_Event(void)
{
	__asm
	{
		mov KeyEventResult, eax;
		call HUD_Key_Event;
		mov eax, KeyEventResult;
		jmp retaddress;
	}
}

DWORD _KeyEvent = (DWORD)&Gateway2_HUD_Key_Event;
__declspec(naked)void Gateway1_HUD_Key_Event(void)
{

	__asm
	{
		push esi;
		mov esi, dword ptr ss : [esp + 0x14];
		mov retaddress, esi;
		push _KeyEvent
			pop esi;
		mov dword ptr ss : [esp + 0x14], esi;
		pop esi;
		ret;
	}
}

int iUpdateResult;
__declspec(naked) void JumpGate_HUD_UpdateClientData() // Thank you Patrick for
{														 // giving me the Client-
	_asm	mov[iUpdateResult], eax						 // Gateway for this Function
	_asm	call HUD_UpdateClientData
	_asm	mov eax, [iUpdateResult]
		_asm	jmp[retaddress]
}

DWORD _UpdateClientData = (DWORD)&JumpGate_HUD_UpdateClientData;
__declspec(naked) void GateWay_HUD_UpdateClientData()
{
	_asm	push esi
	_asm	mov esi, [esp + 0x10]
		_asm	mov[retaddress], esi
	_asm	push[_UpdateClientData]
		_asm	pop esi
	_asm	mov[esp + 0x10], esi
	_asm	pop esi
	_asm	retn
}

__declspec(naked) void Gateway2_HUD_Frame(void)
{
	__asm call HUD_Frame
	__asm jmp retaddress
}

DWORD _Frame = (DWORD)&Gateway2_HUD_Frame;
__declspec(naked) void Gateway1_HUD_Frame(void)
{
	__asm push esi
	__asm mov esi, dword ptr ss : [esp + 0x0c]
		__asm mov retaddress, esi
	__asm push _Frame
	__asm pop esi
	__asm mov dword ptr ss : [esp + 0x0c], esi
	__asm pop esi
	__asm ret
}

__declspec(naked) void Gateway_NullStub(void)
{
	__asm ret;
}

//////////////////////////////////////////////////////////////////////////
CLIENT gClient = { NULL };
extern COffsets g_offsetScanner;
extern cl_clientslots_s* g_pSlots;

BOOL ActivateClient()
{
	// Copy client  to local structure
	memcpy(&gClient, (LPVOID)g_pClient, sizeof(CLIENT));

	g_Engine.Con_Printf(XorStr("Game Protocol %d\n"), g_offsetScanner.BuildInfo.Protocol);

	DWORD failTick = 0;
	while (GetModuleHandleA(XorStr("vgui2.dll")) == nullptr)
	{
		++failTick;
		Sleep(500);

		if (failTick >= 20)
		{
			g_offsetScanner.BuildInfo.Protocol = 48;
			g_Engine.pfnConsolePrint(XorStr("vgui2.dll Not Found!\n"));
			break;
		}
	}

	if (g_offsetScanner.BuildInfo.Protocol >= 48 || g_offsetScanner.BuildInfo.Build >= 4554 || g_pSlots == nullptr)
	{
		// Hook client  functions
		// g_pClient->Initialize = (INITIALIZE_FUNCTION)&Initialize;
		// g_pClient->HUD_Init = (decltype(g_pClient->HUD_Init))&HUD_Init;
		g_pClient->HUD_Frame = (HUD_FRAME_FUNCTION)&HUD_Frame;
		g_pClient->HUD_Redraw = (decltype(g_pClient->HUD_Redraw))&HUD_Redraw;
		g_pClient->HUD_PlayerMove = (HUD_CLIENTMOVE_FUNCTION)&HUD_PlayerMove;
		g_pClient->CL_CreateMove = (HUD_CL_CREATEMOVE_FUNCTION)&CL_CreateMove;
		g_pClient->V_CalcRefdef = (HUD_V_CALCREFDEF_FUNCTION)&PreV_CalcRefdef;
		g_pClient->HUD_AddEntity = (HUD_ADDENTITY_FUNCTION)&HUD_AddEntity;
		g_pClient->HUD_PostRunCmd = (HUD_POSTRUNCMD_FUNCTION)&HUD_PostRunCmd;
		g_pClient->HUD_Key_Event = (HUD_KEY_EVENT_FUNCTION)&HUD_Key_Event;
		g_pClient->HUD_UpdateClientData = (HUD_UPDATECLIENTDATA_FUNCTION)&HUD_UpdateClientData;
		// g_pClient->HUD_GetStudioModelInterface = (HUD_STUDIO_INTERFACE_FUNCTION)&HUD_GetStudioModelInterface;
		g_pClient->IN_MouseEvent = &IN_MouseEvent;

		g_Engine.pfnConsolePrint(XorStr("Install Defalut Hooks...\n"));
	}
	else
	{
		/*
		g_pSlots[3] = (DWORD)Gateway1_HUD_Redraw;
		g_pSlots[4] = (DWORD)GateWay_HUD_UpdateClientData;
		g_pSlots[6] = (DWORD)Gateway1_HUD_PlayerMove;
		g_pSlots[14] = (DWORD)Gateway1_CL_CreateMove;
		g_pSlots[19] = (DWORD)Gateway1_V_CalcRefdef;
		g_pSlots[20] = (DWORD)Gateway1_HUD_AddEntity;
		g_pSlots[25] = (DWORD)Gateway1_HUD_PostRunCmd;
		g_pSlots[33] = (DWORD)Gateway1_HUD_Frame;
		g_pSlots[34] = (DWORD)Gateway1_HUD_Key_Event;
		*/

		g_pSlots->HUD_Redraw = (decltype(g_pSlots->HUD_Redraw))Gateway1_HUD_Redraw;
		g_pSlots->HUD_UpdateClientData = (decltype(g_pSlots->HUD_UpdateClientData))GateWay_HUD_UpdateClientData;
		g_pSlots->HUD_PlayerMove = (decltype(g_pSlots->HUD_PlayerMove))Gateway1_HUD_PlayerMove;
		g_pSlots->CL_CreateMove = (decltype(g_pSlots->CL_CreateMove))Gateway1_CL_CreateMove;
		g_pSlots->V_CalcRefdef = (decltype(g_pSlots->V_CalcRefdef))Gateway1_V_CalcRefdef;
		g_pSlots->HUD_AddEntity = (decltype(g_pSlots->HUD_AddEntity))Gateway1_HUD_AddEntity;
		g_pSlots->HUD_PostRunCmd = (decltype(g_pSlots->HUD_PostRunCmd))Gateway1_HUD_PostRunCmd;
		g_pSlots->HUD_Frame = (decltype(g_pSlots->HUD_Frame))Gateway1_HUD_Frame;
		g_pSlots->HUD_Key_Event = (decltype(g_pSlots->HUD_Key_Event))Gateway1_HUD_Key_Event;
		g_pSlots->IN_MouseEvent = IN_MouseEvent;

		g_Engine.pfnConsolePrint(XorStr("Install LTFX Slots...\n"));
	}

	// 移除子弹击中墙壁掉灰和火花
	g_pEngine->pEfxAPI->R_BulletImpactParticles = (decltype(g_pEngine->pEfxAPI->R_BulletImpactParticles))Gateway_NullStub;
	g_pEngine->pEfxAPI->R_StreakSplash = (decltype(g_pEngine->pEfxAPI->R_StreakSplash))Gateway_NullStub;
	g_pEngine->pEfxAPI->R_Sprite_Smoke = (decltype(g_pEngine->pEfxAPI->R_Sprite_Smoke))Gateway_NullStub;

	bClientActive = TRUE;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
xcommand_t g_pfnOldNextWeapon = nullptr;
xcommand_t g_pfnOldPrevWeapon = nullptr;

void CmdFunc_NextWeapon(void)
{
	if (g_pWeaponSwitch == nullptr || !g_pWeaponSwitch->m_pFastSwitch->value ||
		gEngfuncs.GetLocalPlayer()->curstate.iuser1 || !Config::fastSwitch)
	{
		if (g_pfnOldNextWeapon != nullptr)
			g_pfnOldNextWeapon();
		return;
	}

	g_pWeaponSwitch->GetNextWpn();
}

void CmdFunc_PrevWeapon(void)
{
	if (g_pWeaponSwitch == nullptr || !g_pWeaponSwitch->m_pFastSwitch->value ||
		gEngfuncs.GetLocalPlayer()->curstate.iuser1 || !Config::fastSwitch)
	{
		if (g_pfnOldPrevWeapon != nullptr)
			g_pfnOldPrevWeapon();
		return;
	}

	g_pWeaponSwitch->GetPrevWpn();
}

int AddCommand(char *cmd_name, void(*function)(void))
{
	//add_log("AddCommand: %s",cmd_name);

	/*
	if (strcmpi(cmd_name, "invnext") == 0)
	{
		g_pfnOldNextWeapon = function;
		gEngfuncs.pfnAddCommand(cmd_name, CmdFunc_NextWeapon);
		return 1;
	}
	else if (strcmpi(cmd_name, "invprev") == 0)
	{
		g_pfnOldPrevWeapon = function;
		gEngfuncs.pfnAddCommand(cmd_name, CmdFunc_PrevWeapon);
		return 1;
	}
	*/

	return 0;
}

cvar_t* RegisterVariable(char *szName, char *szValue, int flags)
{
	//add_log("RegisterVariable: %s",szName);
	cvar_t* pResult = g_Engine.pfnGetCvarPointer(szName);
	if (pResult != NULL)
		return pResult;

	return g_Engine.pfnRegisterVariable(szName, szValue, flags);
}

void Hooked_FireBullets(ULONG	cShots, Vector  vecSrc, Vector	vecDirShooting, Vector	vecSpread, float flDistance, int iBulletType, int iTracerFreq = 4, int iDamage = 0, entvars_t *pevAttacker = NULL)
{
	g_oFireBullets(cShots, vecSrc, vecDirShooting, vecSpread, flDistance, iBulletType, iTracerFreq, iDamage, pevAttacker);
	g_local.vSpread = vecSpread;
	g_local.vSource = vecSrc;
}

//==================================================================================
// Copy enginefuncs_s struct to local gEngfuncs and setup engine hooks
//==================================================================================
BOOL ActivateEngine()
{
	if (IsBadReadPtr((LPCVOID)g_pEngine, sizeof DWORD))
	{
		return FALSE;
	}

	if (g_pEngine->pfnHookUserMsg && g_pEngine->pfnHookEvent)
	{
		memcpy(&gEngfuncs, g_pEngine, sizeof(cl_enginefunc_t));
		if (g_pStudio->GetModelByIndex)
		{
			memcpy(&IEngineStudio, g_pStudio, sizeof(IEngineStudio));
		}
		else
		{
			return FALSE;
		}

		bEngineActive = TRUE;
		return TRUE;
	}

	return FALSE;
}
//==================================================================================