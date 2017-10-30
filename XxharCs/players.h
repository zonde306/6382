#ifndef __PLAYERS_H__
#define __PLAYERS_H__
#include "./Engine/util_vector.h"

extern struct cl_enginefuncs_s gEngfuncs;
extern engine_studio_api_s* pStudio;
struct spread_info
{
	unsigned int random_seed;
	int recoil;
	float gtime;
	float prevtime;
	float brokentime; // This is only supposed to be set to zero when you buy the sg550 or the g3sg1
						// not when you reload, switch weapons or pick up a weapon, this is do to the
						// cs bugs for these guns (valve fix your code please)
	float spreadvar;
	float recoiltime;
	bool firing;
	int WeaponState;
	int prcflags;
};

struct sMe
{
	int iClip;
	int iAmmo;
	int iHealth;
	int iWeaponId;
	int iArmor;
	int iKills;
	int iDeaths;
	int iHs;
	int iMoney;
	int iCrosshairId;

	int iFOV;
	float pmVelocity[3];
	float punchangle[3];
	float pmEyePos[3];
	int pmFlags;
	float viewAngles[3];
	float sin_yaw, minus_cos_yaw;
	bool inZoomMode;
	int pmMoveType;
	int team;
	struct cl_entity_s * ent;
	int entindex;
	bool alive;
	spread_info spread;

	float maxspeed;
	float fNextPrimAttack;
	float fNextSecAttack;
	bool bInReloading;

	float aimTrigRadius;
	float aimTrigDiff;
	std::string sServerName;

	void DoAntiAim(struct usercmd_s *usercmd);
	void DoAntiAim2(struct usercmd_s *usercmd);
	void DoBunnyHop(struct usercmd_s *usercmd);
	void DoAutoPistol(struct usercmd_s *usercmd);
	void DoTriggerBot(struct usercmd_s *usercmd);
	void AdjustSpeed(double speed);
	void DoFastWalk();
	void DoFastRun(struct usercmd_s *usercmd);

	void DoAntiRecoil(struct usercmd_s* usercmd, float frametime);
	void DoAntiSpread(struct usercmd_s* usercmd);
	void DoSilentAngles(struct usercmd_s* usercmd, float* aimangles);
	bool DoSmoothAngles(float* Source, float* Destination, float* NewDestination, float Factor);
	void DoSilentAngles(float* aimangles, struct usercmd_s* usercmd);
};

enum 
{ 
     SEQUENCE_IDLE     = 0, 
     SEQUENCE_SHOOT    = 1, 
     SEQUENCE_RELOAD   = 2, 
     SEQUENCE_DIE      = 4, 
     SEQUENCE_THROW    = 8, 
     SEQUENCE_ARM_C4   = 16, 
     SEQUENCE_SHIELD   = 32,
	 SEQUENCE_SHIELD_SIDE = 64
};

//Players Info
class PlayerInfo 
{
protected:
	friend class VecPlayers;
	void init( int _entindex) 
	{ 
		team=2;
		entinfo.name="\\missing-name\\";
		entinfo.model="missing-model";
		alive=false;
		entindex = _entindex;
		distance = 1000.0;
		visible  = 0;
		strcpy(m_weapon,"N-A");
		m_updated = false;
		bGotHead = false;
		fixHbAim = false;
		canAim = false;
		bDrawn = false;
	}
private:
	hud_player_info_t entinfo;

public:
	void updateEntInfo()
	{
		gEngfuncs.pfnGetPlayerInfo(entindex, &entinfo);
		if(!entinfo.name ) { entinfo.name  = "\\missing-name\\"; }
		if(!entinfo.model) { entinfo.model = "unknown model";    } 
	}
	const char* getName() const { return entinfo.name; }

	int    team;
	int    iInfo;
	float  distance;
	bool    visible;
	float  fovangle;   // minimum fov a player is in
	vec3_t vHitbox;
	bool bGotHead;
	bool fixHbAim;
	int canAim;
	bool bDrawn;
	Vector hitbox[13];
	Vector bone[MAXSTUDIOBONES];
	float SoundOrigin[3];

	void setAlive() { alive = true;  }
	void setDead() { alive = false;  }
	bool isAlive () { return alive!=false; }
	void drawBone(int bone1, int bone2, byte r, byte g, byte b, byte a);

	struct cl_entity_s * getEnt() { return gEngfuncs.GetEntityByIndex(entindex); }
	// position update queries
	enum   { UPDATE_MISSING=0, UPDATE_ADDENT=1, UPDATE_SOUND_RADAR=2};
	// weapon update:
	void updateSoundRadar(void) { m_lastUpdateType=UPDATE_SOUND_RADAR; }
	const char* getWeapon() { return m_weapon; }
	void setWeapon(const char* newname)
	{
		register int len = strlen(newname);
		if(len && len<30) { strcpy(m_weapon,newname); }
	}
	// position update reporting
	void   updateClear    ()
	{ 
		bDrawn = false;
		m_lastUpdateType=UPDATE_MISSING; 
	}
	void SetOrigin(const float* neworg)
	{
		VectorCopy(neworg,m_origin);
	}
	void   updateAddEntity (const float* neworg)
	{ 
		if(alive)
		{
			m_lastUpdateType=UPDATE_ADDENT; 
			m_lastUpdateTime=(double)timeGetTime()/ 1000.0;

			VectorCopy(neworg,m_origin); 
		}
	}

	bool   isUpdated       ()    { return m_lastUpdateType!=UPDATE_MISSING; }
	bool   isUpdatedAddEnt ()    { return m_lastUpdateType==UPDATE_ADDENT;  }
	int    updateType()          { return m_lastUpdateType; }
	const Vector& origin()       { return m_origin; }
	float  timeSinceLastUpdate() { return (float)((double)timeGetTime()/ 1000.0-m_lastUpdateTime); }

private:
	int     m_lastUpdateType;
	double  m_lastUpdateTime;
	Vector   m_origin;
	char    m_weapon[32];
	bool    m_updated;

	bool alive;
	int entindex;

public:
	PlayerInfo() { init(0); }
};

enum{  MAX_VPLAYERS =36 };
class VecPlayers
{
private:
	PlayerInfo* players;
public:
	~VecPlayers() { delete[] players; }
	VecPlayers() 
	{ 
		players = new PlayerInfo[MAX_VPLAYERS];
		for(int i=0;i<MAX_VPLAYERS;i++) { players[i].init(i); }
	}

	inline PlayerInfo& operator [] (unsigned int i)
	{
		if(i>=MAX_VPLAYERS) {return players[0];}
		else                {return players[i];}
	}
	
	inline unsigned int size() { return MAX_VPLAYERS; }
};
//==============================================================================
extern VecPlayers g_playerList;
extern sMe g_local;

typedef struct _ENTITIES{
	float Origin[3];
	bool Alive;
	bool Visible;
	bool Valid;
	POINT ESP;
	POINT Head;
	float BoxHeight;
	int team;
}ENTITIES,*PENTITIES;

extern ENTITIES g_playerDrawList[33];

//////////////////////////////////////////////////////////////////////////
bool isValidEnt(cl_entity_s *ent);
bool bIsEntValid(cl_entity_s * ent,int index);
void UpdateMe(void);
void playerCalcExtraData(int ax, cl_entity_s* ent);
void AtRoundStart();
void InitVisuals(void);
void drawPlayerEsp(int ax);
extern int getSeqInfo(int ax);
extern float mainViewAngles[3];

bool CalcScreen(const float *origin, float *vecScreen);

bool ValidTarget(cl_entity_s* ent);

typedef struct
{
	float flMatrix[4][4];
}WorldToScreenMatrix_t;

bool GetWorldToScreenMatrix(WorldToScreenMatrix_t *pWorldToScreenMatrix, float *flOrigin, float *flOut);
bool WorldToScreen(float *flOrigin, float *flOut);
void CalcAngle(Vector src, Vector end, Vector& out);
float GetAntiAimAngles();
float GetNoRecoilValue(int weaponId);

#endif