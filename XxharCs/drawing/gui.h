#pragma once
#include "drawing.h"

class cGui
{
public:
	void window(int x, int y, int w, int h, float step, char* title);
	void InitFade(void);
	void FillRGBA(GLfloat x, GLfloat y, int w, int h, UCHAR r, UCHAR g, UCHAR b, UCHAR a);
	void FillRGBA(GLfloat x, GLfloat y, int w, int h, ColorEntry* clr);
	void DrawFadeRGBA(GLfloat x, GLfloat y, int w, int h, int r, int g, int b, int a);
	void DrawFadeRGBA(GLfloat x, GLfloat y, int w, int h, ColorEntry* clr);
	void DrawInfoBoxLeft(void);
	void DrawInfoBoxMode(void);
	void DrawTitleBox(int x, int y, int w, int h, char* title);
	void DrawTitleBox2(int x, int y, int w, int h);
private:
	void DrawContentBox(int x, int y, int w, int h, float step);
	void DrawFade(int x, int y, int w);
	void DrawFade2(int x, int y, int w);
};

extern cGui g_gui;
