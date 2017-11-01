#ifndef _SWFX_H
#define _SWFX_H

#include <Windows.h>
#include "clientdll.h"

#define FX_TIME	0.6
struct WR
{
	int iSlot;
	int iID;
	char name[64];
	HCSPRITE hSPR;
	wrect_t rc;
};
class CSwitchWeaponEffect
{
public:
	CSwitchWeaponEffect(void);

public:
	void Draw(void);
	bool CanDraw(void);
	void ParseWeapon(char *pName,int iSlot,int iID);
	int GetPrevWpn(void);
	int GetNextWpn(void);
	int GetWpnSlot(int iWpn);
	

public:
	SCREENINFO sScreenInfo;
	int m_iBits;
	int m_iWpn;
	float m_fTime;
	cvar_t *m_pFastSwitch,*m_pDisplayTime;
	
private:
	struct WR m_WR[32];
	int m_iRes;
	float m_fDisplayX;
	float m_fDisplayY;
};
extern CSwitchWeaponEffect *g_pWeaponSwitch;

#endif