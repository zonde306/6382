#pragma once
#include "Engine/interface.h"

#define BASEUI_INTERFACE_VERSION		"BaseUI001"

class IBaseUI : public IBaseInterface
{
public:
	virtual void Initialize(CreateInterfaceFn *factories, int count) = 0;
	virtual void Start(struct cl_enginefuncs_s *engineFuncs, int interfaceVersion) = 0;
	virtual void Shutdown(void) = 0;
	virtual int Key_Event(int down, int keynum, const char *pszCurrentBinding) = 0;
	virtual void CallEngineSurfaceProc(void *hwnd, unsigned int msg, unsigned int wparam, long lparam) = 0;
	virtual void Paint(int x, int y, int right, int bottom) = 0;
	virtual void HideGameUI(void) = 0;
	virtual void ActivateGameUI(void) = 0;
	virtual bool IsGameUIVisible(void) = 0;
	virtual void HideConsole(void) = 0;
	virtual void ShowConsole(void) = 0;
};
