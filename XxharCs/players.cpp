﻿#include <Windows.h>
#include "gamehooking.h"
#include "players.h"
#include "cvars.h"
#include "./drawing/drawing.h"
#include "./drawing/radar.h"

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
	int   boxradius = (300.0*90.0) / (distance*fCurrentFOV);	 
	BOUND_VALUE(boxradius,1,200);

	gDrawBoxAtScreenXY(vecScreen[0],vecScreen[1],0,255,0,150,boxradius);
	
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
	return sqrt(a*a + b*b + c*c);
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

inline void _fastcall VectorAngles(const float *forward, float *angles)
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
		yaw = (atan2(forward[1], forward[0]) * 180 / M_PI);
		if (yaw < 0)
			yaw += 360;
		tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
		pitch = (atan2(-forward[2], tmp) * 180 / M_PI);
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

#define M_PI			3.14159265358979323846
#define M_PI_F			(float)M_PI

void sMe::DoTriggerBot(usercmd_s * usercmd)
{
	if ((usercmd->buttons & IN_ATTACK) || this->iClip <= 0)
		return;
	
	// float finalAngle[3] = { 0.0f, 0.0f, 0.0f };
	Vector finalAngle;

	if (cvars.norecoil)
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

		VectorCopy(g_playerDrawList[i].Origin, EntViewVec);
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

float GetNoRecoilValue(int weaponId)
{
	switch (weaponId)
	{
	case WEAPONLIST_DEAGLE:
		return 2.0f;
	case WEAPONLIST_GLOCK18:
		return 0.05f;
	case WEAPONLIST_USP:
		return 1.5f;
	case WEAPONLIST_P228:
		return 0.5f;
	case WEAPONLIST_FIVESEVEN:
		return 0.5f;
	case WEAPONLIST_ELITE:
		return 1.8f;
	case WEAPONLIST_M3:
		return 1.5f;
	case WEAPONLIST_XM1014:
		return 1.5f;
	case WEAPONLIST_MP5:
		return 1.3f;
	case WEAPONLIST_TMP:
		return 1.6f;
	case WEAPONLIST_P90:
		return 1.6f;
	case WEAPONLIST_MAC10:
		return 1.6f;
	case WEAPONLIST_UMP45:
		return 1.3f;
	case WEAPONLIST_AK47:
		return 1.83f;
	case WEAPONLIST_M4A1:
		return 1.73f;
	case WEAPONLIST_AUG:
		return 1.8f;
	case WEAPONLIST_SG552:
		return 1.7f;
	case WEAPONLIST_GALIL:
		return 2.0f;
	case WEAPONLIST_FAMAS:
		return 2.3f;
	case WEAPONLIST_SCOUT:
		return 0.1f;
	case WEAPONLIST_AWP:
		return 1.0f;
	case WEAPONLIST_SG550:
		return 1.73f;
	case WEAPONLIST_G3SG1:
		return 1.73f;
	case WEAPONLIST_M249:
		return 1.85f;
	}

	return 0.0f;
}

void sMe::DoAntiAim(usercmd_s * usercmd)
{
	if(cvars.antiaim)
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
			if (cvars.antiaim_y)
			{
				fTime = gEngfuncs.GetClientTime();
				usercmd->viewangles.y = fmod(fTime * cvars.antiaim_y * 360.0f, 360.0f);
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
			usercmd->viewangles.x = -cvars.antiaim_x;
			XJitter = !XJitter;
		}
		else
		{
			usercmd->viewangles.x = cvars.antiaim_x;
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
	static int ForwardSpeed, BackSpeed, SideSpeed = 0;

	if (GetAsyncKeyState(VK_LSHIFT) & 0x8000)
	{
		double WalkSpeed = 149 / this->maxspeed;
		gEngfuncs.pfnGetCvarPointer("cl_movespeedkey")->value = WalkSpeed;

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

			gEngfuncs.pfnGetCvarPointer("cl_forwardspeed")->value = ForwardSpeed;
			gEngfuncs.pfnGetCvarPointer("cl_backspeed")->value = BackSpeed;
			gEngfuncs.pfnGetCvarPointer("cl_sidespeed")->value = SideSpeed;
		}
	}
	else
	{
		ForwardSpeed = 400; gEngfuncs.pfnGetCvarPointer("cl_forwardspeed")->value = ForwardSpeed;
		BackSpeed = 400; gEngfuncs.pfnGetCvarPointer("cl_backspeed")->value = BackSpeed;
		SideSpeed = 400; gEngfuncs.pfnGetCvarPointer("cl_sidespeed")->value = SideSpeed;
		gEngfuncs.pfnGetCvarPointer("cl_movespeedkey")->value = 0.52;
	}
}

void sMe::DoFastRun(usercmd_s * usercmd)
{
	static bool _FastRun = false;
	if ((usercmd->buttons&IN_FORWARD && usercmd->buttons&IN_MOVELEFT) ||
		(usercmd->buttons&IN_BACK && usercmd->buttons&IN_MOVERIGHT))
	{
		if (_FastRun) { _FastRun = false; usercmd->sidemove -= 89.6; usercmd->forwardmove -= 89.6; }
		else { _FastRun = true;  usercmd->sidemove += 89.6; usercmd->forwardmove += 89.6; }
	}
	else if ((usercmd->buttons&IN_FORWARD && usercmd->buttons&IN_MOVERIGHT) ||
		(usercmd->buttons&IN_BACK && usercmd->buttons&IN_MOVELEFT))
	{
		if (_FastRun) { _FastRun = false; usercmd->sidemove -= 89.6; usercmd->forwardmove += 89.6; }
		else { _FastRun = true;  usercmd->sidemove += 89.6; usercmd->forwardmove -= 89.6; }
	}
	else if (usercmd->buttons&IN_FORWARD || usercmd->buttons&IN_BACK)
	{
		if (_FastRun) { _FastRun = false; usercmd->sidemove -= 126.6; }
		else { _FastRun = true;  usercmd->sidemove += 126.6; }
	}
	else if (usercmd->buttons&IN_MOVELEFT || usercmd->buttons&IN_MOVERIGHT)
	{
		if (_FastRun) { _FastRun = false; usercmd->forwardmove -= 126.6; }
		else { _FastRun = true;  usercmd->forwardmove += 126.6; }
	}
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
		if (cvars.antiaim_x == 1)
		{
			//Down
			usercmd->viewangles.x = 88;
		}
		else if (cvars.antiaim_x == 2)
		{
			//Up
			usercmd->viewangles.x = -88;
		}
		else if (cvars.antiaim_x == 3)
		{
			//FakeDown
			usercmd->viewangles.x = 1620;
		}
		else if (cvars.antiaim_x == 4)
		{
			//FakeUp
			usercmd->viewangles.x = -1619;
		}
		else if (cvars.antiaim_x == 5)
		{
			//Spin X
			usercmd->viewangles.x = cos(fTime * 11 * 3.14159265358979323846) * 360.0f;
			usercmd->viewangles.x = cos(fTime * 11 * 3.14159265358979323846) * 90.0f;
		}
		else if (cvars.antiaim_x == 6)
		{
			//Static
			usercmd->viewangles.x = 0;
		}

		if (cvars.antiaim_y == 1)
		{
			//Backwards
			usercmd->viewangles.y = usercmd->viewangles.y + 180;
		}
		else if (cvars.antiaim_y == 2)
		{
			//Static Left
			usercmd->viewangles.y = 90;
		}
		else if (cvars.antiaim_y == 3)
		{
			//Static Right
			usercmd->viewangles.y = 270;
		}
		if (cvars.antiaim_y == 4)
		{
			//Left
			usercmd->viewangles.y = usercmd->viewangles.y + 90;
		}
		else if (cvars.antiaim_y == 5)
		{
			//Right
			usercmd->viewangles.y = usercmd->viewangles.y + 270;
		}
		else if (cvars.antiaim_y == 6)
		{
			//Spin Y
			usercmd->viewangles.y = sin(fTime * 11 * 3.14159265358979323846) * 360.0f;
		}
		else if (cvars.antiaim_y == 7)
		{
			//Static
			usercmd->viewangles.y = 0;
		}
		else if (cvars.antiaim_y == 8)
		{
			int random = rand() % 100;

			if (random < 98)
				// Look backwards
				usercmd->viewangles.y -= 180;

			// Jitter
			if (random < 15)
			{
				float change = -70 + (rand() % (int)(140 + 1));
				usercmd->viewangles.y += change;
			}
			if (random == 69)
			{
				float change = -90 + (rand() % (int)(180 + 1));
				usercmd->viewangles.y += change;
			}

		}
		else if (cvars.antiaim_y == 9)
		{
			//Jitter
			int random = 1 + (rand() % (int)(100 - 1 + 1));
			if (random < 10)
			{
				usercmd->viewangles.y += 180;
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
		if (cvars.antiaim_x == 3 || cvars.antiaim_x == 4)
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