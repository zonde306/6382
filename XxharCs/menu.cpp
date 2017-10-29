#include "menu.h"
#include "./drawing/gui.h"
#include "./Misc/xorstr.h"
#include "./Engine/keydefs.h"
#include "cvars.h"

GLuint g_oglBase;
HDC g_hOglHdc;
bool g_bOglFirstInit = false;
int g_iOglViewportCount = 0;
int g_iOglLastKey = 0;
GLint	g_oglVp[4];
menu_s	g_oglMenu;
draw_s	g_oglDraw;

void Menu::PrintDescription(const std::string & description, ColorEntry * pClr)
{
	PrintWithFont(10, screeninfo.iHeight - 111, pClr->r, pClr->g, pClr->b,
		description.c_str());
}

void Menu::Init()
{
	
}

MenuItem::MenuItem(const std::string & title, float * pointer, const std::string & description,
	bool hasMin, float minValue, bool hasMax, float maxValue, float addSetp, float subSetp,
	bool asInteger) : title(title), pointer(pointer), description(description),
	hasMin(hasMin), minValue(minValue), hasMax(hasMax), maxValue(maxValue), incSetp(addSetp),
	decSetp(subSetp), hasInteger(asInteger)
{

}

MenuStack::MenuStack(const std::string & title, const std::string & description) :
	title(title), description(description)
{

}

/*
MenuOptions *Menu::GetOptionByIndex(unsigned int id)
{
	if (id<VMenu.size())
		return &VMenu[id];
	else
		return &VMenu[0];
}
*/

size_t Menu::iMenuSize()
{
	return menuList.size();
}

void Menu::ClearMenu()
{
	menuList.clear();
}

Menu::Menu()
{
	
}

MenuStack & Menu::AddMenuClass(const std::string & title, const std::string & description)
{
	menuList.emplace_back(title, description);
	return *(menuList.rbegin());
}

/*
void Menu::AddMenu(bool bSubMenu, char *cVarName, float *fValue, float fStep, float fMin, float fMax, char *cVarDescription)
{
	//MenuOptions tmp={bSubMenu,cVarName,fValue,fStep,fMin,fMax,cVarDescription};

	MenuOptions tmp;
	tmp.bSubMenu = bSubMenu;
	//tmp.cVarName=cVarName;
	strcpy(tmp.cVarName, cVarName);//Fix!
	tmp.fValue = fValue;
	tmp.fStep = fStep;
	tmp.fMin = fMin;
	tmp.fMax = fMax;
	//tmp.cVarDescription=cVarDescription;
	strcpy(tmp.cVarDescription, cVarDescription);//Fix!

	VMenu.push_back(tmp);
}
*/

extern int lengthcharenginefont;
void Menu::MenuDraw(int y)
{
	if (!MenuActivated)
		return;
	int i, x, w, h = 0, l = 0, l2 = 0, lmax = 0;
	//w = 300;
	
	/*
	for (i = 0; i<iMenuSize(); i++)
	{
		h += 11;
		l = g_tableFont.iGetWidth(GetOptionByIndex(i)->cVarName);
		lmax = (lmax<l) ? l : lmax;
	}
	*/

	int index = -1;
	std::string description;
	for (const MenuStack& key : menuList)
	{
		// 遍历索引
		++index;

		h += 11;
		l = key.title.length();
		lmax = (lmax<l) ? l : lmax;

		if (index == cursor)
			description = key.description;

		if (index == opening)
		{
			for (const MenuItem& value : key.item)
			{
				++index;
				
				h += 11;
				l = value.title.length();
				lmax = (lmax<l) ? l : lmax;

				if (index == cursor)
					description = value.description;
			}
		}
	}
	
	l = g_tableFont.iGetWidth("459.45");
	lmax += 22 + l + 14;

	PrintDescription(description, colorList.get(8));

	if (!y)
		y = (screeninfo.iHeight / 2) - (h / 2);
	x = screeninfo.iWidth - 70 - lmax;
	w = lmax;
	g_gui.window(x, y, w, h, 0.5, XorStr("===== Main Menu ====="));

	/*
	for (i = 0; i<iMenuSize(); i++)
	{
		ColorEntry *pClr;
		if (i == iSelected)
			pClr = colorList.get(2);
		else
			pClr = colorList.get(8);
		if (i == iSelected)
		{
			g_gui.DrawTitleBox2(x, y + (11 * i) - 1, w - 2, 12);
			//	DrawSmoothBox(x, y + (14*i)-1,w-2,14,0,255,255,1,1,1,0.5);
			//		gEngfuncs.pfnFillRGBA(x, y + (14*i)-1,w-2,14,0,255,0,255);
			//		colorBorder(x, y + (14*i)-1,w-2,14,0,0,0,255);
		}
		
		PrintWithFont(x, y + i * 11, pClr->r, pClr->g, pClr->b, GetOptionByIndex(i)->cVarName);
		if (!GetOptionByIndex(i)->bSubMenu && !AimAdjMenu && !BurstAdj)
			PrintWithFont(x + w - l - 7, y + i * 11, pClr->r, pClr->g, pClr->b, "%2.2f", *GetOptionByIndex(i)->fValue);
	}
	*/

	if (!menuList.empty())
	{
		index = -1;
		for (const MenuStack& key : menuList)
		{
			// 遍历索引
			++index;
			
			// 菜单类别
			if (index == cursor)
			{
				// 当前选中的是菜单类别
				PrintWithFont(x, y + i * 11, 255, 128, 0, key.title.c_str());
			}
			else if(index == opening)
			{
				// 未选中菜单类别，但是当前类别已经打开了
				PrintWithFont(x, y + i * 11, 0, 255, 128, key.title.c_str());
			}
			else
			{
				// 未选中菜单类别
				PrintWithFont(x, y + i * 11, 255, 255, 255, key.title.c_str());
			}

			// 已经打开的菜单类别
			if (index == opening && !key.item.empty())
			{
				// 类别的第一项
				openBegin = index + 1;
				
				// 菜单项目
				for (const MenuItem& value : key.item)
				{
					++index;

					// 当前选中的菜单项目
					if (index == cursor)
					{
						// 已经选中的菜单项目使用蓝色
						if (value.hasInteger)
						{
							PrintWithFont(x, y + i * 11, 0, 128, 255, " %s %.0f",
								value.title.c_str(), *(value.pointer));
						}
						else
						{
							PrintWithFont(x, y + i * 11, 0, 128, 255, " %s %.f",
								value.title.c_str(), *(value.pointer));
						}
					}
					else
					{
						// 未选中的菜单项目使用白色
						if (value.hasInteger)
						{
							PrintWithFont(x, y + i * 11, 255, 255, 255, " %s %.0f",
								value.title.c_str(), *(value.pointer));
						}
						else
						{
							PrintWithFont(x, y + i * 11, 255, 255, 255, " %s %.f",
								value.title.c_str(), *(value.pointer));
						}
					}
				}

				// 类别的最后一项
				openEnd = index;
			}
		}

		if (index > -1)
		{
			// 一共绘制了多少个项目(类别和菜单项)
			menuDrawing = index + 1;
		}
	}
}

int Menu::MenuKey(int keynum)
{
	if (!MenuActivated || menuList.empty())
		return 1;

	if (keynum == K_UPARROW)
	{
		--cursor;
		if (cursor < 0)
			cursor = menuDrawing - 1;
		
		return 0;
	}
	else if (keynum == K_DOWNARROW)
	{
		++cursor;
		if (cursor >= menuDrawing)
			cursor = 0;

		return 0;
	}
	else if (keynum == K_LEFTARROW)
	{
		if (opening > -1)
		{
			if (cursor >= openBegin && cursor <= openEnd)
			{
				// 菜单项目操作(增加)
				MenuItem& item = menuList[opening].item[cursor - openBegin];
				*item.pointer += item.incSetp;
				if (item.hasMax && *item.pointer > item.maxValue)
					*item.pointer = item.maxValue;
			}
			else
			{
				// 菜单类别操作(打开)
				if (cursor > openEnd)
				{
					cursor -= openEnd - openBegin;
					opening = cursor;
				}
				else
				{
					opening = cursor;
				}
			}
		}
		else
		{
			// 菜单类别操作(打开)
			opening = cursor;
		}
		
		/*
		if (GetOptionByIndex(iSelected)->fValue)
		{
			if ((*GetOptionByIndex(iSelected)->fValue - GetOptionByIndex(iSelected)->fStep) < GetOptionByIndex(iSelected)->fMin)
			{
				if (GetOptionByIndex(iSelected)->bSubMenu)
				{
					MenuOptions *tmp = GetOptionByIndex(iSelected);
					float *pfValue = tmp->fValue;
					float fValue = *tmp->fValue;
					ClearMenu();
					*tmp->fValue = fValue;
					*tmp->fValue = tmp->fMax;
					Init();
					iSelected = CalculateRelativePosition(pfValue);
				}
				else
					*GetOptionByIndex(iSelected)->fValue = GetOptionByIndex(iSelected)->fMax;
			}
			else
			{
				if (GetOptionByIndex(iSelected)->bSubMenu)
				{
					MenuOptions *tmp = GetOptionByIndex(iSelected);
					float *pfValue = tmp->fValue;
					float fValue = *tmp->fValue;
					ClearMenu();
					*tmp->fValue = fValue;
					*tmp->fValue -= tmp->fStep;
					Init();
					iSelected = CalculateRelativePosition(pfValue);
				}
				else
					*GetOptionByIndex(iSelected)->fValue -= GetOptionByIndex(iSelected)->fStep;
			}
		}
		*/
		return 0;
	}
	else if (keynum == K_RIGHTARROW) //rightarrow || rightbutton
	{
		if (opening > -1)
		{
			if (cursor >= openBegin && cursor <= openEnd)
			{
				// 菜单项目操作(减少)
				MenuItem& item = menuList[opening].item[cursor - openBegin];
				*item.pointer -= item.decSetp;
				if (item.hasMin && (*item.pointer) < item.minValue)
					*item.pointer = item.maxValue;
			}
			else
			{
				// 菜单类别操作(关闭)
				if (cursor > openEnd)
				{
					cursor -= openEnd - openBegin;
					opening = -1;
				}
				else
				{
					opening = -1;
				}
			}
		}
		else
		{
			// 菜单类别操作(关闭)
			opening = -1;
		}
		
		/*
		if (GetOptionByIndex(iSelected)->fValue)
		{
			if ((*GetOptionByIndex(iSelected)->fValue + GetOptionByIndex(iSelected)->fStep) > GetOptionByIndex(iSelected)->fMax)
			{
				if (GetOptionByIndex(iSelected)->bSubMenu)
				{
					MenuOptions *tmp = GetOptionByIndex(iSelected);
					float *pfValue = tmp->fValue;
					float fValue = *tmp->fValue;
					ClearMenu();
					*tmp->fValue = fValue;
					*tmp->fValue = tmp->fMin;
					//*GetOptionByIndex(iSelected)->fValue = GetOptionByIndex(iSelected)->fMin;
					Init();
					iSelected = CalculateRelativePosition(pfValue);
				}
				else
					*GetOptionByIndex(iSelected)->fValue = GetOptionByIndex(iSelected)->fMin;
			}
			else
			{
				if (GetOptionByIndex(iSelected)->bSubMenu)
				{
					MenuOptions *tmp = GetOptionByIndex(iSelected);
					float *pfValue = tmp->fValue;
					float fValue = *tmp->fValue;
					ClearMenu();
					*tmp->fValue = fValue;
					*tmp->fValue = tmp->fStep;
					//*GetOptionByIndex(iSelected)->fValue += GetOptionByIndex(iSelected)->fStep;
					Init();
					iSelected = CalculateRelativePosition(pfValue);
				}
				else
					*GetOptionByIndex(iSelected)->fValue += GetOptionByIndex(iSelected)->fStep;
			}
		}
		*/
		return 0;
	}
	return 1;
}

