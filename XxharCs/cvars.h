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
	bool barrel;
	bool entityesp;
	bool autostrafe;

	int antiaim_y = 1;
	int antiaim_x = 1;
	int radar_size = 80;
	int radar_x = 80;
	int radar_y = 80;
	int miniradar_size = 70;
};

namespace Config
{
	extern float glAimbot;
	extern float glAimThrough;
	extern float noRecoil;
	extern float noSpread;
	extern float glWallhack;
	extern float glNoSmoker;
	extern float glNoFlash;
	extern float glNoSky;
	extern float glLambert;
	extern float glTransparent;
	extern float glWhiteWalls;
	extern float glWireframe;
	extern float glWireframeModels;
	extern float glHudColored;
	extern float bunnyHop;
	extern float rapidFire;
	extern float triggerBot;
	extern float glHeadBox;
	extern float antiAim;
	extern float speedHack;
	extern float fastRun;
	extern float fastWalk;
	extern float boneEsp;
	extern float radar;
	extern float miniRadar;
	extern float quakeGun;
	extern float barrel;
	extern float entityEsp;
	extern float autoStrafe;
	extern float nameEsp;
	extern float weaponEsp;
	extern float crosshair;
	extern float fastSwitch;
	extern float useHitbox;
	extern float knifeBot;

	extern float radar_x;
	extern float radar_y;
	extern float radar_size;
	extern float miniradar_size;
	extern float antiaim_x;
	extern float antiaim_y;
	extern float trigger_prospread;
	extern float trigger_radius;
	extern float trigger_draw;
};

extern CCvars cvars;

#endif