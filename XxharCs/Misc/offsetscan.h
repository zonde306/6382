#pragma once
#include <Windows.h>
#include "../Engine/interface.h"

struct Color24
{
	BYTE R, G, B;
};

typedef Color24 TColor24;
typedef Color24 *PColor24;

class COffsets
{
public:
	BYTE HLType;
	DWORD ClBase, ClSize, ClEnd;
	DWORD HwBase, HwSize, HwEnd;
	DWORD VgBase, VgSize;

	CreateInterfaceFn Engine_CreateInterface;
	CreateInterfaceFn VGui2_CreateInterface;
	CreateInterfaceFn GameUI_CreateInterface;

	bool Initialize(void);
	void *GameConsole(void);
	void *SpeedHackPtr(void);
	void *ClientFuncs(void);
	void *EngineFuncs(void);
	void* GetCurosrTeam();
	void* Slots();
	void* FireBullets();
	void* FireBullets3();
	void* InitPoint();
	void* SendPacket();
	void* VGuiPaint();
	void* GetWeaponByID();
	void* GlobalVars();
	void* ExportPointer();
	DWORD EngineStudio(void);
	void GameInfo(void);
	void *Sound(void);
	void Error(const char* Msg);
	DWORD GetModuleSize(const DWORD Address);
	DWORD FarProc(const DWORD Address, DWORD LB, DWORD HB);
	void ConsoleColorInitalize();
	DWORD ClientBase(void);

	struct GameInfo_s
	{
		char* GameName;
		char* GameVersion;
		BYTE Protocol;
		int Build;
	};

	GameInfo_s BuildInfo;
};
