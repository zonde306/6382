#ifndef __INSTALL_H__
#define __INSTALL_H__

typedef int(*FnUserMsgHook)(const char* pszName, int iSize, void* pbuf);
typedef struct _UserMgsList
{
	DWORD unknown1, unknown2;
	CHAR msgname[16];
	struct _UserMgsList *next;
	FnUserMsgHook address;
} UserMgsList, *PUserMgsList;

typedef void(*PreS_DynamicSound_t)(int, DWORD, char *, float *, DWORD, DWORD, DWORD, DWORD);
void InstallCheat();
void UninstallCheat();

#endif