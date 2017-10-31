#ifndef __CVARS_H__
#define __CVARS_H__

class CCvars
{
public:
	bool dmAimbot;
	bool aimthrough;
	bool norecoil = true;
	bool nospread = true;
	bool wallhack;
	bool smokeRemoval;
	bool flashRemoval;
	bool skyRemoval;
	bool lambert;
	bool transparentWalls;
	bool whiteWalls;
	bool wireframe;
	bool wireframeModels;
	bool hud;
	bool bunnyhop = true;
	bool rapidfire = true;
	bool triggerbot;
	bool openglbox;
	bool usehitbox = true;
	bool antiaim;
	bool speedhack;
	bool fastwalk;
	bool fastrun;
	bool boneesp;
	bool radar;
	bool miniradar;
	bool quake;

	int antiaim_y = 1;
	int antiaim_x = 1;
	int radar_size = 80;
	int radar_x = 80;
	int radar_y = 80;
	int miniradar_size = 70;
};

extern CCvars cvars;

#endif