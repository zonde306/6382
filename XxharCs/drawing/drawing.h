#pragma once
#include <windows.h>
#include <gl\gl.h>
#include <vector>
#include <string>
#include "../clientdll.h"
#include "../Engine/ISurface.h"

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

#define COLOR_A(_clr)				((_clr & 0xFF000000) >> 24)
#define COLOR_R(_clr)				((_clr & 0x00FF0000) >> 16)
#define COLOR_G(_clr)				((_clr & 0x0000FF00) >> 8)
#define COLOR_B(_clr)				((_clr & 0x000000FF) >> 0)
#define COLOR_ARGB(_a,_r,_g,_b)		((_a << 24)|(_r << 16)|(_g << 8)|(_b << 0))
#define COLOR_RGBA(_r,_g,_b,_a)		COLOR_ARGB(_a,_r,_g,_b)
#define COLOR_RGB(_r,_g,_b)			COLOR_ARGB(255,_r,_g,_b)

namespace drawing
{
	extern vgui::HFont fontEsp, fontMenu;
	extern bool bInPaint;
	
	enum FontRenderFlag_t
	{
		FONT_LEFT = 0,
		FONT_RIGHT = 1,
		FONT_CENTER = 2
	};

	void SetupFonts();
	std::string FindFonts(const std::string & name);
	void DrawLine(int x1, int y1, int x2, int y2, DWORD color);
	void DrawRect(int x, int y, int w, int h, DWORD color);
	void DrawFillRect(int x, int y, int w, int h, DWORD color);
	void DrawCornerRect(int x, int y, int w, int h, int length, DWORD color);
	void DrawCircle(int x, int y, int radius, DWORD color, int resolution = 8);
	void DrawFillCircle(int x, int y, int radius, DWORD color, int resolution = 8);
	void DrawString(int x, int y, DWORD color, FontRenderFlag_t alignment, const char* text, ...);
	void DrawString(int x, int y, DWORD color, FontRenderFlag_t alignment, const wchar_t* text, ...);
	void DrawText(int x, int y, DWORD color, FontRenderFlag_t alignment, const char* text, ...);
	void DrawText(int x, int y, DWORD color, FontRenderFlag_t alignment, const wchar_t* text, ...);
};
