#include <Windows.h>
#include "gamehooking.h"
#include "players.h"
#include "cvars.h"
#include "./drawing/drawing.h"
#include "./drawing/radar.h"
#include "NoSpread.h"
#include "./Misc/xorstr.h"

#pragma comment(lib,"Winmm.lib")

#ifndef M_PI
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif

#define M_PI_F		((float)(M_PI))	// Shouldn't collide with anything.
#ifndef RAD2DEG
#define RAD2DEG(x)  ((float)(x) * (float)(180.f / M_PI_F))
#define RadiansToDegrees RAD2DEG
#endif

#ifndef DEG2RAD
#define DEG2RAD(x)  ((float)(x) * (float)(M_PI_F / 180.f))
#define DegreesToRadians DEG2RAD
#endif

#define BOUND_VALUE(var,min,max) if((var)>(max)){(var)=(max);};if((var)<(min)){(var)=(min);}

int			displayCenterX = 640;
int			displayCenterY = 480;
float		fCurrentFOV = 90;
SCREENINFO	screeninfo;
FnGetCrossHairTeam GetCrossHairTeam;
DWORD* g_pCrossHairTeam;

float mainViewAngles[3];

sMe g_local;
VecPlayers g_playerList;

int Cstrike_SequenceInfo[] = 
{
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0, // 0..9   
	0,	1,	2,	0,	1,	2,	0,	1,	2,	0, // 10..19 
	1,	2,	0,	1,	1,	2,	0,	1,	1,	2, // 20..29 
	0,	1,	2,	0,	1,	2,	0,	1,	2,	0, // 30..39 
	1,	2,	0,	1,	2,	0,	1,	2,	0,	1, // 40..49 
	2,	0,	1,	2,	0,	0,	0,	8,	0,	8, // 50..59 
	0, 16,	0, 16,	0,	0,	1,	1,	2,	0, // 60..69 
	1,	1,	2,	0,	1,	0,	1,	0,	1,	2, // 70..79 
	0,	1,	2, 	32, 40, 32, 40, 32, 32, 32, // 80..89
   	33, 64, 33, 34, 64, 65, 34, 32, 32, 4, // 90..99
	4,	4,	4,	4,	4,	4,	4,	4,	4,	4, // 100..109
	4                                      	// 110
};

//////////////////////////////////////////////////////////////////////////
void InitVisuals(void)
{
	static bool bOnce = false;
	if(bOnce) return;
	screeninfo.iSize = sizeof(SCREENINFO);
	gEngfuncs.pfnGetScreenInfo(&screeninfo);
	displayCenterX = screeninfo.iWidth/2;
	displayCenterY = screeninfo.iHeight/2;
	bOnce = true;
}

//////////////////////////////////////////////////////////////////////////
bool CalcScreen(const float *origin, float *vecScreen)
{
	int result = gEngfuncs.pTriAPI->WorldToScreen( const_cast<float*>(origin),vecScreen);

	if(vecScreen[0] < 1 && vecScreen[1] < 1 && vecScreen[0] > -1 && vecScreen[1] > -1 && !result)
	{
		vecScreen[0] =  vecScreen[0] * displayCenterX + displayCenterX;
		vecScreen[1] = -vecScreen[1] * displayCenterY + displayCenterY;
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////




void drawPlayerEsp(int ax)
{
	if(strstr(g_playerList[ax].getName(), "\\noname\\")) return;

	
	// rest of code is only for players on screen:
	float vecScreen[2];
	if( !CalcScreen(g_playerList[ax].origin(),vecScreen) ) { return; }	

	// distance and boxradius also needed for esp offset
	float distance = g_playerList[ax].distance/22.0f;
	extern float fCurrentFOV;
	int   boxradius = (int)((300.0f * 90.0f) / (distance * fCurrentFOV));	 
	BOUND_VALUE(boxradius,1,200);

	gDrawBoxAtScreenXY((int)vecScreen[0],(int)vecScreen[1],0,255,0,150,boxradius);
	
}


//////////////////////////////////////////////////////////////////////////
static inline void strcpy_x2(char* dest, const char* pos)
{
	do{ 
		if(*pos=='.'||*pos=='-'||*pos=='_') {*dest=0;break; } // cut off ".wav"
		*dest=*pos;
		++dest;
		++pos; 
	}
	while(*pos);
}

bool GetWorldToScreenMatrix(WorldToScreenMatrix_t *pWorldToScreenMatrix, float *flOrigin, float *flOut)
{
	flOut[0] = pWorldToScreenMatrix->flMatrix[0][0] * flOrigin[0] + pWorldToScreenMatrix->flMatrix[1][0] * flOrigin[1] + pWorldToScreenMatrix->flMatrix[2][0] * flOrigin[2] + pWorldToScreenMatrix->flMatrix[3][0];
	flOut[1] = pWorldToScreenMatrix->flMatrix[0][1] * flOrigin[0] + pWorldToScreenMatrix->flMatrix[1][1] * flOrigin[1] + pWorldToScreenMatrix->flMatrix[2][1] * flOrigin[2] + pWorldToScreenMatrix->flMatrix[3][1];
	float flZ = pWorldToScreenMatrix->flMatrix[0][3] * flOrigin[0] + pWorldToScreenMatrix->flMatrix[1][3] * flOrigin[1] + pWorldToScreenMatrix->flMatrix[2][3] * flOrigin[2] + pWorldToScreenMatrix->flMatrix[3][3];

	if (flZ != 0.0f)
	{
		float flTmp = 1.0f / flZ;
		flOut[0] *= flTmp;
		flOut[1] *= flTmp;
	}

	return (flZ <= 0.0f);
}

bool WorldToScreen(float *flOrigin, float *flOut)
{
	WorldToScreenMatrix_t *pWorldToScreenMatrix = (WorldToScreenMatrix_t*)((DWORD)GetModuleHandleA("hw.dll") + 0xE956A0);
	if (!GetWorldToScreenMatrix(pWorldToScreenMatrix, flOrigin, flOut))
	{
		flOut[0] = (flOut[0] * (screeninfo.iWidth / 2)) + screeninfo.iWidth / 2;
		flOut[1] = (-flOut[1] * (screeninfo.iHeight / 2)) + screeninfo.iHeight / 2;
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
static inline float GetDistanceFrom(const float* pos);
void playerSound(int index, const float*const origin, const char*const sample)
{
	UpdateMe();
	if(index == g_local.ent->index) return;

	// fill infos:
	PlayerInfo& player = g_playerList[index];

	// calc player data 
	player.updateEntInfo();
	player.updateSoundRadar();
	player.SetOrigin(origin);
	
	player.distance  = GetDistanceFrom (player.origin());
	if(player.distance<1)
		player.distance=1; // avoid division by zero
	
	player.visible   = false;

	// set weapon
	if( sample && sample[0]=='w' && sample[1]=='e' && sample[7]=='/' ) // ex: weapons/ak-1.wav
	{
		const char* pos = sample+8;
		if(strlen(pos)<30)
		{
			char tmp[32];
			strcpy_x2(tmp,pos);

			if(!strcmp(tmp,"zoom")){ strcpy(tmp,"awp"); } // zooming means usually awp

			if(strcmp(tmp,"knife")) // dont update for knife
			{
				player.setWeapon(tmp);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void UpdateMe(void)
{
	g_local.ent    = gEngfuncs.GetLocalPlayer();
	g_local.entindex = (g_local.ent != nullptr ? g_local.ent->index : -1);

	static cl_entity_s dummy;
	memset((char*)&dummy,0,sizeof(dummy));
	if(!g_local.ent){ g_local.ent = &dummy; }
}

//////////////////////////////////////////////////////////////////////////
bool isValidEnt(cl_entity_s *ent) 
{

	if(ent && (ent != gEngfuncs.GetLocalPlayer())  && ent->player && !ent->curstate.spectator &&
		ent->curstate.solid && !(ent->curstate.messagenum < gEngfuncs.GetLocalPlayer()->curstate.messagenum)) 
		return true;
	else 
		return false;
}

//////////////////////////////////////////////////////////////////////////
// fix for ogc
bool bIsEntValid(cl_entity_s * ent,int index)
{

	if (g_playerList[index].updateType() == 2 || g_playerList[index].timeSinceLastUpdate() < 0.3)
		return true;
	UpdateMe(); // Avoid Crash
	cl_entity_s* localEnt = gEngfuncs.GetEntityByIndex(g_local.ent->index);

	if(ent && !(ent->curstate.effects & EF_NODRAW) && ent->player && !ent->curstate.spectator 
		&& ent->curstate.solid && !(ent->curstate.messagenum < localEnt->curstate.messagenum))
		return true;

	return false;
}

//////////////////////////////////////////////////////////////////////////
void playerCalcExtraData(int ax, cl_entity_s* ent)
{
	PlayerInfo& r = g_playerList[ax];

	// playername and model
	r.updateEntInfo();

	// distance
	r.distance  = GetDistanceFrom (r.origin());
	
	if(r.distance<1)
		r.distance=1; // avoid division by zero
	
	g_playerList[ax].bGotHead = false;
}

char* gGetWeaponName( int weaponmodel ) 
{ 
    static char weapon[50]; 
    weapon[0]=0; 
    model_s* mdl = pStudio->GetModelByIndex( weaponmodel ); 
    if( !mdl ){ return weapon; } 
    char* name = mdl->name;  if( !name )          { return weapon; } 
     
    int len = strlen(name);  if( len>48 || len<10){ return weapon; } 
    if (len > 25)//to catch shield string 
    { 
        strcpy(weapon,name+23); len -= 23; 
    } 
    else//no shield 
    { 
        strcpy(weapon,name+9); len -=9; 
    } 
    if(len>4)weapon[len-4]=(char)0; 
        return weapon; 
} 

//////////////////////////////////////////////////////////////////////////
static inline float GetDistanceFrom(const float* pos)
{	
	register double a = pos[0] - g_local.pmEyePos[0];
	register double b = pos[1] - g_local.pmEyePos[1];
	register double c = pos[2] - g_local.pmEyePos[2];
	return sqrtf((float)(a * a + b * b + c * c));
}


//==================================================================================
// AtMapChange
//==================================================================================
void AtMapChange()
{
	g_local.iKills = 0;
	g_local.iHs = 0;
	g_local.iDeaths = 0;
	mapLoaded = false;
	overview_loadcurrent();
}

//==================================================================================
// AtRoundStart - Called every start of a round
//==================================================================================
void AtRoundStart()
{
	static char currentMap[100];
	if( strcmp(currentMap,gEngfuncs.pfnGetLevelName()) )
	{
		strcpy(currentMap,gEngfuncs.pfnGetLevelName());
		AtMapChange();
	}

	for(int i=0;i<MAX_VPLAYERS;i++) 
	{ 
		g_playerList[i].updateClear();
		g_playerList[i].bGotHead = false;
		g_playerList[i].setAlive();
		g_playerList[i].killed = false;
		g_playerList[i].deathTotal = 0;
		g_playerList[i].deathPosition = Vector();
	}

}

bool bIsEntValid(cl_entity_s * ent)
{
	cl_entity_s* localEnt = gEngfuncs.GetLocalPlayer();

	if(ent && ent != localEnt && !(ent->curstate.effects & EF_NODRAW) && ent->player && !ent->curstate.spectator 
		&& ent->curstate.solid && !(ent->curstate.messagenum < localEnt->curstate.messagenum))
		return true;

	return false;
}

bool ValidTarget(cl_entity_s* ent)
{
	int nPlayers = g_playerList.size();
	if(!bIsEntValid(ent)) { return false; }

	int entindex = ent->index;

	for(int px = 0; px < nPlayers; px++)
	{
		PlayerInfo& pi = g_playerList[px];

		if( (pi.getEnt() == ent) && (pi.team != g_local.team) )
		{
			return true;
		}
	}

	return false;
}

/*
void sMe::DoBunnyHop(struct usercmd_s *usercmd)
{
	if(!(g_local.pmFlags & FL_ONGROUND)) 
	{
		//gEngfuncs.pfnClientCmd("-jump");
		usercmd->buttons &= ~IN_JUMP;
	}
	else if(g_local.pmFlags & FL_ONGROUND)
	{
		//gEngfuncs.pfnClientCmd("+jump");
		usercmd->buttons &= IN_JUMP;
	}
}*/

void sMe::DoBunnyHop(struct usercmd_s *usercmd)
{
    if((usercmd->buttons & IN_JUMP) && !(this->pmFlags & FL_ONGROUND)) 
    {
        usercmd->buttons &= ~IN_JUMP;
    }
}

void sMe::DoAutoPistol(usercmd_s * usercmd)
{
	if (this->iWeaponId == WEAPONLIST_GLOCK18 || this->iWeaponId == WEAPONLIST_USP ||
		this->iWeaponId == WEAPONLIST_P228 || this->iWeaponId == WEAPONLIST_DEAGLE ||
		this->iWeaponId == WEAPONLIST_FIVESEVEN || this->iWeaponId == WEAPONLIST_ELITE)
	{
		static bool lastFired = false;
		if (this->iClip <= 0 && (usercmd->buttons & IN_ATTACK))
			usercmd->buttons &= ~IN_ATTACK;

		if ((usercmd->buttons & IN_ATTACK) && this->fNextPrimAttack <= (usercmd->msec / 1000))
		{
			if ((usercmd->buttons & IN_ATTACK) && lastFired)
			{
				usercmd->buttons &= ~IN_ATTACK;
				lastFired = false;
			}
			else
			{
				usercmd->buttons |= IN_ATTACK;
				lastFired = true;
			}
		}
	}
}

void _fastcall VectorAngles(const float *forward, float *angles)
{
	float tmp, yaw, pitch;
	if (forward[1] == 0 && forward[0] == 0)
	{
		yaw = 0;
		if (forward[2] > 0)
			pitch = 270;
		else
			pitch = 90;
	}
	else
	{
		yaw = (atan2(forward[1], forward[0]) * 180 / M_PI_F);
		if (yaw < 0)
			yaw += 360;
		tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
		pitch = (atan2(-forward[2], tmp) * 180 / M_PI_F);
		if (pitch < 0)
			pitch += 360;
	}
	angles[0] = pitch;
	angles[1] = yaw;
	angles[2] = 0;
	while (angles[0]<-89) { angles[0] += 180; angles[1] += 180; }
	while (angles[0]>89) { angles[0] -= 180; angles[1] += 180; }
	while (angles[1]<-180) { angles[1] += 360; }
	while (angles[1]>180) { angles[1] -= 360; }
}

#ifndef M_PI
#define M_PI			3.14159265358979323846
#define M_PI_F			(float)M_PI
#endif

void sMe::DoTriggerBot(usercmd_s * usercmd)
{
	if (this->iClip <= 0)
		return;
	
	/*
	if (GetCrossHairTeam != nullptr)
	{
		// 准星显示敌人的信息，1=队友，2=敌人
		int _team = GetCrossHairTeam();

		if (_team == 1)
			usercmd->buttons &= ~IN_ATTACK;
		else if (_team == 2)
		{
			usercmd->buttons |= IN_ATTACK;
			return;
		}
	}
	*/

	if (g_pCrossHairTeam != nullptr)
	{
		if(*g_pCrossHairTeam == 1)
			usercmd->buttons &= ~IN_ATTACK;
		else if (*g_pCrossHairTeam == 2)
		{
			usercmd->buttons |= IN_ATTACK;
			return;
		}
	}

	if (usercmd->buttons & IN_ATTACK)
		return;

	// float finalAngle[3] = { 0.0f, 0.0f, 0.0f };
	Vector finalAngle;

	if (Config::noRecoil)
	{
		finalAngle[0] = usercmd->viewangles.x;
		finalAngle[1] = usercmd->viewangles.y;
	}
	else
	{
		finalAngle[0] = usercmd->viewangles.x + this->punchangle[0];
		finalAngle[1] = usercmd->viewangles.y + this->punchangle[1];
	}

	// gEngfuncs.GetViewAngles(finalAngle);
	finalAngle[2] = 0.0f;
	
	Vector forward, right, up;
	gEngfuncs.pfnAngleVectors(finalAngle, forward, right, up);
	forward *= 3000.0f;
	forward = Vector(this->pmEyePos) + right;
	
	pmtrace_s* trace = gEngfuncs.PM_TraceLine(this->pmEyePos, forward, PM_TRACELINE_ANYVISIBLE,
		PM_STUDIO_BOX | PM_GLASS_IGNORE, this->entindex);
	if (trace->ent > 0 && trace->ent <= 32 && trace->ent != this->entindex)
	{
		this->iCrosshairId = trace->ent;
		if (g_playerList[trace->ent].isAlive() && g_playerList[trace->ent].team != this->team)
		{
			usercmd->buttons |= IN_ATTACK;
			return;
		}
	}

	static float EntViewVec[3], distance;
	for (int i = 1; i <= 32; ++i)
	{
		if (i == this->entindex || !g_playerList[i].isAlive() || g_playerList[i].team == this->team)
			continue;

		VectorCopy(g_entityDrawList[i].Origin, EntViewVec);
		EntViewVec[2] += 25.0f;

		// 获取瞄准向量
		VectorSubtract(EntViewVec, this->pmEyePos, EntViewVec);

		// 获取瞄准角度
		VectorAngles(EntViewVec, up);

		// 确保角度是正确的
		up[0] = -up[0];
		if (up[1] > 180.0f)
			up[1] -= 360.0f;

		// 获取角度差异
		VectorSubtract(finalAngle, up, forward);

		// 确保角度是正确的
		if (forward[1] > 180.0f)
			forward[1] -= 360.0f;

		// 检查平移差异
		if (forward[1] > 45.0f || forward[1] < -45.0f)
			continue;

		// 两个角度之间的差异
		forward[2] = sqrtf(forward[0] * forward[0] + forward[1] * forward[1]);
		distance = sqrtf(EntViewVec[0] * EntViewVec[0] + EntViewVec[1] * EntViewVec[1]);

		// 计算差异范围
		distance = atan2f(25.0f, distance) * 180.0f / M_PI_F;

		this->aimTrigRadius = distance;
		this->aimTrigDiff = forward[2];

		if (forward[2] < distance)
		{
			usercmd->buttons |= IN_ATTACK;
			return;
		}
	}
}

//bInRad - return true if crosshair in trigger radius, also calculation prospread.
bool sMe::bInRad(float fScreen[2], float Fov, struct usercmd_s* cmd)
{
	if (Config::trigger_prospread > 0.0f)
	{
		Vector spreadangles, forward, spread;
		pmtrace_t tr;

		VectorAdd(cmd->viewangles, this->punchangle, spreadangles);

		gEngfuncs.pfnAngleVectors(spreadangles, forward, NULL, NULL);

		gEngfuncs.pEventAPI->EV_SetTraceHull(2);
		gEngfuncs.pEventAPI->EV_PlayerTrace(this->pmEyePos, this->pmEyePos +
			(forward * 3000.0f), PM_GLASS_IGNORE, -1, &tr);

		CalcScreen(tr.endpos, spread);

		spread.x = screeninfo.iWidth - spread.x;
		spread.y = screeninfo.iHeight - spread.y;

		float fScreenFinal[2] = {
			(float)(abs(spread.x - fScreen[0])),
			(float)(abs(spread.y - fScreen[1]))
		};

		return (fScreenFinal[0] <= Fov && fScreenFinal[1] <= Fov);
	}
	else
	{
		float ScreenOut[2] = {
			(float)(abs((screeninfo.iWidth / 2) - fScreen[0])),
			(float)(abs((screeninfo.iHeight / 2) - fScreen[1]))
		};

		return (ScreenOut[0] <= Fov && ScreenOut[1] <= Fov);
	}
}

void sMe::CorrectMovement(const Vector & vOldAngles, usercmd_s * pCmd, float fOldForward, float fOldSidemove)
{
	float deltaView = pCmd->viewangles[1] - vOldAngles[1];
	float f1;
	float f2;

	if (vOldAngles[1] < 0.f)
		f1 = 360.0f + vOldAngles[1];
	else
		f1 = vOldAngles[1];

	if (pCmd->viewangles[1] < 0.0f)
		f2 = 360.0f + pCmd->viewangles[1];
	else
		f2 = pCmd->viewangles[1];

	if (f2 < f1)
		deltaView = abs(f2 - f1);
	else
		deltaView = 360.0f - abs(f1 - f2);
	deltaView = 360.0f - deltaView;

	pCmd->forwardmove = cos(DEG2RAD(deltaView)) * fOldForward + cos(DEG2RAD(deltaView + 90.f)) * fOldSidemove;
	pCmd->sidemove = sin(DEG2RAD(deltaView)) * fOldForward + sin(DEG2RAD(deltaView + 90.f)) * fOldSidemove;
}

void sMe::FixMovement(usercmd_s * pCmd, const Vector & Out)
{
	Vector vecViewForward, vecViewRight, vecViewUp;
	AngleVectors(Out, vecViewForward, vecViewRight, vecViewUp);

	Vector vecModForward, vecModRight, vecModUp;
	AngleVectors(pCmd->viewangles, vecModForward, vecModRight, vecModUp);

	vecViewForward.Normalize();
	vecViewRight.Normalize();
	vecViewUp.Normalize();

	Vector vecMoveFix[3] =
	{
		Vector(pCmd->forwardmove * vecViewForward),
		Vector(pCmd->sidemove * vecViewRight),
		Vector(pCmd->upmove * vecViewUp),
	};
	pCmd->forwardmove =
		DotProduct(vecMoveFix[0], vecModForward) +
		DotProduct(vecMoveFix[1], vecModForward) +
		DotProduct(vecMoveFix[2], vecModForward);
	pCmd->sidemove =
		DotProduct(vecMoveFix[0], vecModRight) +
		DotProduct(vecMoveFix[1], vecModRight) +
		DotProduct(vecMoveFix[2], vecModRight);
	pCmd->upmove =
		DotProduct(vecMoveFix[0], vecModUp) +
		DotProduct(vecMoveFix[1], vecModUp) +
		DotProduct(vecMoveFix[2], vecModUp);
}

void sMe::DoTriggerBot2(usercmd_s * usercmd)
{
	if (this->iWeaponId == WEAPONLIST_KNIFE || this->iWeaponId == WEAPONLIST_C4 ||
		this->iWeaponId == WEAPONLIST_FLASHBANG || this->iWeaponId == WEAPONLIST_SMOKEGRENADE ||
		this->iWeaponId == WEAPONLIST_HEGRENADE || this->iWeaponId <= 0 || !this->alive ||
		this->iClip <= 0 || this->bInReloading || this->fNextPrimAttack > (usercmd->msec / 1000))
		return;

	for (int i = 1; i <= 32; ++i)
	{
		if (i == this->entindex || g_playerList[i].getEnt() == nullptr ||
			!g_playerList[i].isAlive() || !g_playerList[i].isUpdated() ||
			g_playerList[i].nearestBone < 0 || g_playerList[i].nearestHitbox < 0 ||
			g_playerList[i].nearestHitbox > 12 || g_playerList[i].nearestBone > 64)
			continue;

		float fScreenBone[2], fScreenHitbox[2];
		if (CalcScreen(g_playerList[i].bone[g_playerList[i].nearestBone], fScreenBone) &&
			CalcScreen(g_playerList[i].hitbox[g_playerList[i].nearestHitbox], fScreenHitbox))
		{
			float distance = VectorDistance(this->pmEyePos, g_playerList[i].origin());
			float fRad = ((POW(Config::trigger_radius) * 10) / (distance * this->iFOV));
			if (bInRad(fScreenBone, fRad, usercmd) || bInRad(fScreenHitbox, fRad, usercmd))
			{
				usercmd->buttons |= IN_ATTACK;
			}

			if (Config::trigger_draw)
			{
				gDrawBoxAtScreenXY(fScreenBone[0] - fRad, fScreenBone[1] - fRad, 255, 0, 0, 255, (fRad * 2));
				gDrawBoxAtScreenXY(fScreenHitbox[0] - fRad, fScreenHitbox[1] - fRad, 255, 0, 0, 255, (fRad * 2));
			}
		}
	}
}

void CalcAngle(Vector src, Vector end, Vector& out)
{
	Vector delta(src - end);
	float hyp = delta.Length2D();
	out.x = RAD2DEG(atan(delta.z / hyp));
	out.y = RAD2DEG(atan(delta.y / delta.x));
	out.z = 0.0f;
	if (delta.x >= 0.0f)
		out.y += 180.0f;
}

float GetAntiAimAngles()
{
	float besttemp = 8192.f;
	Vector  vecAngles = Vector(0, -90, 0);
	for (int i = 1; i < gEngfuncs.GetMaxClients(); i++) //we loop through all connected players
	{
		cl_entity_s *pEntity = gEngfuncs.GetEntityByIndex(i);
		if (!g_playerList[i].isAlive() && g_playerList[i].team == g_local.team) //we check if the entity is valid and not in our team
			continue;

		Vector vecOrigin = const_cast<Vector&>(g_playerList[g_local.entindex].origin()); //we get our origin
		Vector vecEnemyOrigin = pEntity->origin; //we get enemy origin
		float temp = VectorDistance(vecOrigin, vecEnemyOrigin); // distance from our origin to enemy origin
		if (temp < besttemp) //if distance to enemy is < 8192 then we proceed
		{
			besttemp = temp;
			CalcAngle(vecOrigin, vecEnemyOrigin, vecAngles); // we calculate a new angles to our enemy
			vecAngles.Normalize(); //we normalize the angle to their origin
		}
	}
	return vecAngles.y - 180; // we substract the angle to them with 180 to be faced backwards
}

extern PDWORD g_pInitPoint;
float GetNoRecoilValue(int weaponId)
{
	switch (weaponId)
	{
		case WEAPONLIST_DEAGLE:
		{
			return 2.0f;
		}
		case WEAPONLIST_GLOCK18:
		{
			return 0.05f;
		}
		case WEAPONLIST_USP:
		{
			return 1.5f;
		}
		case WEAPONLIST_P228:
		{
			return 0.5f;
		}
		case WEAPONLIST_FIVESEVEN:
		{
			return 0.5f;
		}
		case WEAPONLIST_ELITE:
		{
			return 1.8f;
		}
		case WEAPONLIST_M3:
		{
			return 1.5f;
		}
		case WEAPONLIST_XM1014:
		{
			return 1.5f;
		}
		case WEAPONLIST_MP5:
		{
			return 1.3f;
		}
		case WEAPONLIST_TMP:
		{
			return 1.6f;
		}
		case WEAPONLIST_P90:
		{
			return 1.6f;
		}
		case WEAPONLIST_MAC10:
		{
			return 1.6f;
		}
		case WEAPONLIST_UMP45:
		{
			return 1.3f;
		}
		case WEAPONLIST_AK47:
		{
			return 1.83f;
		}
		case WEAPONLIST_M4A1:
		{
			return 1.73f;
		}
		case WEAPONLIST_AUG:
		{
			return 1.8f;
		}
		case WEAPONLIST_SG552:
		{
			return 1.7f;
		}
		case WEAPONLIST_GALIL:
		{
			return 2.0f;
		}
		case WEAPONLIST_FAMAS:
		{
			return 2.3f;
		}
		case WEAPONLIST_SCOUT:
		{
			return 0.1f;
		}
		case WEAPONLIST_AWP:
		{
			return 1.0f;
		}
		case WEAPONLIST_SG550:
		{
			return 1.73f;
		}
		case WEAPONLIST_G3SG1:
		{
			return 1.73f;
		}
		case WEAPONLIST_M249:
		{
			return 1.85f;
		}
	}

	return 0.0f;
}

void sMe::DoAntiAim(usercmd_s * usercmd)
{
	if(Config::antiAim)
	{
		cl_entity_t *pLocal;
		Vector viewforward, viewright, viewup, aimforward, aimright, aimup, vTemp;
		float newforward, newright, newup, fTime;
		float forward = usercmd->forwardmove;
		float right = usercmd->sidemove;
		float up = usercmd->upmove;
		pLocal = gEngfuncs.GetLocalPlayer();
		if (!pLocal) return;
		if (pLocal->curstate.movetype == MOVETYPE_WALK)
			gEngfuncs.pfnAngleVectors(Vector(0.0f, usercmd->viewangles.y, 0.0f), viewforward, viewright, viewup);
		else
			gEngfuncs.pfnAngleVectors(usercmd->viewangles, viewforward, viewright, viewup);

		if (pLocal->curstate.movetype == MOVETYPE_WALK)
		{
			if (Config::antiaim_y)
			{
				fTime = gEngfuncs.GetClientTime();
				usercmd->viewangles.y = fmod(fTime * Config::antiaim_y * 360.0f, 360.0f);
			}
		}

		if (pLocal->curstate.movetype == MOVETYPE_WALK)
			gEngfuncs.pfnAngleVectors(Vector(0.0f, usercmd->viewangles.y, 0.0f), aimforward, aimright, aimup);
		else
			gEngfuncs.pfnAngleVectors(usercmd->viewangles, aimforward, aimright, aimup);

		newforward = DotProduct(forward*viewforward.Normalize(), aimforward) + DotProduct(right*viewright.Normalize(), aimforward) + DotProduct(up*viewup.Normalize(), aimforward);
		newright = DotProduct(forward*viewforward.Normalize(), aimright) + DotProduct(right*viewright.Normalize(), aimright) + DotProduct(up*viewup.Normalize(), aimright);
		newup = DotProduct(forward*viewforward.Normalize(), aimup) + DotProduct(right*viewright.Normalize(), aimup) + DotProduct(up*viewup.Normalize(), aimup);

		usercmd->forwardmove = newforward;
		usercmd->sidemove = newright;
		usercmd->upmove = newup;

		static bool XJitter = true;
		if (XJitter == true)
		{
			usercmd->viewangles.x = -Config::antiaim_x;
			XJitter = !XJitter;
		}
		else
		{
			usercmd->viewangles.x = Config::antiaim_x;
			XJitter = !XJitter;
		}
	}
}

extern PDWORD g_pSpeedBooster;
void sMe::AdjustSpeed(double speed)
{
	if (g_pSpeedBooster == nullptr)
		return;
	
	static double lastSpeed = 1.0;

	if (speed != lastSpeed)
	{
		*(double*)g_pSpeedBooster = max(1.0, speed * 1000.0);
		lastSpeed = speed;
	}
}

void sMe::DoFastWalk()
{
	if (this->moveXYspeed == 0.0f || this->fallSpeed != 0.0f)
		return;
	
	static int ForwardSpeed, BackSpeed, SideSpeed = 0;

	static cvar_s* cl_movespeedkey = gEngfuncs.pfnGetCvarPointer(XorStr("cl_movespeedkey"));
	if (cl_movespeedkey == nullptr)
		cl_movespeedkey = gEngfuncs.pfnGetCvarPointer(XorStr("cl_movespeedkey"));

	static cvar_s* cl_forwardspeed = gEngfuncs.pfnGetCvarPointer(XorStr("cl_forwardspeed"));
	static cvar_s* cl_backspeed = gEngfuncs.pfnGetCvarPointer(XorStr("cl_backspeed"));
	static cvar_s* cl_sidespeed = gEngfuncs.pfnGetCvarPointer(XorStr("cl_sidespeed"));

	if(cl_forwardspeed == nullptr)
		cl_forwardspeed = gEngfuncs.pfnGetCvarPointer(XorStr("cl_forwardspeed"));
	if(cl_backspeed == nullptr)
		cl_backspeed = gEngfuncs.pfnGetCvarPointer(XorStr("cl_backspeed"));
	if(cl_sidespeed == nullptr)
		cl_sidespeed = gEngfuncs.pfnGetCvarPointer(XorStr("cl_sidespeed"));

	if (GetAsyncKeyState(VK_LSHIFT) & 0x8000)
	{
		double WalkSpeed = 149 / this->maxspeed;

		cl_movespeedkey->value = WalkSpeed;

		if (this->pmFlags & FL_ONGROUND)
		{
			Vector Velocity = this->pmVelocity;
			int Speed = abs((int)roundf(sqrt(Velocity[0] * Velocity[0] + Velocity[1] * Velocity[1])));

			if (Speed <= 0) { ForwardSpeed = 149; BackSpeed = 149; SideSpeed = 149; }
			if (Speed > 200) { ForwardSpeed -= 11; BackSpeed -= 11; SideSpeed -= 11; }
			if (Speed > 149 && Speed < 200) { ForwardSpeed -= 13; BackSpeed -= 13; SideSpeed -= 13; }
			if (Speed < 149 && Speed > 0) { ForwardSpeed++; BackSpeed++; SideSpeed++; }

			if (ForwardSpeed < 1) ForwardSpeed = 1;
			if (BackSpeed < 1)    BackSpeed = 1;
			if (SideSpeed < 1)    SideSpeed = 1;

			cl_forwardspeed->value = ForwardSpeed;
			cl_backspeed->value = BackSpeed;
			cl_sidespeed->value = SideSpeed;
		}
	}
	else
	{
		ForwardSpeed = 400; cl_forwardspeed->value = ForwardSpeed;
		BackSpeed = 400; cl_backspeed->value = BackSpeed;
		SideSpeed = 400; cl_sidespeed->value = SideSpeed;
		cl_movespeedkey->value = 0.52f;
	}
}

void sMe::DoFastRun(usercmd_s * usercmd)
{
	if (this->moveXYspeed == 0.0f || this->fallSpeed != 0.0f || !(this->pmFlags & FL_ONGROUND))
		return;
	
	static bool _FastRun = false;
	if ((usercmd->buttons&IN_FORWARD && usercmd->buttons&IN_MOVELEFT) ||
		(usercmd->buttons&IN_BACK && usercmd->buttons&IN_MOVERIGHT))
	{
		if (_FastRun) { _FastRun = false; usercmd->sidemove -= 89.6f; usercmd->forwardmove -= 89.6f; }
		else { _FastRun = true;  usercmd->sidemove += 89.6f; usercmd->forwardmove += 89.6f; }
	}
	else if ((usercmd->buttons&IN_FORWARD && usercmd->buttons&IN_MOVERIGHT) ||
		(usercmd->buttons&IN_BACK && usercmd->buttons&IN_MOVELEFT))
	{
		if (_FastRun) { _FastRun = false; usercmd->sidemove -= 89.6f; usercmd->forwardmove += 89.6f; }
		else { _FastRun = true;  usercmd->sidemove += 89.6f; usercmd->forwardmove -= 89.6f; }
	}
	else if (usercmd->buttons&IN_FORWARD || usercmd->buttons&IN_BACK)
	{
		if (_FastRun) { _FastRun = false; usercmd->sidemove -= 126.6f; }
		else { _FastRun = true;  usercmd->sidemove += 126.6f; }
	}
	else if (usercmd->buttons&IN_MOVELEFT || usercmd->buttons&IN_MOVERIGHT)
	{
		if (_FastRun) { _FastRun = false; usercmd->forwardmove -= 126.6f; }
		else { _FastRun = true;  usercmd->forwardmove += 126.6f; }
	}
}

void sMe::DoAutoStrafe(usercmd_s * cmd)
{
	static bool m_switch = true;
	float current_y = cmd->viewangles.y;
	cmd->forwardmove = 17.75 * this->maxspeed / this->groundspeed;
	if (m_switch)
	{
		cmd->sidemove = -this->maxspeed;
		cmd->viewangles.y -= 20;
	}
	else
	{
		cmd->sidemove = +this->maxspeed;
		cmd->viewangles.y += 40;
	}
	cmd->viewangles.y = current_y;
	m_switch = !m_switch;
}

bool sMe::DoKinfeBot(usercmd_s * cmd)
{
	cl_entity_s *ent;
	float flDistance = 32;
	int iEntity;
	pmtrace_t gVis;
	vec3_t vForward, vecEnd;

	gEngfuncs.pfnAngleVectors(cmd->viewangles, vForward, NULL, NULL);

	for (int iCount = 1; iCount <= 2; iCount++)
	{
		vecEnd[0] = this->pmEyePos[0] + vForward[0] * flDistance;
		vecEnd[1] = this->pmEyePos[1] + vForward[1] * flDistance;
		vecEnd[2] = this->pmEyePos[2] + vForward[2] * flDistance;

		gEngfuncs.pEventAPI->EV_SetTraceHull(2);
		gEngfuncs.pEventAPI->EV_PlayerTrace(this->pmEyePos, vecEnd, PM_STUDIO_BOX, -1, &gVis);

		iEntity = gEngfuncs.pEventAPI->EV_IndexFromTrace(&gVis);

		ent = gEngfuncs.GetEntityByIndex(iEntity);

		if (ent->player)
		{
			if (g_playerList[iEntity].team != this->team)
			{
				if (iCount == 1)
				{
					cmd->buttons |= IN_ATTACK2;

					return true;
				}
				else if (iCount == 2)
				{
					cmd->buttons |= IN_ATTACK;

					return true;
				}
			}
			else
			{
				if (iCount == 2)
					return false;
			}
		}
		else
		{
			if (iCount == 2)
				return false;
		}

		flDistance += 16.0f;
	}

	return false;
}

void sMe::DoAntiRecoil(usercmd_s * usercmd, float frametime)
{
	vec3_t viewforward, viewright, viewup, aimforward, aimright, aimup;

	float newforward, newright, newup;
	float forward = usercmd->forwardmove;
	float right = usercmd->sidemove;
	float up = usercmd->upmove;

	////////////////////////////////////////////////
	gEngfuncs.pfnAngleVectors(usercmd->viewangles, viewforward, viewright, viewup);
	float punch[3], length;
	VectorCopy(punchangle, punch);
	length = VectorLength(punch);
	length -= (10.0 + length * 0.5) * frametime;
	length = max(length, 0.0);
	VectorScale(punch, length, punch);
	usercmd->viewangles[0] += punch[0] + punch[0];
	usercmd->viewangles[1] += punch[1] + punch[1];
	gEngfuncs.pfnAngleVectors(usercmd->viewangles, aimforward, aimright, aimup);
	/////////////////////////////////////////////////

	newforward = DotProduct(forward * viewforward.Normalize(), aimforward) +
		DotProduct(right * viewright.Normalize(), aimforward) +
		DotProduct(up * viewup.Normalize(), aimforward
		);

	newright = DotProduct(forward * viewforward.Normalize(), aimright) +
		DotProduct(right * viewright.Normalize(), aimright) +
		DotProduct(up * viewup.Normalize(), aimright
		);

	newup = DotProduct(forward * viewforward.Normalize(), aimup) +
		DotProduct(right * viewright.Normalize(), aimup) +
		DotProduct(up * viewup.Normalize(), aimup
		);

	usercmd->forwardmove = newforward;
	usercmd->sidemove = newright;
	usercmd->upmove = newup;
}

extern cNoSpread gNoSpread;
void sMe::DoAntiSpread(usercmd_s * usercmd)
{
	vec3_t viewforward, viewright, viewup, aimforward, aimright, aimup;

	float newforward, newright, newup;
	float forward = usercmd->forwardmove;
	float right = usercmd->sidemove;
	float up = usercmd->upmove;

	////////////////////////////////////////////////
	gEngfuncs.pfnAngleVectors(usercmd->viewangles, viewforward, viewright, viewup);
	float offset[3];
	gNoSpread.GetSpreadOffset(g_local.spread.random_seed, 1, usercmd->viewangles, g_local.pmVelocity, offset);
	
	/*
	float lenght = sqrt(1.0f - (offset[1] * offset[1]));
	offset[0] = -(cmd->viewangles[0] + RAD2DEG(asin(Forward[2] / lenght) - atan(Spread[1])));
	offset[1] = RAD2DEG(atan2(SpreadAtOriginAngles[1], sqrt(lenght * lenght - Forward[2] * Forward[2])));
	*/

	usercmd->viewangles[0] += offset[0];
	usercmd->viewangles[1] += offset[1];
	usercmd->viewangles[2] += offset[2];
	gEngfuncs.pfnAngleVectors(usercmd->viewangles, aimforward, aimright, aimup);
	/////////////////////////////////////////////////

	newforward = DotProduct(forward * viewforward.Normalize(), aimforward) +
		DotProduct(right * viewright.Normalize(), aimforward) +
		DotProduct(up * viewup.Normalize(), aimforward
		);

	newright = DotProduct(forward * viewforward.Normalize(), aimright) +
		DotProduct(right * viewright.Normalize(), aimright) +
		DotProduct(up * viewup.Normalize(), aimright
		);

	newup = DotProduct(forward * viewforward.Normalize(), aimup) +
		DotProduct(right * viewright.Normalize(), aimup) +
		DotProduct(up * viewup.Normalize(), aimup
		);

	usercmd->forwardmove = newforward;
	usercmd->sidemove = newright;
	usercmd->upmove = newup;
}

void sMe::DoNoSpread(usercmd_s * usercmd)
{
	Vector Forward;
	gEngfuncs.pfnAngleVectors(usercmd->viewangles, Forward, nullptr, nullptr);
	
	float LengthXZ = sqrt(1 - (this->vSource[1] * this->vSource[1]));

	Vector QAdjusterAngles;
	QAdjusterAngles[0] = -(usercmd->viewangles[0] + RAD2DEG(asin(Forward[2] / LengthXZ) - atan(this->vSpread[1])));
	QAdjusterAngles[1] = RAD2DEG(atan2(this->vSource[1], sqrt(LengthXZ * LengthXZ - Forward[2] * Forward[2])));

	usercmd->viewangles[0] += QAdjusterAngles[0];
	usercmd->viewangles[1] += QAdjusterAngles[1];
}

void sMe::DoSilentAngles(usercmd_s * usercmd, float * aimangles)
{
	vec3_t viewforward, viewright, viewup, aimforward, aimright, aimup;

	float newforward, newright, newup;
	float forward = usercmd->forwardmove;
	float right = usercmd->sidemove;
	float up = usercmd->upmove;

	/////////////////////////////////////////////
	gEngfuncs.pfnAngleVectors(Vector(0.0f, aimangles[1], 0.0f), viewforward, viewright, viewup);
	VectorCopy(aimangles, usercmd->viewangles);
	gEngfuncs.pfnAngleVectors(Vector(0.0f, usercmd->viewangles[1], 0.0f), aimforward, aimright, aimup);
	//////////////////////////////////////////

	newforward = DotProduct(forward * viewforward.Normalize(), aimforward) +
		DotProduct(right * viewright.Normalize(), aimforward) +
		DotProduct(up * viewup.Normalize(), aimforward
		);

	newright = DotProduct(forward * viewforward.Normalize(), aimright) +
		DotProduct(right * viewright.Normalize(), aimright) +
		DotProduct(up * viewup.Normalize(), aimright
		);

	newup = DotProduct(forward * viewforward.Normalize(), aimup) +
		DotProduct(right * viewright.Normalize(), aimup) +
		DotProduct(up * viewup.Normalize(), aimup
		);

	usercmd->forwardmove = newforward;
	usercmd->sidemove = newright;
	usercmd->upmove = newup;
}

#define MIN_SMOOTH 2.0f
#define MAX_SMOOTH 10.0f

bool sMe::DoSmoothAngles(float * Source, float * Destination, float * NewDestination, float Factor)
{
	if (Factor < MIN_SMOOTH) Factor = MIN_SMOOTH;
	if (Factor > MAX_SMOOTH) Factor = MAX_SMOOTH;

	VectorSubtract(Destination, Source, NewDestination);

	if (NewDestination[1] > 180.0f)
		NewDestination[1] -= 360.0f;

	if (NewDestination[1] < -180.0f)
		NewDestination[1] += 360.0f;

	// Do you like approximate values ? I do not.
	if (NewDestination[0] < (MAX_SMOOTH - Factor + 3.0f) &&
		NewDestination[0] > -(MAX_SMOOTH - Factor + 3.0f) &&
		NewDestination[1] < (MAX_SMOOTH - Factor + 3.0f) &&
		NewDestination[1] > -(MAX_SMOOTH - Factor + 3.0f))
	{
		NewDestination[0] = Destination[0];
		NewDestination[1] = Destination[1];
		return true;
	}

	NewDestination[0] = NewDestination[0] / Factor + Source[0];
	NewDestination[1] = NewDestination[1] / Factor + Source[1];

	if (NewDestination[1] > 360.0f)
		NewDestination[1] -= 360.0f;

	if (NewDestination[1] <   0.0f)
		NewDestination[1] += 360.0f;

	return false;
}

void sMe::DoSilentAngles(float * aimangles, usercmd_s * usercmd)
{
	vec3_t viewforward, viewright, viewup, aimforward, aimright, aimup;

	float newforward, newright, newup;
	float forward = usercmd->forwardmove;
	float right = usercmd->sidemove;
	float up = usercmd->upmove;

	/////////////////////////////////////////////
	gEngfuncs.pfnAngleVectors(Vector(0.0f, aimangles[1], 0.0f), viewforward, viewright, viewup);
	VectorCopy(aimangles, usercmd->viewangles);
	gEngfuncs.pfnAngleVectors(Vector(0.0f, usercmd->viewangles[1], 0.0f), aimforward, aimright, aimup);
	//////////////////////////////////////////

	newforward = DotProduct(forward * viewforward.Normalize(), aimforward) +
		DotProduct(right * viewright.Normalize(), aimforward) +
		DotProduct(up * viewup.Normalize(), aimforward
		);

	newright = DotProduct(forward * viewforward.Normalize(), aimright) +
		DotProduct(right * viewright.Normalize(), aimright) +
		DotProduct(up * viewup.Normalize(), aimright
		);

	newup = DotProduct(forward * viewforward.Normalize(), aimup) +
		DotProduct(right * viewright.Normalize(), aimup) +
		DotProduct(up * viewup.Normalize(), aimup
		);

	usercmd->forwardmove = newforward;
	usercmd->sidemove = newright;
	usercmd->upmove = newup;
}

void sMe::DoQuakeGuns(int mode)
{
	cl_entity_s* viewModel = gEngfuncs.GetViewModel();
	if (viewModel == nullptr || viewModel->model == nullptr || viewModel->model->name == nullptr ||
		viewModel->model->name[0] == '\0')
		return;

	// 去除 models/v_
	const char* model = viewModel->model->name + 9;
	float originOffset = 0.0f;
	#define MODEL_CMP(name,_val)	if(_stricmp(model, XorStr(##name)) == 0)\
		originOffset = _val;

	if (mode == 1)
	{
		MODEL_CMP("glock18.mdl", -4.55f);
		MODEL_CMP("usp.mdl", -4.64f);
		MODEL_CMP("p228.mdl", -4.65f);
		MODEL_CMP("deagle.mdl", -4.71f);
		MODEL_CMP("fiveseven.mdl", -4.84f);
		MODEL_CMP("m3.mdl", -5.03f);
		MODEL_CMP("xm1014.mdl", -5.82f);
		MODEL_CMP("mac10.mdl", -5.05f);
		MODEL_CMP("tmp.mdl", -6.47f);
		MODEL_CMP("mp5.mdl", -5.53f);
		MODEL_CMP("ump45.mdl", -6.07f);
		MODEL_CMP("p90.mdl", -4.32f);
		MODEL_CMP("scout.mdl", -5.14f);
		MODEL_CMP("awp.mdl", -6.02f);
		MODEL_CMP("famas.mdl", -4.84f);
		MODEL_CMP("aug.mdl", -6.22f);
		MODEL_CMP("m4a1.mdl", -6.74f);
		MODEL_CMP("sg550.mdl", -7.11f);
		MODEL_CMP("ak47.mdl", -6.79f);
		MODEL_CMP("g3sg1.mdl", -6.19f);
		MODEL_CMP("sg552.mdl", -5.27f);
		MODEL_CMP("galil.mdl", -4.78f);
		MODEL_CMP("m249.mdl", -5.13f);
	}
	else if (mode == 2)
	{
		MODEL_CMP("glock18.mdl", -2.05f);
		MODEL_CMP("usp.mdl", -2.14f);
		MODEL_CMP("p228.mdl", -2.15f);
		MODEL_CMP("deagle.mdl", -2.21f);
		MODEL_CMP("fiveseven.mdl", -2.34f);
		MODEL_CMP("m3.mdl", -2.53f);
		MODEL_CMP("xm1014.mdl", -3.32f);
		MODEL_CMP("mac10.mdl", -2.55f);
		MODEL_CMP("tmp.mdl", -3.97f);
		MODEL_CMP("mp5.mdl", -3.03f);
		MODEL_CMP("ump45.mdl", -3.57f);
		MODEL_CMP("p90.mdl", -1.82f);
		MODEL_CMP("scout.mdl", -2.94f);
		MODEL_CMP("awp.mdl", -3.52f);
		MODEL_CMP("famas.mdl", -2.34f);
		MODEL_CMP("aug.mdl", -3.72f);
		MODEL_CMP("m4a1.mdl", -4.24f);
		MODEL_CMP("sg550.mdl", -4.91f);
		MODEL_CMP("ak47.mdl", -4.29f);
		MODEL_CMP("g3sg1.mdl", -3.99f);
		MODEL_CMP("sg552.mdl", -2.87f);
		MODEL_CMP("galil.mdl", -2.28f);
		MODEL_CMP("m249.mdl", -2.93f);
	}

	if (originOffset == 0.0f)
		return;

	VectorCopy(mainViewAngles, viewModel->angles);

	Vector forward, right, up;
	gEngfuncs.pfnAngleVectors(mainViewAngles, forward, right, up);

	viewModel->origin[0] += forward[0] + up[0] + right[0] * originOffset;
	viewModel->origin[1] += forward[1] + up[1] + right[1] * originOffset;
	viewModel->origin[2] += forward[2] + up[2] + right[2] * originOffset;
}

#define NORMALIZE(v,dist) \
		if(dist==0){ \
			v[0]=0; \
			v[1]=0; \
			v[2]=1; \
		} else { \
			dist=1/dist; \
			v[0]=v[0]*dist; \
			v[1]=v[1]*dist; \
			v[2]=v[2]*dist; \
		}

void sMe::DoAntiAim2(usercmd_s * usercmd)
{
	if (!(usercmd->buttons&IN_ATTACK) &&
		!(usercmd->buttons&IN_USE) && g_local.alive &&
		(this->iWeaponId != WEAPONLIST_C4 && this->iWeaponId != WEAPONLIST_FLASHBANG &&
			this->iWeaponId != WEAPONLIST_SMOKEGRENADE && this->iWeaponId != WEAPONLIST_HEGRENADE))
	{
		static bool yFlip;
		cl_entity_t *pLocal;
		Vector viewforward, viewright, viewup, aimforward, aimright, aimup, v;
		float newforward, newright, newup, fTime;
		float forward = usercmd->forwardmove;
		float right = usercmd->sidemove;
		float up = usercmd->upmove;
		bool movetype_walk = (gEngfuncs.GetLocalPlayer()->curstate.movetype == MOVETYPE_WALK);
		if (movetype_walk) {
			v[0] = 0.0f;
			v[1] = usercmd->viewangles[1];
			v[2] = 0.0f;
			gEngfuncs.pfnAngleVectors(v, viewforward, viewright, viewup);
		}
		else
		{
			gEngfuncs.pfnAngleVectors(usercmd->viewangles, viewforward, viewright, viewup);
		}
		fTime = gEngfuncs.GetClientTime();
		if (Config::antiaim_x == 1)
		{
			//Down
			usercmd->viewangles.x = 88;
		}
		else if (Config::antiaim_x == 2)
		{
			//Up
			usercmd->viewangles.x = -88;
		}
		else if (Config::antiaim_x == 3)
		{
			//FakeDown
			usercmd->viewangles.x = 1620;
		}
		else if (Config::antiaim_x == 4)
		{
			//FakeUp
			usercmd->viewangles.x = -1619;
		}
		else if (Config::antiaim_x == 5)
		{
			//Spin X
			usercmd->viewangles.x = cos(fTime * 11 * 3.14159265358979323846) * 360.0f;
			usercmd->viewangles.x = cos(fTime * 11 * 3.14159265358979323846) * 90.0f;
		}
		else if (Config::antiaim_x == 6)
		{
			//Static
			usercmd->viewangles.x = 0;
		}

		if (Config::antiaim_y == 1)
		{
			//Backwards
			usercmd->viewangles.y = usercmd->viewangles.y + 180;
		}
		else if (Config::antiaim_y == 2)
		{
			//Static Left
			usercmd->viewangles.y = 90;
		}
		else if (Config::antiaim_y == 3)
		{
			//Static Right
			usercmd->viewangles.y = 270;
		}
		if (Config::antiaim_y == 4)
		{
			//Left
			usercmd->viewangles.y = usercmd->viewangles.y + 90;
		}
		else if (Config::antiaim_y == 5)
		{
			//Right
			usercmd->viewangles.y = usercmd->viewangles.y + 270;
		}
		else if (Config::antiaim_y == 6)
		{
			//Spin Y
			usercmd->viewangles.y = sin(fTime * 11 * 3.14159265358979323846) * 360.0f;
		}
		else if (Config::antiaim_y == 7)
		{
			//Static
			usercmd->viewangles.y = 0;
		}
		else if (Config::antiaim_y == 8)
		{
			int random = rand() % 100;

			if (random < 98)
				// Look backwards
				usercmd->viewangles.y -= 180;

			// Jitter
			if (random < 15)
			{
				float change = -70.f + (rand() % (int)(140 + 1));
				usercmd->viewangles.y += change;
			}
			if (random == 69)
			{
				float change = -90.f + (rand() % (int)(180 + 1));
				usercmd->viewangles.y += change;
			}

		}
		else if (Config::antiaim_y == 9)
		{
			//Jitter
			int random = 1 + (rand() % (int)(100 - 1 + 1));
			if (random < 10)
			{
				usercmd->viewangles.y += 180.f;
			}
			if (random > 90)
			{
				usercmd->viewangles.y = usercmd->viewangles.y;
			}
		}
		if (movetype_walk)
		{
			v[0] = 0.0f;
			v[1] = usercmd->viewangles[1];
			v[2] = 0.0f;
			gEngfuncs.pfnAngleVectors(v, aimforward, aimright, aimup);
		}
		else
		{
			gEngfuncs.pfnAngleVectors(usercmd->viewangles, aimforward, aimright, aimup);
		}
		float flLen1 = (float)sqrt(viewforward[0] * viewforward[0] + viewforward[1] * viewforward[1] + viewforward[2] * viewforward[2]);
		float flLen2 = (float)sqrt(viewright[0] * viewright[0] + viewright[1] * viewright[1] + viewright[2] * viewright[2]);
		float flLen3 = (float)sqrt(viewup[0] * viewup[0] + viewup[1] * viewup[1] + viewup[2] * viewup[2]);
		NORMALIZE(viewforward, flLen1);
		NORMALIZE(viewright, flLen2);
		NORMALIZE(viewup, flLen3);
		viewforward[0] = forward*viewforward[0];
		viewforward[1] = forward*viewforward[1];
		viewforward[2] = forward*viewforward[2];
		viewright[0] = right*viewright[0];
		viewright[1] = right*viewright[1];
		viewright[2] = right*viewright[2];
		viewup[0] = up*viewup[0];
		viewup[1] = up*viewup[1];
		viewup[2] = up*viewup[2];
		newforward = ((viewforward[0] * aimforward[0] + viewforward[1] * aimforward[1] + viewforward[2] * aimforward[2]) +
			(viewright[0] * aimforward[0] + viewright[1] * aimforward[1] + viewright[2] * aimforward[2]) +
			(viewup[0] * aimforward[0] + viewup[1] * aimforward[1] + viewup[2] * aimforward[2]));
		newright = ((viewforward[0] * aimright[0] + viewforward[1] * aimright[1] + viewforward[2] * aimright[2]) +
			(viewright[0] * aimright[0] + viewright[1] * aimright[1] + viewright[2] * aimright[2]) +
			(viewup[0] * aimright[0] + viewup[1] * aimright[1] + viewup[2] * aimright[2]));
		newup = ((viewforward[0] * aimup[0] + viewforward[1] * aimup[1] + viewforward[2] * aimup[2]) +
			(viewright[0] * aimup[0] + viewright[1] * aimup[1] + viewright[2] * aimup[2]) +
			(viewup[0] * aimup[0] + viewup[1] * aimup[1] + viewup[2] * aimup[2]));
		if (Config::antiaim_x == 3 || Config::antiaim_x == 4)
		{
			usercmd->forwardmove = -newforward;
			usercmd->sidemove = newright;
			usercmd->upmove = newup;
		}
		else
		{
			usercmd->forwardmove = newforward;
			usercmd->sidemove = newright;
			usercmd->upmove = newup;
		}
	}
}

int getSeqInfo(int ax)
{
	return Cstrike_SequenceInfo[g_playerList[ax].getEnt()->curstate.sequence];
}

void PlayerInfo::drawBone(int bone1, int bone2, byte r, byte g, byte b, byte a)
{
	float boneScreen1[2], boneScreen2[2];
	if (CalcScreen(this->bone[bone1], boneScreen1) && CalcScreen(this->bone[bone2], boneScreen2))
	{
		DrawLine((int)boneScreen1[0], (int)boneScreen1[1], (int)boneScreen2[0], (int)boneScreen2[1],
			r, g, b, a);
	}
}

float* NormalizeX(float* v)
{
	float dist = (float)sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	if (dist == 0) {
		v[0] = 0;
		v[1] = 0;
		v[2] = 1;
	}
	else {
		dist = 1 / dist;
		v[0] = v[0] * dist;
		v[1] = v[1] * dist;
		v[2] = v[2] * dist;
	}
	return v;
}

float AngleBetween(Vector a, Vector b)
{
	float x1 = 0, y1 = 0, z1 = 0, x2 = 0, y2 = 0, z2 = 0;
	x1 = a.x;
	y1 = a.y;
	z1 = a.z;
	x2 = b.x;
	y2 = b.y;
	z2 = b.z;
	float dist = sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2) + pow(z1 - z2, 2));
	float dist2 = sqrt(pow(x1 - x2, 2) + pow(z1 - z2, 2));
	return acos(dist2 / dist) * 180 / M_PI_F;
}

void GetSpreadOffset(struct usercmd_s* usercmd)
{
	double SpreadCone = g_local.spread.spreadvar;//GetSpread();
	double CosineInput = 0, CosinePitch = 0, PitchBreakingPoint = 0, PitchInput = 0;
	double InputRoll = 0;
	//	double RadiansPitch=0;
	double ReciprocalYaw_1 = 0, ReciprocalYaw_2 = 0, TrapGodConstant = 0, UpVal = 0, Yaw_1 = 0;


	//	vec3_t my_QAngles;
	float my_QAngles[3];
	float my_QOldAngles[3];
	float my_QAdjusterAngles[3];
	float* my_QAdjusterAngles_ptr = 0;
	float my_QInputAngles[3];
	float* my_QInputAngles_ptr = 0;
	float my_QNewAngles[3];
	float* my_QNewAngles_ptr = 0;
	float my_QTestAngles[3];
	float* my_QTestAngles_ptr = 0;
	//	float Difference[3];
	//	float* Difference_ptr = 0;

	float my_QInputRoll[3];
	float* my_QInputRoll_ptr = 0;

	Vector vecRandom;
	Vector vecDir, vecForward, vecRight, vecUp;
	Vector vecReciprocalVector, vecInputRight, vecInputRight2;

	Vector vecServerForward;
	Vector vecServerRight;
	Vector vecServerUp;

	///////////////////////

	// Trap God Constant computation goes here

	TrapGodConstant = 0;

	///////////////////////

	my_QAngles[0] = usercmd->viewangles[0];
	my_QAngles[1] = 0;
	my_QAngles[2] = 0;

	//	VectorCopy ( usercmd->viewangles, my_QAngles );

	//
	my_QOldAngles[0] = my_QAngles[0];
	my_QOldAngles[1] = my_QAngles[1];
	my_QOldAngles[2] = my_QAngles[2];

	//
	gEngfuncs.pfnAngleVectors(my_QAngles, vecForward, vecRight, vecUp);

	//
	vecDir = vecForward + (-SpreadCone * vecRandom[0] * vecRight) + (-SpreadCone * vecRandom[1] * vecUp);
	vecDir.Normalize();

	//
	PitchBreakingPoint = (atan2(vecDir[1], sqrt((1 - (vecDir[1] * vecDir[1])))) * 180 / M_PI);

	if (PitchBreakingPoint < 0) {
		PitchBreakingPoint += 360;
	}

	if (PitchBreakingPoint > 180) {
		PitchBreakingPoint -= 360;
	}
	else if (PitchBreakingPoint < -180) {
		PitchBreakingPoint += 360;
	}

	if (PitchBreakingPoint < 0) {
		PitchBreakingPoint = 90 + PitchBreakingPoint;
	}
	else {
		PitchBreakingPoint = 90 - PitchBreakingPoint;
	}

	//
	VectorCopy(ToEulerAngles(vecDir), my_QAngles);

	//
	gEngfuncs.pfnAngleVectors(my_QAngles, vecForward, vecRight, vecUp);

	//
	UpVal = vecUp[2];

	//

	my_QAngles[0] = my_QOldAngles[0];
	my_QAngles[1] = my_QOldAngles[1];
	my_QAngles[2] = my_QOldAngles[2];

	//
	CosineInput = my_QAngles[0] * (M_PI * 2 / 360);
	CosinePitch = cos(CosineInput);

	if (CosinePitch != 0)
	{
		Yaw_1 = 1 / CosinePitch;
	}
	else {
		Yaw_1 = 0;
	}

	Yaw_1 *= vecDir[1];

	vecInputRight[1] = Yaw_1;

	if (Yaw_1 >= 1 || Yaw_1 <= -1)
	{
		vecInputRight[0] = 0;
	}
	else {
		vecInputRight[0] = sqrt((1 - (vecInputRight[1] * vecInputRight[1])));
	}

	vecInputRight[2] = 0;

	//

	my_QAdjusterAngles[0] = 0;
	my_QAdjusterAngles[1] = 0;
	my_QAdjusterAngles[2] = 0;

	//

	my_QAdjusterAngles[1] = (atan2(vecInputRight[1], vecInputRight[0]) * 180 / M_PI);

	if (my_QAdjusterAngles[1] < 0)
	{
		my_QAdjusterAngles[1] += 360;
	}

	//

	my_QAdjusterAngles_ptr = NormalizeX(my_QAdjusterAngles);

	vecInputRight2[1] = vecDir[1];

	if (vecDir[1] >= 1 || vecDir[1] <= -1)
	{
		vecInputRight[0] = 0;
	}
	else {
		vecInputRight2[0] = sqrt((1 - (vecInputRight2[1] * vecInputRight2[1])));
	}

	vecInputRight2[2] = 0;

	if (vecInputRight[0] != 0)
	{
		ReciprocalYaw_1 = vecInputRight[1] / vecInputRight[0];
	}
	else {
		ReciprocalYaw_1 = 0;
	}

	if (vecInputRight2[0] != 0)
	{
		ReciprocalYaw_2 = vecInputRight2[1] / vecInputRight2[0];
	}
	else {
		ReciprocalYaw_2 = 0;
	}

	vecReciprocalVector[0] = ReciprocalYaw_1;
	vecReciprocalVector[1] = ReciprocalYaw_2;
	vecReciprocalVector[2] = 0;

	if (vecReciprocalVector[0] != 0)
	{
		PitchInput = 1 / vecReciprocalVector[0];
	}
	else
	{
		PitchInput = 0;
	}

	PitchInput *= vecReciprocalVector[1];

	//

	my_QInputAngles[0] = 0;
	my_QInputAngles[1] = 0;
	my_QInputAngles[2] = 0;

	//

	if (my_QAngles[0] < 0)
	{
		my_QInputAngles[0] = asin(-PitchInput) * 180 / M_PI;
	}
	else {
		my_QInputAngles[0] = asin(PitchInput) * 180 / M_PI;
	}

	if (my_QInputAngles[0] < 0)
	{
		my_QInputAngles[0] += 360;
	}

	//
	my_QInputAngles_ptr = NormalizeX(my_QInputAngles);

	double TempPitch = my_QAngles[0];

	if (TempPitch < 0)
	{
		TempPitch = -TempPitch;
	}

	if (my_QAngles[0] < 0)
	{
		my_QInputAngles[0] += (45 - TempPitch) * 2;
	}
	else
	{
		my_QInputAngles[0] -= (45 - TempPitch) * 2;
	}

	my_QInputAngles[1] = 0;
	my_QInputAngles[2] = 0;

	//
	my_QInputAngles_ptr = NormalizeX(my_QInputAngles);


	//
	gEngfuncs.pfnAngleVectors(my_QInputAngles, vecForward, vecRight, vecUp);

	//
	vecDir[0] = vecForward[0] + (SpreadCone * vecRandom[0] * vecRight[0]) + (SpreadCone * vecRandom[1] * vecUp[0]);
	vecDir[1] = 0;
	vecDir[2] = vecForward[2] + (SpreadCone * vecRandom[0] * vecRight[2]) + (SpreadCone * vecRandom[1] * vecUp[2]);

	vecDir.Normalize();

	//
	if (my_QAngles[0] < 0)
	{
		my_QAdjusterAngles[0] = (asin(vecDir[2]) * 180 / M_PI);
	}
	else
	{
		my_QAdjusterAngles[0] = (asin(-vecDir[2]) * 180 / M_PI);
	}

	if (my_QAdjusterAngles[0] < 0)
	{
		my_QAdjusterAngles[0] += 360;
	}

	//
	my_QAdjusterAngles_ptr = NormalizeX(my_QAdjusterAngles);

	//

	my_QNewAngles[0] = 0;
	my_QNewAngles[1] = 0;
	my_QNewAngles[2] = 0;

	//

	my_QNewAngles[0] = my_QOldAngles[0];
	my_QNewAngles[1] = my_QOldAngles[1];
	my_QNewAngles[2] = my_QOldAngles[2];

	//
	if (my_QAngles[0] < 0)
	{
		my_QNewAngles[0] += my_QNewAngles[0] + my_QAdjusterAngles[0];
	}
	else
	{
		my_QNewAngles[0] += my_QNewAngles[0] - my_QAdjusterAngles[0];
	}

	my_QNewAngles[1] = my_QNewAngles[1] + my_QAdjusterAngles[1];
	my_QNewAngles[2] = 0;

	//
	my_QNewAngles_ptr = NormalizeX(my_QNewAngles);


	////////////////////////// update

	gEngfuncs.pfnAngleVectors(my_QAngles, vecServerForward, vecServerRight, vecServerUp);

	my_QTestAngles[0] = 0;
	my_QTestAngles[1] = 0;
	my_QTestAngles[2] = 0;
	VectorCopy(ToEulerAngles(vecDir), my_QTestAngles);

	my_QTestAngles_ptr = NormalizeX(my_QTestAngles);

	my_QTestAngles[0] = 90 - AngleBetween(vecDir, vecServerUp);
	my_QTestAngles[1] = 90 - AngleBetween(vecDir, vecServerRight);
	my_QTestAngles_ptr = NormalizeX(my_QTestAngles);

	my_QNewAngles[0] += my_QTestAngles[0];
	my_QNewAngles[1] += my_QTestAngles[1];
	my_QNewAngles[2] += my_QTestAngles[2];

	my_QNewAngles_ptr = NormalizeX(my_QNewAngles);

	if (my_QAngles[0] < -PitchBreakingPoint || my_QAngles[0] > PitchBreakingPoint)
	{
		gEngfuncs.pfnAngleVectors(my_QNewAngles, vecServerForward, vecServerRight, vecServerUp);

		InputRoll = vecUp[1] / UpVal;

		my_QInputRoll[0] = (asin(InputRoll) * 180 / M_PI);

		if (my_QInputRoll[0] < 0)
		{
			my_QInputRoll[0] += 360;
		}

		//
		my_QInputRoll_ptr = NormalizeX(my_QInputRoll);


		my_QInputRoll[0] *= -1;

		my_QNewAngles[0] = my_QOldAngles[0];
		my_QNewAngles[1] = my_QOldAngles[1];
		my_QNewAngles[2] = my_QOldAngles[2];

		my_QNewAngles[2] = my_QInputRoll[0] + TrapGodConstant;

		gEngfuncs.pfnAngleVectors(my_QNewAngles, vecForward, vecRight, vecUp);

		vecDir = vecForward + (-SpreadCone * vecRandom[0] * vecRight) + (-SpreadCone * vecRandom[1] * vecUp);

		vecDir.Normalize();

		//	QNewAngles = vecDir.ToEulerAngles ( &vecUp );
		VectorCopy(ToEulerAngles(&vecUp, vecDir), my_QNewAngles);

		my_QNewAngles_ptr = NormalizeX(my_QNewAngles);

		my_QNewAngles[2] += TrapGodConstant;

		my_QNewAngles_ptr = NormalizeX(my_QNewAngles);

	}

	//////////////////////////

	//
	// this loop below is entirely optional
	/*	for ( int Iterator = 0; Iterator < 20; ++Iterator ) // recover to 1.e-16 or so
	{
	gEngfuncs.pfnAngleVectors(my_QNewAngles, vecForward, vecRight, vecUp);

	vecDir = vecForward + ( SpreadCone * vecRandom[0] * vecRight ) + ( SpreadCone * vecRandom[1] * vecUp );

	vecDir.Normalize();

	gEngfuncs.pfnAngleVectors(my_QAngles, vecServerForward, vecServerRight, vecServerUp);

	my_QTestAngles[0]=0;
	my_QTestAngles[1]=0;
	my_QTestAngles[2]=0;
	VectorCopy(ToEulerAngles(vecDir), my_QTestAngles);

	//
	my_QTestAngles_ptr = NormalizeX(my_QTestAngles);

	//	QAngle Difference = QAngles - QTestAngles;
	Difference[0] = my_QAngles[0] - my_QTestAngles[0];
	Difference[1] = my_QAngles[1] - my_QTestAngles[1];
	Difference[2] = my_QAngles[2] - my_QTestAngles[2];

	//
	Difference_ptr = NormalizeX(Difference);

	//	QNewAngles += Difference;
	my_QNewAngles[0] += Difference[0];
	my_QNewAngles[1] += Difference[1];
	my_QNewAngles[2] += Difference[2];

	//
	my_QNewAngles_ptr = NormalizeX(my_QNewAngles);
	}*/

	/*
	//
	// this code below would only apply in CSS/CSGO, or any Valve shooter where the roll angle is accessible.  Otherwise for 1.6 it is useless.
	if ( QAngles[0] < -PitchBreakingPoint || QAngles[0] > PitchBreakingPoint )
	{
	QNewAngles.AngleVectors ( &vecForward, &vecRight, &vecUp );
	InputRoll = vecUp[1] / UpVal;
	QInputRoll[0] = ( asin ( InputRoll ) * 180 / M_PI );
	QInputRoll.Normalize();
	QInputRoll[0] *= -1;
	QNewAngles = QOldAngles;
	QNewAngles[2] = QInputRoll[0] + TrapGodConstant;
	QNewAngles.AngleVectors ( &vecForward, &vecRight, &vecUp );
	vecDir = vecForward + ( -SpreadCone * vecRandom[0] * vecRight ) + ( -SpreadCone * vecRandom[1] * vecUp );
	vecDir.Normalize();
	QNewAngles = vecDir.ToEulerAngles ( &vecUp );
	QNewAngles.Normalize();
	QNewAngles[2] += TrapGodConstant;
	QNewAngles.Normalize();
	}*/


	//

	/*	float outangles[3]={0};
	outangles[0] = usercmd->viewangles[0] - my_QNewAngles[0];
	outangles[1] = usercmd->viewangles[1] - my_QNewAngles[1];
	outangles[2] = 0;

	usercmd->viewangles[0] += outangles[0];
	usercmd->viewangles[1] += outangles[1];
	usercmd->viewangles[2] += outangles[2];

	usercmd->viewangles[0] = my_QNewAngles[0];
	usercmd->viewangles[1] = my_QNewAngles[1];
	usercmd->viewangles[2] = my_QNewAngles[2];*/
}


