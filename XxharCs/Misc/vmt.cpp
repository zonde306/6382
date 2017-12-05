#include "vmt.h"

CVMTHookManager::CVMTHookManager()
{
}

CVMTHookManager::CVMTHookManager(PVOID object)
{
	Init(object);
}

CVMTHookManager::~CVMTHookManager()
{
	// Unhook();
}

bool CVMTHookManager::Init(PVOID object)
{
	origin = *(PDWORD*)object;
	count = GetCount();
	if (count <= 0)
		return false;

	copy = (PDWORD)VirtualAlloc(NULL, sizeof(void*) * count, MEM_COMMIT, PAGE_READWRITE);
	if (copy == NULL)
		return false;

	RtlCopyMemory(copy, origin, sizeof(void*) * count);
	this->object = (PDWORD)object;

	return true;
}

int CVMTHookManager::GetCount()
{
	int index = 0;
	DWORD** table = (DWORD**)origin;
	__try
	{
		for (DWORD* fn; (fn = table[index]) != nullptr; ++index)
		{
			if (!CanReadPointer(fn))
				break;
		}
	}
	__except (EXCEPTION_CONTINUE_EXECUTION)
	{
		__asm nop;
	}

	return index;
}

bool CVMTHookManager::Hook()
{
	__try
	{
		*object = (DWORD)copy;
	}
	__except (EXCEPTION_CONTINUE_EXECUTION)
	{
		return false;
	}

	return true;
}

bool CVMTHookManager::Unhook() noexcept
{
	__try
	{
		*object = (DWORD)origin;
	}
	__except (EXCEPTION_CONTINUE_EXECUTION)
	{
		return false;
	}

	return true;
}

bool CVMTHookManager::CanReadPointer(PVOID pointer)
{
	if (pointer == nullptr)
		return false;
	
	MEMORY_BASIC_INFORMATION mbi;
	if (!VirtualQuery(pointer, &mbi, sizeof(mbi)))
		return false;

	if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS))
		return false;

	return (mbi.Protect & (PAGE_READWRITE | PAGE_EXECUTE_READWRITE | PAGE_READONLY | PAGE_WRITECOPY | PAGE_EXECUTE_READ | PAGE_EXECUTE_WRITECOPY));
}

PVOID CVMTHookManager::HookFunction(int index, PVOID func)
{
	if (index < 0 || index >= count)
		return nullptr;

	copy[index] = (DWORD)func;
	return (PVOID)(origin[index]);
}

PVOID CVMTHookManager::UnhookFunction(int index)
{
	if (index < 0 || index >= count)
		return nullptr;

	PVOID func = (PVOID)(copy[index]);
	copy[index] = origin[index];
	return func;
}

PVOID CVMTHookManager::GetOriginalFunction(int index)
{
	if (index < 0 || index >= count)
		return nullptr;

	return (PVOID)(origin[index]);
}

template<typename R, typename ...Arg>
std::function<R(Arg...)> CVMTHookManager::SetupHook(int index, std::function<R(Arg...)> func)
{
	using Fn = R(Arg...);
	
	if (index < 0 || index >= count)
		return std::function<Fn>();

	copy[index] = (DWORD)(func.target<Fn>());
	return std::function<Fn>((Fn)(origin[index]));
}

template<typename R, typename ...Arg>
std::function<R(Arg...)> CVMTHookManager::UninstallHook(int index)
{
	using Fn = R(Arg...);
	
	if (index < 0 || index >= count)
		return nullptr;

	std::function<Fn> func = (Fn*)(copy[index]);
	copy[index] = origin[index];
	return std::move(func);
}

template<typename R, typename ...Arg>
std::function<R(Arg...)> CVMTHookManager::GetOriginFunction(int index)
{
	using Fn = R(Arg...);

	if (index < 0 || index >= count)
		return std::function<Fn>();

	return std::function<Fn>((Fn)(origin[index]));
}

template<typename R, typename ...Arg>
R CVMTHookManager::invoke(int index, Arg ...arg)
{
	using Fn = R(Arg...);

	if (index < 0 || index >= count)
		return std::function<Fn>();

	return std::forward<R>(((Fn)(origin[index]))(std::forward<Arg>(arg)...));
}
