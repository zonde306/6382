#pragma once
#include <Windows.h>
#include <functional>

class CVMTHookManager
{
public:
	CVMTHookManager();
	CVMTHookManager(PVOID object);
	~CVMTHookManager();

	// 初始化
	bool Init(PVOID object);

	// 获取虚表大小
	int GetCount();

	// 启动虚表 Hook
	bool Hook();

	// 卸载虚表 Hook
	bool Unhook() noexcept;

	// 检查指针是否有效
	bool CanReadPointer(PVOID pointer);

	// 添加虚表 Hook
	PVOID HookFunction(int index, PVOID func);

	// 卸载已经添加的虚表 Hook
	PVOID UnhookFunction(int index);

	// 获取原函数
	PVOID GetOriginalFunction(int index);

	// 添加虚表 Hook
	template<typename R, typename ...Arg>
	std::function<R(Arg...)> SetupHook(int index, std::function<R(Arg...)> func);

	template<typename R, typename ...Arg>
	std::function<R(Arg...)> UninstallHook(int index);

	// 获取原函数
	template<typename R, typename ...Arg>
	std::function<R(Arg...)> GetOriginFunction(int index);

	// 直接调用原函数
	template<typename R, typename ...Arg>
	R invoke(int index, Arg ...arg);

private:
	int count;
	PDWORD origin;
	PDWORD copy;
	PDWORD object;
};
