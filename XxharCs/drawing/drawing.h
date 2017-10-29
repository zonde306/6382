#pragma once
#include <windows.h>
#include <gl\gl.h>
#include <vector>
#include <string>
#include "../clientdll.h"

struct ColorEntry
{
	int r, g, b, a;
	float onebased_r, onebased_g, onebased_b, onebased_a;
};

class ColorManager
{
public:
	ColorManager() { Init(); }
	ColorEntry * get(int index);
private:
	int currentIndex;
	std::vector<ColorEntry> entrys;
	void add(int r, int g, int b, int a);
	void Init();
};

extern ColorManager colorList;
extern float fCurrentFOV;
extern int	displayCenterX;
extern int	displayCenterY;
extern bool oglSubtractive;
extern SCREENINFO	screeninfo;

int DrawLen(char *fmt);
void DrawLine(int x1, int y1, int x2, int y2, byte r, byte g, byte b, byte a);
void DrawSmoothBox(int x, int y, int width, int height, float ubr, float ubg, float ubb, float lbr, float lbg, float lbb, float ba);
void DrawHudStringCenter(int x, int y, int r, int g, int b, const char *fmt, ...);
void DrawHudString(int x, int y, int r, int g, int b, const char *fmt, ...);
void DrawConStringCenter(int x, int y, int r, int g, int b, const char *fmt, ...);
void DrawConString(int x, int y, int r, int g, int b, const char *fmt, ...);
void gDrawBoxAtScreenXY(int x, int y, int r, int g, int b, int alpha, int radius);
void whiteBorder(int x, int y, int w, int h);
void blackBorder(int x, int y, int w, int h);
void tintArea(int x, int y, int w, int h, struct ColorEntry* clr);
void tintArea(int x, int y, int w, int h, int r, int g, int b, int a);
void colorBorder(int x, int y, int w, int h, int r, int g, int b, int a);
void dddBorder(int x, int y, int w, int h, int a);
void dddBorder2(int x, int y, int w, int h, int a);
void PrintWithFont(int x, int y, int r, int g, int b, const char *fmt, ...);
void PrintWithFont(int x, int y, int r, int g, int b, std::string text);
void DrawCrosshair(int mode);
void gDrawFilledBoxAtLocation(float* origin, DWORD color, int radius);
