#include "drawing.h"
#include "tablefont.h"
#include "../clientdll.h"
#include "../misc/xorstr.h"
#include "../players.h"
#include "../opengl.h"
#include "../Engine/ISurface.h"
#include "../Engine/IEngineVGui.h"

ColorManager colorList;
extern cl_enginefuncs_s gEngfuncs;

vgui::ISurface* g_pSurface;
vgui::IEngineVGui* g_pEngineVGui;

void ColorManager::Init()
{
	add(000, 200, 255, 180);	// Menu Title - 0
	add(225, 225, 225, 180);	// Menu Content - 1
	add(255, 000, 000, 255);	// Terrorist - 2
	add(000, 100, 255, 255);	// Counter-Terrorist - 3
	add(000, 255, 000, 255);	// Aimbot Target - 4
	add(255, 255, 255, 255);	// Entity Esp Weapon - 5
	add(255, 150, 000, 255);	// Entity Esp Hostage - 6
	add(000, 000, 000, 000);	// Menu - Deselected - 7
	add(255, 255, 255, 255);	// Menu - Selected - 8
	add(255, 255, 255, 200);   // Crosshair - 9
	add(255, 000, 000, 200);   // Crosshair - 10
	add(255, 000, 255, 255);	// DefusalKit - 11
	add(000, 000, 255, 255);	// ConsoleColor - 12
	add(255, 255, 000, 255);	// Flashbang Trace - 13
}

ColorEntry * ColorManager::get(int index)
{
	if (index<entrys.size())
		return &entrys[index];
	else
		return &entrys[0];
}

void ColorManager::add(int r, int g, int b, int a)
{
	ColorEntry tmp;
	tmp.r = r;
	tmp.g = g;
	tmp.b = b;
	tmp.a = a;
	tmp.onebased_r = r / 255;
	tmp.onebased_g = g / 255;
	tmp.onebased_b = b / 255;
	tmp.onebased_a = a / 255;
	entrys.push_back(tmp);
	currentIndex++;
}

extern bool oglSubtractive;
extern int heightenginefont;
extern int lengthcharenginefont;

//==============================================================================
void DrawSmoothBox(int x, int y, int width, int height, float ubr, float ubg, float ubb, float lbr, float lbg, float lbb, float ba)
{
	glDisable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);

	if(glEnable_detour != nullptr)
		glEnable_detour(GL_BLEND);
	else
		glEnable(GL_BLEND);

	if(glBlendFunc_detour != nullptr)
		glBlendFunc_detour(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	else
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if(glBegin_detour != nullptr)
		glBegin_detour(GL_QUADS);
	else
		glBegin(GL_QUADS);

	glColor4f(ubr, ubg, ubb, ba);
	glVertex2i(x, y);
	glColor4f(ubr, ubg, ubb, ba);
	glVertex2i(x + width, y);
	glColor4f(lbr, lbg, lbb, ba);
	glVertex2i(x + width, y + height);
	glColor4f(lbr, lbg, lbb, ba);
	glVertex2i(x, y + height);

	if (glEnd_detour != nullptr)
		glEnd_detour();
	else
		glEnd();

	if (glEnable_detour != nullptr)
		glEnable_detour(GL_TEXTURE_2D);
	else
		glEnable(GL_TEXTURE_2D);
}

/*
void blackBorder(int x, int y, int w, int h)
{
	oglSubtractive = true;
	gEngfuncs.pfnFillRGBA(x - 1, y - 1, w + 2, 1, 0, 0, 0, 254); //top
	gEngfuncs.pfnFillRGBA(x - 1, y, 1, h - 1, 0, 0, 0, 254);	//left
	gEngfuncs.pfnFillRGBA(x + w, y, 1, h - 1, 0, 0, 0, 254);	//right
	gEngfuncs.pfnFillRGBA(x - 1, y + h - 1, w + 2, 1, 0, 0, 0, 254); //bottom
	oglSubtractive = false;
}
*/

void DrawCrosshair(int mode)
{
	ColorEntry* color = colorList.get(9);  // "cross"
	ColorEntry* colo2 = colorList.get(10); // "cross2"

	int r = color->r, g = color->g, b = color->b, a = color->a;
	int R = colo2->r, G = colo2->g, B = colo2->b, A = colo2->a;

	oglSubtractive = true;

	int tcross = mode;
	if (g_local.inZoomMode)
		mode = 6;

	if (mode == 1)
	{
		drawing::DrawFillRect(displayCenterX - 14, displayCenterY, 9, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX + 5, displayCenterY, 9, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX, displayCenterY - 14, 1, 9, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX, displayCenterY + 5, 1, 9, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX, displayCenterY, 1, 1, COLOR_RGBA(R, G, B, A)); // center
	}
	else if (mode == 2)
	{
		drawing::DrawFillRect(displayCenterX - 14, displayCenterY, 9, 2, COLOR_RGBA(r, g, b, a)); // left
		drawing::DrawFillRect(displayCenterX + 6, displayCenterY, 9, 2, COLOR_RGBA(r, g, b, a)); // right
		drawing::DrawFillRect(displayCenterX, displayCenterY - 14, 2, 9, COLOR_RGBA(r, g, b, a)); // top
		drawing::DrawFillRect(displayCenterX, displayCenterY + 7, 2, 9, COLOR_RGBA(r, g, b, a)); // bottom
		drawing::DrawFillRect(displayCenterX, displayCenterY, 2, 2, COLOR_RGBA(R, G, B, A)); // center
	}
	else if (mode == 3)
	{
		drawing::DrawFillRect(displayCenterX - 25, displayCenterY, 50, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX, displayCenterY - 25, 1, 50, COLOR_RGBA(r, g, b, a));

		drawing::DrawFillRect(displayCenterX - 5, displayCenterY, 10, 1, COLOR_RGBA(R, G, B, A));
		drawing::DrawFillRect(displayCenterX, displayCenterY - 5, 1, 10, COLOR_RGBA(R, G, B, A));
	}
	else if (mode == 4)
	{
		drawing::DrawFillRect(displayCenterX - 25, displayCenterY, 50, 2, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX, displayCenterY - 25, 2, 50, COLOR_RGBA(r, g, b, a));

		drawing::DrawFillRect(displayCenterX - 5, displayCenterY, 10, 2, COLOR_RGBA(R, G, B, A));
		drawing::DrawFillRect(displayCenterX, displayCenterY - 5, 2, 10, COLOR_RGBA(R, G, B, A));
	}
	else if (mode == 5)
	{
		int iX = int(displayCenterX), iY = int(displayCenterY), iSize = 25;
		// border top
		tintArea(iX - iSize, iY - iSize, 2 * iSize, 1, 255, 255, 255, 180);
		//border bottom
		tintArea(iX - iSize, iY + iSize, 2 * iSize, 1, 255, 255, 255, 180);
		//border left
		tintArea(iX - iSize, iY - iSize + 1, 1, 2 * iSize - 1, 255, 255, 255, 180);
		//border right
		tintArea(iX + iSize, iY - iSize, 1, 2 * iSize + 1, 255, 255, 255, 180);

		//cross
		tintArea(iX, iY - iSize + 1, 1, 2 * iSize - 1, 0, 160, 0, 180);
		tintArea(iX - iSize + 1, iY, 2 * iSize - 1, 1, 0, 160, 0, 180);
	}
	else if (mode == 6)
	{
		drawing::DrawFillRect(0, displayCenterY, 2 * displayCenterX, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX, 0, 1, 2 * displayCenterY, COLOR_RGBA(r, g, b, a));

		drawing::DrawFillRect(displayCenterX - 5, displayCenterY, 10, 1, COLOR_RGBA(R, G, B, A));
		drawing::DrawFillRect(displayCenterX, displayCenterY - 5, 1, 10, COLOR_RGBA(R, G, B, A));
	}
	else if (mode == 7)
	{
		drawing::DrawFillRect(displayCenterX, displayCenterY, 1, 1, COLOR_RGBA(255, 0, 0, 255));
		drawing::DrawFillRect(displayCenterX - 1, displayCenterY + 1, 1, 1, COLOR_RGBA(255, 0, 0, 255));
		drawing::DrawFillRect(displayCenterX - 2, displayCenterY + 2, 1, 1, COLOR_RGBA(255, 0, 0, 255));
		drawing::DrawFillRect(displayCenterX - 3, displayCenterY + 3, 1, 1, COLOR_RGBA(255, 0, 0, 255));
		drawing::DrawFillRect(displayCenterX - 5, displayCenterY + 5, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX - 6, displayCenterY + 6, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX - 7, displayCenterY + 7, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX - 8, displayCenterY + 8, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX - 9, displayCenterY + 9, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX - 10, displayCenterY + 10, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX - 11, displayCenterY + 11, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX - 12, displayCenterY + 12, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX - 13, displayCenterY + 13, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX - 14, displayCenterY + 14, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX - 15, displayCenterY + 15, 1, 1, COLOR_RGBA(r, g, b, a));

		drawing::DrawFillRect(displayCenterX - 1, displayCenterY - 1, 1, 1, COLOR_RGBA(255, 0, 0, 255));
		drawing::DrawFillRect(displayCenterX - 2, displayCenterY - 2, 1, 1, COLOR_RGBA(255, 0, 0, 255));
		drawing::DrawFillRect(displayCenterX - 3, displayCenterY - 3, 1, 1, COLOR_RGBA(255, 0, 0, 255));
		drawing::DrawFillRect(displayCenterX - 5, displayCenterY - 5, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX - 6, displayCenterY - 6, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX - 7, displayCenterY - 7, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX - 8, displayCenterY - 8, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX - 9, displayCenterY - 9, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX - 10, displayCenterY - 10, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX - 11, displayCenterY - 11, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX - 12, displayCenterY - 12, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX - 13, displayCenterY - 13, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX - 14, displayCenterY - 14, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX - 15, displayCenterY - 15, 1, 1, COLOR_RGBA(r, g, b, a));

		drawing::DrawFillRect(displayCenterX + 1, displayCenterY + 1, 1, 1, COLOR_RGBA(255, 0, 0, 255));
		drawing::DrawFillRect(displayCenterX + 2, displayCenterY + 2, 1, 1, COLOR_RGBA(255, 0, 0, 255));
		drawing::DrawFillRect(displayCenterX + 3, displayCenterY + 3, 1, 1, COLOR_RGBA(255, 0, 0, 255));
		drawing::DrawFillRect(displayCenterX + 5, displayCenterY + 5, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX + 6, displayCenterY + 6, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX + 7, displayCenterY + 7, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX + 8, displayCenterY + 8, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX + 9, displayCenterY + 9, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX + 10, displayCenterY + 10, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX + 11, displayCenterY + 11, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX + 12, displayCenterY + 12, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX + 13, displayCenterY + 13, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX + 14, displayCenterY + 14, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX + 15, displayCenterY + 15, 1, 1, COLOR_RGBA(r, g, b, a));

		drawing::DrawFillRect(displayCenterX + 1, displayCenterY - 1, 1, 1, COLOR_RGBA(255, 0, 0, 255));
		drawing::DrawFillRect(displayCenterX + 2, displayCenterY - 2, 1, 1, COLOR_RGBA(255, 0, 0, 255));
		drawing::DrawFillRect(displayCenterX + 3, displayCenterY - 3, 1, 1, COLOR_RGBA(255, 0, 0, 255));
		drawing::DrawFillRect(displayCenterX + 5, displayCenterY - 5, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX + 6, displayCenterY - 6, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX + 7, displayCenterY - 7, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX + 8, displayCenterY - 8, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX + 9, displayCenterY - 9, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX + 10, displayCenterY - 10, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX + 11, displayCenterY - 11, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX + 12, displayCenterY - 12, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX + 13, displayCenterY - 13, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX + 14, displayCenterY - 14, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX + 15, displayCenterY - 15, 1, 1, COLOR_RGBA(r, g, b, a));

	}
	else if (mode == 8)
	{
		drawing::DrawFillRect(displayCenterX - 14, displayCenterY, 9, 2, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX + 5, displayCenterY, 9, 2, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX - 14, displayCenterY, 9, -1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX + 5, displayCenterY, 9, -1, COLOR_RGBA(r, g, b, a));

		drawing::DrawFillRect(displayCenterX, displayCenterY - 14, 2, 9, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX, displayCenterY + 6, 2, 9, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX, displayCenterY - 14, -1, 9, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX, displayCenterY + 6, -1, 9, COLOR_RGBA(r, g, b, a));

		drawing::DrawFillRect(displayCenterX - 3, displayCenterY, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX + 3, displayCenterY, 1, 1, COLOR_RGBA(r, g, b, a));

		drawing::DrawFillRect(displayCenterX, displayCenterY - 3, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX, displayCenterY + 3, 1, 1, COLOR_RGBA(r, g, b, a));
	}
	else if (mode == 9)
	{
		drawing::DrawFillRect(displayCenterX - 14, displayCenterY, 9, 2, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX + 5, displayCenterY, 9, 2, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX - 14, displayCenterY, 9, -1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX + 5, displayCenterY, 9, -1, COLOR_RGBA(r, g, b, a));

		drawing::DrawFillRect(displayCenterX, displayCenterY - 14, 2, 9, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX, displayCenterY + 6, 2, 9, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX, displayCenterY - 14, -1, 9, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX, displayCenterY + 6, -1, 9, COLOR_RGBA(r, g, b, a));

		drawing::DrawFillRect(displayCenterX - 3, displayCenterY, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX + 3, displayCenterY, 1, 1, COLOR_RGBA(r, g, b, a));

		drawing::DrawFillRect(displayCenterX, displayCenterY - 3, 1, 1, COLOR_RGBA(r, g, b, a));
		drawing::DrawFillRect(displayCenterX, displayCenterY + 3, 1, 1, COLOR_RGBA(r, g, b, a));

		drawing::DrawFillRect(displayCenterX - 1, displayCenterY, 3, 1, COLOR_RGBA(R, G, B, A));
		drawing::DrawFillRect(displayCenterX, displayCenterY - 1, 1, 3, COLOR_RGBA(R, G, B, A));
	}
	else if (mode == 10)
	{
		//Lineas ppales
		drawing::DrawFillRect(displayCenterX - 20, displayCenterY, 40, 2, COLOR_RGBA(0, 0, 0, 255));
		drawing::DrawFillRect(displayCenterX, displayCenterY - 20, 2, 40, COLOR_RGBA(0, 0, 0, 255));
		//Izquierda
		drawing::DrawFillRect(displayCenterX - 20, displayCenterY - 10, 1, 22, COLOR_RGBA(0, 0, 0, 255));
		drawing::DrawFillRect(displayCenterX - 15, displayCenterY - 5, 1, 12, COLOR_RGBA(0, 0, 0, 255));
		drawing::DrawFillRect(displayCenterX - 10, displayCenterY - 2, 1, 6, COLOR_RGBA(0, 0, 0, 255));
		//Derecha
		drawing::DrawFillRect(displayCenterX + 20, displayCenterY - 10, 1, 22, COLOR_RGBA(0, 0, 0, 255));
		drawing::DrawFillRect(displayCenterX + 15, displayCenterY - 5, 1, 12, COLOR_RGBA(0, 0, 0, 255));
		drawing::DrawFillRect(displayCenterX + 10, displayCenterY - 2, 1, 6, COLOR_RGBA(0, 0, 0, 255));
		//Arriba
		drawing::DrawFillRect(displayCenterX - 10, displayCenterY - 20, 22, 1, COLOR_RGBA(0, 0, 0, 255));
		drawing::DrawFillRect(displayCenterX - 5, displayCenterY - 15, 12, 1, COLOR_RGBA(0, 0, 0, 255));
		drawing::DrawFillRect(displayCenterX - 2, displayCenterY - 10, 6, 1, COLOR_RGBA(0, 0, 0, 255));
		//Abajo
		drawing::DrawFillRect(displayCenterX - 10, displayCenterY + 20, 22, 1, COLOR_RGBA(0, 0, 0, 255));
		drawing::DrawFillRect(displayCenterX - 5, displayCenterY + 15, 12, 1, COLOR_RGBA(0, 0, 0, 255));
		drawing::DrawFillRect(displayCenterX - 2, displayCenterY + 10, 6, 1, COLOR_RGBA(0, 0, 0, 255));
		//Punto blanco
		drawing::DrawFillRect(displayCenterX, displayCenterY, 2, 2, COLOR_RGBA(R, G, B, A));
	}
	oglSubtractive = false;

	mode = tcross;
}
//==============================================================================
int DrawLen(char *fmt)
{
	int len = 0;
	for (char * p = fmt; *p; p++) len += screeninfo.charWidths[*p];
	return len;
}
void DrawLine(int x1, int y1, int x2, int y2, byte r, byte g, byte b, byte a)
{
	if (glPushMatrix_detour != nullptr)
		glPushMatrix_detour();
	else
		glPushMatrix();

	glLoadIdentity();
	glDisable(GL_TEXTURE_2D);

	if(glEnable_detour != nullptr)
		glEnable_detour(GL_BLEND);
	else
		glEnable(GL_BLEND);

	if(glBlendFunc_detour != nullptr)
		glBlendFunc_detour(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	else
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor4ub(r, g, b, a);
	glLineWidth(1.0f);

	if(glBegin_detour != nullptr)
		glBegin_detour(GL_LINES);
	else
		glBegin(GL_LINES);

	glVertex2i(x1, y1);
	glVertex2i(x2, y2);

	if (glEnd_detour != nullptr)
		glEnd_detour();
	else
		glEnd();

	glColor3f(1.0f, 1.0f, 1.0f);

	if (glEnable_detour != nullptr)
		glEnable_detour(GL_TEXTURE_2D);
	else
		glEnable(GL_TEXTURE_2D);
	
	glDisable(GL_BLEND);

	if (glPopMatrix_detour != nullptr)
		glPopMatrix_detour();
	else
		glPopMatrix();
}
//==============================================================================
void DrawHudStringCenter(int x, int y, int r, int g, int b, const char *fmt, ...)
{
	va_list va_alist;
	char buf[256];

	va_start(va_alist, fmt);
	_vsnprintf(buf, sizeof(buf), fmt, va_alist);
	va_end(va_alist);

	// y-check
	int borderY = displayCenterY * 2 - 18;
	if (y<0 || y>borderY) { return; }

	int drawLen = DrawLen(buf);
	x = x - drawLen / 2;

	int borderX = displayCenterX * 2 - 11;
	int minX = x;
	int maxX = x + drawLen;
	bool needSingleCheck = (minX<1 || maxX>borderX);

	if (needSingleCheck)
	{
		for (char * p = buf; *p; p++)
		{
			int next = x + screeninfo.charWidths[*p];
			// IMPORTANT NOTE: when drawing admin-mod style charactters
			//    you MAY NOT provide x/y coordinates that cause drawing
			//    off screen. This causes HL to crash or just quit
			if (x>0 && x<borderX)
				gEngfuncs.pfnDrawCharacter(x, y, *p, r, g, b);
			x = next;
		}
	}
	else {
		for (char * p = buf; *p; p++)
		{
			int next = x + screeninfo.charWidths[*p];

			// IMPORTANT NOTE: when drawing admin-mod style charactters
			//    you MAY NOT provide x/y coordinates that cause drawing
			//    off screen. This causes HL to crash or just quit
			gEngfuncs.pfnDrawCharacter(x, y, *p, r, g, b);
			x = next;
		}
	}
}

//========================================================================================
void DrawHudString(int x, int y, int r, int g, int b, const char *fmt, ...)
{
	va_list va_alist;
	char buf[256];

	va_start(va_alist, fmt);
	_vsnprintf(buf, sizeof(buf), fmt, va_alist);
	va_end(va_alist);

	// y-check
	int borderY = displayCenterY * 2 - 18;
	if (y<0 || y>borderY) { return; }

	bool needSingleCheck = false;
	int borderX = displayCenterX * 2 - 11;

	int drawLen = DrawLen(buf);
	if (x<1) { needSingleCheck = true; }
	else
	{
		int maxX = x + drawLen;
		needSingleCheck = (maxX>borderX);
	}

	if (needSingleCheck)
	{
		for (char * p = buf; *p; p++)
		{
			int next = x + screeninfo.charWidths[*p];
			// IMPORTANT NOTE: when drawing admin-mod style charactters
			//    you MAY NOT provide x/y coordinates that cause drawing
			//    off screen. This causes HL to crash or just quit
			if (x>0 && x<borderX)
				gEngfuncs.pfnDrawCharacter(x, y, *p, r, g, b);
			x = next;
		}
	}
	else {
		for (char * p = buf; *p; p++)
		{
			int next = x + screeninfo.charWidths[*p];
			// IMPORTANT NOTE: when drawing admin-mod style charactters
			//    you MAY NOT provide x/y coordinates that cause drawing
			//    off screen. This causes HL to crash or just quit
			gEngfuncs.pfnDrawCharacter(x, y, *p, r, g, b);
			x = next;
		}
	}
}

//========================================================================================

void DrawConString(int x, int y, int r, int g, int b, const char *fmt, ...)
{
	va_list va_alist;
	char buf[256];

	va_start(va_alist, fmt);
	_vsnprintf(buf, sizeof(buf), fmt, va_alist);
	va_end(va_alist);

	int length, height;

	y -= 2;
	//y += 4;
	gEngfuncs.pfnDrawConsoleStringLen(buf, &length, &height);
	gEngfuncs.pfnDrawSetTextColor(r / 255.0f, g / 255.0f, b / 255.0f);
	gEngfuncs.pfnDrawConsoleString(x, y, buf);
}

//========================================================================================

void DrawConStringCenter(int x, int y, int r, int g, int b, const char *fmt, ...)
{
	va_list va_alist;
	char buf[256];

	va_start(va_alist, fmt);
	_vsnprintf(buf, sizeof(buf), fmt, va_alist);
	va_end(va_alist);

	int length, height;

	gEngfuncs.pfnDrawConsoleStringLen(buf, &length, &height);
	x -= length / 2;
	y -= 2;
	//y += 4;
	gEngfuncs.pfnDrawSetTextColor(r / 255.0f, g / 255.0f, b / 255.0f);
	gEngfuncs.pfnDrawConsoleString(x, y, buf);
}

//========================================================================================

void gDrawBoxAtScreenXY(int x, int y, int r, int g, int b, int alpha, int radius = 1)
{
	int radius2 = radius << 1;
	oglSubtractive = true;
	// gEngfuncs.pfnFillRGBA(x - radius + 2, y - radius, radius2 - 2, 2, r, g, b, alpha);
	// gEngfuncs.pfnFillRGBA(x - radius, y - radius, 2, radius2, r, g, b, alpha);
	// gEngfuncs.pfnFillRGBA(x - radius, y + radius, radius2, 2, r, g, b, alpha);
	// gEngfuncs.pfnFillRGBA(x + radius, y - radius, 2, radius2 + 2, r, g, b, alpha);

	drawing::DrawFillRect(x - radius + 2, y - radius, radius2 - 2, 2, COLOR_RGBA(r, g, b, alpha));
	drawing::DrawFillRect(x - radius, y - radius, 2, radius2, COLOR_RGBA(r, g, b, alpha));
	drawing::DrawFillRect(x - radius, y + radius, radius2, 2, COLOR_RGBA(r, g, b, alpha));
	drawing::DrawFillRect(x + radius, y - radius, 2, radius2 + 2, COLOR_RGBA(r, g, b, alpha));

	oglSubtractive = false;
}
void gDrawFilledBoxAtLocation(float* origin, DWORD color, int radius)
{
	float vecScreen[2];
	if (!CalcScreen(origin, vecScreen)) { return; }

	int red = (color >> 24);
	int green = (color >> 16) & 0xFF;
	int blue = (color >> 8) & 0xFF;
	int alpha = (color) & 0xFF;
	int radius2 = radius << 1;

	// gEngfuncs.pfnFillRGBA(vecScreen[0] - radius, vecScreen[1] - radius, radius2, radius2, red, green, blue, alpha);
	drawing::DrawFillRect(vecScreen[0] - radius, vecScreen[1] - radius, radius2, radius2, COLOR_RGBA(red, green, blue, alpha));
}
//========================================================================================

void whiteBorder(int x, int y, int w, int h)
{
	oglSubtractive = true;
	// gEngfuncs.pfnFillRGBA(x - 1, y - 1, w + 2, 1, 255, 255, 255, 255); //top
	// gEngfuncs.pfnFillRGBA(x - 1, y, 1, h - 1, 255, 255, 255, 255);	//left
	// gEngfuncs.pfnFillRGBA(x + w, y, 1, h - 1, 255, 255, 255, 255);	//right
	// gEngfuncs.pfnFillRGBA(x - 1, y + h - 1, w + 2, 1, 255, 255, 255, 255); //bottom
	drawing::DrawFillRect(x - 1, y - 1, w + 2, 1, COLOR_RGBA(255, 255, 255, 255));
	drawing::DrawFillRect(x - 1, y, 1, h - 1, COLOR_RGBA(255, 255, 255, 255));
	drawing::DrawFillRect(x + w, y, 1, h - 1, COLOR_RGBA(255, 255, 255, 255));
	drawing::DrawFillRect(x - 1, y + h - 1, w + 2, 1, COLOR_RGBA(255, 255, 255, 255));
	oglSubtractive = false;
}

//========================================================================================

void blackBorder(int x, int y, int w, int h)
{
	oglSubtractive = true;
	/*
	gEngfuncs.pfnFillRGBA(x-1,y-1,w+2,  1,000,255,255,255); //top
	gEngfuncs.pfnFillRGBA(x-1,  y,  1,h-1,000,255,255,255); //left
	gEngfuncs.pfnFillRGBA(x+w,  y,  1,h-1,000,255,255,255); //right
	gEngfuncs.pfnFillRGBA(x-1,y+h-1,w+2,1,000,255,255,255); //bottom
	*/

	/*
	gEngfuncs.pfnFillRGBA(x - 1, y - 1, w + 2, 1, 0, 0, 0, 254); //top
	gEngfuncs.pfnFillRGBA(x - 1, y, 1, h - 1, 0, 0, 0, 254);	//left
	gEngfuncs.pfnFillRGBA(x + w, y, 1, h - 1, 0, 0, 0, 254);	//right
	gEngfuncs.pfnFillRGBA(x - 1, y + h - 1, w + 2, 1, 0, 0, 0, 254); //bottom
	*/

	drawing::DrawFillRect(x - 1, y - 1, w + 2, 1, COLOR_RGBA(0, 0, 0, 254));
	drawing::DrawFillRect(x - 1, y, 1, h - 1, COLOR_RGBA(0, 0, 0, 254));
	drawing::DrawFillRect(x + w, y, 1, h - 1, COLOR_RGBA(0, 0, 0, 254));
	drawing::DrawFillRect(x - 1, y + h - 1, w + 2, 1, COLOR_RGBA(0, 0, 0, 254));

	oglSubtractive = false;
}

//========================================================================================

void tintArea(int x, int y, int w, int h, ColorEntry* clr)
{
	oglSubtractive = true;
	// gEngfuncs.pfnFillRGBA(x, y, w, h, clr->r, clr->g, clr->b, clr->a);
	drawing::DrawFillRect(x, y, w, h, COLOR_RGBA(clr->r, clr->g, clr->b, clr->a));
	oglSubtractive = false;
}

//========================================================================================

void tintArea(int x, int y, int w, int h, int r, int g, int b, int a)
{
	oglSubtractive = true;
	// gEngfuncs.pfnFillRGBA(x, y, w, h, r, g, b, a);
	drawing::DrawFillRect(x, y, w, h, COLOR_RGBA(r, g, b, a));
	oglSubtractive = false;
}

//========================================================================================

void colorBorder(int x, int y, int w, int h, int r, int g, int b, int a)
{
	oglSubtractive = true;
	gEngfuncs.pfnFillRGBA(x - 1, y - 1, w + 2, 1, r, g, b, a); //top
	gEngfuncs.pfnFillRGBA(x - 1, y, 1, h - 1, r, g, b, a);	//left
	gEngfuncs.pfnFillRGBA(x + w, y, 1, h - 1, r, g, b, a);	//right
	gEngfuncs.pfnFillRGBA(x - 1, y + h - 1, w + 2, 1, r, g, b, a); //bottom
	oglSubtractive = false;
}

//========================================================================================

void dddBorder(int x, int y, int w, int h, int a)
{
	oglSubtractive = true;
	gEngfuncs.pfnFillRGBA(x - 1, y - 1, w + 2, 1, 255, 255, 255, a); //top
	gEngfuncs.pfnFillRGBA(x - 1, y, 1, h - 1, 255, 255, 255, a);	//left
	gEngfuncs.pfnFillRGBA(x + w, y, 1, h - 1, 0, 0, 0, a);	//right
	gEngfuncs.pfnFillRGBA(x - 1, y + h - 1, w + 2, 1, 0, 0, 0, a); //bottom
	oglSubtractive = false;
}

//========================================================================================

void dddBorder2(int x, int y, int w, int h, int a)
{
	oglSubtractive = true;
	gEngfuncs.pfnFillRGBA(x - 1, y - 1, w + 2, 1, 0, 0, 0, a); //top
	gEngfuncs.pfnFillRGBA(x - 1, y, 1, h - 1, 0, 0, 0, a);	//left
	gEngfuncs.pfnFillRGBA(x + w, y, 1, h - 1, 255, 255, 255, a);	//right
	gEngfuncs.pfnFillRGBA(x - 1, y + h - 1, w + 2, 1, 255, 255, 255, a); //bottom
	oglSubtractive = false;
}

//========================================================================================
void PrintWithFont(int x, int y, int r, int g, int b, const char *fmt, ...)
{
	va_list va_alist;
	char buf[256];

	va_start(va_alist, fmt);
	_vsnprintf(buf, sizeof(buf), fmt, va_alist);
	va_end(va_alist);
	g_tableFont.drawString(false, x, y, r, g, b, buf);
}

void PrintWithFont(int x, int y, int r, int g, int b, std::string text)
{
	g_tableFont.drawString(false, x, y, r, g, b, text.c_str());
}

vgui::HFont drawing::fontEsp = 0, drawing::fontMenu = 0;
bool drawing::bInPaint = false;

void drawing::SetupFonts()
{
	fontEsp = g_pSurface->CreateFont();
	fontMenu = g_pSurface->CreateFont();

	if (!g_pSurface->AddGlyphSetToFont(fontEsp,
		"Microsoft YaHei Light & Microsoft YaHei UI Light", 14, FW_BOLD, NULL, NULL,
		vgui::ISurface::FONTFLAG_DROPSHADOW, 0, 0))
	{
		g_pSurface->AddGlyphSetToFont(fontEsp, "Tahoma", 14, FW_BOLD, NULL, NULL,
			vgui::ISurface::FONTFLAG_DROPSHADOW, 0, 0);

		g_pSurface->AddGlyphSetToFont(fontMenu, "Tahoma", 16, FW_BOLD, NULL, NULL,
			vgui::ISurface::FONTFLAG_DROPSHADOW, 0, 0);
	}
	else
	{
		g_pSurface->AddGlyphSetToFont(fontEsp,
			"Microsoft YaHei Light & Microsoft YaHei UI Light", 14, FW_BOLD, NULL, NULL,
			vgui::ISurface::FONTFLAG_DROPSHADOW, 0, 0);

		g_pSurface->AddGlyphSetToFont(fontMenu,
			"Microsoft YaHei Light & Microsoft YaHei UI Light", 16, FW_BOLD, NULL, NULL,
			vgui::ISurface::FONTFLAG_DROPSHADOW, 0, 0);
	}
}

std::string drawing::FindFonts(const std::string & name)
{
	//
	// This code is not as safe as it should be.
	// Assumptions we make:
	//  -> GetWindowsDirectoryA does not fail.
	//  -> The registry key exists.
	//  -> The subkeys are ordered alphabetically
	//  -> The subkeys name and data are no longer than 260 (MAX_PATH) chars.
	//

	char buffer[MAX_PATH];
	HKEY registryKey;

	GetWindowsDirectoryA(buffer, MAX_PATH);
	std::string fontsFolder = buffer + std::string("\\Fonts\\");

	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts", 0,
		KEY_READ, &registryKey))
	{
		return "";
	}

	uint32_t valueIndex = 0;
	char valueName[MAX_PATH];
	uint8_t valueData[MAX_PATH];
	std::wstring wsFontFile;

	for (;;)
	{
		uint32_t valueNameSize = MAX_PATH;
		uint32_t valueDataSize = MAX_PATH;
		uint32_t valueType;

		auto error = RegEnumValueA(registryKey, valueIndex, valueName,
			reinterpret_cast<DWORD*>(&valueNameSize), 0, reinterpret_cast<DWORD*>(&valueType),
			valueData, reinterpret_cast<DWORD*>(&valueDataSize));

		valueIndex++;

		if (error == ERROR_NO_MORE_ITEMS)
		{
			RegCloseKey(registryKey);
			break;
		}

		if (error || valueType != REG_SZ)
		{
			continue;
		}

		if (_strnicmp(name.data(), valueName, name.size()) == 0)
		{
			RegCloseKey(registryKey);
			return (fontsFolder + std::string((char*)valueData, valueDataSize));
		}
	}

	return "";
}

void drawing::DrawLine(int x1, int y1, int x2, int y2, DWORD color)
{
	g_pSurface->DrawSetColor(COLOR_R(color), COLOR_G(color), COLOR_B(color), COLOR_A(color));
	g_pSurface->DrawLine(x1, y1, x2, y2);
}

void drawing::DrawRect(int x, int y, int w, int h, DWORD color)
{
	g_pSurface->DrawSetColor(COLOR_R(color), COLOR_G(color), COLOR_B(color), COLOR_A(color));
	g_pSurface->DrawOutlinedRect(x, y, x + w, y + h);
}

void drawing::DrawFillRect(int x, int y, int w, int h, DWORD color)
{
	g_pSurface->DrawSetColor(COLOR_R(color), COLOR_G(color), COLOR_B(color), COLOR_A(color));
	g_pSurface->DrawFilledRect(x, y, x + w, y + h);
}

void drawing::DrawCornerRect(int x, int y, int w, int h, int length, DWORD color)
{
	if (length == 0)
	{
		if (w == 0)
			return;

		length = w / 3;
	}

	g_pSurface->DrawSetColor(COLOR_R(color), COLOR_G(color), COLOR_B(color), COLOR_A(color));

	// 左上
	g_pSurface->DrawLine(x, y, x + length, y);
	g_pSurface->DrawLine(x, y, x, y - length);

	// 右上
	g_pSurface->DrawLine(x + w, y, x + w - length, y);
	g_pSurface->DrawLine(x + w, y, x + w, y - length);

	// 左下
	g_pSurface->DrawLine(x, y + h, x + length, y + h);
	g_pSurface->DrawLine(x, y + h, x, y + h + length);

	// 右下
	g_pSurface->DrawLine(x + w, y + h, x + w - length, y + h);
	g_pSurface->DrawLine(x + w, y + h, x + w, y + h + length);
}

void drawing::DrawCircle(int x, int y, int radius, DWORD color, int resolution)
{
	std::vector<int> vertice_x, vertice_y;

	float curAngle;
	float angle = (float)((2.0f * M_PI_F) / resolution);
	for (int i = 0; i <= resolution; ++i)
	{
		curAngle = i * angle;
		vertice_x.emplace_back((int)(x + radius * cos(curAngle)));
		vertice_y.emplace_back((int)(y - radius * sin(curAngle)));
	}

	g_pSurface->DrawSetColor(COLOR_R(color), COLOR_G(color), COLOR_B(color), COLOR_A(color));
	g_pSurface->DrawPolyLine(&vertice_x[0], &vertice_y[0], vertice_x.size());
}

void drawing::DrawFillCircle(int x, int y, int radius, DWORD color, int resolution)
{
	
}

void drawing::DrawString(int x, int y, DWORD color, FontRenderFlag_t alignment, const char * text, ...)
{
	va_list ap;
	char buffer_oigin[1024];
	va_start(ap, text);
	vsprintf_s(buffer_oigin, text, ap);
	va_end(ap);

	wchar_t buffer_output[1024];
	MultiByteToWideChar(CP_UTF8, 0, buffer_oigin, 1024, buffer_output, 1024);

	int width, height;
	g_pSurface->GetTextSize(fontMenu, buffer_output, width, height);
	if (alignment & FONT_RIGHT)
		x -= width;
	if (alignment & FONT_CENTER)
		x -= width / 2;

	g_pSurface->DrawSetTextFont(fontMenu);
	g_pSurface->DrawSetTextColor(COLOR_R(color), COLOR_G(color), COLOR_B(color), COLOR_A(color));
	g_pSurface->DrawSetTextPos(x, y - height / 2);
	g_pSurface->DrawPrintText(buffer_output, wcslen(buffer_output));
}

void drawing::DrawString(int x, int y, DWORD color, FontRenderFlag_t alignment, const wchar_t * text, ...)
{
	va_list ap;
	wchar_t buffer_output[1024];
	va_start(ap, text);
	int length = vswprintf_s(buffer_output, text, ap);
	va_end(ap);

	int width, height;
	g_pSurface->GetTextSize(fontMenu, buffer_output, width, height);
	if (alignment & FONT_RIGHT)
		x -= width;
	if (alignment & FONT_CENTER)
		x -= width / 2;

	g_pSurface->DrawSetTextFont(fontMenu);
	g_pSurface->DrawSetTextColor(COLOR_R(color), COLOR_G(color), COLOR_B(color), COLOR_A(color));
	g_pSurface->DrawSetTextPos(x, y - height / 2);
	g_pSurface->DrawPrintText(buffer_output, length);
}

void drawing::DrawText(int x, int y, DWORD color, FontRenderFlag_t alignment, const char * text, ...)
{
	va_list ap;
	char buffer_oigin[1024];
	va_start(ap, text);
	vsprintf_s(buffer_oigin, text, ap);
	va_end(ap);

	wchar_t buffer_output[1024];
	MultiByteToWideChar(CP_UTF8, 0, buffer_oigin, 1024, buffer_output, 1024);

	int width, height;
	g_pSurface->GetTextSize(fontEsp, buffer_output, width, height);
	if (alignment & FONT_RIGHT)
		x -= width;
	if (alignment & FONT_CENTER)
		x -= width / 2;

	g_pSurface->DrawSetTextFont(fontEsp);
	g_pSurface->DrawSetTextColor(COLOR_R(color), COLOR_G(color), COLOR_B(color), COLOR_A(color));
	g_pSurface->DrawSetTextPos(x, y - height / 2);
	g_pSurface->DrawPrintText(buffer_output, wcslen(buffer_output));
}

void drawing::DrawText(int x, int y, DWORD color, FontRenderFlag_t alignment, const wchar_t * text, ...)
{
	va_list ap;
	wchar_t buffer_output[1024];
	va_start(ap, text);
	int length = vswprintf_s(buffer_output, text, ap);
	va_end(ap);

	int width, height;
	g_pSurface->GetTextSize(fontEsp, buffer_output, width, height);
	if (alignment & FONT_RIGHT)
		x -= width;
	if (alignment & FONT_CENTER)
		x -= width / 2;

	g_pSurface->DrawSetTextFont(fontEsp);
	g_pSurface->DrawSetTextColor(COLOR_R(color), COLOR_G(color), COLOR_B(color), COLOR_A(color));
	g_pSurface->DrawSetTextPos(x, y - height / 2);
	g_pSurface->DrawPrintText(buffer_output, length);
}
