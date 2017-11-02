#include "miniradar.h"
#include "drawing.h"
#include "../clientdll.h"
#include "../players.h"
#include "../cvars.h"

extern bool oglSubtractive;
extern cl_enginefuncs_s gEngfuncs;
extern int displayCenterX;
extern int displayCenterY;

static void calcRadarPoint(const float* origin, int& screenx, int& screeny)
{
	float dx = origin[0] - g_local.pmEyePos[0];
	float dy = origin[1] - g_local.pmEyePos[1];

	// rotate
	float yaw = mainViewAngles[1] * (M_PI / 180.0);
	g_local.sin_yaw = sin(yaw);
	g_local.minus_cos_yaw = -cos(yaw);
	float x = dy*g_local.minus_cos_yaw + dx*g_local.sin_yaw;
	float y = dx*g_local.minus_cos_yaw - dy*g_local.sin_yaw;
	float range = 2500.0f;//float& range = cvar.radar_range;
	if (fabs(x)>range || fabs(y)>range)
	{
		// clipping
		if (y>x)
		{
			if (y>-x)
				x = range*x / y, y = range;
			else
				y = -range*y / x, x = -range;
		}
		else
		{
			if (y>-x)
				y = range*y / x, x = range;
			else
				x = -range*x / y, y = -range;
		}
	}
	screenx = displayCenterX + int(x / range*float(Config::miniradar_size));
	screeny = displayCenterY + int(y / range*float(Config::miniradar_size));
	//screenx = displayCenterX+int(x/cvar.radar_range*float(cvar.radar_size));
	//screeny = displayCenterY+int(y/cvar.radar_range*float(cvar.radar_size));
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////


inline void calcRadarPointX(const float* origin, int& screenx, int& screeny)
{
	calcRadarPoint(origin, screenx, screeny);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void drawMiniRadarPoint(const float* origin, int r, int g, int b, bool addbox, int boxsize)
{
	int screenx, screeny;
	calcRadarPointX(origin, screenx, screeny);
	//if(!blink)
	oglSubtractive = true;
	gEngfuncs.pfnFillRGBA(screenx - 1, screeny - 1, boxsize, boxsize, r, g, b, 255);
	oglSubtractive = false;
	if (addbox)blackBorder(screenx - 1, screeny - 1, boxsize, boxsize + 1);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void drawRadarFrame()
{
	int radar_x = displayCenterX;
	int radar_y = displayCenterY;
	int  size = Config::miniradar_size;
	// ColorEntry* cDivider = colorList.get(0);
	oglSubtractive = true;
	gEngfuncs.pfnFillRGBA(radar_x, radar_y - size, 1, 2 * size, 255, 128, 0, 255);
	gEngfuncs.pfnFillRGBA(radar_x - size, radar_y, 2 * size, 1, 255, 128, 0, 255);
	oglSubtractive = false;
}
