#ifndef __INSTALL_H__
#define __INSTALL_H__
#include "weaponlist.h"
#include "Engine/ISurface.h"

typedef int(*FnUserMsgHook)(const char* pszName, int iSize, void* pbuf);
typedef struct _UserMgsList
{
	DWORD unknown1, unknown2;
	CHAR msgname[16];
	struct _UserMgsList *next;
	FnUserMsgHook address;
} UserMgsList, *PUserMgsList;

typedef void(__cdecl* FnSendPacket)();
typedef int(__cdecl *FnVGuiPaint)();
typedef weapon_t*(__cdecl* FnGetWeaponByID)(int weaponId);
typedef void(*PreS_DynamicSound_t)(int, DWORD, const char*, float[3], DWORD, DWORD, DWORD, DWORD);
DWORD WINAPI InstallCheat(LPVOID params);
void UninstallCheat();

typedef void(__thiscall *FnPaintTraverse)(PVOID, vgui::IPanel*, bool, bool);
void __fastcall Hooked_PaintTraverse(vgui::IPanel*, PVOID, vgui::IPanel*, bool, bool);

#endif