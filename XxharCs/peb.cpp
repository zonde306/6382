#include <windows.h>
#include "peb.h"

/********************************************************************************************\
  TEB Structs
\********************************************************************************************/

typedef LONG NTSTATUS;
typedef NTSTATUS
(NTAPI *PPOST_PROCESS_INIT_ROUTINE)(
    VOID
);

typedef struct _LSA_UNICODE_STRING {
  USHORT Length;
  USHORT MaximumLength;
  PWSTR  Buffer;
} LSA_UNICODE_STRING, *PLSA_UNICODE_STRING, UNICODE_STRING, *PUNICODE_STRING;

typedef struct _GDI_TEB_BATCH
{
    ULONG Offset;
    HANDLE HDC;
    ULONG Buffer[0x136];
} GDI_TEB_BATCH, *PGDI_TEB_BATCH;

typedef struct _TEB_ACTIVE_FRAME_CONTEXT
{
    ULONG Flags;
    LPSTR FrameName;
} TEB_ACTIVE_FRAME_CONTEXT, *PTEB_ACTIVE_FRAME_CONTEXT;

typedef struct _TEB_ACTIVE_FRAME
{
    ULONG Flags;
    struct _TEB_ACTIVE_FRAME *Previous;
    PTEB_ACTIVE_FRAME_CONTEXT Context;
} TEB_ACTIVE_FRAME, *PTEB_ACTIVE_FRAME;

typedef struct _W32PROCESSINFO
{
    PVOID UserHandleTable;
    HANDLE hUserHeap;
    PVOID UserHeapDelta;
    PVOID hModUser;
    PVOID LocalClassList;
    PVOID GlobalClassList;
    PVOID SystemClassList;
    PVOID psi;
} W32PROCESSINFO, *PW32PROCESSINFO;

typedef struct _W32THREADINFO
{
    W32PROCESSINFO pi; /* [USER] */
    W32PROCESSINFO kpi; /* [KERNEL] */
    PVOID pDeskInfo;
    ULONG Hooks;
    PVOID ClientThreadInfo;
} W32THREADINFO, *PW32THREADINFO;

typedef struct _CLIENT_ID {
  HANDLE  UniqueProcess;
  HANDLE  UniqueThread;
} CLIENT_ID, *PCLIENT_ID;

typedef struct _PEB_FREE_BLOCK
{
    struct _PEB_FREE_BLOCK* Next;
    ULONG Size;
} PEB_FREE_BLOCK, *PPEB_FREE_BLOCK;

typedef struct _PEB_LDR_DATA
{
     ULONG Length;
     UCHAR Initialized;
     PVOID SsHandle;
     LIST_ENTRY InLoadOrderModuleList;
     LIST_ENTRY InMemoryOrderModuleList;
     LIST_ENTRY InInitializationOrderModuleList;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef struct _PEB
{
    UCHAR InheritedAddressSpace;
    UCHAR ReadImageFileExecOptions;
    UCHAR BeingDebugged;
#if (NTDDI_VERSION >= NTDDI_WS03)
    struct
    {
        UCHAR ImageUsesLargePages:1;
    #if (NTDDI_VERSION >= NTDDI_LONGHORN)
        UCHAR IsProtectedProcess:1;
        UCHAR IsLegacyProcess:1;
        UCHAR SpareBits:5;
    #else
        UCHAR SpareBits:7;
    #endif
    };
#else
    BOOLEAN SpareBool;
#endif
    HANDLE Mutant;
    PVOID ImageBaseAddress;
    PPEB_LDR_DATA Ldr;
    struct _RTL_USER_PROCESS_PARAMETERS *ProcessParameters;
    PVOID SubSystemData;
    PVOID ProcessHeap;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    struct _RTL_CRITICAL_SECTION *FastPebLock;
    PVOID AltThunkSListPtr;
    PVOID IFEOKey;
    ULONG Spare;
    union
    {
        PVOID* KernelCallbackTable;
        PVOID UserSharedInfoPtr;
    }UNNAMED;
    ULONG SystemReserved[1];
    ULONG SpareUlong;
#else
    PVOID FastPebLock;
    PPEBLOCKROUTINE FastPebLockRoutine;
    PPEBLOCKROUTINE FastPebUnlockRoutine;
    ULONG EnvironmentUpdateCount;
    PVOID* KernelCallbackTable;
    PVOID EventLogSection;
    PVOID EventLog;
#endif
    PPEB_FREE_BLOCK FreeList;
    ULONG TlsExpansionCounter;
    PVOID TlsBitmap;
    ULONG TlsBitmapBits[0x2];
    PVOID ReadOnlySharedMemoryBase;
    PVOID ReadOnlySharedMemoryHeap;
    PVOID* ReadOnlyStaticServerData;
    PVOID AnsiCodePageData;
    PVOID OemCodePageData;
    PVOID UnicodeCaseTableData;
    ULONG NumberOfProcessors;
    ULONG NtGlobalFlag;
    LARGE_INTEGER CriticalSectionTimeout;
    ULONG HeapSegmentReserve;
    ULONG HeapSegmentCommit;
    ULONG HeapDeCommitTotalFreeThreshold;
    ULONG HeapDeCommitFreeBlockThreshold;
    ULONG NumberOfHeaps;
    ULONG MaximumNumberOfHeaps;
    PVOID* ProcessHeaps;
    PVOID GdiSharedHandleTable;
    PVOID ProcessStarterHelper;
    ULONG GdiDCAttributeList;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    struct _RTL_CRITICAL_SECTION *LoaderLock;
#else
    PVOID LoaderLock;
#endif
    ULONG OSMajorVersion;
    ULONG OSMinorVersion;
    USHORT OSBuildNumber;
    USHORT OSCSDVersion;
    ULONG OSPlatformId;
    ULONG ImageSubSystem;
    ULONG ImageSubSystemMajorVersion;
    ULONG ImageSubSystemMinorVersion;
    ULONG ImageProcessAffinityMask;
    ULONG GdiHandleBuffer[0x22];
    PPOST_PROCESS_INIT_ROUTINE PostProcessInitRoutine;
    struct _RTL_BITMAP *TlsExpansionBitmap;
    ULONG TlsExpansionBitmapBits[0x20];
    ULONG SessionId;
#if (NTDDI_VERSION >= NTDDI_WINXP)
    ULARGE_INTEGER AppCompatFlags;
    ULARGE_INTEGER AppCompatFlagsUser;
    PVOID pShimData;
    PVOID AppCompatInfo;
    UNICODE_STRING CSDVersion;
    struct _ACTIVATION_CONTEXT_DATA *ActivationContextData;
    struct _ASSEMBLY_STORAGE_MAP *ProcessAssemblyStorageMap;
    struct _ACTIVATION_CONTEXT_DATA *SystemDefaultActivationContextData;
    struct _ASSEMBLY_STORAGE_MAP *SystemAssemblyStorageMap;
    ULONG MinimumStackCommit;
#endif
#if (NTDDI_VERSION >= NTDDI_WS03)
    PVOID *FlsCallback;
    LIST_ENTRY FlsListHead;
    struct _RTL_BITMAP *FlsBitmap;
    ULONG FlsBitmapBits[4];
    ULONG FlsHighIndex;
#endif
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    PVOID WerRegistrationData;
    PVOID WerShipAssertPtr;
#endif
} PEB, *PPEB;

typedef struct _TEB
{
    NT_TIB Tib;                           //0x000
    PVOID EnvironmentPointer;             //0x01c
    CLIENT_ID ClientId;                   //0x020
    PVOID ActiveRpcHandle;                //0x028
    PVOID ThreadLocalStoragePointer;      //0x02C
    struct _PEB *ProcessEnvironmentBlock; //0x030
    ULONG LastErrorValue;                 //0x034
    ULONG CountOfOwnedCriticalSections;   //0x038
    PVOID CsrClientThread;                //0x03C
    PW32THREADINFO Win32ThreadInfo;   //0x040
    ULONG User32Reserved[0x1A];           //0x044
    ULONG UserReserved[5];
    PVOID WOW32Reserved;
    LCID CurrentLocale;
    ULONG FpSoftwareStatusRegister;
    PVOID SystemReserved1[0x36];
    LONG ExceptionCode;
    struct _ACTIVATION_CONTEXT_STACK *ActivationContextStackPointer;
#ifdef _WIN64
    UCHAR SpareBytes1[24];
#else
    UCHAR SpareBytes1[0x24];
#endif
    ULONG TxFsContext;
    GDI_TEB_BATCH GdiTebBatch;
    CLIENT_ID RealClientId;
    PVOID GdiCachedProcessHandle;
    ULONG GdiClientPID;
    ULONG GdiClientTID;
    PVOID GdiThreadLocalInfo;
    SIZE_T Win32ClientInfo[62];
    PVOID glDispatchTable[0xE9];
    SIZE_T glReserved1[0x1D];
    PVOID glReserved2;
    PVOID glSectionInfo;
    PVOID glSection;
    PVOID glTable;
    PVOID glCurrentRC;
    PVOID glContext;
    NTSTATUS LastStatusValue;
    UNICODE_STRING StaticUnicodeString;
    WCHAR StaticUnicodeBuffer[0x105];
    PVOID DeallocationStack;
    PVOID TlsSlots[0x40];
    LIST_ENTRY TlsLinks;
    PVOID Vdm;
    PVOID ReservedForNtRpc;
    PVOID DbgSsReserved[0x2];
    ULONG HardErrorDisabled;
#ifdef _WIN64
    PVOID Instrumentation[11];
#else
    PVOID Instrumentation[9];
#endif
    GUID ActivityId;
    PVOID SubProcessTag;
    PVOID EtwTraceData;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    PVOID EtwLocalData;
#endif
    PVOID WinSockData;
    ULONG GdiBatchCount;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    BOOLEAN SpareBool0;
    BOOLEAN SpareBool1;
    BOOLEAN SpareBool2;
#else
    BOOLEAN InDbgPrint;
    BOOLEAN FreeStackOnTermination;
    BOOLEAN HasFiberData;
#endif
    UCHAR IdealProcessor;
    ULONG GuaranteedStackBytes;
    PVOID ReservedForPerf;
    PVOID ReservedForOle;
    ULONG WaitingOnLoaderLock;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    PVOID SavedPriorityState;
#else
    ULONG SparePointer1;
#endif
    ULONG SoftPatchPtr1;
    ULONG SoftPatchPtr2;
    PVOID *TlsExpansionSlots;
    ULONG ImpersonationLocale;
    ULONG IsImpersonating;
    PVOID NlsCache;
    PVOID pShimData;
    ULONG HeapVirualAffinity;
    PVOID CurrentTransactionHandle;
    PTEB_ACTIVE_FRAME ActiveFrame;
#if (NTDDI_VERSION >= NTDDI_WS03)
    PVOID FlsData;
#endif
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    PVOID PreferredLangauges;
    PVOID UserPrefLanguages;
    PVOID MergedPrefLanguages;
    ULONG MuiImpersonation;
    union
    {
        struct
        {
            USHORT SpareCrossTebFlags:16;
        };
        USHORT CrossTebFlags;
    };
    union
    {
        struct
        {
            USHORT DbgSafeThunkCall:1;
            USHORT DbgInDebugPrint:1;
            USHORT DbgHasFiberData:1;
            USHORT DbgSkipThreadAttach:1;
            USHORT DbgWerInShipAssertCode:1;
            USHORT DbgIssuedInitialBp:1;
            USHORT DbgClonedThread:1;
            USHORT SpareSameTebBits:9;
        };
        USHORT SameTebFlags;
    };
    PVOID TxnScopeEntercallback;
    PVOID TxnScopeExitCAllback;
    PVOID TxnScopeContext;
    ULONG LockCount;
    ULONG ProcessRundown;
    ULONGLONG LastSwitchTime;
    ULONGLONG TotalSwitchOutTime;
    LARGE_INTEGER WaitReasonBitMap;
#else
    UCHAR SafeThunkCall;
    UCHAR BooleanSpare[3];
#endif
} TEB, *PTEB;

typedef struct _LDR_DATA_TABLE_ENTRY{
     LIST_ENTRY InLoadOrderLinks;
     LIST_ENTRY InMemoryOrderLinks;
     LIST_ENTRY InInitializationOrderLinks;
     PVOID DllBase;
     PVOID EntryPoint;
     ULONG SizeOfImage;
     UNICODE_STRING FullDllName;
     UNICODE_STRING BaseDllName;
     ULONG Flags;
     WORD LoadCount;
     WORD TlsIndex;
     union{
          LIST_ENTRY HashLinks;
          struct{
               PVOID SectionPointer;
               ULONG CheckSum;
          };
     }U1;
     union{
          ULONG TimeDateStamp;
          PVOID LoadedImports;
     }U2;
     struct _ACTIVATION_CONTEXT * EntryPointActivationContext;
     PVOID PatchInformation;
     LIST_ENTRY ForwarderLinks;
     LIST_ENTRY ServiceTagLinks;
     LIST_ENTRY StaticLinks;
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;



BOOL RemoveEntryList(PLIST_ENTRY Entry)
{
     PLIST_ENTRY Blink,Flink;
     Flink=Entry->Flink;Blink=Entry->Blink;
     Blink->Flink=Flink;Flink->Blink=Blink;
     return (BOOL)(Flink==Blink);
}
/*
#define UNLINK(Entry)\
     Flink=Entry->Flink;Blink=Entry->Blink;\
     Blink->Flink=Flink;Flink->Blink=Blink;
*/

/*PTEB getTib()
{
    PTEB pTib;
    __asm {
        mov EAX, FS:[0x18]
        mov [pTib], EAX
    }
    return pTib;
}*/

BOOL ClearPEHeader(HINSTANCE hModule)
{
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hModule;
	if( pDosHeader->e_magic == IMAGE_DOS_SIGNATURE )
	{
		PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)( (DWORD)pDosHeader + (DWORD)pDosHeader->e_lfanew );
		if( pNtHeader->Signature == IMAGE_NT_SIGNATURE )
		{
			DWORD dwOld, dwOld2, dwSize = pNtHeader->OptionalHeader.SizeOfHeaders;
			if( VirtualProtect( (LPVOID)pDosHeader, dwSize, PAGE_EXECUTE_READWRITE, &dwOld ) )
			{
				ZeroMemory(pDosHeader,dwSize);
				VirtualProtect((LPVOID)pDosHeader,dwSize,dwOld,&dwOld2);
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL WINAPI HideDll(HINSTANCE hInstance)
{
     PTEB teb=NtCurrentTeb();
     if(teb)
     {
          PLDR_DATA_TABLE_ENTRY pDTEntry;
          PPEB_LDR_DATA ldrData=teb->ProcessEnvironmentBlock->Ldr;
          pDTEntry=(PLDR_DATA_TABLE_ENTRY)ldrData->InLoadOrderModuleList.Flink;
          while(pDTEntry->DllBase)
          {
               if(pDTEntry->DllBase==hInstance)
               {
                    if(pDTEntry->InInitializationOrderLinks.Blink==NULL)
                         return FALSE;
                    //PLIST_ENTRY Blink,Flink;
                    if(RemoveEntryList(&pDTEntry->InLoadOrderLinks))
						if(RemoveEntryList(&pDTEntry->InMemoryOrderLinks))
							if(RemoveEntryList(&pDTEntry->InInitializationOrderLinks))
								if(RemoveEntryList(&pDTEntry->U1.HashLinks))
									if(ClearPEHeader(hInstance))
									{
										ZeroMemory(pDTEntry->BaseDllName.Buffer,pDTEntry->BaseDllName.Length);
										ZeroMemory(pDTEntry,sizeof(LDR_DATA_TABLE_ENTRY));
										return TRUE;
									}
               }
               pDTEntry=(PLDR_DATA_TABLE_ENTRY)pDTEntry->InLoadOrderLinks.Flink;
          }
     }
     return FALSE;
}
