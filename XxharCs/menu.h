#ifndef __MENU_H__
#define __MENU_H__

#include <vector>
#include <string>
#include <functional>
#include "./drawing/drawing.h"
#include "./drawing/tablefont.h"

typedef struct {
   int count;
   int maxcount;
   bool active;
}menu_s;

typedef struct {
   bool menu;
   bool chair;
   bool esp;
   bool info;
   bool enable;
}draw_s;

extern GLuint g_oglBase;
extern HDC g_hOglHdc;
extern bool g_bOglFirstInit;
extern int g_iOglViewportCount;
extern int g_iOglLastKey;
extern GLint	g_oglVp[4];
extern menu_s	g_oglMenu;
extern draw_s	g_oglDraw;

struct MenuItem
{
public:
	MenuItem(const std::string& title, float* pointer, const std::string& description = "",
		bool hasMin = false, float minValue = 0.0f, bool hasMax = false, float maxValue = 0.0f,
		float addSetp = 1.0f, float subSetp = 1.0f, bool asInteger = false);

	std::string title;
	std::string description;

	float minValue, maxValue, incSetp, decSetp;
	bool hasMin, hasMax, hasInteger;
	float* pointer;

	typedef std::function<void(const MenuItem* item, float oldValue, float newValue)> callback_t;
	callback_t OnValueChanged;
};

struct MenuStack
{
public:
	MenuStack(const std::string& title, const std::string& description);

	std::string title;
	std::string description;
	std::vector<MenuItem> item;
};

class Menu
{
public:
	void Init();
	void PrintDescription(const std::string& description, ColorEntry *pClr);

	// void AddMenu(bool bSubMenu, char *cVarName, float *fValue, float fStep, float fMin, float fMax, char *cVarDescription);

	void ClearMenu();
	MenuStack& AddMenuClass(const std::string& title, const std::string& description);

	/*
	int CalculateRelativePosition(float *fValue)
	{
		int i;
		for (i = 0; i<iMenuSize(); i++)
			if (VMenu[i].fValue == fValue)
				return i;
		return iSelected;
	}

	MenuOptions *GetOptionByIndex(unsigned int id);
	*/

	size_t iMenuSize();

	void MenuDraw(int y);
	int MenuKey(int keynum);

	bool MenuActivated;

	Menu();

private:
	std::vector<MenuStack> menuList;
	
	// 菜单光标(选中的项目)位置
	int cursor = 0;

	// 菜单当前打开的类别
	int opening = -1;

	// 当前菜单一共绘制了多少个项目
	int menuDrawing = 0;

	// 标记开启的菜单类别的区间
	int openBegin = 0, openEnd = 0;
};

extern Menu g_menu;

#endif