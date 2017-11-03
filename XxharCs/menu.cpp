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
Menu g_menu;

extern cl_enginefuncs_s gEngfuncs;

void Menu::PrintDescription(const std::string & description, ColorEntry * pClr)
{
	/*
	PrintWithFont(10, screeninfo.iHeight - 111, pClr->r, pClr->g, pClr->b,
		description.c_str());
	*/

	gEngfuncs.pfnDrawSetTextColor(pClr->r, pClr->g, pClr->b);
	gEngfuncs.pfnDrawConsoleString(10, screeninfo.iHeight - 111, (char*)description.c_str());
}

void Menu::Init()
{
	{
		MenuStack& menu = AddMenuClass(XorStr("::Aimbot"));
		menu.item.emplace_back(XorStr("Auto Aim"), &Config::glAimbot, XorStr("自动瞄准"),
			true, 0.0f, true, 1.0f, 1.0f, 1.0f, true);
		menu.item.emplace_back(XorStr("Aim Through"), &Config::glAimThrough, "",
			true, 0.0f, true, 1.0f, 1.0f, 1.0f, true);
		menu.item.emplace_back(XorStr("No Recoil"), &Config::noRecoil, XorStr("无后坐力"),
			true, 0.0f, true, 1.0f, 1.0f, 1.0f, true);
		menu.item.emplace_back(XorStr("No Spread"), &Config::noSpread, XorStr("子弹不会扩散"),
			true, 0.0f, true, 1.0f, 1.0f, 1.0f, true);
		menu.item.emplace_back(XorStr("Rapid Fire"), &Config::rapidFire, XorStr("手枪连射"),
			true, 0.0f, true, 1.0f, 1.0f, 1.0f, true);
		menu.item.emplace_back(XorStr("Trigger Bot"), &Config::triggerBot, XorStr("自动开枪"),
			true, 0.0f, true, 2.0f, 1.0f, 1.0f, true);
		menu.item.emplace_back(XorStr("Trigger Bot Spread"), &Config::trigger_prospread, XorStr("自动开枪预测后坐力"),
			true, 0.0f, true, 1.0f, 1.0f, 1.0f, true);
		menu.item.emplace_back(XorStr("Trigger Bot Draw"), &Config::trigger_draw, XorStr("自动开枪显示位置"),
			true, 0.0f, true, 1.0f, 1.0f, 1.0f, true);
		menu.item.emplace_back(XorStr("Knife Bot"), &Config::knifeBot, XorStr("自动刀砍人"),
			true, 0.0f, true, 1.0f, 1.0f, 1.0f, true);
	}

	{
		MenuStack& menu = AddMenuClass(XorStr("::HvH"));
		menu.item.emplace_back(XorStr("Anti Aim"), &Config::antiAim, XorStr("让别人更能瞄准你"),
			true, 0.0f, true, 2.0f, 1.0f, 1.0f, true);
		menu.item.emplace_back(XorStr("Anti Aim X"), &Config::antiaim_x, "",
			true, 1.0f, true, 6.0f, 1.0f, 1.0f, true);
		menu.item.emplace_back(XorStr("Anti Aim Y"), &Config::antiaim_y, "",
			true, 1.0f, true, 9.0f, 1.0f, 1.0f, true);
	}

	{
		MenuStack& menu = AddMenuClass(XorStr("::OpenGL"));
		menu.item.emplace_back(XorStr("No Smoker"), &Config::glNoSmoker, XorStr("防烟雾弹"),
			true, 0.0f, true, 1.0f, 1.0f, 1.0f, true);
		menu.item.emplace_back(XorStr("No Flashing"), &Config::glNoFlash, XorStr("防闪光弹"),
			true, 0.0f, true, 1.0f, 1.0f, 1.0f, true);
		menu.item.emplace_back(XorStr("Wall Hack"), &Config::glWallhack, XorStr("敌人模型透视"),
			true, 0.0f, true, 1.0f, 1.0f, 1.0f, true);
		menu.item.emplace_back(XorStr("No Sky"), &Config::glNoSky, XorStr("天空变黑"),
			true, 0.0f, true, 1.0f, 1.0f, 1.0f, true);
		menu.item.emplace_back(XorStr("Lambert"), &Config::glLambert, XorStr("手臂和枪高亮"),
			true, 0.0f, true, 1.0f, 1.0f, 1.0f, true);
		menu.item.emplace_back(XorStr("Transparent"), &Config::glTransparent, XorStr("墙壁变成透明"),
			true, 0.0f, true, 1.0f, 1.0f, 1.0f, true);
		menu.item.emplace_back(XorStr("White Walls"), &Config::glWhiteWalls, XorStr("墙壁变成白色"),
			true, 0.0f, true, 1.0f, 1.0f, 1.0f, true);
		menu.item.emplace_back(XorStr("Wireframe"), &Config::glWireframe, XorStr("墙壁变成线框"),
			true, 0.0f, true, 1.0f, 1.0f, 1.0f, true);
		menu.item.emplace_back(XorStr("Wireframe Models"), &Config::glWireframeModels, XorStr("模型变成线框"),
			true, 0.0f, true, 1.0f, 1.0f, 1.0f, true);
		menu.item.emplace_back(XorStr("HUD Color"), &Config::glHudColored, XorStr("修改 HUD 颜色"),
			true, 0.0f, true, 1.0f, 1.0f, 1.0f, true);
		menu.item.emplace_back(XorStr("Header Box"), &Config::glHeadBox, XorStr("在玩家头部显示一个框"),
			true, 0.0f, true, 1.0f, 1.0f, 1.0f, true);
	}

	{
		MenuStack& menu = AddMenuClass(XorStr("::KZ"));
		menu.item.emplace_back(XorStr("Auto Bhop"), &Config::bunnyHop, XorStr("自动连跳"),
			true, 0.0f, true, 1.0f, 1.0f, 1.0f, true);
		menu.item.emplace_back(XorStr("Auto Strafe"), &Config::autoStrafe, XorStr("连跳自动同步(加速)"),
			true, 0.0f, true, 1.0f, 1.0f, 1.0f, true);
		menu.item.emplace_back(XorStr("Speed Hack"), &Config::speedHack, XorStr("按住 E 加速"),
			true, 0.0f, true, 32.0f, 2.0f, 2.0f, true);
		menu.item.emplace_back(XorStr("Fast Run"), &Config::fastRun, XorStr("快速跑步"),
			true, 0.0f, true, 1.0f, 1.0f, 1.0f, true);
		menu.item.emplace_back(XorStr("Fast Walk"), &Config::fastWalk, XorStr("快速静音走路"),
			true, 0.0f, true, 1.0f, 1.0f, 1.0f, true);
	}

	{
		MenuStack& menu = AddMenuClass(XorStr("::Radar"));
		menu.item.emplace_back(XorStr("Radar"), &Config::radar, XorStr("雷达(全景需要地图支持)"),
			true, 0.0f, true, 1.0f, 1.0f, 1.0f, true);
		menu.item.emplace_back(XorStr("Radar Offset X"), &Config::radar_x, XorStr("雷达 x 坐标"),
			true, 80.0f, true, 500.0f, 10.0f, 10.0f, true);
		menu.item.emplace_back(XorStr("Radar Offset Y"), &Config::radar_y, XorStr("雷达 y 坐标"),
			true, 80.0f, true, 500.0f, 10.0f, 10.0f, true);
		menu.item.emplace_back(XorStr("Radar Size"), &Config::radar_size, XorStr("雷达大小"),
			true, 80.0f, true, 200.0f, 10.0f, 10.0f, true);
		menu.item.emplace_back(XorStr("Mini Radar"), &Config::miniRadar, XorStr("准星雷达"),
			true, 0.0f, true, 1.0f, 1.0f, 1.0f, true);
		menu.item.emplace_back(XorStr("Mini Radar Size"), &Config::miniradar_size, XorStr("准星雷达大小"),
			true, 50.0f, true, 200.0f, 10.0f, 10.0f, true);
	}

	{
		MenuStack& menu = AddMenuClass(XorStr("::Visual"));
		menu.item.emplace_back(XorStr("Barrel"), &Config::barrel, XorStr("玩家显示瞄位置"),
			true, 0.0f, true, 3.0f, 1.0f, 1.0f, true);
		menu.item.emplace_back(XorStr("Entity ESP"), &Config::entityEsp, XorStr("显示实体"),
			true, 0.0f, true, 1.0f, 1.0f, 1.0f, true);
		menu.item.emplace_back(XorStr("Quake Gun"), &Config::quakeGun, XorStr("更改持枪姿势"),
			true, 0.0f, true, 2.0f, 1.0f, 1.0f, true);
		menu.item.emplace_back(XorStr("Name ESP"), &Config::nameEsp, XorStr("显示玩家名字"),
			true, 0.0f, true, 1.0f, 1.0f, 1.0f, true);
		menu.item.emplace_back(XorStr("Weapon ESP"), &Config::weaponEsp, XorStr("显示玩家武器"),
			true, 0.0f, true, 1.0f, 1.0f, 1.0f, true);
		menu.item.emplace_back(XorStr("Bone ESP"), &Config::boneEsp, XorStr("显示玩家骨骼"),
			true, 0.0f, true, 1.0f, 1.0f, 1.0f, true);
		menu.item.emplace_back(XorStr("CrossHair"), &Config::crosshair, XorStr("准星"),
			true, 0.0f, true, 10.0f, 1.0f, 1.0f, true);
		menu.item.emplace_back(XorStr("GunSwitchFx"), &Config::fastSwitch, XorStr("快速切枪"),
			true, 0.0f, true, 1.0f, 1.0f, 1.0f, true);
	}
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
	// int i, x, w, h = 0, l = 0, l2 = 0, lmax = 0;
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
	size_t line = 0, maxlen = 0, curlen = 0;
	std::string description, maxLenStr;
	for (const MenuStack& key : menuList)
	{
		// 遍历索引
		++index;
		++line;
		curlen = key.title.length();
		if (curlen > maxlen)
		{
			maxlen = curlen;
			maxLenStr = key.title;
		}

		if (index == cursor)
			description = key.description;

		if (index == opening)
		{
			for (const MenuItem& value : key.item)
			{
				++index;
				++line;

				curlen = value.title.length();
				if (curlen > maxlen)
				{
					maxlen = curlen;
					maxLenStr = value.title;
				}

				if (index == cursor)
					description = value.description;
			}
		}
	}
	
	int fontWidth, fontHeight;
	gEngfuncs.pfnDrawConsoleStringLen(maxLenStr.c_str(), &fontWidth, &fontHeight);

	int x = 100;
	g_gui.window(x, y, fontWidth + 50, fontHeight * line + 5, 0.5, XorStr("高科技菜单"));
	PrintDescription(description, colorList.get(8));
	x += 2;

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

	char buffer[32];
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
				// PrintWithFont(x, y + i * 11, 255, 128, 0, key.title.c_str());
				PrintDescription(key.description, colorList.get(8));
				gEngfuncs.pfnDrawSetTextColor(255, 128, 0);
			}
			else if(index == opening)
			{
				// 未选中菜单类别，但是当前类别已经打开了
				// PrintWithFont(x, y + i * 11, 0, 255, 128, key.title.c_str());
				gEngfuncs.pfnDrawSetTextColor(0, 255, 128);
			}
			else
			{
				// 未选中菜单类别
				// PrintWithFont(x, y + i * 11, 255, 255, 255, key.title.c_str());
				gEngfuncs.pfnDrawSetTextColor(255, 255, 255);
			}

			gEngfuncs.pfnDrawConsoleString(x, y, (char*)key.title.c_str());
			y += fontHeight;

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
						PrintDescription(value.description, colorList.get(8));
						gEngfuncs.pfnDrawSetTextColor(0, 128, 255);
						gEngfuncs.pfnDrawConsoleString(x + 2, y, (char*)value.title.c_str());

						if (value.hasInteger)
						{
							/*
							PrintWithFont(x, y + i * 11, 0, 128, 255, " %s %.0f",
								value.title.c_str(), *(value.pointer));
							*/

							sprintf_s(buffer, "%.0f", *(value.pointer));
							gEngfuncs.pfnDrawSetTextColor(128, 255, 255);
						}
						else
						{
							/*
							PrintWithFont(x, y + i * 11, 0, 128, 255, " %s %.f",
								value.title.c_str(), *(value.pointer));
							*/

							sprintf_s(buffer, "%.f", *(value.pointer));
							gEngfuncs.pfnDrawSetTextColor(128, 255, 255);
						}
					}
					else
					{
						// 未选中的菜单项目使用白色
						gEngfuncs.pfnDrawSetTextColor(255, 255, 255);
						gEngfuncs.pfnDrawConsoleString(x + 2, y, (char*)value.title.c_str());

						if (value.hasInteger)
						{
							/*
							PrintWithFont(x, y + i * 11, 255, 255, 255, " %s %.0f",
								value.title.c_str(), *(value.pointer));
							*/

							sprintf_s(buffer, "%.0f", *(value.pointer));
						}
						else
						{
							/*
							PrintWithFont(x, y + i * 11, 255, 255, 255, " %s %.f",
								value.title.c_str(), *(value.pointer));
							*/

							sprintf_s(buffer, "%.f", *(value.pointer));
						}
					}

					// int padding = fontWidth - (value.title.length() + 2 + strlen(buffer));
					gEngfuncs.pfnDrawConsoleString(x + fontWidth + 20 - strlen(buffer), y, buffer);
					y += fontHeight;
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
	else if (keynum == K_RIGHTARROW)
	{
		if (opening > -1)
		{
			if (cursor >= openBegin && cursor <= openEnd)
			{
				// 菜单项目操作(增加)
				MenuItem& item = menuList[opening].item[cursor - openBegin];
				*item.pointer += item.incSetp;
				if (item.hasMax && *item.pointer > item.maxValue)
					*item.pointer = item.minValue;
			}
			else
			{
				// 菜单类别操作(打开)
				if (cursor > openEnd)
				{
					cursor -= openEnd - openBegin + 1;
					if (cursor == opening)
						opening = -1;
					else
						opening = cursor;
				}
				else
				{
					if (cursor == opening)
						opening = -1;
					else
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
	else if (keynum == K_LEFTARROW) //rightarrow || rightbutton
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
					cursor -= openEnd - openBegin + 1;
					if (cursor == opening)
						opening = -1;
					else
						opening = cursor;
				}
				else
				{
					if (cursor == opening)
						opening = -1;
					else
						opening = cursor;
				}
			}
		}
		else
		{
			// 菜单类别操作(关闭)
			opening = cursor;
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

