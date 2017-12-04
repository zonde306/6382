#pragma warning (disable:4786)
#include <Windows.h>
#include <string.h>
#include "gamehooking.h"
#include "weaponlist.h"
#include "install.h"
#include "players.h"
#include "nospread.h"

extern FnGetWeaponByID g_pfnGetWeaponByID;
extern globalvars_t* g_pGlobals;
FnGetWeaponByID g_pfnGetWeaponByID;

using namespace std;

vector<Weapon_List> g_weaponList;

bool IsCurWeaponSilenced(void)
{
	Weapon_List *tmpWeapon;

	tmpWeapon = GetCurWeapon();
	if (tmpWeapon)
	{
		if (tmpWeapon->Id == WEAPONLIST_M4A1 && tmpWeapon->weapondata.m_iWeaponState & M4A1_SILENCER)
			return true;

		if (tmpWeapon->Id == WEAPONLIST_USP && tmpWeapon->weapondata.m_iWeaponState & USP_SILENCER)
			return true;
	}

	return false;
}
bool IsLeftElite(void)
{
	Weapon_List *tmpWeapon;

	tmpWeapon = GetCurWeapon();
	if (tmpWeapon)
	{
		if (tmpWeapon->Id == WEAPONLIST_M4A1 && tmpWeapon->weapondata.m_iWeaponState & ELITE_LEFT)
			return true;
	}

	return false;
}

bool IsCurWeaponInBurst(void)
{
	Weapon_List *tmpWeapon;

	tmpWeapon = GetCurWeapon();
	if (tmpWeapon)
	{
		if (tmpWeapon->Id == WEAPONLIST_GLOCK18 && tmpWeapon->weapondata.m_iWeaponState & GLOCK18_BURST)
			return true;
	}

	return false;
}


int GetCurPenetration(void)
{
	Weapon_List *tmpWeapon;

	tmpWeapon = GetCurWeapon();
	if (tmpWeapon)
		return tmpWeapon->penetrate;

	return WALL_PEN0;
}


Weapon_List *GetCurWeapon(void)
{
	for (size_t i=0;i<g_weaponList.size();i++)
	{
		if (g_weaponList[i].Active && g_weaponList[i].CurWeapon)
			return &g_weaponList[i];
	}
	return NULL;
}


int GetCurWeaponAmmo(void)
{
	Weapon_List *tmpWeapon;

	tmpWeapon = GetCurWeapon();
	if (tmpWeapon)
		return tmpWeapon->CAmmo;

	return 0;
}


int GetCurRecoil(void)
{
	Weapon_List *tmpWeapon;

	tmpWeapon = GetCurWeapon();
	if (tmpWeapon)
		return tmpWeapon->weapondata.m_fInZoom;

	return 0;
}


bool IsReloading(void)
{
	Weapon_List *tmpweapon;

	tmpweapon = GetCurWeapon();
	if (tmpweapon && tmpweapon->weapondata.m_fInReload)
		return true;

	return false;
}


bool CanCurWeaponAttack(void)
{
	Weapon_List *tmpweapon;

	tmpweapon = GetCurWeapon();
	if (tmpweapon && tmpweapon->weapondata.m_flNextPrimaryAttack <= 0)
		return true;

	return false;
}


static Weapon_List *GetSecWeapon(void)
{
	size_t i;

	for (i=0;i<g_weaponList.size();i++)
	{
		if (g_weaponList[i].Active && g_weaponList[i].Slot == 1)
			return &g_weaponList[i];
	}

	return NULL;
}


static Weapon_List *GetPriWeapon(void)
{
	size_t i;

	for (i=0;i<g_weaponList.size();i++)
	{
		if (g_weaponList[i].Active && g_weaponList[i].Slot == 0)
			return &g_weaponList[i];
	}

	return NULL;
}


int GetWeaponIndexByID(int WeaponID)
{
	size_t i;

	for (i=0;i<g_weaponList.size();i++)
	{
		if (g_weaponList[i].Id == WeaponID)
			return i;
	}

	return -1;
}


int GetCurWeaponId(void)
{
	Weapon_List *tmpWeapon;

	tmpWeapon = GetCurWeapon();
	if (tmpWeapon)
		return tmpWeapon->Id;

	return -1;
}


int GetPriWeaponId(void)
{
	Weapon_List *tmpWeapon;

	tmpWeapon = GetPriWeapon();
	if (tmpWeapon)
		return tmpWeapon->Id;

	return -1;
}


int GetSecWeaponId(void)
{
	Weapon_List *tmpWeapon;

	tmpWeapon = GetSecWeapon();
	if (tmpWeapon)
		return tmpWeapon->Id;

	return -1;
}


const char *GetPriWeaponCmd(void)
{
	Weapon_List *tmpWeapon;

	tmpWeapon = GetPriWeapon();
	if (tmpWeapon)
		return tmpWeapon->command;

	return NULL;
}


const char *GetSecWeaponCmd(void)
{
	Weapon_List *tmpWeapon;

	tmpWeapon = GetSecWeapon();
	if (tmpWeapon)
		return tmpWeapon->command;

	return NULL;
}


const char *GetPriWeaponName(void)
{
	Weapon_List *tmpWeapon;

	tmpWeapon = GetPriWeapon();
	if (tmpWeapon)
		return tmpWeapon->name;

	return NULL;
}


const char *GetSecWeaponName(void)
{
	Weapon_List *tmpWeapon;

	tmpWeapon = GetSecWeapon();
	if (tmpWeapon)
		return tmpWeapon->name;

	return NULL;
}


bool HasPriWeapon(void)
{
	Weapon_List *tmpWeapon;

	tmpWeapon = GetPriWeapon();
	if (tmpWeapon)
		return true;

	return false;
}


bool HasSecWeapon(void)
{
	Weapon_List *tmpWeapon;

	tmpWeapon = GetSecWeapon();
	if (tmpWeapon)
		return true;

	return false;
}


int GetPrimaryAmmoX(void)
{
	Weapon_List *tmpWeapon;

	tmpWeapon = GetPriWeapon();
	if (tmpWeapon)
		return tmpWeapon->XAmmo;

	return -1;
}


int GetSecondaryAmmoX(void)
{
	Weapon_List *tmpWeapon;

	tmpWeapon = GetSecWeapon();
	if (tmpWeapon)
		return tmpWeapon->XAmmo;

	return -1;
}


int GetPrimaryAmmo(void)
{
	Weapon_List *tmpWeapon;

	tmpWeapon = GetPriWeapon();
	if (tmpWeapon)
		return tmpWeapon->CAmmo;

	return -1;
}


int GetSecondaryAmmo(void)
{
	Weapon_List *tmpWeapon;

	tmpWeapon = GetSecWeapon();
	if (tmpWeapon)
		return tmpWeapon->CAmmo;

	return -1;
}


bool IsCurWeaponPri(void)
{
	Weapon_List *tmpWeapon;

	tmpWeapon = GetPriWeapon();
	if (tmpWeapon && tmpWeapon->CurWeapon)
		return true;

	return false;
}


bool IsCurWeaponSec(void)
{
	Weapon_List *tmpWeapon;

	tmpWeapon = GetSecWeapon();
	if (tmpWeapon && tmpWeapon->CurWeapon)
		return true;

	return false;
}


bool IsCurWeaponGun(void)
{
	Weapon_List *tmpWeapon;

	tmpWeapon = GetCurWeapon();
	if (tmpWeapon && (tmpWeapon->Slot == 0 || tmpWeapon->Slot == 1))
		return true;

	return false;
}


bool CurWeaponHasAmmo(void)
{
	Weapon_List *tmpWeapon;

	tmpWeapon = GetCurWeapon();
	if (tmpWeapon && tmpWeapon->CAmmo > 0)
		return true;

	return false;
}


bool IsCurWeaponKnife(void)
{
	Weapon_List *tmpWeapon;

	tmpWeapon = GetCurWeapon();
	if (tmpWeapon && tmpWeapon->Id == WEAPONLIST_KNIFE)
		return true;

	return false;
}


bool IsCurWeaponNonAttack(void)
{
	Weapon_List *tmpWeapon;

	tmpWeapon = GetCurWeapon();
	if (tmpWeapon && (tmpWeapon->Id == WEAPONLIST_HEGRENADE || tmpWeapon->Id == WEAPONLIST_FLASHBANG || tmpWeapon->Id == WEAPONLIST_C4 || tmpWeapon->Id == WEAPONLIST_SMOKEGRENADE))
		return true;

	return false;
}


bool IsCurWeaponNade(void)
{
	Weapon_List *tmpWeapon;

	tmpWeapon = GetCurWeapon();
	if (tmpWeapon && (tmpWeapon->Id == WEAPONLIST_HEGRENADE || tmpWeapon->Id == WEAPONLIST_FLASHBANG || tmpWeapon->Id == WEAPONLIST_SMOKEGRENADE))
		return true;

	return false;
}


bool IsCurWeaponC4(void)
{
	Weapon_List *tmpWeapon;

	tmpWeapon = GetCurWeapon();
	if (tmpWeapon && tmpWeapon->Id == WEAPONLIST_C4)
		return true;

	return false;
}


const char *GetCurWeaponName(void)
{
	Weapon_List *tmpWeapon;

	tmpWeapon = GetCurWeapon();
	if (tmpWeapon)
		return tmpWeapon->name;

	return NULL;
}


int GetCurWeaponID(void)
{
	Weapon_List *tmpWeapon;

	tmpWeapon = GetCurWeapon();
	if (tmpWeapon)
		return tmpWeapon->Id;

	return 0;
}


bool HasWeaponName(const char *weaponname)
{
	size_t i, len;

	len = strlen(weaponname);

	for (i=0;i<g_weaponList.size();i++)
	{
		if (g_weaponList[i].Active && len == g_weaponList[i].len && !memcmp(g_weaponList[i].name, weaponname, len))
			return true;
	}

	return false;
}


void WeaponListUpdate(int WeaponBits)
{
	size_t i;
	static int OldWeaponBits = 0;

	if (WeaponBits != OldWeaponBits)
	{
		OldWeaponBits = WeaponBits;

		for (i=0;i<g_weaponList.size();i++)
		{
			if (WeaponBits & (1 << g_weaponList[i].Id))
			{
				g_weaponList[i].Active = true;
			}
			else
			{
				g_weaponList[i].Active = false;
				g_weaponList[i].CAmmo = 0;
			}
		}
	}
}


void WeaponListCurWeapon(int CurWeapon, int WeaponID, int Ammo)
{
	int index;

	index = GetWeaponIndexByID(WeaponID);
	if (index != -1)
		g_weaponList[index].CAmmo = Ammo;
}


void WeaponListAmmoX(int ID, int Count)
{
	size_t i;

	for (i=0;i<g_weaponList.size();i++)
	{
		if (g_weaponList[i].AmmoType == ID)
			g_weaponList[i].XAmmo = Count;
	}
}


void SetWeaponData(struct Weapon_List *weapon)
{
	switch (weapon->Id)
	{
		case WEAPONLIST_P228:
			weapon->ClipCap = CLIPCAP_P228;
			weapon->penetrate = WALL_PEN0;
			break;
		case WEAPONLIST_UNKNOWN1:
			weapon->ClipCap = 0;
			weapon->penetrate = WALL_PEN0;
			break;
		case WEAPONLIST_SCOUT:
			weapon->ClipCap = CLIPCAP_SCOUT;
			weapon->penetrate = WALL_PEN2;
			break;
		case WEAPONLIST_HEGRENADE:
			weapon->ClipCap = 0;
			weapon->penetrate = WALL_PEN0;
			break;
		case WEAPONLIST_XM1014:
			weapon->ClipCap = CLIPCAP_XM1014;
			weapon->penetrate = WALL_PEN0;
			break;
		case WEAPONLIST_C4:
			weapon->ClipCap = 0;
			weapon->penetrate = WALL_PEN0;
			break;
		case WEAPONLIST_MAC10:
			weapon->ClipCap = CLIPCAP_MAC10;
			weapon->penetrate = WALL_PEN0;
			break;
		case WEAPONLIST_AUG:
			weapon->ClipCap = CLIPCAP_AUG;
			weapon->penetrate = WALL_PEN1;
			break;
		case WEAPONLIST_SMOKEGRENADE:
			weapon->ClipCap = 0;
			weapon->penetrate = WALL_PEN0;
			break;
		case WEAPONLIST_ELITE:
			weapon->ClipCap = CLIPCAP_ELITE;
			weapon->penetrate = WALL_PEN0;
			break;
		case WEAPONLIST_FIVESEVEN:
			weapon->ClipCap = CLIPCAP_FIVESEVEN;
			weapon->penetrate = WALL_PEN0;
			break;
		case WEAPONLIST_UMP45:
			weapon->ClipCap = CLIPCAP_UMP45;
			weapon->penetrate = WALL_PEN0;
			break;
		case WEAPONLIST_SG550:
			weapon->ClipCap = CLIPCAP_SG550;
			weapon->penetrate = WALL_PEN1;
			break;
		case WEAPONLIST_UNKNOWN2:
			weapon->ClipCap = 0;
			weapon->penetrate = WALL_PEN0;
			break;
		case WEAPONLIST_UNKNOWN3:
			weapon->ClipCap = 0;
			weapon->penetrate = WALL_PEN0;
			break;
		case WEAPONLIST_USP:
			weapon->ClipCap = CLIPCAP_USP;
			weapon->penetrate = WALL_PEN0;
			break;
		case WEAPONLIST_GLOCK18:
			weapon->ClipCap = CLIPCAP_GLOCK18;
			weapon->penetrate = WALL_PEN0;
			break;
		case WEAPONLIST_AWP:
			weapon->ClipCap = CLIPCAP_AWP;
			weapon->penetrate = WALL_PEN2;
			break;
		case WEAPONLIST_MP5:
			weapon->ClipCap = CLIPCAP_MP5;
			weapon->penetrate = WALL_PEN0;
			break;
		case WEAPONLIST_M249:
			weapon->ClipCap = CLIPCAP_M249;
			weapon->penetrate = WALL_PEN1;
			break;
		case WEAPONLIST_M3:
			weapon->ClipCap = CLIPCAP_M3;
			weapon->penetrate = WALL_PEN0;
			break;
		case WEAPONLIST_M4A1:
			weapon->ClipCap = CLIPCAP_M4A1;
			weapon->penetrate = WALL_PEN1;
			break;
		case WEAPONLIST_TMP:
			weapon->ClipCap = CLIPCAP_TMP;
			weapon->penetrate = WALL_PEN0;
			break;
		case WEAPONLIST_G3SG1:
			weapon->ClipCap = CLIPCAP_G3SG1;
			weapon->penetrate = WALL_PEN1;
			break;
		case WEAPONLIST_FLASHBANG:
			weapon->ClipCap = 0;
			weapon->penetrate = WALL_PEN0;
			break;
		case WEAPONLIST_DEAGLE:
			weapon->ClipCap = CLIPCAP_DEAGLE;
			weapon->penetrate = WALL_PEN1;
			break;
		case WEAPONLIST_SG552:
			weapon->ClipCap = CLIPCAP_SG552;
			weapon->penetrate = WALL_PEN1;
			break;
		case WEAPONLIST_AK47:
			weapon->ClipCap = CLIPCAP_AK47;
			weapon->penetrate = WALL_PEN1;
			break;
		case WEAPONLIST_KNIFE:
			weapon->ClipCap = 0;
			weapon->penetrate = WALL_PEN0;
			break;
		case WEAPONLIST_P90:
			weapon->ClipCap = CLIPCAP_P90;
			weapon->penetrate = WALL_PEN0;
			break;
		default:
			weapon->ClipCap = 0;
			weapon->penetrate = WALL_PEN0;
			break;
	}
}


void WeaponListAdd(const char *weaponname, int ammo1type, int max1, int ammo2type, int max2, int slot, int slotpos, int id, int flags)
{
	struct Weapon_List dummy;
	int index, len;
	
	g_weaponList.push_back(dummy);

	index = g_weaponList.size() - 1;

	len = strlen(weaponname);
	if (len > MAX_WEAPON_NAME)
		return;

	memcpy(g_weaponList[index].command, weaponname, len + 1);

	if (len > 7 && !memcmp(weaponname, "weapon_", 7))
	{
		weaponname = weaponname + 7;
		g_weaponList[index].len = len - 7;
	}
	else
		g_weaponList[index].len = len;

	// Catch the special case of mp5navy where the msghook name differs from model name mp5
	if (g_weaponList[index].len == 7 && !memcmp(weaponname, "mp5navy", 7))
		g_weaponList[index].len = 3;

	memcpy(g_weaponList[index].name, weaponname, g_weaponList[index].len);
	g_weaponList[index].name[g_weaponList[index].len] = 0;

	g_weaponList[index].AmmoType = ammo1type;
	if (max1 == 255)
		g_weaponList[index].Max1 = -1;
	else
		g_weaponList[index].Max1 = max1;

	g_weaponList[index].Ammo2Type = ammo2type;
	if (max2 == 255)
		g_weaponList[index].Max2 = -1;
	else
		g_weaponList[index].Max2 = max2;

	g_weaponList[index].Slot = slot;
	g_weaponList[index].SlotPos = slotpos;

	g_weaponList[index].Id = id;
	g_weaponList[index].Flags = flags;

	g_weaponList[index].CAmmo = 0;
	g_weaponList[index].XAmmo = 0;

	g_weaponList[index].Active = false;
	g_weaponList[index].CurWeapon = false;

	SetWeaponData(&g_weaponList[index]);

	memset(&g_weaponList[index].weapondata, 0, sizeof(weapon_data_t));
}

const char *GetWeaponNameByID(int Id)
{
	int index;

	index = GetWeaponIndexByID(Id);
	if (index != -1)
			return g_weaponList[index].name;

	return NULL;
}

const char *GetWeaponName(int weaponmodel)
{
	return "unknown";
}

// ===================================================================================
// Spread and recoil handling
float GetSpreadFactor()
{
	if (g_pfnGetWeaponByID == nullptr || g_pGlobals == nullptr)
		return 0.0f;
	
	// Retrieve weapon info
	weapon_t* pWeapon = g_pfnGetWeaponByID(g_local.iWeaponId);
	int WeaponID = pWeapon->WeaponID;
	eWeaponBit WeaponBit = pWeapon->WeaponBit;
	bool OnGround = (g_local.pmFlags &FL_ONGROUND) != 0;
	bool Ducking = (g_local.pmFlags &FL_DUCKING) != 0;
	float FOV = (float)g_local.iFOV;
	float Speed = g_local.pmVelocity.Length2D();
	float SpreadVar = pWeapon->SpreadVar;
	float Recoil = (float)pWeapon->Recoil;
	float SpreadFactor = pWeapon->SpreadVar;

	// Factor spread based by weapon
	if (WeaponID == WEAPON_P228) // 1
	{
		// Calc spreadvar
		if (pWeapon->PredRecoil != 0.0f)
		{
			SpreadVar = pWeapon->SpreadVar - (0.325f - (g_pGlobals->time - pWeapon->PredRecoil)) * 0.3f;
			if (SpreadVar < 0.6f) SpreadVar = 0.6f;
			if (SpreadVar > 0.9f) SpreadVar = 0.9f;
		}

		// Factor spread
		if (OnGround)
		{
			if (Speed > 0.0f)
				SpreadFactor = (1.0f - SpreadVar) * 0.255f;
			else if (Ducking == true)
				SpreadFactor = (1.0f - SpreadVar) * 0.075f;
			else
				SpreadFactor = (1.0f - SpreadVar) * 0.15f;
		}
		else
			SpreadFactor = (1.0f - SpreadVar) * 1.5f;
	}

	if (WeaponID == WEAPON_SCOUT) // 3
	{
		// Factor spread
		if (OnGround)
		{
			if (Speed <= 170.0f)
			{
				if (Ducking)
					SpreadFactor = 0.0f;
				else
					SpreadFactor = 0.007f;
			}
			else
				SpreadFactor = 0.075f;
		}
		else
			SpreadFactor = 0.2f;

		if (FOV == 90.0f)
			SpreadFactor += 0.025f;
	}

	if (WeaponID == WEAPON_MAC10) // 7
	{
		// Calc spreadvar
		if (Recoil) Recoil -= 1;
		float Spread = (Recoil * Recoil * Recoil / 200) + 0.6f;
		SpreadVar = fminf(1.65f, Spread);

		// Factor spread
		if (OnGround)
			SpreadFactor = SpreadVar * 0.03f;
		else
			SpreadFactor = SpreadVar * 0.375f;
	}

	if (WeaponID == WEAPON_AUG) // 8
	{
		// Calc spreadvar
		if (Recoil) Recoil -= 1;
		float Spread = (Recoil * Recoil * Recoil / 215) + 0.3f;
		SpreadVar = fminf(1.0f, Spread);

		// Factor spread
		if (OnGround)
		{
			if (Speed <= 140)
				SpreadFactor = SpreadVar * 0.02f;
			else
				SpreadFactor = SpreadVar * 0.07f + 0.035f;
		}
		else
			SpreadFactor = SpreadVar * 0.4f + 0.035f;
	}

	if (WeaponID == WEAPON_ELITE) // 10 - inaccurate, todo: center guns based on wich is firing
	{
		// Calc spreadvar
		if (pWeapon->PredRecoil != 0.0f)
		{
			SpreadVar = pWeapon->SpreadVar - (0.325f - (g_pGlobals->time - pWeapon->PredRecoil)) * 0.275f;
			if (SpreadVar < 0.55f) SpreadVar = 0.55f;
			if (SpreadVar > 0.88f) SpreadVar = 0.88f;
		}

		// Factor spread
		if (OnGround)
		{
			if (Speed > 0.0f)
				SpreadFactor = (1.0f - SpreadVar) * 0.175f;
			else if (Ducking == true)
				SpreadFactor = (1.0f - SpreadVar) * 0.08f;
			else
				SpreadFactor = (1.0f - SpreadVar) * 0.1f;
		}
		else
			SpreadFactor = (1.0f - SpreadVar) * 1.3f;
	}

	if (WeaponID == WEAPON_FIVESEVEN) // 11
	{
		// Calc spreadvar
		if (pWeapon->PredRecoil != 0.0f)
		{
			SpreadVar = pWeapon->SpreadVar - (0.275f - (g_pGlobals->time - pWeapon->PredRecoil)) * 0.25f;
			if (SpreadVar < 0.725f) SpreadVar = 0.725f;
			if (SpreadVar > 0.92f) SpreadVar = 0.92f;
		}

		// Factor spread
		if (OnGround)
		{
			if (Speed > 0)
				SpreadFactor = (1.0f - SpreadVar) * 0.255f;
			else if (Ducking == true)
				SpreadFactor = (1.0f - SpreadVar) * 0.075f;
			else
				SpreadFactor = (1.0f - SpreadVar) * 0.15f;
		}
		else
			SpreadFactor = (1.0f - SpreadVar) * 1.5f;
	}

	if (WeaponID == WEAPON_UMP45) // 12
	{
		// Calc spreadvar
		if (Recoil) Recoil -= 1;
		float Spread = (Recoil * Recoil / 210) + 0.5f;
		SpreadVar = fminf(1.0f, Spread);

		// Factor spread
		if (OnGround)
			SpreadFactor = SpreadVar * 0.04f;
		else
			SpreadFactor = SpreadVar * 0.24f;
	}

	if (WeaponID == WEAPON_SG550) // 13
	{
		// Calc spreadvar
		if (pWeapon->PredRecoil != 0.0f)
		{
			SpreadVar = (g_pGlobals->time - pWeapon->PredRecoil) * 0.35f + 0.65f;
			if (SpreadVar > 0.98f)
				SpreadVar = 0.98f;
		}

		// Factor spread
		if (OnGround)
		{
			if (Speed > 0.0f)
				SpreadFactor = (1.0f - SpreadVar) * 0.15f;
			else if (Ducking == true)
				SpreadFactor = (1.0f - SpreadVar) * 0.04f;
			else
				SpreadFactor = (1.0f - SpreadVar) * 0.05f;
		}
		else
			SpreadFactor = (1.0f - SpreadVar) * 0.45f;

		if (FOV == 90.0f)
			SpreadVar += 0.025f;
	}

	if (WeaponID == WEAPON_GALIL) // 14
	{
		// Calc spreadvar
		if (Recoil) Recoil -= 1;
		float Spread = (Recoil * Recoil * Recoil / 200) + 0.35f;
		SpreadVar = fminf(1.25f, Spread);

		// Factor spread
		if (OnGround)
		{
			if (Speed <= 140.0f)
				SpreadFactor = SpreadVar * 0.0375f;
			else
				SpreadFactor = SpreadVar * 0.07f + 0.04f;
		}
		else
			SpreadFactor = SpreadVar * 0.3f + 0.04f;
	}

	if (WeaponID == WEAPON_FAMAS) // 15
	{
		// Calc spreadvar
		if (Recoil) Recoil -= 1;
		float Spread = (Recoil * Recoil * Recoil / 215) + 0.3f;
		SpreadVar = fminf(1.0f, Spread);

		// Factor spread
		if (OnGround)
		{
			if (Speed <= 140.0f)
				SpreadFactor = SpreadVar * 0.02f;
			else
				SpreadFactor = SpreadVar * 0.07f + 0.03f;
		}
		else
			SpreadFactor = SpreadVar * 0.3f + 0.03f;

		if (WeaponBit != WEAPONBIT_FAMASBURST)
			SpreadFactor += 0.01f;

		if (WeaponBit == WEAPONBIT_FAMASBURST && pWeapon->BurstSpread != 0.0f)
			SpreadFactor += 0.05f;
	}

	if (WeaponID == WEAPON_USP) // 16
	{
		// Calc spreadvar
		if (pWeapon->PredRecoil != 0.0f)
		{
			SpreadVar = pWeapon->SpreadVar - (0.3f - (g_pGlobals->time - pWeapon->PredRecoil)) * 0.275f;
			if (SpreadVar < 0.6f) SpreadVar = 0.6f;
			if (SpreadVar > 0.92f) SpreadVar = 0.92f;
		}

		// Factor spread
		if (WeaponBit == WEAPONBIT_USPSILENCER)
		{
			if (OnGround)
			{
				if (Speed > 0.0f)
					SpreadFactor = (1.0f - SpreadVar) * 0.225f;
				else if (Ducking == true)
					SpreadFactor = (1.0f - SpreadVar) * 0.08f;
				else
					SpreadFactor = (1.0f - SpreadVar) * 0.1f;
			}
			else
				SpreadFactor = (1.0f - SpreadVar) * 1.2f;
		}
		else
		{
			if (OnGround)
			{
				if (Speed > 0.0f)
					SpreadFactor = (1.0f - SpreadVar) * 0.25f;
				else if (Ducking == true)
					SpreadFactor = (1.0f - SpreadVar) * 0.125f;
				else
					SpreadFactor = (1.0f - SpreadVar) * 0.15f;
			}
			else
				SpreadFactor = (1.0f - SpreadVar) * 1.3f;
		}
	}

	if (WeaponID == WEAPON_GLOCK18) // 17
	{
		// Calc spreadvar
		if (pWeapon->PredRecoil != 0.0f)
		{
			SpreadVar = pWeapon->SpreadVar - (0.325f - (g_pGlobals->time - pWeapon->PredRecoil)) * 0.275f;
			if (SpreadVar < 0.6f) SpreadVar = 0.6f;
			if (SpreadVar > 0.9f) SpreadVar = 0.9f;
		}

		// Factor Spread
		if (WeaponBit >= WEAPONBIT_GLOCKBURST)
		{
			if (OnGround)
			{
				if (Speed > 0.0f)
					SpreadFactor = (1.0f - SpreadVar) * 0.185f;
				else if (Ducking == true)
					SpreadFactor = (1.0f - SpreadVar) * 0.095f;
				else
					SpreadFactor = (1.0f - SpreadVar) * 0.3f;
			}
			else
				SpreadFactor = (1.0f - SpreadVar) * 1.2f;
		}
		else
		{
			if (OnGround)
			{
				if (Speed > 0.0f)
					SpreadFactor = (1.0f - SpreadVar) * 0.165f;
				else if (Ducking == true)
					SpreadFactor = (1.0f - SpreadVar) * 0.075f;
				else
					SpreadFactor = (1.0f - SpreadVar) * 0.1f;
			}
			else
				SpreadFactor = 1.0f - SpreadVar;
		}

		if (!WeaponBit)
			SpreadFactor -= 0.05f;

		if (pWeapon->PistolBurstSpread != 0.0f)
			SpreadFactor += 0.1f;

	}

	if (WeaponID == WEAPON_AWP) // 18
	{
		// Factor spread
		if (OnGround)
		{
			if (Speed <= 10)
			{
				if (Ducking)
					SpreadFactor = 0.0f;
				else
					SpreadFactor = 0.001f;
			}
			else if (Speed <= 140)
				SpreadFactor = 0.1f;
			else
				SpreadFactor = 0.25f;
		}
		else
			SpreadFactor = 0.85f;

		if (FOV == 90.0f)
			SpreadFactor += 0.08f;
	}

	if (WeaponID == WEAPON_MP5) // 19
	{
		// Calc spreadvar
		if (Recoil) Recoil -= 1;
		float Spread = (Recoil * Recoil) * 0.004543389368468878f + 0.45f;
		SpreadVar = fminf(0.75f, Spread);

		// Factor spread
		if (OnGround)
			SpreadFactor = SpreadVar * 0.04f;
		else
			SpreadFactor = SpreadVar * 0.2f;
	}

	if (WeaponID == WEAPON_M249) // 20
	{
		// Calc spreadvar
		if (Recoil != 0.0f) Recoil -= 1;
		float Spread = (Recoil * Recoil * Recoil / 175) + 0.4f;
		SpreadVar = fminf(0.9f, Spread);

		// Factor spread
		if (OnGround)
		{
			if (Speed <= 140.0f)
				SpreadFactor = SpreadVar * 0.03f;
			else
				SpreadFactor = SpreadVar * 0.095f + 0.045f;
		}
		else
			SpreadFactor = SpreadVar * 0.5f + 0.045f;
	}

	if (WeaponID == WEAPON_M4A1) // 22
	{
		// Calc spreadvar
		if (Recoil != 0.0f) Recoil -= 1;
		float Spread = (Recoil * Recoil * Recoil / 220) + 0.3f;
		SpreadVar = fminf(1.0f, Spread);

		// Factor spread
		if (OnGround)
		{
			if (Speed <= 140.0f)
			{
				if (WeaponBit == WEAPONBIT_M4A1SILENCER)
					SpreadFactor = SpreadVar * 0.025f;
				else
					SpreadFactor = SpreadVar * 0.02f;
			}
			else
				SpreadFactor = SpreadVar * 0.07f + 0.035f;
		}
		else
			SpreadFactor = SpreadVar * 0.4f + 0.035f;
	}

	if (WeaponID == WEAPON_TMP) // 23
	{
		// Calc spreadvar
		if (Recoil) Recoil -= 1;
		float Spread = (Recoil * Recoil * Recoil / 200) + 0.55f;
		SpreadVar = fminf(1.4f, Spread);

		// Factor spread
		if (OnGround)
			SpreadFactor = SpreadVar * 0.03f;
		else
			SpreadFactor = SpreadVar * 0.25f;
	}

	if (WeaponID == WEAPON_G3SG1) // 24
	{
		// Calc spreadvar
		if (pWeapon->PredRecoil == 0.0f)
			SpreadVar = 0.98f;
		else
		{
			SpreadVar = (g_pGlobals->time - pWeapon->PredRecoil) * 0.3f + 0.55f;
			if (SpreadVar > 0.98f) SpreadVar = 0.98f;
		}

		// Factor spread
		if (OnGround)
		{
			if (Speed > 0.0f)
				SpreadFactor = (1.0f - SpreadVar) * 0.15f;
			else if (Ducking == true)
				SpreadFactor = (1.0f - SpreadVar) * 0.035f;
			else
				SpreadFactor = (1.0f - SpreadVar) * 0.055f;
		}
		else
			SpreadFactor = (1.0f - SpreadVar) * 0.45f;

		if (FOV == 90.0f)
			SpreadFactor += 0.025f;
	}

	if (WeaponID == WEAPON_DEAGLE) // 26 - inaccurate (did i forget something - punchangles done differently?)
	{
		// Calc spreadvar
		if (pWeapon->PredRecoil != 0.0f)
		{
			SpreadVar = pWeapon->SpreadVar - (0.4f - (g_pGlobals->time - pWeapon->PredRecoil)) * 0.35f;
			if (SpreadVar < 0.55f) SpreadVar = 0.55f;
			if (SpreadVar > 0.9f) SpreadVar = 0.9f;
		}

		// Factor spread
		if (OnGround)
		{
			if (Speed > 0.0f)
				SpreadFactor = (1.0f - SpreadVar) * 0.25f;
			else if (Ducking == true)
				SpreadFactor = (1.0f - SpreadVar) * 0.115f;
			else
				SpreadFactor = (1.0f - SpreadVar) * 0.13f;
		}
		else
			SpreadFactor = (1.0f - SpreadVar) * 1.5f;
	}

	if (WeaponID == WEAPON_SG552) // 27
	{
		// Calc spreadvar
		if (Recoil) Recoil -= 1;
		float Spread = (Recoil * Recoil * Recoil / 220) + 0.3f;
		SpreadVar = fminf(1.0f, Spread);

		// Factor spread
		if (OnGround)
		{
			if (Speed <= 140.0f)
				SpreadFactor = SpreadVar * 0.02f;
			else
				SpreadFactor = SpreadVar * 0.075f + 0.035f;
		}
		else
			SpreadFactor = SpreadVar * 0.45f + 0.035f;
	}

	if (WeaponID == WEAPON_AK47) // 28
	{
		// Calculate spreadvar
		if (Recoil) Recoil -= 1;
		float Spread = (Recoil * Recoil * Recoil / 200) + 0.35f;
		SpreadVar = fminf(1.25f, Spread);

		// Factor spread
		if (OnGround)
		{
			if (Speed <= 140.0f)
				SpreadFactor = SpreadVar * 0.0275f;
			else
				SpreadFactor = SpreadVar * 0.07f + 0.04f;
		}
		else
			SpreadFactor = SpreadVar * 0.4f + 0.04f;
	}

	if (WeaponID == WEAPON_P90) // 30
	{
		// Calc spreadvar
		if (Recoil) Recoil -= 1;
		float Spread = (Recoil * Recoil / 175) + 0.45f;
		SpreadVar = fminf(1.0f, Spread);

		// Factor spread
		if (OnGround)
		{
			if (Speed <= 170.0f)
				SpreadFactor = SpreadVar * 0.045f;
			else
				SpreadFactor = SpreadVar * 0.115f;
		}
		else
			SpreadFactor = SpreadVar * 0.3f;
	}

	return SpreadFactor;
}

bool UpdateSpreadAngles()
{
	// Check if weapon is ready
	if (!IsWeaponReady())
		return false;

	// Check if weapon type is pistol or higher
	if (GetWeaponType(g_local.iWeaponId) < WEAPONTYPE_PISTOL)
		return false;

	// Calculate spread angle
	Vector vAngles, vSpreadAngles, vSpread, vF, vR, vU;

	float flSpreadVar = GetSpreadFactor();
	float flSpreadX = gNoSpread.UTIL_SharedRandomFloat(g_local.iRandomSeed + 1, -0.5f, 0.5f) + gNoSpread.UTIL_SharedRandomFloat(g_local.iRandomSeed + 2, -0.5f, 0.5f);
	float flSpreadY = gNoSpread.UTIL_SharedRandomFloat(g_local.iRandomSeed + 3, -0.5f, 0.5f) + gNoSpread.UTIL_SharedRandomFloat(g_local.iRandomSeed + 4, -0.5f, 0.5f);

	flSpreadX *= flSpreadVar;
	flSpreadY *= flSpreadVar;

	gEngfuncs.pfnAngleVectors(g_local.viewAngles, vF, vR, vU);
	vSpread = vF + vR * flSpreadX + vU * flSpreadY;
	VectorAngles(vSpread, vSpreadAngles);
	vSpreadAngles[0] *= -1;

	VectorSubtract(g_local.viewAngles, vSpreadAngles, vAngles);

	// Apply spread
	g_local.vSpread.x = vAngles.x;
	g_local.vSpread.y = vAngles.y;
	g_local.vSpread.z = 0.0f;

	return true;
}

void UpdatePunchAngles(ref_params_t * pParams)
{
	VectorCopy(pParams->punchangle, g_local.punchangle);
	if (g_local.punchangle.Length2D() == 0.0f)
		return;

	float flLength = (float)VectorNormalize(g_local.punchangle);
	float flTmp = flLength - (0.5f * flLength + 10.0f) * pParams->frametime;
	if (flTmp < 0.0f)
		flTmp = 0.0f;

	VectorScale(g_local.punchangle, flTmp, g_local.punchangle);
}

bool IsWeaponReady()
{
	if (g_pfnGetWeaponByID == nullptr)
		return false;
	
	// Check if weapon is ready (ptrs etc retrieved)
	weapon_t* pWeapon = g_pfnGetWeaponByID(g_local.iWeaponId);
	if (pWeapon == NULL)
		return false;

	if (pWeapon->Ammo == 0)
		return false;

	if (pWeapon->Reloading)
		return false;

	return true;
}

eWeaponType GetWeaponType(int WeaponID)
{
	// Simplifies things when doing nospread, aimbot etc. anything that required to lock out weapons
	eWeaponType WeaponType = WEAPONTYPE_NONE;
	if (WeaponID == WEAPON_P228 || WeaponID == WEAPON_USP || WeaponID == WEAPON_GLOCK18 || WeaponID == WEAPON_DEAGLE || WeaponID == WEAPON_ELITE || WeaponID == WEAPON_FIVESEVEN)
		WeaponType = WEAPONTYPE_PISTOL;
	if (WeaponID == WEAPON_M3 || WeaponID == WEAPON_XM1014)
		WeaponType = WEAPONTYPE_SHOTGUN;
	if (WeaponID == WEAPON_MAC10 || WeaponID == WEAPON_UMP45 || WeaponID == WEAPON_MP5 || WeaponID == WEAPON_TMP || WeaponID == WEAPON_P90)
		WeaponType = WEAPONTYPE_SMG;
	if (WeaponID == WEAPON_AUG || WeaponID == WEAPON_GALIL || WeaponID == WEAPON_FAMAS || WeaponID == WEAPON_M4A1 || WeaponID == WEAPON_AK47 || WeaponID == WEAPON_SG552 || WeaponID == WEAPON_M249)
		WeaponType = WEAPONTYPE_RIFLE;
	if (WeaponID == WEAPON_SCOUT || WeaponID == WEAPON_G3SG1 || WeaponID == WEAPON_SG550 || WeaponID == WEAPON_AWP)
		WeaponType = WEAPONTYPE_SNIPER;
	if (WeaponID == WEAPON_NONE || WeaponID == WEAPON_UNKNOWN1 || WeaponID == WEAPON_HEGRENADE || WeaponID == WEAPON_C4 || WeaponID == WEAPON_SMOKEGRENADE || WeaponID == WEAPON_FLASHBANG)
		WeaponType = WEAPONTYPE_MISC;
	if (WeaponID == WEAPON_KNIFE)
		WeaponType = WEAPONTYPE_KNIFE;

	return WeaponType;
}
