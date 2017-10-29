#include <windows.h>
#include <unordered_map>
#include "gamehooking.h"
#include "NoSpread.h"
#include "clientdll.h"
#include "install.h"
#include "./Misc/xorstr.h"

//////////////////////////////////////////////////////////////////////////
_CLIENT_* pClient;

BOOL bClientActive = FALSE;
BOOL bEngineActive = FALSE;

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

//////////////////////////////////////////////////////////////////////////
int	HookUserMsg (char *szMsgName, pfnUserMsgHook pfn)
{
#define REDIRECT_MESSAGE(name) \
		else if (!strcmpi(szMsgName,#name)) \
	{ \
	name##Org = pfn; \
	return gEngfuncs.pfnHookUserMsg (szMsgName, ##name ); \
}

	int retval = gEngfuncs.pfnHookUserMsg (szMsgName, pfn);
	
	if(0){}
	REDIRECT_MESSAGE( TeamInfo    )
	REDIRECT_MESSAGE( CurWeapon   )
	REDIRECT_MESSAGE( ScoreAttrib )
	REDIRECT_MESSAGE( SetFOV      )
	REDIRECT_MESSAGE( Health	  )
	REDIRECT_MESSAGE( Battery     )
	REDIRECT_MESSAGE( ScoreInfo	  )
	REDIRECT_MESSAGE( DeathMsg    ) 
	REDIRECT_MESSAGE( SayText     )
	REDIRECT_MESSAGE( TextMsg     )
	REDIRECT_MESSAGE( ResetHUD    )
	REDIRECT_MESSAGE( Damage	  )
	REDIRECT_MESSAGE( AmmoX		  )
	REDIRECT_MESSAGE( Money		  )
	else if (!strcmp(szMsgName,"WeaponList")) // Because the Class is called like the Msg
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

//////////////////////////////////////////////////////////////////////////
CLIENT gClient = { NULL };
BOOL ActivateClient()
{
	// Copy client  to local structure
	memcpy( &gClient, (LPVOID)g_pClient, sizeof (CLIENT) );
	
	// Hook client  functions
	g_pClient->Initialize =					( INITIALIZE_FUNCTION)			&Initialize;
	g_pClient->HUD_Init =						( decltype(g_pClient->HUD_Init) )			&HUD_Init;
	g_pClient->HUD_Frame =					( HUD_FRAME_FUNCTION )			&HUD_Frame;
	g_pClient->HUD_Redraw =					( decltype(g_pClient->HUD_Redraw) )			&HUD_Redraw;
	g_pClient->HUD_PlayerMove =				( HUD_CLIENTMOVE_FUNCTION)		&HUD_PlayerMove;
	g_pClient->CL_CreateMove =				( HUD_CL_CREATEMOVE_FUNCTION )	&CL_CreateMove;
	g_pClient->V_CalcRefdef =					( HUD_V_CALCREFDEF_FUNCTION )	&PreV_CalcRefdef;
	g_pClient->HUD_AddEntity =				( HUD_ADDENTITY_FUNCTION )		&HUD_AddEntity;
	g_pClient->HUD_PostRunCmd =				( HUD_POSTRUNCMD_FUNCTION )		&HUD_PostRunCmd;
	g_pClient->HUD_Key_Event =				( HUD_KEY_EVENT_FUNCTION )		&HUD_Key_Event;
	g_pClient->HUD_UpdateClientData =			( HUD_UPDATECLIENTDATA_FUNCTION)&HUD_UpdateClientData;
	g_pClient->HUD_GetStudioModelInterface =  ( HUD_STUDIO_INTERFACE_FUNCTION) &HUD_GetStudioModelInterface;

	bClientActive = TRUE;
	return TRUE;
}

int AddCommand(char *cmd_name, void(*function)(void))
{
	//add_log("AddCommand: %s",cmd_name);
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

//==================================================================================
// Copy enginefuncs_s struct to local gEngfuncs and setup engine hooks
//==================================================================================
BOOL ActivateEngine()
{
	if(IsBadReadPtr((LPCVOID)g_pEngine, sizeof DWORD))
	{
		return FALSE;
	}

	if(g_pEngine->pfnHookUserMsg && g_pEngine->pfnHookEvent )
	{
		memcpy( &gEngfuncs, g_pEngine, sizeof( cl_enginefunc_t ) );
		if( g_pStudio->GetModelByIndex )
		{
			memcpy( &IEngineStudio, g_pStudio, sizeof( IEngineStudio ) );
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