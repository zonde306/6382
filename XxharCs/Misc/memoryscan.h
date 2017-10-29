#pragma once

#include <Windows.h>
#include <string>
#include <chrono>
#include <thread>

BOOL __comparemem(const UCHAR *buff1, const UCHAR *buff2, UINT size);
ULONG __findmemoryclone(const ULONG start, const ULONG end, const ULONG clone, UINT size);
ULONG __findreference(const ULONG start, const ULONG end, const ULONG address);

#define CompareMemory(Buff1, Buff2, Size) __comparemem((const UCHAR *)Buff1, (const UCHAR *)Buff2, (UINT)Size)
#define FindMemoryClone(Start, End, Clone, Size) __findmemoryclone((const ULONG)Start, (const ULONG)End, (const ULONG)Clone, (UINT)Size)
#define FindReference(Start, End, Address) __findreference((const ULONG)Start, (const ULONG)End, (const ULONG)Address)

HMODULE GetModuleHandleSafe(const std::string& pszModuleName);
DWORD FindPattern(const std::string& szPattern);
DWORD FindPattern(DWORD dwAddress, DWORD dwFinalAddress, const std::string& szPattern);
DWORD FindPattern(const std::string& szModules, const std::string& szPattern);
DWORD FindPattern(const std::string & szModules, const std::string & szPattern, std::string szMask);
DWORD GetModuleBase(const std::string& ModuleName, DWORD ProcessID);
DWORD GetModuleBase(const std::string& ModuleName, DWORD* ModuleSize, DWORD ProcessID);
DWORD FindProccess(const std::string& proccessName);
