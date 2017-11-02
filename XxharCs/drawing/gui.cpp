#include "gui.h"
#include "drawing.h"
#include "tablefont.h"
#include "../Misc/xorstr.h"
#include "../clientdll.h"
#include "../players.h"

float r[15];
float g[15];
float b[15];
extern int heightenginefont;
extern bool oglSubtractive;
extern cl_enginefuncs_s gEngfuncs;
cGui g_gui;

void cGui::FillRGBA(GLfloat x, GLfloat y, int w, int h, UCHAR r, UCHAR g, UCHAR b, UCHAR a)
{
	oglSubtractive = true;
	gEngfuncs.pfnFillRGBA(x, y, w, h, r, g, b, a);
	oglSubtractive = false;
	/*
	glPushMatrix();
	glLoadIdentity();
	glDisable(GL_TEXTURE_2D);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4ub(r,g,b,a);
	glBegin(GL_QUADS);
	glVertex2f(x,y);
	glVertex2f(x+w,y);
	glVertex2f(x+w,y+h);
	glVertex2f(x,y+h);
	glEnd();
	glDisable(GL_BLEND);

	glPopMatrix();
	glEnable(GL_TEXTURE_2D);
	*/
}

void cGui::FillRGBA(GLfloat x, GLfloat y, int w, int h, ColorEntry* clr)
{
	oglSubtractive = true;
	gEngfuncs.pfnFillRGBA(x, y, w, h, clr->r, clr->g, clr->b, clr->a);
	oglSubtractive = false;
	/*
	glPushMatrix();
	glLoadIdentity();
	glDisable(GL_TEXTURE_2D);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4ub(clr->r,clr->g,clr->b,clr->a);
	glBegin(GL_QUADS);
	glVertex2f(x,y);
	glVertex2f(x+w,y);
	glVertex2f(x+w,y+h);
	glVertex2f(x,y+h);
	glEnd();
	glDisable(GL_BLEND);

	glPopMatrix();
	glEnable(GL_TEXTURE_2D);
	*/
}

void cGui::DrawFadeRGBA(GLfloat x, GLfloat y, int w, int h, int r, int g, int b, int a)
{
	x -= 28;
	y += h;
	for (int i = 0; i<h; i++)
		FillRGBA(x + 2 * i, y - i, w, 1, r, g, b, a);
}

void cGui::DrawFadeRGBA(GLfloat x, GLfloat y, int w, int h, ColorEntry* clr)
{
	x -= 28;
	y += 15;
	for (int i = 0; i<h; i++)
		FillRGBA(x + 2 * i, y - i, w, 1, clr->r, clr->g, clr->b, 180);
	//for(int i=0; i < 15; i++)
	//FillRGBA(x+2*i, y+i, w, 1, clr->r, clr->g, clr->b, 180);
}

void cGui::InitFade(void)
{
	/*	r[0] = 151; g[0] = 201; b[0] = 251;
	r[1] = 146; g[1] = 194; b[1] = 251;
	r[2] = 136;	g[2] = 186; b[2] = 251;
	r[3] = 130;	g[3] = 185;	b[3] = 251;
	r[4] = 128;	g[4] = 184; b[4] = 250;
	r[5] = 125; g[5] = 183; b[5] = 250;
	r[6] = 126;	g[6] = 183;	b[6] = 250;
	r[7] = 115;	g[7] = 177;	b[7] = 249;
	r[8] = 103;	g[8] = 170; b[8] = 249;
	r[9] = 91;	g[9] = 164;	b[9] = 249;
	r[10]= 79;	g[10]= 157;	b[10]= 248;
	r[11]= 55;	g[11]= 144;	b[11]= 248;
	r[12]= 46;	g[12]= 139;	b[12]= 248;
	r[13]= 37;	g[13]= 134;	b[13]= 247;
	r[14]= 23;	g[14]= 126;	b[14]= 247;*/

	for (int i = 0; i < 15; i++)
	{
		r[i] = colorList.get(0)->r;
		g[i] = colorList.get(0)->g - (i * 10);
		b[i] = colorList.get(0)->b - (i * 17);
	}

}

extern SCREENINFO screeninfo;

void Print(int x, int y, int step, const char* fmt, ...)
{
	va_list va_alist;
	char buf[256];

	va_start(va_alist, fmt);
	_vsnprintf(buf, sizeof(buf), fmt, va_alist);
	va_end(va_alist);
	g_tableFont.drawString(false, x, y + step * 11, 255, 255, 255, buf);
}
int ammo = 0;

void cGui::DrawInfoBoxLeft(void)
{
	int x = 10;
	int y = screeninfo.iHeight - 76;
	int w;
	w = 10 + g_tableFont.iGetWidth(XorStr("Kills: %i / Deaths: %i"), g_local.iKills, g_local.iDeaths);
	//int y = screeninfo.iHeight/2;
	window(x, y, w, 66, 3, XorStr("InfoBox"));
	ammo = g_local.iClip;
	if (ammo < 0) ammo = 0;
	
	Print(x, y, 0, XorStr("Health: %i"), g_local.alive ? g_local.iHealth : 0);
	Print(x, y, 1, XorStr("Armor: %i"), g_local.alive ? g_local.iArmor : 0);
	Print(x, y, 2, XorStr("Clip: %i"), g_local.alive ? ammo : 0);
	Print(x, y, 3, XorStr("Killed: %i"), g_local.iKills, g_local.iDeaths);//, g_local.iKills*100/(g_local.iKills+g_local.iDeaths));
	Print(x, y, 4, XorStr("HeadShots: %i"), g_local.iHs);//,g_local.iHs*g_local.iKills/100);
	Print(x, y, 5, XorStr("Money: %i"), g_local.iMoney);
	
}
void cGui::DrawInfoBoxMode(void)
{
	int x = 10;
	int y = screeninfo.iHeight - 130;
	int w;
	w = 10 + g_tableFont.iGetWidth(XorStr("Kills: %i / Deaths: %i"), g_local.iKills, g_local.iDeaths);
	//int y = screeninfo.iHeight/2;
	window(x, y, w, 10, 3, XorStr("Current Mode"));
	
	Print(x, y, 0, XorStr("None Mode"));
}

void cGui::DrawFade(int x, int y, int w)
{
	//x-=28;
	int i;//,r2=0,g2=150,b2=255;
	y += 15;
	for (i = 0; i<15; i++)
		FillRGBA(x, y - i, w, 1, r[i], g[i], b[i], 180);
}
void cGui::DrawFade2(int x, int y, int w)
{
	//x-=28;
	int i;//,r2=0,g2=150,b2=255;
	y += 11;
	for (i = 0; i<11; i++)
		FillRGBA(x, y - i, w, 1, r[i], g[i], b[i], 180);
}
void cGui::DrawTitleBox(int x, int y, int w, int h, char* title)
{
	//x+=28;
	DrawFade(x, y, w);					// Fade Background
	/*FillRGBA(x+2,y,w,1,0,0,0,255);*/FillRGBA(x, y, w, 1, 0, 0, 0, 255); // Outline - Top
	/*DrawFadeRGBA(x,y,3,h,colorList.get(7));*/FillRGBA(x, y, 1, h, 0, 0, 0, 255); // Outline - Left
	/*DrawFadeRGBA(x+w,y,3,h,colorList.get(7));*/FillRGBA(x + w, y, 1, h, 0, 0, 0, 255); // Outline - Right
	FillRGBA(x, y + h, w, 1, 0, 0, 0, 255);
	//whiteBorder(x+4,y+5,8,8);			// White Border

	FillRGBA(x + 5,y + 6, 5, 5, 136, 131, 127, 200); // Box Filling
	blackBorder(x + 5,y + 6, 5, 5); // Black Border
	//FillRGBA(x+5,y+6,5,5, 136, 131, 127, 200); // Box Filling
	//blackBorder(x+5,y+6,5,5);			// Black Border

	// PrintWithFont(x + 18, y + 5, 255, 255, 255, title);// Window Title
	gEngfuncs.pfnDrawSetTextColor(255, 255, 255);
	gEngfuncs.pfnDrawConsoleString(x + 18, y + 15, title);
	
	/*if(cvar.guifont)g_tableFont.drawString(true,x
	//-2
	+w/2,y+5,255,255,255,title);
	else DrawConStringCenter(x
	//-28
	+w/2,y+3,255,255,255,title);*/
}
void cGui::DrawTitleBox2(int x, int y, int w, int h)
{
	//x+=28;
	DrawFade2(x - 1, y - 2, w + 1);					// Fade Background
	/*FillRGBA(x+2,y,w,1,0,0,0,255);*///FillRGBA(x, y, w, 1, 0, 0, 0, 255); // Outline - Top
	/*DrawFadeRGBA(x,y,3,h,colorList.get(7));*///FillRGBA(x, y, 1, h, 0, 0, 0, 255); // Outline - Left
	/*DrawFadeRGBA(x+w,y,3,h,colorList.get(7));*///FillRGBA(x+w,y,1, h, 0, 0, 0, 255); // Outline - Right
	//	FillRGBA(x,y+h,w,1,0,0,0,255);
	//whiteBorder(x+4,y+5,8,8);			// White Border

	//	FillRGBA(x+5,cvar.guifont?y+6:y+(h/3+1),5,cvar.guifont?5:h/3,136,131,127,200); // Box Filling
	//	blackBorder(x+5,cvar.guifont?y+6:y+(h/3+1),5,cvar.guifont?5:h/3); // Black Border
	//FillRGBA(x+5,y+6,5,5, 136, 131, 127, 200); // Box Filling
	//blackBorder(x+5,y+6,5,5);			// Black Border


	/*if(cvar.guifont)g_tableFont.drawString(true,x
	//-2
	+w/2,y+5,255,255,255,title);
	else DrawConStringCenter(x
	//-28
	+w/2,y+3,255,255,255,title);*/
}

void cGui::DrawContentBox(int x, int y, int w, int h, float step)
{
	if (step)
		FillRGBA(x, y, w, h, 64, 64, 64, 200);//FillRGBA(x,y,w,h,136,131,127,200);
	blackBorder(x + 1, y + 1, w - 1, h);		 // Content Box Outlines
}

void cGui::window(int x, int y, int w, int h, float step, char* title)
{
	DrawTitleBox(x - 2, y - (17), w, 15, title); // Title Box
	DrawContentBox(x - 2, y - 2, w, h, step);  // Content Box
}
