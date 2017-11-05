#pragma warning (disable:4786) // vc++ stl "truncated browser info"   

#include <Windows.h>   

#include "gamehooking.h"  
#include "engine/wrect.h"   
#include "engine/cl_dll.h"   
#include "engine/event_api.h"   
#include "engine/pmtrace.h"   
#include "engine/pm_defs.h"   
#include "engine/cl_entity.h"   
#include "eventhook.h" 
#include "players.h"
#include "cvars.h"
#include "./Misc/xorstr.h"

void(*glock1Org)(struct event_args_s *args);
void(*glock2Org)(struct event_args_s *args);
void(*shotgun1Org)(struct event_args_s *args);
void(*shotgun2Org)(struct event_args_s *args);
void(*mp5Org)(struct event_args_s *args);
void(*pythonOrg)(struct event_args_s *args);
void(*gaussOrg)(struct event_args_s *args);
void(*gaussspinOrg)(struct event_args_s *args);
void(*trainOrg)(struct event_args_s *args);
void(*vehicleOrg)(struct event_args_s *args);
void(*uspOrg)(struct event_args_s *args);
void(*mp5nOrg)(struct event_args_s *args);
void(*ak47Org)(struct event_args_s *args);
void(*augOrg)(struct event_args_s *args);
void(*deagleOrg)(struct event_args_s *args);
void(*g3sg1Org)(struct event_args_s *args);
void(*sg550Org)(struct event_args_s *args);
void(*glock18Org)(struct event_args_s *args);
void(*m249Org)(struct event_args_s *args);
void(*m3Org)(struct event_args_s *args);
void(*m4a1Org)(struct event_args_s *args);
void(*mac10Org)(struct event_args_s *args);
void(*p90Org)(struct event_args_s *args);
void(*p228Org)(struct event_args_s *args);
void(*awpOrg)(struct event_args_s *args);
void(*scoutOrg)(struct event_args_s *args);
void(*sg552Org)(struct event_args_s *args);
void(*tmpOrg)(struct event_args_s *args);
void(*fivesevenOrg)(struct event_args_s *args);
void(*ump45Org)(struct event_args_s *args);
void(*xm1014Org)(struct event_args_s *args);
void(*elite_leftOrg)(struct event_args_s *args);
void(*elite_rightOrg)(struct event_args_s *args);
void(*knifeOrg)(struct event_args_s *args);
void(*decal_resetOrg)(struct event_args_s *args);
void(*createsmokeOrg)(struct event_args_s *args);


void Event_usp(struct event_args_s *args)
{
	if (args->entindex == g_local.ent->index)
	{
		if (g_local.spread.prevtime)
		{
			g_local.spread.spreadvar = g_local.spread.spreadvar - (0.275 * (0.3 - (g_local.spread.gtime - g_local.spread.prevtime)));

			if (g_local.spread.spreadvar > 0.92)
				g_local.spread.spreadvar = 0.92f;
			else if (g_local.spread.spreadvar < 0.6)
				g_local.spread.spreadvar = 0.6f;
		}

		g_local.spread.recoil++;
		g_local.spread.prevtime = g_local.spread.gtime;
	}

	uspOrg(args);
}


void Event_mp5n(struct event_args_s *args)
{
	if (args->entindex == g_local.ent->index)
	{
		g_local.spread.recoil++;

		g_local.spread.spreadvar = g_local.spread.recoil * g_local.spread.recoil * 0.004543389368468878 + 0.45;

		if (g_local.spread.spreadvar > 0.75)
			g_local.spread.spreadvar = 0.75f;

		g_local.spread.prevtime = g_local.spread.gtime;
		g_local.spread.firing = true;
	}

	mp5nOrg(args);
}


void Event_ak47(struct event_args_s *args)
{
	if (args->entindex == g_local.ent->index)
	{
		g_local.spread.recoil++;
		g_local.spread.firing = true;

		g_local.spread.spreadvar = g_local.spread.recoil * g_local.spread.recoil * g_local.spread.recoil / 200 + 0.35;

		if (g_local.spread.spreadvar > 1.25)
			g_local.spread.spreadvar = 1.25f;

		g_local.spread.prevtime = g_local.spread.gtime;
	}

	ak47Org(args);
}


void Event_aug(struct event_args_s *args)
{
	if (args->entindex == g_local.ent->index)
	{
		g_local.spread.recoil++;
		g_local.spread.firing = true;

		g_local.spread.spreadvar = g_local.spread.recoil * g_local.spread.recoil * g_local.spread.recoil / 215 + 0.3;

		if (g_local.spread.spreadvar > 1.0)
			g_local.spread.spreadvar = 1.0f;

		g_local.spread.prevtime = g_local.spread.gtime;
	}

	augOrg(args);
}


void Event_deagle(struct event_args_s *args)
{
	if (args->entindex == g_local.ent->index)
	{
		if (g_local.spread.prevtime)
		{
			g_local.spread.spreadvar = g_local.spread.spreadvar - (0.35 * (0.4 - (g_local.spread.gtime - g_local.spread.prevtime)));

			if (g_local.spread.spreadvar > 0.9)
				g_local.spread.spreadvar = 0.9f;
			else if (g_local.spread.spreadvar < 0.55)
				g_local.spread.spreadvar = 0.55f;
		}

		g_local.spread.recoil++;
		g_local.spread.prevtime = g_local.spread.gtime;
	}

	deagleOrg(args);
}


void Event_g3sg1(struct event_args_s *args)
{
	if (args->entindex == g_local.ent->index)
	{
		if (g_local.spread.brokentime)
		{
			g_local.spread.spreadvar = 0.55 + (0.3 * (g_local.spread.gtime - g_local.spread.brokentime));

			if (g_local.spread.spreadvar > 0.98)
				g_local.spread.spreadvar = 0.98f;
		}

		g_local.spread.recoil++;
		g_local.spread.brokentime = g_local.spread.gtime;
		g_local.spread.firing = true;
	}

	g3sg1Org(args);
}


void Event_sg550(struct event_args_s *args)
{
	if (args->entindex == g_local.ent->index)
	{
		if (g_local.spread.brokentime)
		{
			g_local.spread.spreadvar = 0.65 + (0.35 * (g_local.spread.gtime - g_local.spread.brokentime));

			if (g_local.spread.spreadvar > 0.98)
				g_local.spread.spreadvar = 0.98f;
		}

		g_local.spread.recoil++;
		g_local.spread.brokentime = g_local.spread.gtime;
		g_local.spread.firing = true;
	}

	sg550Org(args);
}


void Event_glock18(struct event_args_s *args)
{
	if (args->entindex == g_local.ent->index)
	{
		if (g_local.spread.prevtime)
		{
			g_local.spread.spreadvar = g_local.spread.spreadvar - (0.275 * (0.325 - (g_local.spread.gtime - g_local.spread.prevtime)));

			if (g_local.spread.spreadvar > 0.9)
				g_local.spread.spreadvar = 0.9f;
			else if (g_local.spread.spreadvar < 0.6)
				g_local.spread.spreadvar = 0.6f;
		}

		g_local.spread.recoil++;
		g_local.spread.prevtime = g_local.spread.gtime;
	}

	glock18Org(args);
}


void Event_m249(struct event_args_s *args)
{
	if (args->entindex == g_local.ent->index)
	{
		g_local.spread.recoil++;
		g_local.spread.firing = true;

		g_local.spread.spreadvar = g_local.spread.recoil * g_local.spread.recoil * g_local.spread.recoil / 175 + 0.4;

		if (g_local.spread.spreadvar > 0.9)
			g_local.spread.spreadvar = 0.9f;

		g_local.spread.prevtime = g_local.spread.gtime;
	}

	m249Org(args);
}


void Event_m3(struct event_args_s *args)
{
	if (args->entindex == g_local.ent->index)
	{
		g_local.spread.recoil++;
		g_local.spread.prevtime = g_local.spread.gtime;
		g_local.spread.firing = true;
	}

	m3Org(args);
}


void Event_m4a1(struct event_args_s *args)
{
	//float spread[2], x, y;   
	//char buffer[256];   

	if (args->entindex == g_local.ent->index)
	{

		g_local.spread.recoil++;
		g_local.spread.firing = true;

		g_local.spread.spreadvar = g_local.spread.recoil * g_local.spread.recoil * g_local.spread.recoil / 220 + 0.3;

		if (g_local.spread.spreadvar > 1)
			g_local.spread.spreadvar = 1.0f;

		g_local.spread.prevtime = g_local.spread.gtime;
	}

	m4a1Org(args);
}


void Event_mac10(struct event_args_s *args)
{
	if (args->entindex == g_local.ent->index)
	{
		g_local.spread.recoil++;
		g_local.spread.firing = true;

		g_local.spread.spreadvar = g_local.spread.recoil * g_local.spread.recoil * g_local.spread.recoil / 200 + 0.6;

		if (g_local.spread.spreadvar > 1.65)
			g_local.spread.spreadvar = 1.65f;

		g_local.spread.prevtime = g_local.spread.gtime;
	}

	mac10Org(args);
}


void Event_p90(struct event_args_s *args)
{
	if (args->entindex == g_local.ent->index)
	{
		g_local.spread.recoil++;
		g_local.spread.firing = true;

		g_local.spread.spreadvar = g_local.spread.recoil * g_local.spread.recoil / 175 + 0.45;

		if (g_local.spread.spreadvar > 1)
			g_local.spread.spreadvar = 1.0f;

		g_local.spread.prevtime = g_local.spread.gtime;
	}

	p90Org(args);
}


void Event_p228(struct event_args_s *args)
{
	if (args->entindex == g_local.ent->index)
	{
		if (g_local.spread.prevtime)
		{
			g_local.spread.spreadvar = g_local.spread.spreadvar - (0.3 * (0.325 - (g_local.spread.gtime - g_local.spread.prevtime)));

			if (g_local.spread.spreadvar > 0.9)
				g_local.spread.spreadvar = 0.9f;
			else if (g_local.spread.spreadvar < 0.6)
				g_local.spread.spreadvar = 0.6f;
		}

		g_local.spread.recoil++;
		g_local.spread.prevtime = g_local.spread.gtime;
	}

	p228Org(args);
}


void Event_awp(struct event_args_s *args)
{
	if (args->entindex == g_local.ent->index)
	{
		g_local.spread.recoil++;
		g_local.spread.prevtime = g_local.spread.gtime;
		g_local.spread.firing = true;
	}

	awpOrg(args);
}


void Event_scout(struct event_args_s *args)
{
	if (args->entindex == g_local.ent->index)
	{
		g_local.spread.recoil++;
		g_local.spread.prevtime = g_local.spread.gtime;
		g_local.spread.firing = true;
	}

	scoutOrg(args);
}


void Event_sg552(struct event_args_s *args)
{
	if (args->entindex == g_local.ent->index)
	{
		g_local.spread.recoil++;
		g_local.spread.firing = true;

		g_local.spread.spreadvar = g_local.spread.recoil * g_local.spread.recoil * g_local.spread.recoil / 220 + 0.3;

		if (g_local.spread.spreadvar > 1)
			g_local.spread.spreadvar = 1.0f;

		g_local.spread.prevtime = g_local.spread.gtime;
	}

	sg552Org(args);
}


void Event_tmp(struct event_args_s *args)
{
	if (args->entindex == g_local.ent->index)
	{
		g_local.spread.recoil++;
		g_local.spread.firing = true;

		g_local.spread.spreadvar = g_local.spread.recoil * g_local.spread.recoil * g_local.spread.recoil / 200 + 0.55;

		if (g_local.spread.spreadvar > 1.4)
			g_local.spread.spreadvar = 1.4f;

		g_local.spread.prevtime = g_local.spread.gtime;
	}

	tmpOrg(args);
}


void Event_fiveseven(struct event_args_s *args)
{
	if (args->entindex == g_local.ent->index)
	{
		if (g_local.spread.prevtime)
		{
			g_local.spread.spreadvar = g_local.spread.spreadvar - (0.25 * (0.275 - (g_local.spread.gtime - g_local.spread.prevtime)));

			if (g_local.spread.spreadvar > 0.92)
				g_local.spread.spreadvar = 0.92f;
			else if (g_local.spread.spreadvar < 0.725)
				g_local.spread.spreadvar = 0.725f;
		}

		g_local.spread.recoil++;
		g_local.spread.prevtime = g_local.spread.gtime;
	}

	fivesevenOrg(args);
}


void Event_ump45(struct event_args_s *args)
{
	if (args->entindex == g_local.ent->index)
	{
		g_local.spread.recoil++;
		g_local.spread.firing = true;

		g_local.spread.spreadvar = g_local.spread.recoil * g_local.spread.recoil / 210 + 0.5;

		if (g_local.spread.spreadvar > 1.0)
			g_local.spread.spreadvar = 1.0f;

		g_local.spread.prevtime = g_local.spread.gtime;
	}

	ump45Org(args);
}


void Event_xm1014(struct event_args_s *args)
{
	if (args->entindex == g_local.ent->index)
	{
		g_local.spread.recoil++;
		g_local.spread.prevtime = g_local.spread.gtime;
		g_local.spread.firing = true;
	}

	xm1014Org(args);
}


void Event_elite_left(struct event_args_s *args)
{
	if (args->entindex == g_local.ent->index)
	{
		if (g_local.spread.prevtime)
		{
			g_local.spread.spreadvar = g_local.spread.spreadvar - (0.275 * (0.325 - (g_local.spread.gtime - g_local.spread.prevtime)));

			if (g_local.spread.spreadvar > 0.88)
				g_local.spread.spreadvar = 0.88f;
			else if (g_local.spread.spreadvar < 0.55)
				g_local.spread.spreadvar = 0.55f;
		}

		g_local.spread.recoil++;
		g_local.spread.prevtime = g_local.spread.gtime;
	}

	elite_leftOrg(args);
}


void Event_elite_right(struct event_args_s *args)
{
	if (args->entindex == g_local.ent->index)
	{
		if (g_local.spread.prevtime)
		{
			g_local.spread.spreadvar = g_local.spread.spreadvar - (0.275 * (0.325 - (g_local.spread.gtime - g_local.spread.prevtime)));

			if (g_local.spread.spreadvar > 0.88)
				g_local.spread.spreadvar = 0.88f;
			else if (g_local.spread.spreadvar < 0.55)
				g_local.spread.spreadvar = 0.55f;
		}

		g_local.spread.recoil++;
		g_local.spread.prevtime = g_local.spread.gtime;
	}

	elite_rightOrg(args);
}

void Event_knife(struct event_args_s *args)
{
	if (args->entindex == g_local.ent->index)
	{
		g_local.spread.recoil++;
		g_local.spread.prevtime = g_local.spread.gtime;
	}

	knifeOrg(args);
}

void Event_decal_reset(struct event_args_s *args)
{
	decal_resetOrg(args);
}

void Event_createsmoke(struct event_args_s *args)
{
	createsmokeOrg(args);
}

//#include <fstream>   
//std::ofstream outstream("D:\\jumpshootdebug.txt");   

void PlaybackEvent(int flags, const edict_t *pInvoker, unsigned short eventindex, float delay, float *origin, float *angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2)
{
	if (Config::noRecoil == 3.0f && g_local.alive && g_local.iWeaponId > 0)
	{
		float ViewAngles[3], NewPunchangle[3];
		static float PrevPunchangle[3];
		static int LastWeapon;

		if ((g_local.iWeaponId == WEAPONLIST_FAMAS) || (g_local.iWeaponId == WEAPONLIST_GALIL))
		{
			NewPunchangle[0] = ((float)iparam1 / 10000000) * 2.0f;
			NewPunchangle[1] = ((float)iparam2 / 10000000) * 2.0f;
		}
		else
		{
			NewPunchangle[0] = ((float)iparam1 / 100) * 2.0f;
			NewPunchangle[1] = ((float)iparam2 / 100) * 2.0f;
		}

		if (((NewPunchangle[0] == 0.0f) && (NewPunchangle[1] == 0.0f)) || (LastWeapon != g_local.iWeaponId))
		{
			VectorClear(PrevPunchangle);
			LastWeapon = g_local.iWeaponId;
		}

		gEngfuncs.GetViewAngles(ViewAngles);

		ViewAngles[0] -= NewPunchangle[0] - PrevPunchangle[0];
		ViewAngles[1] -= NewPunchangle[1] - PrevPunchangle[1];

		gEngfuncs.SetViewAngles(ViewAngles);

		PrevPunchangle[0] = NewPunchangle[0];
		PrevPunchangle[1] = NewPunchangle[1];
	}

	gEngfuncs.pfnPlaybackEvent(flags, pInvoker, eventindex, delay, origin, angles, fparam1, fparam2, iparam1, iparam2, bparam1, bparam2);
}

void HookEvent(char *name, void(*pfnEvent)(struct event_args_s *args))
{
	gEngfuncs.pfnHookEvent(name, pfnEvent);
	
	if (!stricmp(name, "events/usp.sc"))
	{
		uspOrg = pfnEvent;
		gEngfuncs.pfnHookEvent(name, Event_usp);
	}
	else if (!stricmp(name, "events/mp5n.sc"))
	{
		mp5nOrg = pfnEvent;
		gEngfuncs.pfnHookEvent(name, Event_mp5n);
	}
	else if (!stricmp(name, "events/ak47.sc"))
	{
		ak47Org = pfnEvent;
		gEngfuncs.pfnHookEvent(name, Event_ak47);
	}
	else if (!stricmp(name, "events/aug.sc"))
	{
		augOrg = pfnEvent;
		gEngfuncs.pfnHookEvent(name, Event_aug);
	}
	else if (!stricmp(name, "events/deagle.sc"))
	{
		deagleOrg = pfnEvent;
		gEngfuncs.pfnHookEvent(name, Event_deagle);
	}
	else if (!stricmp(name, "events/g3sg1.sc"))
	{
		g3sg1Org = pfnEvent;
		gEngfuncs.pfnHookEvent(name, Event_g3sg1);
	}
	else if (!stricmp(name, "events/sg550.sc"))
	{
		sg550Org = pfnEvent;
		gEngfuncs.pfnHookEvent(name, Event_sg550);
	}
	else if (!stricmp(name, "events/glock18.sc"))
	{
		glock18Org = pfnEvent;
		gEngfuncs.pfnHookEvent(name, Event_glock18);
	}
	else if (!stricmp(name, "events/m249.sc"))
	{
		m249Org = pfnEvent;
		gEngfuncs.pfnHookEvent(name, Event_m249);
	}
	else if (!stricmp(name, "events/m3.sc"))
	{
		m3Org = pfnEvent;
		gEngfuncs.pfnHookEvent(name, Event_m3);
	}
	else if (!stricmp(name, "events/m4a1.sc"))
	{
		m4a1Org = pfnEvent;
		gEngfuncs.pfnHookEvent(name, Event_m4a1);
	}
	else if (!stricmp(name, "events/mac10.sc"))
	{
		mac10Org = pfnEvent;
		gEngfuncs.pfnHookEvent(name, Event_mac10);
	}
	else if (!stricmp(name, "events/p90.sc"))
	{
		p90Org = pfnEvent;
		gEngfuncs.pfnHookEvent(name, Event_p90);
	}
	else if (!stricmp(name, "events/p228.sc"))
	{
		p228Org = pfnEvent;
		gEngfuncs.pfnHookEvent(name, Event_p228);
	}
	else if (!stricmp(name, "events/awp.sc"))
	{
		awpOrg = pfnEvent;
		gEngfuncs.pfnHookEvent(name, Event_awp);
	}
	else if (!stricmp(name, "events/scout.sc"))
	{
		scoutOrg = pfnEvent;
		gEngfuncs.pfnHookEvent(name, Event_scout);
	}
	else if (!stricmp(name, "events/sg552.sc"))
	{
		sg552Org = pfnEvent;
		gEngfuncs.pfnHookEvent(name, Event_sg552);
	}
	else if (!stricmp(name, "events/tmp.sc"))
	{
		tmpOrg = pfnEvent;
		gEngfuncs.pfnHookEvent(name, Event_tmp);
	}
	else if (!stricmp(name, "events/fiveseven.sc"))
	{
		fivesevenOrg = pfnEvent;
		gEngfuncs.pfnHookEvent(name, Event_fiveseven);
	}
	else if (!stricmp(name, "events/ump45.sc"))
	{
		ump45Org = pfnEvent;
		gEngfuncs.pfnHookEvent(name, Event_ump45);
	}
	else if (!stricmp(name, "events/xm1014.sc"))
	{
		xm1014Org = pfnEvent;
		gEngfuncs.pfnHookEvent(name, Event_xm1014);
	}
	else if (!stricmp(name, "events/elite_left.sc"))
	{
		elite_leftOrg = pfnEvent;
		gEngfuncs.pfnHookEvent(name, Event_elite_left);
	}
	else if (!stricmp(name, "events/elite_right.sc"))
	{
		elite_rightOrg = pfnEvent;
		gEngfuncs.pfnHookEvent(name, Event_elite_right);
	}
	else if (!stricmp(name, "events/knife.sc"))
	{
		knifeOrg = pfnEvent;
		gEngfuncs.pfnHookEvent(name, Event_knife);
		gEngfuncs.pfnConsolePrint(XorStr("EventHook Hooked\n"));
	}
	else if (!stricmp(name, "events/decal_reset.sc"))
	{
		decal_resetOrg = pfnEvent;
		gEngfuncs.pfnHookEvent(name, Event_decal_reset);
	}
	else if (!stricmp(name, "events/createsmoke.sc"))
	{
		createsmokeOrg = pfnEvent;
		gEngfuncs.pfnHookEvent(name, Event_createsmoke);
	}
}