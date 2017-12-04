#include "swfx.h"

const int WPN_BIT_1 = (1 << WEAPONLIST_SCOUT) | (1 << WEAPONLIST_XM1014) | (1 << WEAPONLIST_MAC10) | (1 << WEAPONLIST_AUG) | (1 << WEAPONLIST_UMP45) | (1 << WEAPONLIST_SG550) | (1 << WEAPONLIST_GALIL) | (1 << WEAPONLIST_FAMAS) | (1 << WEAPONLIST_AWP) | (1 << WEAPONLIST_MP5) | (1 << WEAPONLIST_M249) | (1 << WEAPONLIST_M3) | (1 << WEAPONLIST_M4A1) | (1 << WEAPONLIST_TMP) | (1 << WEAPONLIST_G3SG1) | (1 << WEAPONLIST_SG552) | (1 << WEAPONLIST_AK47) | (1 << WEAPONLIST_P90);
const int WPN_BIT_2 = (1 << WEAPONLIST_P228) | (1 << WEAPONLIST_ELITE) | (1 << WEAPONLIST_FIVESEVEN) | (1 << WEAPONLIST_USP) | (1 << WEAPONLIST_GLOCK18) | (1 << WEAPONLIST_DEAGLE);
const int WPN_BIT_3 = (1 << WEAPONLIST_KNIFE);
const int WPN_BIT_4 = (1 << WEAPONLIST_HEGRENADE);
//const int WPN_BIT_4 = (1<<WEAPONLIST_SMOKEGRENADE)|(1<<WEAPONLIST_FLASHBANG)|(1<<WEAPONLIST_HEGRENADE);
const int WPN_BIT_5 = (1 << WEAPONLIST_C4);

CSwitchWeaponEffect *g_pWeaponSwitch=NULL;
extern cl_enginefuncs_s gEngfuncs;

CSwitchWeaponEffect::CSwitchWeaponEffect(void)
{
	memset(m_WR,0,sizeof(m_WR));
	sScreenInfo.iSize = sizeof(sScreenInfo);
	gEngfuncs.pfnGetScreenInfo(&sScreenInfo);
	if (sScreenInfo.iWidth < 640)
		m_iRes = 320;
	else
		m_iRes = 640;

	m_iWpn = 0;
	m_fTime = 0;

	m_fDisplayY = sScreenInfo.iHeight * 0.5f;
	m_fDisplayY= (m_fDisplayY <= sScreenInfo.iHeight - 260.0f ? m_fDisplayY : sScreenInfo.iHeight - 260.0f);

	m_fDisplayX = sScreenInfo.iWidth - 135.f - 40.f;


	m_pFastSwitch = gEngfuncs.pfnRegisterVariable("cl_swfx_enable", "1", FCVAR_ARCHIVE);
	m_pDisplayTime = gEngfuncs.pfnRegisterVariable("cl_swfx_time", "0.6", FCVAR_ARCHIVE);
}
void CSwitchWeaponEffect::Draw()
{
	if(m_fTime<gEngfuncs.GetClientTime()) return;
	static float fDelta ,fPercent,fDelta2;
	fDelta = m_fTime - gEngfuncs.GetClientTime();
	fPercent = fDelta / m_pDisplayTime->value;
	fDelta = 40.0f * fPercent;
	fDelta2 = 20.0f * fPercent;
	if(m_iBits & ( 1<< 31))
	{
		for(int i=1;i<31;i++)
		{
			if(m_iBits & (1<<i))
			{
				if(i == m_iWpn)
				{
					gEngfuncs.pfnSPR_Set(m_WR[i].hSPR,255*fPercent,255*fPercent,255*fPercent);
					gEngfuncs.pfnSPR_DrawAdditive(0,m_fDisplayX-fDelta,m_WR[i].iSlot*55+m_fDisplayY,&m_WR[i].rc);
					
				}
				else 
				{
					if(i == WEAPONLIST_FLASHBANG || i == WEAPONLIST_SMOKEGRENADE) continue ;  // need to find a better way to display
					gEngfuncs.pfnSPR_Set(m_WR[i].hSPR,100*fPercent,100*fPercent,100*fPercent);
					gEngfuncs.pfnSPR_DrawAdditive(0,m_fDisplayX+fDelta2,m_WR[i].iSlot*55+m_fDisplayY,&m_WR[i].rc);
				}
			}
		}
	}
}
bool CSwitchWeaponEffect::CanDraw(void)
{
	return 1;
}
void CSwitchWeaponEffect::ParseWeapon(char *pName,int iSlot,int iID)
{
	char name[64];
	m_WR[iID].iSlot = iSlot;
	sprintf(m_WR[iID].name,"%s",pName);
	sprintf(name,"sprites/%s.txt",pName);
	int iTotal;
	client_sprite_t *pSpriteList = gEngfuncs.pfnSPR_GetList(name,&iTotal);
	for(int i=0;i<iTotal;i++)
	{
		if (pSpriteList->iRes == m_iRes)
		{
			if(!strcmp(pSpriteList->szName,"weapon"))
			{
				sprintf(name,"sprites/%s.spr",pSpriteList->szSprite);
				m_WR[iID].hSPR = gEngfuncs.pfnSPR_Load(name);
				m_WR[iID].rc = pSpriteList->rc;
				return;
			}
		}
		pSpriteList++;
	}
}
int CSwitchWeaponEffect::GetWpnSlot(int iWpn)
{
	if(WPN_BIT_1 & (1<<iWpn)) return 1;
	if(WPN_BIT_2 & (1<<iWpn)) return 2;
	if(WPN_BIT_3 & (1<<iWpn)) return 3;
	if(WPN_BIT_4 & (1<<iWpn)) return 4;
	if(WPN_BIT_5 & (1<<iWpn)) return 5;

	return 0;
}
int CSwitchWeaponEffect::GetPrevWpn(void)
{
	int iSlot = GetWpnSlot(m_iWpn);
	int iSlot2 = iSlot -1 ;
	if(iSlot2 == 0 ) iSlot2 = 5;
	while(iSlot2 != iSlot)
	{
		if(iSlot2 < 1) iSlot2 = 5;
		for(int i=1;i<31;i++)
		{
			if(m_iBits & (1<<i))
			{
				if(iSlot2 == GetWpnSlot(i))
				{
					gEngfuncs.pfnServerCmd(m_WR[i].name);
					return i;
				}
			}
		}
		iSlot2 --;
	}
	return 0;
}
int CSwitchWeaponEffect::GetNextWpn(void)
{
	int iSlot = GetWpnSlot(m_iWpn);
	int iSlot2 = iSlot + 1;
	
	while(iSlot2 != iSlot)
	{
		if(iSlot2 > 5) iSlot2 = 1;
		for(int i=1;i<31;i++)
		{
			if(m_iBits & (1<<i))
			{
				if(iSlot2 == GetWpnSlot(i))
				{
					gEngfuncs.pfnServerCmd(m_WR[i].name);
					return i;
				}
			}
		}
		iSlot2++;
	}
	return 0;
}