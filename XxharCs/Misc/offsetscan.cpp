﻿#include "offsetscan.h"
#include "memoryscan.h"
#include "../install.h"
#include "../clientdll.h"
#include "xorstr.h"

extern cl_clientfunc_t *g_pClient;
extern cl_enginefunc_t *g_pEngine;
extern engine_studio_api_t *g_pStudio;
extern cl_clientslots_s* g_pSlots;

#define RENDERTYPE_UNDEFINED	0
#define RENDERTYPE_SOFTWARE		1
#define RENDERTYPE_HARDWARE		2

#define ENGINE_LAUNCHER_API_VERSION 1

extern PColor24 Console_TextColor;

bool COffsets::Initialize(void)
{
	DWORD GameUI = (DWORD)GetModuleHandle("GameUI.dll");
	DWORD vgui = (DWORD)GetModuleHandle("vgui.dll");
	DWORD vgui2 = (DWORD)GetModuleHandle("vgui2.dll");
	DWORD particleman = (DWORD)GetModuleHandle("particleman.dll");

	HLType = RENDERTYPE_UNDEFINED;

	HwBase = (DWORD)GetModuleHandle("hw.dll"); // Hardware
	if (HwBase == NULL)
	{
		HwBase = (DWORD)GetModuleHandle("sw.dll"); // Software
		if (HwBase == NULL)
		{
			HwBase = (DWORD)GetModuleHandle(NULL); // Non-Steam?
			if (HwBase == NULL) // Unknown client type
			{
				Error("Unknown client type.");
			}
			else
				HLType = RENDERTYPE_UNDEFINED;
		}
		else
			HLType = RENDERTYPE_SOFTWARE;
	}
	else
		HLType = RENDERTYPE_HARDWARE;

	HwSize = (DWORD)GetModuleSize(HwBase);
	if (HwSize == NULL)
	{
		switch (HwSize)
		{
		case RENDERTYPE_HARDWARE: HwSize = 0x122A000; break;
		case RENDERTYPE_UNDEFINED: HwSize = 0x2116000; break;
		case RENDERTYPE_SOFTWARE: HwSize = 0xB53000; break;
		default:Error("Couldn't find module size.");
		}
	}
	HwEnd = HwBase + HwSize - 1;

	ClBase = (DWORD)GetModuleHandle("client.dll");
	if (ClBase == NULL)
	{
		ClBase = HwBase;
		ClSize = HwSize;
	}
	else
	{
		ClSize = GetModuleSize(ClBase);
		if (ClBase == NULL)
		{
			ClSize = 0x159000;
		}
	}
	ClEnd = ClBase + ClSize - 1;

	VgBase = (DWORD)GetModuleHandle("GameUI.dll");
	VgSize = (DWORD)0x000E4000;

	return (ClBase && HwBase && GameUI && vgui && vgui2 && particleman);
}

void COffsets::Error(const char* Msg)
{
	MessageBoxA(0, Msg, "Fatal Error", MB_OK | MB_ICONERROR);
	ExitProcess(1);
	// __halt();
}

DWORD COffsets::GetModuleSize(const DWORD Address)
{
	return PIMAGE_NT_HEADERS(Address + (DWORD)PIMAGE_DOS_HEADER(Address)->e_lfanew)->OptionalHeader.SizeOfImage;
}

DWORD COffsets::FarProc(const DWORD Address, DWORD LB, DWORD HB)
{
	if (!Address) return true;
	return ((Address < LB) || (Address > HB));
}

bool CompareMemBlock(BYTE *bAddress, BYTE *bCode, int iCodeLen)
{
	for (int j = 0; j < iCodeLen; bAddress++, bCode++, j++)
	{
		if ((*bAddress != *bCode) && (*bCode != 0xFF))
			return false;
	}

	return true;
}

DWORD FindCodeSignature(DWORD dwStart, DWORD dwLength, BYTE *bCode, int nCodeLen, int nCodeNum)
{
	for (DWORD i = dwStart; (i + nCodeLen) < (dwStart + dwLength); i++)
	{
		if (CompareMemBlock((BYTE *)i, bCode, nCodeLen))
			return (DWORD)(i + nCodeNum);
	}
	return 0;
}

void *COffsets::GameConsole(void)
{
	while (!GetModuleHandle("GameUI.dll") && !GetModuleHandle("vgui.dll") && !GetModuleHandle("vgui2.dll"))
		Sleep(100);
	BYTE bPushAddrPattern[] = { 0x68, 0x90, 0x90, 0x90, 0x90 };
	char cVgBase[] = "GameConsole003";
	DWORD VgBaseString = FindCodeSignature(VgBase, VgBase + VgSize, (PBYTE)cVgBase, sizeof(cVgBase) - 1, 0);
	*(DWORD *)(bPushAddrPattern + 1) = VgBaseString;
	DWORD Addres = 0;
	Addres = FindCodeSignature(VgBase, VgBase + VgSize, bPushAddrPattern, 5, -0);
	for (int i = 1; i < 70; i++)
	{
		Addres++;
		if (*(BYTE *)(DWORD *)Addres == 0xB8)
		{
			return (void*)*(DWORD *)(Addres + 1);
		}
	}
}

void COffsets::ConsoleColorInitalize()
{
	DWORD VgBasePtr = (DWORD)this->GameConsole();
	DWORD Panel = (*(DWORD*)(VgBasePtr + 8) - VgBasePtr);
	Console_TextColor = PColor24(Panel + VgBasePtr + 288 + sizeof(unsigned));
	if (*(unsigned*)(unsigned(Console_TextColor) + 8) != 0)
	{
		Console_TextColor = PColor24(Panel + VgBasePtr + 288 + sizeof(unsigned) + sizeof(unsigned));
	}
}

unsigned Absolute(DWORD Addr)
{
	return unsigned(Addr) + *(unsigned*)(Addr)+4;
}

// Game Version Info, auto offset by _or_75
void COffsets::GameInfo(void)
{
	static bool search = false;
	BYTE bPushAddrPattern[] = { 0x68, 0x90, 0x90, 0x90, 0x90 };
	char cProt[] = "fullinfo <complete info string>";
	DWORD dwGameInfo = FindCodeSignature(HwBase, HwBase + HwSize, (PBYTE)cProt, sizeof(cProt) - 1, 0);
	*(DWORD *)(bPushAddrPattern + 1) = dwGameInfo;
	DWORD Addres = 0;
	Addres = FindCodeSignature(HwBase, HwBase + HwSize, (PBYTE)bPushAddrPattern, 5, -0);
	for (int i = 1; i < 40; i++)
	{
		Addres--;
		if (*(BYTE *)(DWORD *)Addres == 0xC3)
			search = true;
		if (search == true)
			for (int i = 1; i < 50; i++)
			{
				Addres--;
				if (*(BYTE *)(DWORD *)Addres == 0x90)
				{
					Addres = Addres + 1;
					goto finish;
				}
			}
	}
finish:
	if (FarProc((DWORD)Addres, HwBase, HwEnd))
		Error("Couldn't find GameInfo pointer.");
	
	BuildInfo.GameName = *(char**)(unsigned(Addres) + 1);
	BuildInfo.GameVersion = *(char**)(unsigned(Addres) + 6);
	BuildInfo.Protocol = *(BYTE*)(unsigned(Addres) + 11);
	Addres = (DWORD)Absolute(unsigned(Addres) + 23);

	/*
	int(*GetBuildNumber)(void) = (int(*)(void))FindPattern(HwBase, HwBase + HwSize, XorStr("A1 ? ? ? ? 83 EC 08 ? 33 ? 85 C0"));
	if(GetBuildNumber == nullptr)
		GetBuildNumber = (int(*)(void))FindPattern(HwBase, HwBase + HwSize, XorStr("55 8B EC 83 EC 08 A1 ? ? ? ? 56 33 F6 85 C0 0F 85 ? ? ? ? 53 33 DB 8B 04 9D"));
	
	if (GetBuildNumber != nullptr)
		BuildInfo.Build = GetBuildNumber();
	*/

	/*
	DWORD build = 0;

	__asm
	{
		call	Addres
		mov		build, eax
	}

	this->BuildInfo.Build = build;
	*/
}

// sound esp, auto offset by _or_75
void *COffsets::Sound(void)
{
	BYTE bPushAddrPattern[] = { 0x68, 0x90, 0x90, 0x90, 0x90 };
	char cSoundString[] = "S_StartDynamicSound: %s volume > 255";
	DWORD dwSoundString = FindCodeSignature(HwBase, HwBase + HwSize, (PBYTE)cSoundString, sizeof(cSoundString) - 1, 0);
	*(DWORD *)(bPushAddrPattern + 1) = dwSoundString;
	DWORD Addres = 0;
	Addres = FindCodeSignature(HwBase, HwBase + HwSize, (PBYTE)bPushAddrPattern, 5, -0);
	for (int i = 1; i < 250; i++)
	{
		Addres--;
		if (*(BYTE *)(DWORD *)Addres == 0x90)
		{
			Addres++;
			return (void*)Addres;
		}
	}
	return (void*)Addres;
}

void * COffsets::Slots()
{
	DWORD address = FindPattern(HwBase, HwBase + HwSize, XorStr("68 ?? ?? ?? ?? FF 15 ?? ?? ?? ?? 68 ?? ?? ?? ?? FF 15 ?? ?? ?? ?? A1 ?? ?? ?? ?? 83 C4 0C"));
	if (address == 0)
		return nullptr;

	return (void*)(address + 0x01);
}

void * COffsets::GetCurosrTeam()
{
	DWORD address = FindPattern(HwBase, HwBase + HwSize, XorStr("A1 ? ? ? ? 8B 0D ? ? ? ? 8D 50 ? 3B D1 7E ? C7 05 60 7F A2 01 ? ? ? ? 83 C8 ? C3 8B 0D ? ? ? ? 56 66 0F ? ? ? ? 66 0F ? ? ? C1 E6 ? 03 F0 89 15 ? ? ? ? 0F BF ? 5E C3 90 90 90 90 90 90 90 90 90 90 90 90 90 90 A1"));
	if (address == 0)
		return nullptr;

	return (void*)address;
}

void *COffsets::SpeedHackPtr(void)
{
	DWORD Old = NULL;
	PCHAR String = "Texture load: %6.1fms";
	DWORD Address = (DWORD)FindMemoryClone(HwBase, HwBase + HwSize, String, strlen(String));
	void* SpeedPtr = (void*)*(DWORD*)(FindReference(HwBase, HwBase + HwSize, Address) - 7);
	if (FarProc((DWORD)SpeedPtr, HwBase, HwEnd))
		Error("Couldn't find SpeedPtr pointer.");
	else
		VirtualProtect(SpeedPtr, sizeof(double), PAGE_READWRITE, &Old);
	return SpeedPtr;
}

void * COffsets::FireBullets()
{
	PCHAR FireString = "%.4f %.4f %.4f";
	DWORD FireBullets = (DWORD)FindMemoryClone(ClBase, ClEnd, FireString, strlen(FireString));
	FireBullets = (DWORD)(FindReference(ClBase, ClEnd, FireBullets) + 0x1C);
	return (PVOID)FireBullets;
}

void *COffsets::ClientFuncs(void)
{
	PCHAR String = "ScreenFade";
	DWORD Address = (DWORD)FindMemoryClone(HwBase, HwBase + HwSize, String, strlen(String));
	void* ClientPtr = (void*)*(DWORD*)(FindReference(HwBase, HwBase + HwSize, Address) + 0x13);
	if (FarProc((DWORD)ClientPtr, HwBase, HwEnd))
		Error("Couldn't find ClientPtr pointer.");
	return ClientPtr;
}

void *COffsets::EngineFuncs(void)
{
	PCHAR String = "ScreenFade";
	DWORD Address = FindMemoryClone(HwBase, HwBase + HwSize, String, strlen(String));
	void* EnginePtr = (void*)*(DWORD*)(FindReference(HwBase, HwBase + HwSize, Address) + 0x0D);
	if (FarProc((DWORD)EnginePtr, HwBase, HwEnd))
		Error("Couldn't find EnginePtr pointer.");
	return EnginePtr;
}

DWORD COffsets::EngineStudio(void)
{
	PCHAR String = "Couldn't get client .dll studio model rendering interface.";
	DWORD Address = FindMemoryClone(HwBase, HwBase + HwSize, String, strlen(String));
	void* EngineStudioPtr = (void*)*(DWORD*)(FindReference(HwBase, HwBase + HwSize, Address) - 0x14);
	if (FarProc((DWORD)EngineStudioPtr, HwBase, HwEnd))
		Error("Couldn't find EngineStudioPtr pointer.");

	/*
	if (g_pClient->HUD_GetStudioModelInterface == nullptr && g_pSlots->HUD_GetStudioModelInterface != nullptr)
		g_pClient->HUD_GetStudioModelInterface = g_pSlots->HUD_GetStudioModelInterface;
	*/
	if (g_pClient->HUD_GetStudioModelInterface == nullptr)
		return 0;

	DWORD dwStudioone = 0;
	for (DWORD i = 0; i <= 60; i++)
	{
		dwStudioone = (DWORD)g_pClient->HUD_GetStudioModelInterface + i;
		if (*(BYTE*)(DWORD*)dwStudioone == 0xB8 || *(BYTE*)(DWORD*)dwStudioone == 0xBF)
		{
			dwStudioone++;
			if (*(WORD*)(DWORD*)EngineStudioPtr == *(WORD*)(DWORD*)(*(DWORD*)dwStudioone))
			{
				return *(DWORD*)dwStudioone;
			}
		}
	}
	return 0;
}

DWORD COffsets::ClientBase(void)
{
	return (DWORD)ClBase;
}
