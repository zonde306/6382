#include <Windows.h>
#include <algorithm>
#include <cmath>
#include <math.h>
#include <tchar.h>
#include <fstream>
#include <ctime>

#include "./Engine/keydefs.h"
#include "opengl.h"
#include "gamehooking.h"
#include "players.h"
#include "weaponlist.h"
#include "playeritems.h"
#include "cvars.h"
#include "NoRecoil.h"
#include "NoSpread.h"
#include "Aimbot.h"
#include "eventhook.h"
#include "./Misc/offsetscan.h"
#include "clientdll.h"
#include "./Misc/offsetscan.h"
#include "./Misc/xorstr.h"
#include "./drawing/radar.h"
#include "./drawing/miniradar.h"
#include "./drawing/drawing.h"
#include "menu.h"
#include "opengl.h"
#include "./drawing/tablefont.h"
#include "Aimbot.h"
#include "swfx.h"
#include "./drawing/gui.h"
#include "imeinput.h"

//////////////////////////////////////////////////////////////////////////
// original variables
//////////////////////////////////////////////////////////////////////////
CCvars cvars;
extern bool bAim;
static bool bunnyhop = false;
PColor24 Console_TextColor;

cl_enginefuncs_s gEngfuncs		= { NULL };  
cl_enginefuncs_s g_Engine;
cl_enginefuncs_s* g_pEngine;
cl_enginefuncs_s* pEngfuncs;
engine_studio_api_s* pStudio;
COffsets g_offsetScanner;
PDWORD g_pDynamicSound;
PDWORD g_pSpeedBooster;

double* globalTime;
engine_studio_api_t IEngineStudio		= { NULL };
extern CLIENT gClient;

extern cl_clientfunc_t *g_pClient;
extern cl_clientfunc_t g_Client;
extern cl_enginefunc_t *g_pEngine;
extern engine_studio_api_t *g_pStudio;
extern engine_studio_api_t g_Studio;
extern SCREENINFO g_Screen;
extern char g_szHackDir[256];

cl_clientfunc_t *g_pClient = NULL;
cl_clientslots_s* g_pSlots = NULL;
//cl_enginefunc_t *g_pEngine = NULL;
engine_studio_api_t *g_pStudio = NULL;
// cl_enginefunc_t g_Engine;
cl_clientfunc_t g_Client;
engine_studio_api_t g_Studio;

SCREENINFO g_Screen;

pfnUserMsgHook TeamInfoOrg=NULL;
pfnUserMsgHook SetFOVOrg=NULL;
pfnUserMsgHook CurWeaponOrg=NULL;
pfnUserMsgHook ScoreAttribOrg=NULL;
pfnUserMsgHook HealthOrg=NULL;
pfnUserMsgHook BatteryOrg=NULL;
pfnUserMsgHook ScoreInfoOrg=NULL;
pfnUserMsgHook DeathMsgOrg=NULL;
pfnUserMsgHook SayTextOrg=NULL;
pfnUserMsgHook ResetHUDOrg=NULL;
pfnUserMsgHook TextMsgOrg=NULL;
pfnUserMsgHook DamageOrg=NULL;
pfnUserMsgHook AmmoXOrg=NULL;
pfnUserMsgHook WeaponListOrg=NULL;
pfnUserMsgHook MoneyOrg=NULL;

extern float fCurrentFOV;
extern int	displayCenterX;
extern int	displayCenterY;
extern bool oglSubtractive;
#define NOT_LTFX_SLOTS()		(g_offsetScanner.BuildInfo.Protocol >= 48 || g_pSlots == nullptr)

void ConsolePrintColor(char* fmt, BYTE R, BYTE G, BYTE B, char* csxx)
{
	TColor24 DefaultColor;
	PColor24 Ptr;
	Ptr = Console_TextColor;
	DefaultColor = *Ptr;
	Ptr->R = R;
	Ptr->G = G;
	Ptr->B = B;
	g_Engine.Con_Printf(fmt, csxx);
	*Ptr = DefaultColor;
}

//////////////////////////////////////////////////////////////////////////
// HUD_Init Client Function
//////////////////////////////////////////////////////////////////////////
void HUD_Init ( void )
{
	gClient.HUD_Init();
}

int Initialize( cl_enginefunc_t *pEnginefuncs, int iVersion )
{
	
	return gClient.Initialize(pEnginefuncs, iVersion);
}

void HookEngine( void )
{
	memcpy( &g_Engine, (LPVOID)g_pEngine, sizeof( cl_enginefunc_t ) );
	g_pEngine->pfnHookUserMsg = HookUserMsg;
}

void HookStudio( void )
{
	memcpy( &g_Studio, (LPVOID)g_pStudio, sizeof( engine_studio_api_t ) );
}

static bool Init = false;

void InitHack()
{
	g_Screen.iSize = sizeof( SCREENINFO );
	gEngfuncs.pfnGetScreenInfo( &g_Screen );

	if (g_pWeaponSwitch == nullptr)
		g_pWeaponSwitch = new CSwitchWeaponEffect();

	gEngfuncs.pfnConsolePrint(XorStr("\nXxharCs MultiHack has been loaded\nby XxharCs\n\nCS 1.6 Version:\n-------------------\n"));
	gEngfuncs.Con_Printf(XorStr("Protocol = %d\n"), g_offsetScanner.BuildInfo.Protocol);
	
	// gEngfuncs.Con_Printf("gEngfuncs = 0x%X\n", (DWORD)g_pEngine);
	// gEngfuncs.Con_Printf("gClient = 0x%X\n", (DWORD)g_pClient);
	// gEngfuncs.Con_Printf("gClient = 0x%X\n", (DWORD)g_pStudio);

	IME_InstallHook();
	g_gui.InitFade();
	g_menu.Init();

	gEngfuncs.pfnAddCommand("nextinv", CmdFunc_NextWeapon);
	gEngfuncs.pfnAddCommand("previnv", CmdFunc_PrevWeapon);
	gEngfuncs.pfnClientCmd("alias invnext nextinv");
	gEngfuncs.pfnClientCmd("alias invprev previnv");

	gEngfuncs.pfnClientCmd("version");
	gEngfuncs.pfnClientCmd("toggleconsole");
}

void HUD_Frame( double time )
{
	if(!Init)
	{
		InitHack();
		g_offsetScanner.ConsoleColorInitalize();
		Init = true;
	}

	if(NOT_LTFX_SLOTS())
		gClient.HUD_Frame( time );
}

int HUD_GetStudioModelInterface ( int version, struct r_studio_interface_s **ppinterface, struct engine_studio_api_s *pstudio )
{
	return gClient.HUD_GetStudioModelInterface(version, ppinterface, pstudio);
}

//////////////////////////////////////////////////////////////////////////
// HUD_Redraw Client Function
//////////////////////////////////////////////////////////////////////////
/*
int GetTeam (char * model)
{
	// Check all terror team models
	if (strstr (model, "arctic") ||
		strstr (model, "guerilla") ||
		strstr (model, "leet") ||
		strstr (model, "militia") ||
		strstr (model, "terror"))
			return 1;

	// Check all counter team models
	else if (strstr (model, "gign") ||
		strstr (model, "gsg9") ||
		strstr (model, "sas") ||
		strstr (model, "urban") ||
		strstr (model, "spetsnaz") ||
		strstr (model, "vip"))
			return 2;

	// No Team / Spec / ...
	return 0;
} */

/*
void DrawBox(int x, int y, int r, int g, int b, int a, int iRadius, int iThickness)
{
	int iRad = iRadius / 2;
	// top
	g_pEngine->pfnFillRGBA(x - iRad, y - iRad, iRadius, iThickness, r, g, b, a);
 
	// bottom
	g_pEngine->pfnFillRGBA(x - iRad, y + iRad, iRadius + (iThickness / 2), iThickness, r, g, b, a);
 
	// left
	g_pEngine->pfnFillRGBA(x - iRad, y - iRad, iThickness, iRadius, r, g, b, a);
 
	// right
	g_pEngine->pfnFillRGBA(x + iRad, y - iRad, iThickness, iRadius, r, g, b, a);
}
/*
void DoEspAim (void)
{
	cl_entity_t *pEnt = NULL, *pLocal = gEngfuncs.GetLocalPlayer();
	hud_player_info_t pInfo;
	
	gEngfuncs.pfnGetPlayerInfo(pLocal-> index, &pInfo);
	
	int iOwnTeam = GetTeam(pInfo.model);  

	float fScreenPos[10];
	float fDistFromCenter[3];
	float fCenterDistance;

	
	int iScreenCenterX = gEngfuncs.GetWindowCenterX();
	int iScreenCenterY = gEngfuncs.GetWindowCenterY();

	
	cl_entity_t *pNearestEnt = NULL;
	float fNearest =-1.0f;
	float fNearestAimTarget[3] = {0,0,0}; 

	for (int i = 0; i <33; i++)
	{
		
		if (pLocal->index == i) continue;
		
		
		pEnt = gEngfuncs.GetEntityByIndex(i);
		if (!isValidEnt(pEnt)) continue;

		
		gEngfuncs.pfnGetPlayerInfo(i, &pInfo);
		int iTeam = GetTeam(pInfo.model); 
		pEnt->origin.z -= -20; 
		if (CalcScreen(pEnt->origin, fScreenPos))
        {
			
			fDistFromCenter[0] = iScreenCenterX - fScreenPos[0];
			fDistFromCenter[1] = iScreenCenterY - fScreenPos[1];
 
			
			fCenterDistance = Square(fDistFromCenter[0], 2) + Square(fDistFromCenter[1], 2); 
			if (iTeam == 1) // Team = Terror = Red
			{
				DrawBox(fScreenPos[0], fScreenPos[1], 255, 10, 10, 255, 25, 2);
			}
			else if (iTeam == 2) // Team = CT = Blue
			{
				DrawBox(fScreenPos[0], fScreenPos[1], 10, 10, 255, 255, 25, 2);
			}
			else 
			{
				continue;
			} 
			if (iTeam == iOwnTeam) continue; 

			if (fNearest <0.0f || fCenterDistance < fNearest)
			{
				
				pNearestEnt = pEnt;
				
				fNearest = fCenterDistance;
				
				fNearestAimTarget[0] = fScreenPos[0];
				fNearestAimTarget[1] = fScreenPos[1];
			}
		}
	} 
	if (pNearestEnt != NULL && fNearestAimTarget[0] > 0 && fNearestAimTarget[1] > 0)
	{
		
		#define AIM_SMOOTH 3

		if (GetAsyncKeyState (VK_LBUTTON))
		{
			
			float x = fNearestAimTarget [0] - iScreenCenterX;
			float y = fNearestAimTarget [1] - iScreenCenterY;

			x /= AIM_SMOOTH;
			y /= AIM_SMOOTH;
			
			fNearestAimTarget [0] = iScreenCenterX + x;
			fNearestAimTarget [1] = iScreenCenterY + y;

			
			SetCursorPos((int)fNearestAimTarget[0], (int)fNearestAimTarget[1]);
			
		}
	} 
}

/*
void Aimbot(int ax)
{
	float vec[2];
	if( !CalcScreen(g_playerList[ax].origin(), vec)) { return; }
	DrawConString(0, 550, 0, 255, 0, "vec: %f %f", vec[1],vec[2]);
}*/

static const int Cstrike_SequenceInfo[] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0..9   
	0, 1, 2, 0, 1, 2, 0, 1, 2, 0, // 10..19 
	1, 2, 0, 1, 1, 2, 0, 1, 1, 2, // 20..29 
	0, 1, 2, 0, 1, 2, 0, 1, 2, 0, // 30..39 
	1, 2, 0, 1, 2, 0, 1, 2, 0, 1, // 40..49 
	2, 0, 1, 2, 0, 0, 0, 8, 0, 8, // 50..59 
	0, 16, 0, 16, 0, 0, 1, 1, 2, 0, // 60..69 
	1, 1, 2, 0, 1, 0, 1, 0, 1, 2, // 70..79 
	0, 1, 2, 32, 40, 32, 40, 32, 32, 32, // 80..89
	33, 64, 33, 34, 64, 65, 34, 32, 32, 4, // 90..99
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, // 100..109
	4                                      	// 110
};

time_t g_iNextScreenShotUpdate = 0;
void HUD_Redraw ( float x, int y )
{
	if(NOT_LTFX_SLOTS())
		gClient.HUD_Redraw(x, y);
	
	time_t currentTime = time(NULL);
	if (g_iNextScreenShotUpdate <= currentTime && g_Screen.iWidth > 0 && g_Screen.iHeight > 0 &&
		g_mScreenShotLock.try_lock())
	{
		// 计时，用于每隔 60 秒触发一次
		g_iNextScreenShotUpdate = currentTime + 60;

		if (g_pOglFakeScreenShot == nullptr)
		{
			// 宽 × 高的 RGB 数组
			g_iFakeScreenShotLen = g_Screen.iWidth * g_Screen.iHeight * 3;
			g_pOglFakeScreenShot = new byte[g_iFakeScreenShotLen];
		}
		else if (g_iFakeScreenShotLen < g_Screen.iWidth * g_Screen.iHeight * 3)
		{
			// 因为屏幕大小改变了，重新申请一块新的内存
			delete[] g_pOglFakeScreenShot;
			g_pOglFakeScreenShot = nullptr;
			g_iFakeScreenShotLen = g_Screen.iWidth * g_Screen.iHeight * 3;
			g_pOglFakeScreenShot = new byte[g_iFakeScreenShotLen];
		}

		if (glReadPixels_detour != nullptr)
			glReadPixels_detour(0, 0, g_Screen.iWidth, g_Screen.iHeight, GL_RGB, GL_UNSIGNED_BYTE, g_pOglFakeScreenShot);
		else
			glReadPixels(0, 0, g_Screen.iWidth, g_Screen.iHeight, GL_RGB, GL_UNSIGNED_BYTE, g_pOglFakeScreenShot);

		g_mScreenShotLock.unlock();
		gEngfuncs.pfnConsolePrint(XorStr("AntiScreenShot Updated\n"));
	}
	
	InitVisuals();

	if (g_oglDraw.menu)
	{
		// DrawMenu(50, 200);
		g_menu.MenuDraw((int)Config::radar_y + (int)Config::radar_size + 50);
	}

	if (Config::glNoFlash)
	{
		screenfade_t sf;
		gEngfuncs.pfnGetScreenFade(&sf);

		if (sf.fadealpha > 64)
		{
			sf.fadealpha = 64;
			gEngfuncs.pfnSetScreenFade(&sf);
		}
	}

	if (Config::radar)
	{
		if(mapLoaded)
			overview_redraw();
		else
			DrawRadar();
	}

	if (Config::miniRadar)
		drawRadarFrame();
	else if(Config::crosshair)
		DrawCrosshair((int)Config::crosshair);

	for (int i = 1; i <= 32; ++i)
	{
		if (!bIsEntValid(g_playerList[i].getEnt(), i))
		{
			g_playerList[i].updateClear();
			continue;
		}

		hud_player_info_t info;
		gEngfuncs.pfnGetPlayerInfo(i, &info);
		cl_entity_s* ent = g_playerList[i].getEnt();
		if (i == g_local.entindex || ent->model == nullptr || info.model == nullptr ||
			info.name == nullptr || ent->model->name == nullptr || !ent->player)
			continue;

		byte r = 255, g = 255, b = 255, a = 255;
		if (g_playerList[i].team == 1)
		{
			r = 255;
			g = 64;
			b = 0;
		}
		else if (g_playerList[i].team == 2)
		{
			r = 0;
			g = 255;
			b = 255;
		}

		if (Config::boneEsp)
		{
			g_playerList[i].drawBone(11, 18, r, g, b, a);
			g_playerList[i].drawBone(10, 11, r, g, b, a);
			g_playerList[i].drawBone(10, 6, r, g, b, a);
			g_playerList[i].drawBone(6, 0, r, g, b, a);
			g_playerList[i].drawBone(6, 24, r, g, b, a);
			g_playerList[i].drawBone(25, 24, r, g, b, a);
			g_playerList[i].drawBone(26, 25, r, g, b, a);

			// g_playerList[i].drawBone(0, 44, r, g, b, a);
			// g_playerList[i].drawBone(0, 50, r, g, b, a);

			if (strstr(info.model, "leet") || strstr(info.model, "arctic") || strstr(info.model, "sas"))
			{
				g_playerList[i].drawBone(43, 0, r, g, b, a);
				g_playerList[i].drawBone(49, 0, r, g, b, a);
				g_playerList[i].drawBone(44, 43, r, g, b, a);
				g_playerList[i].drawBone(50, 49, r, g, b, a);
			}
			else
			{
				g_playerList[i].drawBone(45, 0, r, g, b, a);
				g_playerList[i].drawBone(51, 0, r, g, b, a);
				g_playerList[i].drawBone(45, 44, r, g, b, a);
				g_playerList[i].drawBone(48, 47, r, g, b, a);
			}
		}

		if (g_playerList[i].team == 1)
		{
			r = 255;
			g = 0;
			b = 0;
		}
		/*
		else if (g_playerList[i].team == 2)
		{
			r = 0;
			g = 0;
			b = 255;
		}
		*/

		if (Config::radar)
			drawRadarPoint(g_playerList[i].origin(), r, g, b, a, true, 4);
		if(Config::miniRadar)
			drawMiniRadarPoint(g_playerList[i].origin(), r, g, b, true, 4);

		float screen[2];
		bool inScreen = CalcScreen(g_playerList[i].origin(), screen);

		if (Config::nameEsp)
		{
			if (info.name != nullptr && info.name[0] != '\0' && inScreen)
				DrawConStringCenter((int)(screen[0] + 15), (int)(screen[1] + 10), r, g, b, info.name);
		}

		if (Config::weaponEsp)
		{
			model_s* weaponModel = g_Studio.GetModelByIndex(ent->curstate.weaponmodel);
			if (weaponModel != nullptr && weaponModel->name != nullptr &&
				weaponModel->name[0] != '\0')
			{
				char weaponName[32];
				if (!strstr(weaponModel->name, "shield"))
				{
					size_t len = strlen(weaponModel->name + 9) - 4;
					strncpy(weaponName, weaponModel->name + 9, len);
					weaponName[len] = '\0';
				}
				else
				{
					size_t len = strlen(weaponModel->name + 16) - 4;
					strncpy(weaponName, weaponModel->name + 16, len);
					weaponName[len] = '\0';
				}

				if(inScreen)
					DrawConStringCenter((int)(screen[0] + 10), (int)(screen[1]), r, g, b, weaponName);
			}
		}
	}

	if (Config::entityEsp)
	{
		cl_entity_s* ent = nullptr;
		for (int i = 33; i < 1024; ++i)
		{
			ent = gEngfuncs.GetEntityByIndex(i);
			if (ent == nullptr || ent->curstate.messagenum + 10 <= g_local.ent->curstate.messagenum ||
				ent->model == nullptr || ent->model->name == nullptr || ent->player)
				continue;

			float screen[2];
			if (!CalcScreen(ent->origin, screen))
				continue;

			std::string entName = ent->model->name;
			size_t wBegin = entName.find("w_");
			if (wBegin != std::string::npos)
			{
				// 去掉 models/w_
				wBegin += 2;
				entName = entName.substr(wBegin, entName.rfind(".mdl") - wBegin);
				
				if (entName == "thighpack")
				{
					// 给 CT 阵营的玩家显示拆弹钳
					if (g_local.team == 2 && Config::radar)
						drawRadarPoint(ent->origin, 128, 0, 255, 255, true, 2);

					g_tableFont.drawString(true, screen[0], screen[1], 128, 0, 255, "thighpack");
				}
				else if (entName == "backpack")
				{
					if(Config::radar)
						drawRadarPoint(ent->origin, 255, 0, 128, 255, true, 3);
					if (Config::miniRadar)
						drawMiniRadarPoint(ent->origin, 255, 0, 128, true, 3);
					
					g_tableFont.drawString(true, screen[0], screen[1], 255, 0, 128, "backpack");
				}
				else if (entName == "c4")
				{
					if (Config::radar)
						drawRadarPoint(ent->origin, 255, 0, 255, 255, true, 3);
					if (Config::miniRadar)
						drawMiniRadarPoint(ent->origin, 255, 0, 255, true, 3);
					
					g_tableFont.drawString(true, screen[0], screen[1], 255, 0, 255, "c4");
				}
				else
				{
					if (Config::radar)
						drawRadarPoint(ent->origin, 255, 128, 128, 255, true, 4);
					
					g_tableFont.drawString(true, screen[0], screen[1], 255, 128, 128, entName.c_str());
				}
			}
			else if (entName.find("hostage") != std::string::npos)
			{
				if (Config::radar)
					drawRadarPoint(ent->origin, 255, 128, 0, 255, true, 4);
				if(Config::miniRadar)
					drawMiniRadarPoint(ent->origin, 255, 128, 0, true, 4);
				// g_tableFont.drawString(true, screen[0], screen[1], 255, 128, 0, "hostage");
			}
		}
	}

	// 滚轮快速切枪
	if(0)
	{
		if (g_pWeaponSwitch->CanDraw())
			g_pWeaponSwitch->Draw();
	}

	//DoEspAim();
	
	
	/*Aimbot.FindTarget();
	Aimbot.DrawAimSpot();
	for (int ax=0;ax<MAX_VPLAYERS;ax++) if (g_playerList[ax].isUpdated() ) { drawPlayerEsp(ax); }
	for(int i=0;i<MAX_VPLAYERS;i++) 
		if(!bIsEntValid(g_playerList[i].getEnt(), i)) g_playerList[i].updateClear();*/

	/*
	cl_entity_t *pLocal = gEngfuncs.GetLocalPlayer();
	hud_player_info_t pInfo;
	gEngfuncs.pfnGetPlayerInfo(pLocal-> index, &pInfo);
	int iOwnTeam = GetTeam(pInfo.model); 
	
	char data[128];

	for (int ax=0;ax<MAX_VPLAYERS;ax++){
		gEngfuncs.pfnGetPlayerInfo(ax, &pInfo);
		int iTeam = GetTeam(pInfo.model); 

		

		if (g_playerList[ax].isUpdated() && g_playerList[ax].isAlive()){
			
			if(iOwnTeam == iTeam)
			{
				sprintf(data, "[%s] ist in meinem team!", g_playerList[ax].getEnt()->model->name);
				gEngfuncs.pfnDrawConsoleString(10, 10+ax*20, data);
			}
			else
			{
				sprintf(data, "[%s] ist nicht in meinem team!", g_playerList[ax].getEnt()->model->name);
				gEngfuncs.pfnDrawConsoleString(10, 10+ax*20, data);
			}
		}
		 
		if(!bIsEntValid(g_playerList[ax].getEnt(), ax))
			g_playerList[ax].updateClear();
	}
	
	*/

	/*// esp
	for (int ax = 0; ax < MAX_VPLAYERS; ax++){
		if (g_playerList[ax].isUpdated() && g_playerList[ax].isAlive())
		{
			//drawPlayerEsp(ax);
		}
		 
		if(!bIsEntValid(g_playerList[ax].getEnt(), ax))
			g_playerList[ax].updateClear();
	}*/
}

//////////////////////////////////////////////////////////////////////////
// HUD_PlayerMove Client Function
//////////////////////////////////////////////////////////////////////////
void HUD_PlayerMove ( struct playermove_s *ppmove, qboolean server )
{
	g_local.pmMoveType = ppmove->movetype;
	VectorCopy(ppmove->velocity,g_local.pmVelocity);

	// copy origin+angles
	gEngfuncs.pEventAPI->EV_LocalPlayerViewheight(g_local.pmEyePos);	
	g_local.pmEyePos[0] += ppmove->origin[0];
	g_local.pmEyePos[1] += ppmove->origin[1];
	g_local.pmEyePos[2] += ppmove->origin[2];
	g_local.pmFlags = ppmove->flags;
	g_local.maxspeed = ppmove->maxspeed;
	g_local.groundspeed = sqrt(ppmove->velocity[0] * ppmove->velocity[0] + ppmove->velocity[1] * ppmove->velocity[1]);
	g_local.airaccele = ppmove->movevars->airaccelerate;
	g_local.pmMoveType = ppmove->movetype;

	VectorCopy(ppmove->angles,g_local.viewAngles);

	if (NOT_LTFX_SLOTS())
		gClient.HUD_PlayerMove(ppmove, server);
}

//////////////////////////////////////////////////////////////////////////
// CL_CreateMove Client Function
//////////////////////////////////////////////////////////////////////////
void CL_CreateMove ( float frametime, struct usercmd_s *cmd, int active )
{
	if (Config::antiAim)
	{
		// 防止被其他开自动瞄准的一枪解决
		g_local.DoAntiAim2(cmd);
	}
	
	if (NOT_LTFX_SLOTS())
		gClient.CL_CreateMove(frametime, cmd, active);

	// ApplyNoRecoil(frametime, g_local.punchangle, cmd->viewangles);
	if (Config::noRecoil)
		g_local.DoAntiRecoil(cmd, frametime);
	
	//nospread
	if (Config::noSpread)
	{
		/*
		float offset[3];
		gNoSpread.GetSpreadOffset(g_local.spread.random_seed, 1, cmd->viewangles, g_local.pmVelocity, offset);
		cmd->viewangles[0] += offset[0];
		cmd->viewangles[1] += offset[1];
		cmd->viewangles[2] += offset[2];
		*/
		g_local.DoAntiSpread(cmd);
	}

	if(Config::bunnyHop) 
	{
		// 自动连跳
		g_local.DoBunnyHop(cmd);
	}

	if (Config::triggerBot)
	{
		// 自动开枪
		g_local.DoTriggerBot(cmd);
	}

	if (Config::rapidFire)
	{
		// 手枪连射
		g_local.DoAutoPistol(cmd);
	}

	if (Config::speedHack > 0.0f)
	{
		// 加速
		if(cmd->buttons & IN_USE)
			g_local.AdjustSpeed(Config::speedHack);
		else
			g_local.AdjustSpeed(1.0);
	}

	if (Config::fastWalk)
	{
		// 快速静音走路
		g_local.DoFastWalk();
	}

	if (Config::fastRun)
	{
		// 快速跑步
		g_local.DoFastRun(cmd);
	}
	
	if (Config::autoStrafe && g_local.alive)
	{
		if (!(g_local.pmFlags & (FL_ONGROUND | FL_INWATER)) && g_local.groundspeed != 0.0f)
			g_local.DoAutoStrafe(cmd);
	}
}

//////////////////////////////////////////////////////////////////////////
// Called before V_CalcRefdef Client Function
//////////////////////////////////////////////////////////////////////////

extern SCREENINFO	screeninfo;
void PreV_CalcRefdef ( struct ref_params_s *pparams )
{
	if(Config::noRecoil)
	{
		VectorCopy(pparams->punchangle, g_local.punchangle);
		//only for visual NoRecoil:
		VectorClear(pparams->punchangle);

		if(!g_local.alive)
		{
			VectorCopy(pparams->vieworg, g_local.pmEyePos);
		}
	}

	if (NOT_LTFX_SLOTS())
	{
		gClient.V_CalcRefdef(pparams);
		PostV_CalcRefdef(pparams);
	}
}

//==================================================================================
// Called after V_CalcRefdef Client Function
//==================================================================================
void PostV_CalcRefdef ( struct ref_params_s *pparams )
{
	if (Config::quakeGun)
		g_local.DoQuakeGuns((int)Config::quakeGun);

	VectorCopy(pparams->viewangles, mainViewAngles);
	VectorCopy(pparams->vieworg, mainViewOrigin);
}

//////////////////////////////////////////////////////////////////////////
// HUD_AddEntity Client Function
//////////////////////////////////////////////////////////////////////////
extern int AddEntResult;
int HUD_AddEntity ( int type, struct cl_entity_s *ent, const char *modelname )
{
	UpdateMe();
	AddEntResult = 1;
	if( isValidEnt(ent) ) 
	{
		if(g_local.alive)
		{
			gEngfuncs.CL_CreateVisibleEntity(ET_PLAYER, ent);
			AddEntResult = 0;
		}
		g_playerList[ent->index].updateAddEntity(ent->origin); 
		g_playerList[ent->index].setAlive();
		g_playerList[ent->index].updateEntInfo();
		ent->curstate.rendermode = kRenderNormal;	// Against the Evil Admins
		ent->curstate.renderfx = kRenderFxNone;		// and WC3 Mod
		playerCalcExtraData(ent->index, ent);

		if (Config::barrel)
		{
			Vector begin, end, forward, right, up;
			VectorCopy(ent->origin, begin);

			if (ent->curstate.usehull)
				begin[2] += 12.0f;
			else
				begin[2] += 17.0f;
			
			gEngfuncs.pfnAngleVectors(ent->angles, forward, right, up);
			forward[2] = -forward[2];

			begin[0] += forward[0] * 10.0f;
			begin[1] += forward[1] * 10.0f;
			begin[2] += forward[2] * 10.0f;
			
			VectorCopy(begin, g_playerList[ent->index].eyePosition);
			VectorAngles(forward, g_playerList[ent->index].eyeAngles);

			end = begin + forward * 999.0f;

			static int laserbeam = 0;
			if (laserbeam == 0)
				laserbeam = gEngfuncs.pEventAPI->EV_FindModelIndex(XorStr("sprites/laserbeam.spr"));

			byte r = 255, g = 255, b = 255;
			if (g_playerList[ent->index].team == 1)
			{
				r = 255;
				g = 0;
				b = 0;
			}
			else if (g_playerList[ent->index].team == 2)
			{
				r = 0;
				g = 0;
				b = 255;
			}

			gEngfuncs.pEfxAPI->R_BeamPoints(begin, end, laserbeam, 0.001f, Config::barrel,
				0.0f, 32.0f, 2.0f, 0, 0.0f, r / 255.0f, g / 255.0f, b / 255.0f);
		}
	}
	else
	{
		ent->curstate.rendermode = kRenderNormal;
		ent->curstate.renderfx = kRenderFxNone;
		ent->curstate.renderamt = 0;
	}

	if(ent->index == g_local.ent->index) 
	{ 
		int px = ent->index;

	}
	if(g_local.ent->curstate.iuser1 == 4 && g_local.ent->curstate.iuser2 == ent->index)
		AddEntResult = 0;

	if (NOT_LTFX_SLOTS())
		gClient.HUD_AddEntity(type, ent, modelname);

	return AddEntResult;
}

//////////////////////////////////////////////////////////////////////////
// HUD_PostRunCmd Client Function
//////////////////////////////////////////////////////////////////////////
void HUD_PostRunCmd ( struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed )
{
	if (NOT_LTFX_SLOTS())
		gClient.HUD_PostRunCmd(from, to, cmd, runfuncs, time, random_seed);

	if (runfuncs && g_local.iWeaponId > 0)
	{
		g_local.fNextPrimAttack = to->weapondata[g_local.iWeaponId].m_flNextPrimaryAttack;
		g_local.fNextSecAttack = to->weapondata[g_local.iWeaponId].m_flNextSecondaryAttack;
		g_local.bInReloading = (to->weapondata[g_local.iWeaponId].m_fInReload != 0);
		// g_local.iClip = to->weapondata[g_local.iWeaponId].m_iClip;
	}

	gNoSpread.HUD_PostRunCmd(from,to,cmd,runfuncs,time,random_seed);
}

//////////////////////////////////////////////////////////////////////////
// HUD_UpdateClientData Client Function
//////////////////////////////////////////////////////////////////////////
void HUD_UpdateClientData(client_data_t *cdata, float flTime)
{
	for (int i = 0; i < MAX_VPLAYERS ;i++)
	{
		if(!bIsEntValid(g_playerList[i].getEnt(),i))
			g_playerList[i].updateClear();
	}

	WeaponListUpdate(cdata->iWeaponBits);
	g_pWeaponSwitch->m_iBits = cdata->iWeaponBits;

	if (NOT_LTFX_SLOTS())
		gClient.HUD_UpdateClientData(cdata, flTime);
}

//////////////////////////////////////////////////////////////////////////
// HUD_Key_Event Client Function
//////////////////////////////////////////////////////////////////////////
extern int KeyEventResult;
int HUD_Key_Event ( int eventcode, int keynum, const char *pszCurrentBinding )
{
	//DoHLHAiming(eventcode);
	if (keynum == K_INS && eventcode)
	{
		g_menu.MenuActivated = !g_menu.MenuActivated;
		KeyEventResult = 0;
	}
	else if (g_menu.MenuActivated && eventcode && !g_menu.MenuKey(keynum))
		KeyEventResult = 0;
	else if(NOT_LTFX_SLOTS())
		KeyEventResult = gClient.HUD_Key_Event(eventcode, keynum, pszCurrentBinding);

	return KeyEventResult;
}

//////////////////////////////////////////////////////////////////////////
// SPR_Load Engine function
//////////////////////////////////////////////////////////////////////////
HCSPRITE SPR_Load ( const char *szPicName )
{
	return gEngfuncs.pfnSPR_Load( szPicName );
}

//////////////////////////////////////////////////////////////////////////
// SPR_Set Engine Function
//////////////////////////////////////////////////////////////////////////
void SPR_Set ( HCSPRITE hPic, int r, int g, int b )
{
	return gEngfuncs.pfnSPR_Set( hPic, r, g, b );
}

//////////////////////////////////////////////////////////////////////////
// SPR_Draw Engine Function
//////////////////////////////////////////////////////////////////////////
void SPR_Draw ( int frame, int x, int y, const struct rect_s *prc )
{
	return gEngfuncs.pfnSPR_Draw( frame, x, y, prc );
}

//////////////////////////////////////////////////////////////////////////
// SPR_DrawHoles Engine Function
//////////////////////////////////////////////////////////////////////////
void SPR_DrawHoles ( int frame, int x, int y, const struct rect_s *prc )
{
	return gEngfuncs.pfnSPR_DrawHoles( frame, x, y, prc );
}

//////////////////////////////////////////////////////////////////////////
// SPR_DrawAdditive Engine Function
//////////////////////////////////////////////////////////////////////////
void SPR_DrawAdditive ( int frame, int x, int y, const struct rect_s *prc )
{
	return gEngfuncs.pfnSPR_DrawAdditive( frame, x, y, prc );
}


//////////////////////////////////////////////////////////////////////////
// DrawCharacter Engine Function
//////////////////////////////////////////////////////////////////////////
int DrawCharacter ( int x, int y, int number, int r, int g, int b )
{
	return gEngfuncs.pfnDrawCharacter( x, y, number, r, g, b );
}

//////////////////////////////////////////////////////////////////////////
// DrawConsoleString Engine Function
//////////////////////////////////////////////////////////////////////////
int DrawConsoleString ( int x, int y, char *string )
{
	return gEngfuncs.pfnDrawConsoleString( x, y, string );
}

//////////////////////////////////////////////////////////////////////////
// FillRGBA Engine Function
//////////////////////////////////////////////////////////////////////////
void FillRGBA ( int x, int y, int width, int height, int r, int g, int b, int a )
{
	return gEngfuncs.pfnFillRGBA( x, y, width, height, r, g, b, a );
}

//////////////////////////////////////////////////////////////////////////
#include "install.h"
PreS_DynamicSound_t g_oDynamicSound;
void PreS_DynamicSound(int entid, DWORD u, char *szSoundFile, float *fOrigin, DWORD dont, DWORD know, DWORD ja, DWORD ck)
{
	try
	{
		if (entid > 0 && entid <= 32)
		{
			g_playerList[entid].setAlive();
			VectorCopy(fOrigin, g_playerList[entid].SoundOrigin);
		}
	}
	catch (...)
	{

	}
	
	return g_oDynamicSound(entid, u, szSoundFile, fOrigin, dont, know, ja, ck);
}

decltype(g_pStudio->StudioEntityLight) g_oStudioEntityLight;
void StudioEntityLight(struct alight_s *plight)
{
	cl_entity_s * ent = g_Studio.GetCurrentEntity();
	int plindex = ent->index;
	if (ent && ent->player && plindex != -1 && /*plindex != g_local.entindex &&*/
		g_playerList[ent->index].isAlive())
	{
		Vector vBBMin, vBBMax;
		typedef float BoneMatrix_t[MAXSTUDIOBONES][3][4];
		model_t *pModel;
		studiohdr_t *pStudioHeader;
		BoneMatrix_t *pBoneMatrix;
		mstudiobbox_t *pHitbox;
		pModel = g_Studio.SetupPlayerModel(plindex);
		pStudioHeader = (studiohdr_t*)g_Studio.Mod_Extradata(pModel);
		pBoneMatrix = (BoneMatrix_t*)g_Studio.StudioGetBoneTransform();
		
		for (int i = 1; i <= 12; i++)
		{
			pHitbox = (mstudiobbox_t*)((byte*)pStudioHeader + pStudioHeader->hitboxindex);
			if (Config::useHitbox) // hitbox
			{
				VectorTransform(pHitbox[i].bbmin, (*pBoneMatrix)[pHitbox[i].bone], vBBMin);
				VectorTransform(pHitbox[i].bbmax, (*pBoneMatrix)[pHitbox[i].bone], vBBMax);

				g_playerList[plindex].hitbox[i] = (vBBMax + vBBMin) * 0.5f;
			}
			else // bone
			{
				g_playerList[plindex].hitbox[i][0] = (*pBoneMatrix)[i][0][3];
				g_playerList[plindex].hitbox[i][1] = (*pBoneMatrix)[i][1][3];
				g_playerList[plindex].hitbox[i][2] = (*pBoneMatrix)[i][2][3];
			}

			for (int i = 0; i < pStudioHeader->numbones; ++i)
			{
				g_playerList[plindex].bone[i][0] = (*pBoneMatrix)[i][0][3];
				g_playerList[plindex].bone[i][1] = (*pBoneMatrix)[i][1][3];
				g_playerList[plindex].bone[i][2] = (*pBoneMatrix)[i][2][3];
			}
		}
	}

	// g_Studio.StudioEntityLight(plight);
	g_oStudioEntityLight(plight);
}

//////////////////////////////////////////////////////////////////////////
int TeamInfo (const char *pszName, int iSize, void *pbuf)
{
	UpdateMe();
	BEGIN_READ(pbuf,iSize);
	int px = READ_BYTE();
	char * teamtext = READ_STRING();
	const char * STR_TERROR = "TERRORIST"; 
	const char * STR_CT = "CT"; 
	const char * STR_UNASSIGNED = "UNASSIGNED"; 
	const char * STR_SPECTATOR = "SPECTATOR"; 
	if  (!strcmpi (teamtext, STR_TERROR)) g_playerList[px].team = 1; 
	else if  (!strcmpi (teamtext, STR_CT)) g_playerList[px].team = 2; 
	else if  (!strcmpi (teamtext, STR_UNASSIGNED)) g_playerList[px].team = 0; 
	else if  (!strcmpi (teamtext, STR_SPECTATOR)) g_playerList[px].team = 3; 
	else { 
		g_playerList[px].team = -1;
	} 

	if(px == gEngfuncs.GetLocalPlayer()->index)
	{
		if  (!strcmpi (teamtext, STR_TERROR)) g_local.team = 1; 
		else if  (!strcmpi (teamtext, STR_CT)) g_local.team = 2; 
		else if  (!strcmpi (teamtext, STR_UNASSIGNED)) g_local.team = 0; 
		else if  (!strcmpi (teamtext, STR_SPECTATOR)) g_local.team = 3; 
		else { 
			g_local.team = -1; 
		} 
	}
	
	return (*TeamInfoOrg)(pszName, iSize, pbuf);
}

//////////////////////////////////////////////////////////////////////////
int CurWeapon(const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	int iState = READ_BYTE();
	int iID    = READ_CHAR();
	int iClip  = READ_CHAR();
	
	if (iState)
	{
		g_local.iClip = iClip;
		g_local.iWeaponId = iID;

		if (iID != g_pWeaponSwitch->m_iWpn)
		{
			g_pWeaponSwitch->m_iWpn = iID;
			g_pWeaponSwitch->m_fTime = gEngfuncs.GetClientTime() + g_pWeaponSwitch->m_pDisplayTime->value;
		}
	}

	return (*CurWeaponOrg)(pszName,iSize,pbuf);
}

//////////////////////////////////////////////////////////////////////////
int ScoreAttrib(const char *pszName, int iSize, void *pbuf )
{
	UpdateMe();
	
	BEGIN_READ(pbuf, iSize);
	
	int idx  = READ_BYTE();
	int info = READ_BYTE();
	
	if(idx == g_local.ent->index)
		g_local.alive = ((info&1)==0);
	
	return (*ScoreAttribOrg)(pszName,iSize,pbuf);
}

//////////////////////////////////////////////////////////////////////////
int SetFOV(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );
	g_local.iFOV = READ_BYTE();
	
	if(!g_local.iFOV)
	{
		g_local.iFOV = 90;
	}

	if(g_local.iFOV == 90)
	{
		g_local.inZoomMode = false;
	}
	else
	{
		 g_local.inZoomMode = true;
	}
	
	fCurrentFOV = g_local.iFOV;
	
	return (*SetFOVOrg)(pszName,iSize,pbuf);
}

//////////////////////////////////////////////////////////////////////////
int Health(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );
	g_local.iHealth = READ_BYTE();

	return (*HealthOrg)(pszName,iSize,pbuf);
}

//////////////////////////////////////////////////////////////////////////
int Battery(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );
	g_local.iArmor = READ_BYTE();
	return (*BatteryOrg)(pszName,iSize,pbuf);
}

//////////////////////////////////////////////////////////////////////////
int ScoreInfo(const char *pszName, int iSize, void *pbuf)
{
	return (*ScoreInfoOrg)(pszName,iSize,pbuf);
}

//////////////////////////////////////////////////////////////////////////
int DeathMsg(const char *pszName, int iSize, void *pbuf)
{
	UpdateMe();
	BEGIN_READ( pbuf, iSize );
	int killer = READ_BYTE();
	int victim = READ_BYTE();
	int headshot = READ_BYTE();
	char* weaponName = READ_STRING();

	if( killer==g_local.ent->index && headshot)
		g_local.iHs++;

	if( killer==g_local.ent->index && victim != g_local.ent->index )
		g_local.iKills++;

	if( victim==g_local.ent->index )
		g_local.iDeaths++;

	g_playerList[victim].setDead();
	g_playerList[victim].updateClear();

	return (*DeathMsgOrg)(pszName,iSize,pbuf);

}

//////////////////////////////////////////////////////////////////////////
int SayText(const char *pszName, int iSize, void *pbuf)
{
	return (*SayTextOrg)(pszName,iSize,pbuf);
}

//////////////////////////////////////////////////////////////////////////
int TextMsg(const char *pszName, int iSize, void *pbuf)
{
	return (*TextMsgOrg)(pszName,iSize,pbuf);
}

//////////////////////////////////////////////////////////////////////////

int ResetHUD(const char *pszName, int iSize, void *pbuf)
{
	AtRoundStart();

	g_mScreenShotLock.lock();
	delete[] g_pOglFakeScreenShot;
	g_pOglFakeScreenShot = nullptr;
	g_iFakeScreenShotLen = 0;
	g_iNextScreenShotUpdate = time(NULL) + 3;
	g_mScreenShotLock.unlock();

	return (*ResetHUDOrg)(pszName,iSize,pbuf);
}

//////////////////////////////////////////////////////////////////////////
int Damage(const char *pszName, int iSize, void *pbuf )
{
	return (*DamageOrg)(pszName,iSize,pbuf);
}

//////////////////////////////////////////////////////////////////////////
int AmmoX(const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ(pbuf, iSize);

	int id = READ_BYTE();
	int count = READ_BYTE();
	
	WeaponListAmmoX(id, count);
	return (*AmmoXOrg)(pszName,iSize,pbuf);
}

//////////////////////////////////////////////////////////////////////////
int WeaponList(const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ(pbuf, iSize);

	char* weaponName = READ_STRING();
	int ammoType1 = READ_CHAR();
	int maxAmmo1 = READ_BYTE();
	int ammoType2 = READ_CHAR();
	int maxAmmo2 = READ_BYTE();
	int slot = READ_CHAR();
	int slotPosition = READ_CHAR();
	int id = READ_CHAR();
	int flags = READ_BYTE();

	WeaponListAdd(weaponName, ammoType1, maxAmmo1, ammoType2, maxAmmo2, slot, slotPosition, id, flags);
	if (slot < 6)
	{
		g_pWeaponSwitch->ParseWeapon(weaponName, slot, id);
	}

	return (*WeaponListOrg)(pszName,iSize,pbuf);
}

//////////////////////////////////////////////////////////////////////////
int Money(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );
	g_local.iMoney = READ_SHORT();
	return (*MoneyOrg)(pszName,iSize,pbuf);
}

//////////////////////////////////////////////////////////////////////////
