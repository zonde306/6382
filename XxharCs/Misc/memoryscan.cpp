#include "memoryscan.h"
#include <windows.h>
#include <TlHelp32.h>

BOOL __comparemem(const UCHAR *buff1, const UCHAR *buff2, UINT size)
{
	for (UINT i = 0; i < size; i++, buff1++, buff2++)
	{
		if ((*buff1 != *buff2) && (*buff2 != 0xFF))
			return FALSE;
	}
	return TRUE;
}
ULONG __findmemoryclone(const ULONG start, const ULONG end, const ULONG clone, UINT size)
{
	for (ULONG ul = start; (ul + size) < end; ul++)
	{
		if (CompareMemory(ul, clone, size))
			return ul;
	}
	return NULL;
}
ULONG __findreference(const ULONG start, const ULONG end, const ULONG address)
{
	UCHAR Pattern[5];
	Pattern[0] = 0x68;
	*(ULONG*)&Pattern[1] = address;
	return FindMemoryClone(start, end, Pattern, sizeof(Pattern) - 1);
}

HMODULE GetModuleHandleSafe(const std::string& pszModuleName)
{
	HMODULE hmModuleHandle = NULL;

	do
	{
		hmModuleHandle = GetModuleHandleA(pszModuleName.c_str());
		std::this_thread::sleep_for(std::chrono::microseconds(100));
	} while (hmModuleHandle == NULL);

	return hmModuleHandle;
}

#define INRANGE( x, a, b ) ( x >= a && x <= b )
#define getBits( x ) ( INRANGE( ( x & ( ~0x20 ) ), 'A', 'F' ) ? ( ( x & ( ~0x20 ) ) - 'A' + 0xa ) : ( INRANGE( x, '0', '9' ) ? x - '0' : 0 ) )
#define getByte( x ) ( getBits( x[ 0 ] ) << 4 | getBits( x[ 1 ] ) )
#define ThreadBasicInformation 0

DWORD FindPattern(DWORD dwAddress, DWORD dwFinalAddress, const std::string& szPattern)
{
	const char *pat = szPattern.c_str();
	DWORD firstMatch = NULL;
	for (DWORD pCur = dwAddress; pCur < dwFinalAddress; pCur++)
	{
		if (!*pat)
			return firstMatch;
		if (*(PBYTE)pat == '\?' || *(BYTE *)pCur == getByte(pat))
		{
			if (!firstMatch)
				firstMatch = pCur;
			if (!pat[2])
				return firstMatch;
			if (*(PWORD)pat == '\?\?' || *(PBYTE)pat != '\?')
				pat += 3;
			else
				pat += 2;
		}
		else
		{
			pat = szPattern.c_str();
			firstMatch = 0;
		}
	}

	return NULL;
}

DWORD FindPattern(const std::string& szPattern)
{
	static HMODULE hmModule = nullptr;
	do
	{
		hmModule = GetModuleHandleA(nullptr);
		std::this_thread::sleep_for(std::chrono::microseconds(100));
	} while (hmModule == nullptr);

	PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)hmModule;
	PIMAGE_NT_HEADERS pNTHeaders = (PIMAGE_NT_HEADERS)(((DWORD)hmModule) + pDOSHeader->e_lfanew);

	DWORD dwAddress = ((DWORD)hmModule) + pNTHeaders->OptionalHeader.BaseOfCode;
	DWORD dwLength = ((DWORD)hmModule) + pNTHeaders->OptionalHeader.SizeOfCode;

	const char *pat = szPattern.c_str();
	DWORD firstMatch = NULL;
	for (DWORD pCur = dwAddress; pCur < dwLength; pCur++)
	{
		if (!*pat)
			return firstMatch;
		if (*(PBYTE)pat == '\?' || *(BYTE *)pCur == getByte(pat))
		{
			if (!firstMatch)
				firstMatch = pCur;
			if (!pat[2])
				return firstMatch;
			if (*(PWORD)pat == '\?\?' || *(PBYTE)pat != '\?')
				pat += 3;
			else
				pat += 2;
		}
		else
		{
			pat = szPattern.c_str();
			firstMatch = 0;
		}
	}

	return NULL;
}

DWORD FindPattern(const std::string& szModules, const std::string& szPattern)
{
	HMODULE hmModule = GetModuleHandleSafe(szModules);
	PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)hmModule;
	PIMAGE_NT_HEADERS pNTHeaders = (PIMAGE_NT_HEADERS)(((DWORD)hmModule) + pDOSHeader->e_lfanew);

	DWORD dwAddress = ((DWORD)hmModule) + pNTHeaders->OptionalHeader.BaseOfCode;
	DWORD dwLength = ((DWORD)hmModule) + pNTHeaders->OptionalHeader.SizeOfCode;

	const char *pat = szPattern.c_str();
	DWORD firstMatch = NULL;
	for (DWORD pCur = dwAddress; pCur < dwLength; pCur++)
	{
		if (!*pat)
			return firstMatch;
		if (*(PBYTE)pat == '\?' || *(BYTE *)pCur == getByte(pat))
		{
			if (!firstMatch)
				firstMatch = pCur;
			if (!pat[2])
				return firstMatch;
			if (*(PWORD)pat == '\?\?' || *(PBYTE)pat != '\?')
				pat += 3;
			else
				pat += 2;
		}
		else
		{
			pat = szPattern.c_str();
			firstMatch = 0;
		}
	}

	return NULL;
}

DWORD FindPattern(const std::string & szModules, const std::string & szPattern, std::string szMask)
{
	HMODULE hmModule = GetModuleHandleSafe(szModules);
	PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)hmModule;
	PIMAGE_NT_HEADERS pNTHeaders = (PIMAGE_NT_HEADERS)(((DWORD)hmModule) + pDOSHeader->e_lfanew);

	BYTE First = szPattern[0];
	PBYTE BaseAddress = (PBYTE)(((DWORD)hmModule) + pNTHeaders->OptionalHeader.BaseOfCode);
	PBYTE Max = (PBYTE)(BaseAddress + pNTHeaders->OptionalHeader.SizeOfCode - szPattern.length());

	const static auto CompareByteArray = [](PBYTE Data, const char* Signature, const char* Mask) -> bool
	{
		for (; *Signature; ++Signature, ++Data, ++Mask)
		{
			if (*Mask == '?' || *Signature == '\x00' || *Signature == '\x2A')
			{
				continue;
			}
			if (*Data != *Signature)
			{
				return false;
			}
		}
		return true;
	};

	if (szMask.length() < szPattern.length())
	{
		for (auto i = szPattern.begin() + (szPattern.length() - 1); i != szPattern.end(); ++i)
			szMask.push_back(*i == '\x2A' || *i == '\x00' ? '?' : 'x');
	}

	for (; BaseAddress < Max; ++BaseAddress)
	{
		if (*BaseAddress != First)
		{
			continue;
		}
		if (CompareByteArray(BaseAddress, szPattern.c_str(), szMask.c_str()))
		{
			return (DWORD)BaseAddress;
		}
	}

	return NULL;
}

DWORD GetModuleBase(const std::string& ModuleName, DWORD ProcessID)
{
	if (ProcessID == 0)
		ProcessID = GetCurrentProcessId();

	MODULEENTRY32 lpModuleEntry = { 0 };
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, ProcessID);
	if (!hSnapShot)
		return NULL;

	lpModuleEntry.dwSize = sizeof(lpModuleEntry);
	BOOL __RunModule = Module32First(hSnapShot, &lpModuleEntry);
	while (__RunModule)
	{
		if (lpModuleEntry.szModule == ModuleName)
		{
			CloseHandle(hSnapShot);
			return (DWORD)lpModuleEntry.modBaseAddr;
		}
		__RunModule = Module32Next(hSnapShot, &lpModuleEntry);
	}
	CloseHandle(hSnapShot);
	return NULL;
}

DWORD GetModuleBase(const std::string& ModuleName, DWORD* ModuleSize, DWORD ProcessID)
{
	if (ProcessID == 0)
		ProcessID = GetCurrentProcessId();

	MODULEENTRY32 lpModuleEntry = { 0 };
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, ProcessID);
	if (!hSnapShot)
		return NULL;

	lpModuleEntry.dwSize = sizeof(lpModuleEntry);
	BOOL __RunModule = Module32First(hSnapShot, &lpModuleEntry);
	while (__RunModule)
	{
		if (lpModuleEntry.szModule == ModuleName)
		{
			CloseHandle(hSnapShot);
			*ModuleSize = lpModuleEntry.dwSize;
			return (DWORD)lpModuleEntry.modBaseAddr;
		}
		__RunModule = Module32Next(hSnapShot, &lpModuleEntry);
	}
	CloseHandle(hSnapShot);
	return NULL;
}

DWORD FindProccess(const std::string& proccessName)
{
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(pe);

	if (!Process32First(snapshot, &pe))
		return 0;

	while (Process32Next(snapshot, &pe))
	{
		if (pe.szExeFile == proccessName)
			return pe.th32ProcessID;
	}

	return 0;
}
