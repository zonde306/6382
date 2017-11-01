#include <Windows.h>
#include <wingdi.h>
#include <iostream>
#include <gl\gl.h>
#include <gl\glu.h>
#include "glaux.h"
#include "glext.h"
#include <tchar.h>

#include "opengl.h"
#include "menu.h"
#include "cvars.h"
#include "gamehooking.h"
#include "players.h"
// #include "detours.h"
#include "Misc/engine.h"
#include "misc/splice.h"
#include "./detours/detourxs.h"
#include "./clientdll.h"
#include "./Misc/xorstr.h"

extern cl_enginefunc_t g_Engine;
#pragma comment(lib,"OpenGL32.lib")
#pragma comment(lib,"GLu32.lib")
#pragma comment(lib,"Gdi32.lib")

//////////////////////////////////////////////////////////////////////////
extern "C" void APIENTRY wglSwapBuffers(HDC dc);
extern "C" GLint __stdcall ogluProject(GLdouble objX, GLdouble objY, GLdouble objZ,
	const GLdouble *model, const GLdouble *proj, const GLint *view,
	GLdouble* winX, GLdouble* winY, GLdouble* winZ);

/*
DETOUR_TRAMPOLINE(void APIENTRY glBegin_detour(GLenum mode), glBegin);
DETOUR_TRAMPOLINE(void APIENTRY glEnd_detour(), glEnd);
DETOUR_TRAMPOLINE(void APIENTRY glClear_detour(GLbitfield mask), glClear);
DETOUR_TRAMPOLINE(void APIENTRY glVertex3fv_detour(const GLfloat *v), glVertex3fv);
DETOUR_TRAMPOLINE(void APIENTRY glVertex3f_detour(GLfloat x, GLfloat y, GLfloat z), glVertex3f);
DETOUR_TRAMPOLINE(void APIENTRY glEnable_detour(GLenum cap), glEnable);
DETOUR_TRAMPOLINE(void APIENTRY glVertex2f_detour(GLfloat x, GLfloat y), glVertex2f);
DETOUR_TRAMPOLINE(void APIENTRY glViewport_detour(GLint x, GLint y, GLsizei width, GLsizei height), glViewport);
DETOUR_TRAMPOLINE(void APIENTRY wglSwapBuffers_detour(HDC dc), wglSwapBuffers);
DETOUR_TRAMPOLINE(void APIENTRY glBlendFunc_detour(GLenum sfactor, GLenum dfactor), glBlendFunc);
DETOUR_TRAMPOLINE(void APIENTRY glPopMatrix_detour(void), glPopMatrix);
DETOUR_TRAMPOLINE(void APIENTRY glTranslatef_detour(GLfloat x, GLfloat y, GLfloat z), glTranslatef);
DETOUR_TRAMPOLINE(void APIENTRY glPushMatrix_detour(void), glPushMatrix);
*/

// typedef void(APIENTRY* FnglBegin)(GLenum);
// typedef void(APIENTRY* FnglEnd)();
typedef void(APIENTRY* FnglClear)(GLbitfield);
typedef void(APIENTRY* FnglVertex3fv)(const GLfloat*);
typedef void(APIENTRY* FnglVertex3f)(GLfloat, GLfloat, GLfloat);
// typedef void(APIENTRY* FnglEnable)(GLenum);
typedef void(APIENTRY* FnglVertex2f)(GLfloat, GLfloat);
typedef void(APIENTRY* FnglViewport)(GLint, GLint, GLsizei, GLsizei);
typedef void(APIENTRY* FnwglSwapBuffers)(HDC);
// typedef void(APIENTRY* FnglBlendFunc)(GLenum, GLenum);
// typedef void(APIENTRY* FnglPopMatrix)();
typedef void(APIENTRY* FnglTranslatef)(GLfloat, GLfloat, GLfloat);
// typedef void(APIENTRY* FnglPushMatrix)();
// typedef void(APIENTRY* FnglReadPixels)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid*);
// typedef BOOL(__stdcall* FnBitBlt)(HDC, int, int, int, int, HDC, int, int, DWORD);

FnglBegin glBegin_detour;
FnglEnd glEnd_detour;
FnglClear glClear_detour;
FnglVertex3fv glVertex3fv_detour;
FnglVertex3f glVertex3f_detour;
FnglEnable glEnable_detour;
FnglVertex2f glVertex2f_detour;
FnglViewport glViewport_detour;
FnwglSwapBuffers wglSwapBuffers_detour;
FnglBlendFunc glBlendFunc_detour;
FnglPopMatrix glPopMatrix_detour;
FnglTranslatef glTranslatef_detour;
FnglPushMatrix glPushMatrix_detour;
FnglReadPixels glReadPixels_detour;
FnBitBlt BitBlt_detour;

ENTITIES g_playerDrawList[33] = { NULL };

float fCamera[3];
float fLastPosition[3];
float fHighestVertex[3];
float fLowestVertex[3];
float fCenterVertex[3];

int viewport[4];
int matrixDepth = 0;
int vertexCount3f = 0;
int maxEnts = 0;
double the_Double = 0.5000000000000000;

bool found_An_Entity = FALSE;



bool oglSubtractive = false;
bool bDrawingScope = false;
bool bDrawnWorld = false;
bool bStartedDrawingEnts = false;
bool bSkyTex = false;
bool bDrawingFlash = false;
bool bDrawingSmoke = false;

bool bSmoke = false;
bool bFlash = false;
bool bCrosshairDrawn = false;
bool bSky = false;
GLfloat coords[4];
//////////////////////////////////////////////////////////////////////////

float WINAPI GetDistance(float * LocationA, float * LocationB)
{
	float DistanceX = LocationA[0] - LocationB[0];
	float DistanceY = LocationA[1] - LocationB[1];
	float DistanceZ = LocationA[2] - LocationB[2];

	return (float)sqrt((DistanceX * DistanceX) + (DistanceY * DistanceY) + (DistanceZ * DistanceZ));
}

void InitMenu() {
	g_oglMenu.count = 0;
	g_oglMenu.active = false;
	g_oglDraw.menu = false;
}

void MenuUp()
{
	if (g_oglMenu.count < 1)
		g_oglMenu.count = g_oglMenu.maxcount - 1;
	else
		g_oglMenu.count--;
}

void MenuDown()
{
	if (g_oglMenu.count > g_oglMenu.maxcount - 2)
		g_oglMenu.count = 0;
	else
		g_oglMenu.count++;
}

void MenuSelect()
{
	switch (g_oglMenu.count)
	{
	case 0:
		cvars.dmAimbot = !cvars.dmAimbot;
		break;

	case 1:
		cvars.aimthrough = !cvars.aimthrough;
		break;

	case 2:
		cvars.norecoil = !cvars.norecoil;
		break;

	case 3:
		cvars.nospread = !cvars.nospread;
		break;

	case 4:
		cvars.wallhack = !cvars.wallhack;
		break;

	case 5:
		cvars.smokeRemoval = !cvars.smokeRemoval;
		break;

	case 6:
		cvars.flashRemoval = !cvars.flashRemoval;
		break;

	case 7:
		cvars.skyRemoval = !cvars.skyRemoval;
		break;

	case 8:
		cvars.lambert = !cvars.lambert;
		break;

	case 9:
		cvars.whiteWalls = !cvars.whiteWalls;
		break;

	case 10:
		cvars.transparentWalls = !cvars.transparentWalls;
		break;

	case 11:
		cvars.wireframe = !cvars.wireframe;
		if (cvars.wireframe)
			cvars.skyRemoval = true;
		else
			cvars.skyRemoval = false;
		break;

	case 12:
		cvars.wireframeModels = !cvars.wireframeModels;
		break;

	case 13:
		cvars.hud = !cvars.hud;
		break;

	case 14:
		cvars.bunnyhop = !cvars.bunnyhop;
		break;

	case 15:
		cvars.rapidfire = !cvars.rapidfire;
		break;

	case 16:
		cvars.triggerbot = !cvars.triggerbot;
		break;

	case 17:
		cvars.openglbox = !cvars.openglbox;
		break;

	case 18:
		cvars.fastrun = !cvars.fastrun;
		break;

	case 19:
		cvars.fastwalk = !cvars.fastwalk;
		break;

	case 20:
		cvars.speedhack = !cvars.speedhack;
		break;

	case 21:
		cvars.antiaim = !cvars.antiaim;
		break;

	case 22:
		cvars.boneesp = !cvars.boneesp;
		break;

	case 23:
		cvars.radar = !cvars.radar;
		break;

	case 24:
		cvars.miniradar = !cvars.miniradar;
		break;

	case 25:
		cvars.quake = !cvars.quake;
		break;

	case 26:
		cvars.barrel = !cvars.barrel;
		break;

	case 27:
		cvars.entityesp = !cvars.entityesp;
		break;

	case 28:
		cvars.autostrafe = !cvars.autostrafe;
		break;

	}
}

void glPrint(float x, float y, float z, float r, float g, float b, const char *fmt, ...)
{
	char text[256];
	va_list	ap;
	if (fmt == NULL)
		return;
	va_start(ap, fmt);
	vsprintf(text, fmt, ap);
	va_end(ap);

	GLfloat  curcolor[4], position[4];
	(*glPushAttrib)(GL_ALL_ATTRIB_BITS);
	(*glGetFloatv)(GL_CURRENT_COLOR, curcolor);
	(*glGetFloatv)(GL_CURRENT_RASTER_POSITION, position);

	(*glDisable)(GL_TEXTURE_2D);
	(*glColor4f)(r, g, b, 1.0f);
	(*glRasterPos3f)(x, y, z);
	(*glPushAttrib)(GL_LIST_BIT);
	(*glListBase)(g_oglBase - 32);
	(*glCallLists)(strlen(text), GL_UNSIGNED_BYTE, text);
	(*glPopAttrib)();
	(*glEnable)(GL_TEXTURE_2D);

	(*glPopAttrib)();
	(*glColor4fv)(curcolor);
	(*glRasterPos2f)(position[0], position[1]);
}

GLvoid BuildFont(GLvoid)
{
	g_hOglHdc = wglGetCurrentDC();
	HFONT	font;
	HFONT	oldfont;
	g_oglBase = (*glGenLists)(96);
	font = CreateFont(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, FF_DONTCARE | DEFAULT_PITCH, "Tahoma");
	oldfont = (HFONT)SelectObject(g_hOglHdc, font);
	wglUseFontBitmaps(g_hOglHdc, 32, 96, g_oglBase);
	SelectObject(g_hOglHdc, oldfont);
	DeleteObject(font);
}

void DrawMenu(int x, int y)
{
	char Entry[29][30];
	int c = 0;

	sprintf_s(Entry[c], "DM-Aimbot  %s", cvars.dmAimbot ? "On" : "Off"); c++;
	sprintf_s(Entry[c], "Aimthrough  %s", cvars.aimthrough ? "On" : "Off"); c++;
	sprintf_s(Entry[c], "No Recoil  %s", cvars.norecoil ? "On" : "Off"); c++;
	sprintf_s(Entry[c], "No Spread  %s", cvars.nospread ? "On" : "Off"); c++;

	sprintf_s(Entry[c], "Wallhack  %s", cvars.wallhack ? "On" : "Off"); c++;
	sprintf_s(Entry[c], "Smoke Removal  %s", cvars.smokeRemoval ? "On" : "Off"); c++;
	sprintf_s(Entry[c], "Flashbang Removal  %s", cvars.flashRemoval ? "On" : "Off"); c++;
	sprintf_s(Entry[c], "Sky Removal  %s", cvars.skyRemoval ? "On" : "Off"); c++;
	sprintf_s(Entry[c], "Lambert  %s", cvars.lambert ? "On" : "Off"); c++;
	sprintf_s(Entry[c], "White Walls  %s", cvars.whiteWalls ? "On" : "Off"); c++;
	sprintf_s(Entry[c], "Transparent Walls  %s", cvars.transparentWalls ? "On" : "Off"); c++;
	sprintf_s(Entry[c], "Wireframe  %s", cvars.wireframe ? "On" : "Off"); c++;
	sprintf_s(Entry[c], "Wireframe Models  %s", cvars.wireframeModels ? "On" : "Off"); c++;
	sprintf_s(Entry[c], "New HUD  %s", cvars.hud ? "On" : "Off"); c++;
	sprintf_s(Entry[c], "Bunnyhop  %s", cvars.bunnyhop ? "On" : "Off"); c++;
	sprintf_s(Entry[c], "Auto Pistol %s", cvars.rapidfire ? "On" : "Off"); c++;
	sprintf_s(Entry[c], "Trigger Bot %s", cvars.triggerbot ? "On" : "Off"); c++;
	sprintf_s(Entry[c], "OpenGL BoxEsp %s", cvars.openglbox ? "On" : "Off"); c++;
	sprintf_s(Entry[c], "Fast Run %s", cvars.fastrun ? "On" : "Off"); c++;
	sprintf_s(Entry[c], "Fast Walk %s", cvars.fastwalk ? "On" : "Off"); c++;
	sprintf_s(Entry[c], "Speed Hack %s", cvars.speedhack ? "On" : "Off"); c++;
	sprintf_s(Entry[c], "Anti Aim %s", cvars.antiaim ? "On" : "Off"); c++;
	sprintf_s(Entry[c], "BoneEsp %s", cvars.boneesp ? "On" : "Off"); c++;
	sprintf_s(Entry[c], "Radar %s", cvars.radar ? "On" : "Off"); c++;
	sprintf_s(Entry[c], "Mini Radar %s", cvars.miniradar ? "On" : "Off"); c++;
	sprintf_s(Entry[c], "Quake Gun %s", cvars.quake ? "On" : "Off"); c++;
	sprintf_s(Entry[c], "Player Barrel %s", cvars.barrel ? "On" : "Off"); c++;
	sprintf_s(Entry[c], "Entity ESP %s", cvars.entityesp ? "On" : "Off"); c++;
	sprintf_s(Entry[c], "AutoStrafe %s", cvars.autostrafe ? "On" : "Off"); c++;

	g_oglMenu.maxcount = c;


	glPrint(x, y - 28, 1.0f, 0.5f, 0.5f, 1.0f, "-------------------------------------");
	glPrint(x, y - 17, 1.0f, 0.5f, 0.5f, 1.0f, "-       XxharCs Multihack        -");
	glPrint(x, y - 6, 1.0f, 0.5f, 0.5f, 1.0f, "-------------------------------------");


	for (int i = 0; i < g_oglMenu.maxcount; i++)
	{
		if (g_oglMenu.count == i)
			glPrint(x, y + (14 * i) + 5, 1.0f, 1.0f, 0.2f, 0.2f, Entry[i]);
		else
			glPrint(x, y + (14 * i) + 5, 1.0f, 1.0f, 1.0f, 1.0f, Entry[i]);
	}
}

//////////////////////////////////////////////////////////////////////////
void APIENTRY hooked_glBegin(GLenum mode)
{
	// entity g_oglDraw start
	if (!bStartedDrawingEnts && bDrawnWorld && cvars.wallhack > 1) {
		if (mode == GL_TRIANGLE_STRIP || mode == GL_TRIANGLE_FAN) {
			bStartedDrawingEnts = true;
			(*glClear_detour)(GL_DEPTH_BUFFER_BIT);
		}
	}

	if (cvars.dmAimbot && (mode == GL_TRIANGLE_STRIP || mode == GL_TRIANGLE_FAN) && found_An_Entity == FALSE)
	{
		found_An_Entity = TRUE;
	}

	if (cvars.dmAimbot && (mode == GL_POLYGON || mode == GL_TRIANGLES || mode == GL_QUADS || mode == GL_QUAD_STRIP) && found_An_Entity == TRUE)
	{
		found_An_Entity = FALSE;
	}


	if (cvars.wallhack)
	{
		if (mode == GL_TRIANGLES || mode == GL_TRIANGLE_STRIP || mode == GL_TRIANGLE_FAN)
		{
			glDepthRange(0, 0.5);
		}
		else
		{
			glDepthRange(0.5, 1);
		}
	}

	if (cvars.smokeRemoval)
	{
		if (mode == GL_QUADS)
		{
			GLfloat smokecol[4];
			(*glGetFloatv)(GL_CURRENT_COLOR, smokecol);

			if ((smokecol[0] == smokecol[1]) && (smokecol[0] == smokecol[2]) && (smokecol[0] != 0.0) && (smokecol[0] != 1.0))
			{
				bSmoke = true;
			}
			else
			{
				bSmoke = false;
			}
		}
	}

	if (cvars.flashRemoval)
	{
		if (mode == GL_QUADS)
		{
			GLfloat flashcol[4];
			(*glGetFloatv)(GL_CURRENT_COLOR, flashcol);

			if ((flashcol[0] == 1.0) && (flashcol[1] == 1.0) && (flashcol[2] == 1.0))
			{
				(*glGetFloatv)(GL_VIEWPORT, coords);
				bFlash = true;
			}
			else
			{
				bFlash = false;
			}
		}
	}

	if (cvars.skyRemoval)
	{
		if (mode == GL_QUADS)
		{
			bSky = true;
		}
		else
		{
			bSky = false;
		}
	}

	if (cvars.transparentWalls)
	{
		if (mode == GL_TRIANGLES || mode == GL_TRIANGLE_STRIP || mode == GL_TRIANGLE_FAN || mode == GL_QUADS)
		{
			GLfloat curcol[4];
			(*glGetFloatv)(GL_CURRENT_COLOR, curcol);

			glDisable(GL_DEPTH_TEST);

			if(glEnable_detour != nullptr)
				glEnable_detour(GL_BLEND);
			else
				glEnable(GL_BLEND);

			(*glBlendFunc_detour)(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glColor4f(curcol[0], curcol[1], curcol[2], 0.5f);
		}
	}

	if (cvars.whiteWalls)
	{
		if (mode == GL_POLYGON)
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
		}
	}

	if (cvars.wireframe)
	{
		if (mode == GL_POLYGON)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glLineWidth(1.0);
			glColor3f(0.0f, 0.7f, 0.0f);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}

	if (cvars.wireframeModels)
	{
		if (mode == GL_TRIANGLE_STRIP || mode == GL_TRIANGLE_FAN)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glLineWidth(1.0);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}

	if (glBegin_detour)
		(*glBegin_detour)(mode);
}

//////////////////////////////////////////////////////////////////////////
void APIENTRY hooked_glClear(GLbitfield mask)
{
	if (mask == GL_DEPTH_BUFFER_BIT && (cvars.wallhack || cvars.skyRemoval))
	{
		(*glClear_detour)(GL_COLOR_BUFFER_BIT);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	}

	(*glClear_detour)(mask);
}

//////////////////////////////////////////////////////////////////////////
void APIENTRY hooked_glVertex3fv(const GLfloat *v)
{
	if (bSmoke || ((cvars.wallhack || cvars.skyRemoval) && bSky && v[2] > 3000.0f))
		return;

	(*glVertex3fv_detour)(v);
}

int GetTeam(char * model)
{
	// Check all terror team models
	if (strstr(model, "arctic") ||
		strstr(model, "guerilla") ||
		strstr(model, "leet") ||
		strstr(model, "militia") ||
		strstr(model, "terror"))
		return 1;

	// Check all counter team models
	else if (strstr(model, "gign") ||
		strstr(model, "gsg9") ||
		strstr(model, "sas") ||
		strstr(model, "urban") ||
		strstr(model, "spetsnaz") ||
		strstr(model, "vip"))
		return 2;

	// No Team / Spec / ...
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void APIENTRY xwglSwapBuffers(HDC hDC)
{
	bDrawnWorld = false;
	bStartedDrawingEnts = false;
	g_iOglViewportCount = 0;

	if (cvars.dmAimbot || cvars.openglbox)
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		glOrtho(0, viewport[2], viewport[3], 0, 0, 1);
		glDisable(GL_DEPTH_TEST);
		glMatrixMode(GL_MODELVIEW);

		if (glPushMatrix_detour != nullptr)
			glPushMatrix_detour();
		else
			glPushMatrix();

		glLoadIdentity();
		glDisable(GL_TEXTURE_2D);

		if(glTranslatef_detour != nullptr)
			glTranslatef_detour(0.375f, 0.375f, 0.0f);
		else
			glTranslatef(0.375f, 0.375f, 0.0f);

		if(glEnable_detour != nullptr)
			glEnable_detour(GL_BLEND);
		else
			glEnable(GL_BLEND);

		if(glBlendFunc_detour != nullptr)
			glBlendFunc_detour(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		else
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		/*
		hud_player_info_t pInfo;
		char data[128];
		*/

		for (int i = 0; i < maxEnts; i++)
		{
			/*
			gEngfuncs.pfnGetPlayerInfo(i, &pInfo);
			int iTeam = GetTeam(pInfo.model);

			sprintf_s(data, "[%d]", iTeam);
			gEngfuncs.pfnDrawConsoleString(10, 10+i*20, data);
			*/

			/*
			if (g_playerList[i].isUpdated() && g_playerList[i].isAlive())
			{
				sprintf_s(data, "%s, %f meters, [%s], [%d]", g_playerList[i].getName(),
					g_playerList[i].distance, g_playerList[i].getEnt()->model->name, iTeam);
				
				gEngfuncs.pfnDrawConsoleString(10, 10+i*20, data);
				//drawPlayerEsp(i);
			}
			*/

			/*
			if(!bIsEntValid(g_playerList[i].getEnt(), i))
				g_playerList[i].updateClear();
			*/

			if (cvars.openglbox)
			{
				float x = (float)g_playerDrawList[i].Head.x;
				float y = (float)g_playerDrawList[i].Head.y;
				float w = 1;
				float h = g_playerDrawList[i].BoxHeight;
				UCHAR r, g, b, a;

				if (g_playerDrawList[i].Visible)
				{
					r = 20; g = 255; b = 20; a = 200;
				}
				else
				{
					r = 255; g = 255; b = 20; a = 200;
				}

				glColor4ub(r, g, b, a);

				if(glBegin_detour != nullptr)
					glBegin_detour(GL_QUADS);
				else
					glBegin(GL_QUADS);

				if (glVertex2f_detour != nullptr)
				{
					glVertex2f_detour(x - h - w, y - h - w);
					glVertex2f_detour(x + h + w, y - h - w);
					glVertex2f_detour(x + h + w, y - h);
					glVertex2f_detour(x - h - w, y - h);

					glVertex2f_detour(x - h - w, y + h - w);
					glVertex2f_detour(x + h + w, y + h - w);
					glVertex2f_detour(x + h + w, y + h);
					glVertex2f_detour(x - h - w, y + h);

					glVertex2f_detour(x - h - w, y - h);
					glVertex2f_detour(x - h, y - h);
					glVertex2f_detour(x - h, y + h);
					glVertex2f_detour(x - h - w, y + h);

					glVertex2f_detour(x + h, y - h);
					glVertex2f_detour(x + h + w, y - h);
					glVertex2f_detour(x + h + w, y + h);
					glVertex2f_detour(x + h, y + h);
				}
				else
				{
					glVertex2f(x - h - w, y - h - w);
					glVertex2f(x + h + w, y - h - w);
					glVertex2f(x + h + w, y - h);
					glVertex2f(x - h - w, y - h);

					glVertex2f(x - h - w, y + h - w);
					glVertex2f(x + h + w, y + h - w);
					glVertex2f(x + h + w, y + h);
					glVertex2f(x - h - w, y + h);

					glVertex2f(x - h - w, y - h);
					glVertex2f(x - h, y - h);
					glVertex2f(x - h, y + h);
					glVertex2f(x - h - w, y + h);

					glVertex2f(x + h, y - h);
					glVertex2f(x + h + w, y - h);
					glVertex2f(x + h + w, y + h);
					glVertex2f(x + h, y + h);
				}

				if (glEnd_detour != nullptr)
					glEnd_detour();
				else
					glEnd();
			}
		}

		if (cvars.dmAimbot && (GetAsyncKeyState(VK_LBUTTON) & 0x8000))
		{
			float fClosestPos = 999999.0f;
			DWORD TargetX = 0;
			DWORD TargetY = 0;

			for (int i = 0; i < maxEnts; i++)
			{
				float fTemp[3];
				float ScrCenter[3];

				ScrCenter[0] = float(viewport[2] / 2);
				ScrCenter[1] = float(viewport[3] / 2);
				ScrCenter[2] = 0.0f;

				fTemp[0] = (float)g_playerDrawList[i].Head.x;
				fTemp[1] = (float)g_playerDrawList[i].Head.y;
				fTemp[2] = 0.0f;

				float nDist = GetDistance(fTemp, ScrCenter);

				if ((cvars.aimthrough || (!cvars.aimthrough && g_playerDrawList[i].Visible)))
				{
					if (nDist < fClosestPos)
					{
						fClosestPos = nDist;
						TargetX = g_playerDrawList[i].Head.x;
						TargetY = g_playerDrawList[i].Head.y;
					}
				}
			}
			
			if (TargetX && TargetY)
			{
				POINT Target;
				DWORD ScrX = viewport[2] / 2;
				DWORD ScrY = viewport[3] / 2;

				if (TargetX > (ScrX)) {
					Target.x = TargetX - (ScrX);
					Target.x /= 4;
					Target.x = +Target.x;
				}

				if (TargetX < (ScrX)) {
					Target.x = (ScrX)-TargetX;
					Target.x /= 4;
					Target.x = -Target.x;
				}

				if (TargetX == (ScrX)) {
					Target.x = 0;
				}

				if (TargetY > (ScrY)) {
					Target.y = TargetY - (ScrY);
					Target.y /= 4;
					Target.y = +Target.y;
				}

				if (TargetY < (ScrY)) {
					Target.y = (ScrY)-TargetY;
					Target.y /= 4;
					Target.y = -Target.y;
				}

				if (TargetY == (ScrY)) {
					Target.y = 0;
				}

				mouse_event(MOUSEEVENTF_MOVE, Target.x, Target.y, NULL, NULL);
			}
		}

		glDisable(GL_BLEND);

		if (glPopMatrix_detour != nullptr)
			glPopMatrix_detour();
		else
			glPopMatrix();

		if(glEnable_detour != nullptr)
			glEnable_detour(GL_TEXTURE_2D);
		else
			glEnable(GL_TEXTURE_2D);

		maxEnts = 0;
	}


	(*wglSwapBuffers_detour)(hDC);

}

//////////////////////////////////////////////////////////////////////////
void APIENTRY hooked_glEnd()
{
	(*glEnd_detour)();
}

//////////////////////////////////////////////////////////////////////////
void APIENTRY hooked_glVertex2f(GLfloat x, GLfloat y)
{
	if (bFlash == true)
	{
		if (y == coords[3])
		{
			GLfloat fcurcol[4];
			glGetFloatv(GL_CURRENT_COLOR, fcurcol);
			glColor4f(fcurcol[0], fcurcol[1], fcurcol[2], 0.01f);
		}
	}

	(*glVertex2f_detour)(x, y);
}

//////////////////////////////////////////////////////////////////////////
void APIENTRY hooked_glVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
	if (cvars.lambert)
	{
		glColor3f(1.0f, 1.0f, 1.0f);
	}

	(*glVertex3f_detour)(x, y, z);

	if (cvars.dmAimbot || cvars.openglbox)
	{
		vertexCount3f++;

		fLastPosition[0] = x;
		fLastPosition[1] = y;
		fLastPosition[2] = z;

		fCenterVertex[0] += x;
		fCenterVertex[1] += y;
		fCenterVertex[2] += z;

		if (z > fHighestVertex[2] || vertexCount3f == 1)
		{
			fHighestVertex[0] = fLastPosition[0];
			fHighestVertex[1] = fLastPosition[1];
			fHighestVertex[2] = fLastPosition[2];
		}

		if (z < fLowestVertex[2] || vertexCount3f == 1)
		{
			fLowestVertex[0] = fLastPosition[0];
			fLowestVertex[1] = fLastPosition[1];
			fLowestVertex[2] = fLastPosition[2];
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void APIENTRY hooked_glEnable(GLenum cap)
{
	/*
	if (bCrosshairDrawn == false)
	{
		glPushMatrix();
		glLoadIdentity();


		GLint iDim[4];
		(*glGetIntegerv)(GL_VIEWPORT, iDim);
		glColor4f(1.0f, 0.0f, 1.0f, 1.0f);
		glLineWidth(3.0f);

		glBegin(GL_LINES);
		glVertex2i(iDim[2] / 2, (iDim[3] / 2) - 12);
		glVertex2i(iDim[2] / 2, (iDim[3] / 2) - 5);

		glVertex2i(iDim[2] / 2, (iDim[3] / 2) + 5);
		glVertex2i(iDim[2] / 2, (iDim[3] / 2) + 12);

		glVertex2i((iDim[2] / 2) - 12, iDim[3] / 2);
		glVertex2i((iDim[2] / 2) - 5, iDim[3] / 2);

		glVertex2i((iDim[2] / 2) + 5, iDim[3] / 2);
		glVertex2i((iDim[2] / 2) + 12, iDim[3] / 2);
		glEnd();

		glColor3f(1.0f, 0.0f, 0.0f);

		glBegin(GL_POINTS);
		glVertex2i((iDim[2] / 2) - 3, iDim[3] / 2);
		glVertex2i((iDim[2] / 2) + 3, iDim[3] / 2);
		glVertex2i(iDim[2] / 2, (iDim[3] / 2) - 3);
		glVertex2i(iDim[2] / 2, (iDim[3] / 2) + 3);
		glEnd();


		glPopMatrix();
		bCrosshairDrawn = true;
	}
	*/

	if (!g_bOglFirstInit)
	{
		BuildFont();						// L鋎t die Schrift
		(*glGetIntegerv)(GL_VIEWPORT, g_oglVp);	// Speichert Informationen 黚er den Bildschirm in g_oglVp ab.
		g_bOglFirstInit = true;
		InitMenu();
	}

	/*
	if (g_oglDraw.enable)
	{
		g_oglDraw.enable = false;

		if (g_oglDraw.menu)
			DrawMenu(50, (g_oglVp[3] / 2) - 60);
	}
	*/

	(*glEnable_detour)(cap);
}

//////////////////////////////////////////////////////////////////////////
void APIENTRY hooked_glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	bCrosshairDrawn = false;

	if (cvars.dmAimbot || cvars.openglbox)
	{
		viewport[0] = x;
		viewport[1] = y;
		viewport[2] = width;
		viewport[3] = height;
	}

	g_iOglViewportCount++;
	if (g_iOglViewportCount > 4)
		g_oglDraw.enable = true;

	if (g_oglDraw.enable)
	{
		if (GetAsyncKeyState(VK_INSERT) & 0x01)
		{
			g_oglMenu.active = !g_oglMenu.active;
			g_oglDraw.menu = !g_oglDraw.menu;
		}
		else if (g_oglMenu.active)
		{
			if (GetAsyncKeyState(VK_UP) & 0x01)
				MenuUp();
			else if (GetAsyncKeyState(VK_DOWN) & 0x01)
				MenuDown();
			else if ((GetAsyncKeyState(VK_LEFT) & 0x01) || (GetAsyncKeyState(VK_RIGHT) & 0x01))
				MenuSelect();
		}

		/*
		if((GetAsyncKeyState(VK_NUMPAD8) < 0) && (g_iOglLastKey != VK_NUMPAD8) && (g_oglMenu.active))
		{
			MenuUp();
			g_iOglLastKey = VK_NUMPAD8;
		}
		else if((GetAsyncKeyState(VK_NUMPAD5) < 0) && (g_iOglLastKey != VK_NUMPAD5))
		{
			g_oglMenu.active = !g_oglMenu.active;
			g_oglDraw.g_oglMenu = !g_oglDraw.g_oglMenu;
			g_iOglLastKey = VK_NUMPAD5;
		}
		else if((GetAsyncKeyState(VK_NUMPAD4) < 0) && (g_iOglLastKey != VK_NUMPAD4) && (g_oglMenu.active))
		{
			MenuSelect();
			g_iOglLastKey = VK_NUMPAD4;
		}
		else if((GetAsyncKeyState(VK_NUMPAD6) < 0) && (g_iOglLastKey != VK_NUMPAD4) && (g_oglMenu.active))
		{
			MenuSelect();
			g_iOglLastKey = VK_NUMPAD4;
		}
		else if((GetAsyncKeyState(VK_NUMPAD2) < 0) && (g_iOglLastKey != VK_NUMPAD4) && (g_oglMenu.active))
		{
			MenuDown();
			g_iOglLastKey = VK_NUMPAD4;
		}
		else if(!(GetAsyncKeyState(VK_NUMPAD2) < 0) &&
				!(GetAsyncKeyState(VK_NUMPAD6) < 0) &&
				!(GetAsyncKeyState(0x50) < 0) &&
				!(GetAsyncKeyState(VK_NUMPAD8) < 0) &&
				!(GetAsyncKeyState(VK_NUMPAD4) < 0))
					g_iOglLastKey = 0;
		*/
	}

	(*glViewport_detour)(x, y, width, height);
}

//////////////////////////////////////////////////////////////////////////
void APIENTRY hooked_glBlendFunc(GLenum sfactor, GLenum dfactor)
{
	if (cvars.hud && dfactor == GL_ONE)
	{
		glColor3f(0.3f, 0.3f, 1.0f);
	}

	if (oglSubtractive)
	{
		sfactor = GL_SRC_ALPHA;
		dfactor = GL_ONE_MINUS_SRC_ALPHA;
	}

	(*glBlendFunc_detour)(sfactor, dfactor);
}

extern engine_studio_api_t g_Studio;
void APIENTRY hooked_glPopMatrix(void)
{
	cl_entity_t* ent = g_Studio.GetCurrentEntity();
	if (ent != nullptr && ent->player)
	{
		g_playerList[ent->index].bDrawn = true;
	}
	
	(*glPopMatrix_detour)();

	if (cvars.dmAimbot || cvars.openglbox)
	{
		matrixDepth--;

		if (vertexCount3f > 550 && GetDistance(fHighestVertex, fCamera) > 50.0f
			&& (fHighestVertex[2] - fLowestVertex[2]) > 30.0f)
		{
			double ModelView[16];
			double ProjView[16];
			double View2D[3];
			double Depthcheck[3];
			GLfloat x = 0.0f, y = 0.0f, pix;
			FLOAT fVertexH[3];
			FLOAT fVertexL[3];
			FLOAT BoxH[2];
			FLOAT BoxL[2];
			FLOAT Head[2];

			fCenterVertex[0] /= vertexCount3f;
			fCenterVertex[1] /= vertexCount3f;
			fCenterVertex[2] /= vertexCount3f;

			fVertexH[0] = fHighestVertex[0];
			fVertexH[1] = fHighestVertex[1];
			fVertexH[2] = fHighestVertex[2];
			fVertexH[2] += 10.0f;

			fVertexL[0] = fHighestVertex[0];
			fVertexL[1] = fHighestVertex[1];
			fVertexL[2] = fHighestVertex[2];
			fVertexL[2] -= 10.0f;

			g_playerDrawList[maxEnts].Visible = false;

			glGetDoublev(GL_MODELVIEW_MATRIX, ModelView);
			glGetDoublev(GL_PROJECTION_MATRIX, ProjView);

			if (ogluProject(fHighestVertex[0], fHighestVertex[1], fHighestVertex[2] - 4, ModelView, ProjView,
				viewport, &View2D[0], &View2D[1], &View2D[2]) == GL_TRUE)
			{
				Head[0] = (float)View2D[0];
				Head[1] = (int)viewport[3] - (float)View2D[1];

				if (ogluProject(fVertexH[0], fVertexH[1], fVertexH[2], ModelView, ProjView,
					viewport, &View2D[0], &View2D[1], &View2D[2]) == GL_TRUE)
				{
					BoxH[0] = (float)View2D[0];
					BoxH[1] = (int)viewport[3] - (float)View2D[1];

					if (ogluProject(fVertexL[0], fVertexL[1], fVertexL[2], ModelView, ProjView,
						viewport, &View2D[0], &View2D[1], &View2D[2]) == GL_TRUE)
					{
						BoxL[0] = (float)View2D[0];
						BoxL[1] = (int)viewport[3] - (float)View2D[1];

						if (ogluProject(fCenterVertex[0], fCenterVertex[1], fCenterVertex[2], ModelView, ProjView,
							viewport, &View2D[0], &View2D[1], &View2D[2]) == GL_TRUE)
						{
							x = (float)View2D[0];
							y = (int)viewport[3] - (float)View2D[1];

							if (ogluProject(fHighestVertex[0], fHighestVertex[1], fHighestVertex[2] + 1, ModelView, ProjView,
								viewport, &Depthcheck[0], &Depthcheck[1], &Depthcheck[2]) == GL_TRUE)
							{
								if (glReadPixels_detour != nullptr)
								{
									glReadPixels_detour((int)Depthcheck[0], (int)Depthcheck[1], 1, 1,
										GL_DEPTH_COMPONENT, GL_FLOAT, &pix);
								}
								else
								{
									glReadPixels((int)Depthcheck[0], (int)Depthcheck[1], 1, 1,
										GL_DEPTH_COMPONENT, GL_FLOAT, &pix);
								}

								if (pix >= Depthcheck[2])
									g_playerDrawList[maxEnts].Visible = true;

								g_playerDrawList[maxEnts].Origin[0] = fCenterVertex[0];
								g_playerDrawList[maxEnts].Origin[1] = fCenterVertex[1];
								g_playerDrawList[maxEnts].Origin[2] = fCenterVertex[2];

								g_playerDrawList[maxEnts].ESP.x = (LONG)x;
								g_playerDrawList[maxEnts].ESP.y = (LONG)y;

								g_playerDrawList[maxEnts].Head.x = (LONG)Head[0];
								g_playerDrawList[maxEnts].Head.y = (LONG)Head[1];

								float BoxHeight = BoxL[1] - BoxH[1];

								BoxHeight = BoxHeight / 4;

								if (BoxHeight < 4)
									g_playerDrawList[maxEnts].BoxHeight = 3;
								else
									g_playerDrawList[maxEnts].BoxHeight = BoxHeight;

								maxEnts++;
							}
						}
					}
				}
			}
		}
		fCenterVertex[0] = 0.0f;
		fCenterVertex[1] = 0.0f;
		fCenterVertex[2] = 0.0f;
	}
}

void APIENTRY hooked_glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
	if (cvars.dmAimbot || cvars.openglbox)
	{
		if (x != 0.0f && y != 0.0f && z != 0.0f)
		{
			fCamera[0] = -x;
			fCamera[1] = -y;
			fCamera[2] = -z;
		}
	}

	(*glTranslatef_detour)(x, y, z);
}

void APIENTRY hooked_glPushMatrix(void)
{
	(*glPushMatrix_detour)();

	if (cvars.dmAimbot || cvars.openglbox)
	{
		matrixDepth++;
		vertexCount3f = 0;
	}
}

size_t g_iFakeScreenShotLen = 0;
byte* g_pOglFakeScreenShot = nullptr;
std::mutex g_mScreenShotLock;
extern SCREENINFO	screeninfo;

void APIENTRY hooked_glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format,
	GLenum type, GLvoid *pixels)
{
	if (format != GL_RGB || type != GL_UNSIGNED_BYTE)
	{
		glReadPixels_detour(x, y, width, height, format, type, pixels);
		return;
	}

	/*
	byte tmpBuffer[sizeof(cvars)];
	CopyMemory(tmpBuffer, &cvars, sizeof(cvars));
	ZeroMemory(&cvars, sizeof(cvars));

	// 等待刷新
	Sleep(0);

	// 调用原函数
	glReadPixels_detour(x, y, width, height, format, type, pixels);
	gEngfuncs.pfnConsolePrint(XorStr("glReadPixels ScreenShot\n"));

	// 还原
	CopyMemory(&cvars, tmpBuffer, sizeof(cvars));
	*/

	if (g_pOglFakeScreenShot != nullptr && g_iFakeScreenShotLen > 0 && g_mScreenShotLock.try_lock())
	{
		__try
		{
			// 将以前的截图复制到里面
			CopyMemory(pixels, g_pOglFakeScreenShot, g_iFakeScreenShotLen);
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			delete[] g_pOglFakeScreenShot;
			g_iFakeScreenShotLen = 0;
			goto none_antiss;
		}
	}
	else
	{
	none_antiss:
		// 返回一个空的截图资源
		glReadPixels_detour(x, y, width, height, GL_ALPHA, type, pixels);
	}

	g_mScreenShotLock.unlock();
}


BOOL __stdcall hooked_BitBlt(HDC hdcDest, int nXDest, int nYDest, int nWidth, int nHeight, HDC hdcSrc,
	int nXSrc, int nYSrc, DWORD dwRop)
{
	if (!(dwRop & SRCCOPY))
	{
		// 不是截图的就直接无视掉
		return BitBlt_detour(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop);
	}

	/*
	byte tmpBuffer[sizeof(cvars)];
	CopyMemory(tmpBuffer, &cvars, sizeof(cvars));
	ZeroMemory(&cvars, sizeof(cvars));

	// 等待刷新
	Sleep(0);
	
	// 调用原函数
	BOOL result = BitBlt_detour(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop);
	gEngfuncs.pfnConsolePrint(XorStr("BitBlt ScreenShot\n"));

	// 还原
	CopyMemory(&cvars, tmpBuffer, sizeof(cvars));
	*/

	// 随机输出黑色和白色图像
	return BitBlt_detour(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, (rand() % 2 ? WHITENESS : BLACKNESS));
}

extern "C" __declspec(naked)
GLint __stdcall ogluProject(GLdouble objX, GLdouble objY, GLdouble objZ,
	const GLdouble *model, const GLdouble *proj, const GLint *view,
	GLdouble* winX, GLdouble* winY, GLdouble* winZ)
{
	__asm
	{
		MOV EDI, EDI
		PUSH EBP
		MOV EBP, ESP
		PUSH ECX
		SUB ESP, 0x40
		FLD QWORD PTR SS : [EBP + 0x8]
		LEA EAX, DWORD PTR SS : [EBP - 0x40]
		FSTP QWORD PTR SS : [EBP - 0x20]
		PUSH EAX							// /Arg3
		FLD QWORD PTR SS : [EBP + 0x10]			// |
		LEA EAX, DWORD PTR SS : [EBP - 0x20]		// |
		FSTP QWORD PTR SS : [EBP - 0x18]		// |
		PUSH EAX							// |Arg2
		FLD QWORD PTR SS : [EBP + 0x18]			// |
		PUSH DWORD PTR SS : [EBP + 0x20]		// |Arg1
		FSTP QWORD PTR SS : [EBP - 0x10]		// |
		FLD1								// |
		FSTP QWORD PTR SS : [EBP - 0x8]			// |
		CALL PartB							// \glu32.
		LEA EAX, DWORD PTR SS : [EBP - 0x20]
		PUSH EAX							// /Arg3
		LEA EAX, DWORD PTR SS : [EBP - 0x40]		// |
		PUSH EAX							// |Arg2
		PUSH DWORD PTR SS : [EBP + 0x24]		// |Arg1
		CALL PartB							// \glu32.
		FLDZ
		FLD QWORD PTR SS : [EBP - 0x8]
		FUCOM ST(0x1)
		FSTSW AX
		FSTP ST(0x1)
		TEST AH, 0x44
		JPE jmp1
		FSTP ST
		XOR EAX, EAX
		JMP jmp2

	jmp1 :
		FLD QWORD PTR SS : [EBP - 0x20]
		MOV EAX, DWORD PTR SS : [EBP + 0x28]
		FDIV ST, ST(0x1)
		FLD QWORD PTR SS : [EBP - 0x18]
		FDIV ST, ST(0x2)
		FLD QWORD PTR SS : [EBP - 0x10]
		FDIVRP ST(0x3), ST
		FLD QWORD PTR DS : [the_Double]
		FMUL ST(0x2), ST
		FADD ST(0x2), ST
		FMUL ST(0x1), ST
		FADD ST(0x1), ST
		FMUL ST(0x3), ST
		FADDP ST(0x3), ST
		FILD DWORD PTR DS : [EAX + 0x8]
		FMULP ST(0x2), ST
		FILD DWORD PTR DS : [EAX]
		FADDP ST(0x2), ST
		FIMUL DWORD PTR DS : [EAX + 0xC]
		FIADD DWORD PTR DS : [EAX + 0x4]
		MOV EAX, DWORD PTR SS : [EBP + 0x2C]
		FXCH ST(0x1)
		FSTP QWORD PTR DS : [EAX]
		MOV EAX, DWORD PTR SS : [EBP + 0x30]
		FSTP QWORD PTR DS : [EAX]
		MOV EAX, DWORD PTR SS : [EBP + 0x34]
		FSTP QWORD PTR DS : [EAX]
		XOR EAX, EAX
		INC EAX

	jmp2 : LEAVE
		RETN 0x30

	PartB :
		MOV EDI, EDI
		PUSH EBP
		MOV EBP, ESP
		MOV EAX, DWORD PTR SS : [EBP + 0x8]
		MOV ECX, DWORD PTR SS : [EBP + 0xC]
		XOR EDX, EDX
		ADD EAX, 0x40
		PUSH ESI

	jmpa :

		FLD QWORD PTR DS : [EAX - 0x40]
		MOV ESI, DWORD PTR SS : [EBP + 0x10]
		FMUL QWORD PTR DS : [ECX]
		FLD QWORD PTR DS : [EAX - 0x20]
		FMUL QWORD PTR DS : [ECX + 0x8]
		FADDP ST(0x1), ST
		FLD QWORD PTR DS : [EAX]
		ADD EAX, 0x8
		FMUL QWORD PTR DS : [ECX + 0x10]
		FADDP ST(0x1), ST
		FLD QWORD PTR DS : [EAX + 0x18]
		FMUL QWORD PTR DS : [ECX + 0x18]
		FADDP ST(0x1), ST
		FSTP QWORD PTR DS : [ESI + EDX * 0x8]
		INC EDX
		CMP EDX, 0x4
		JL jmpa
		POP ESI
		POP EBP
		RETN 0x0C
	}
}

DetourXS g_glBegin_detour, g_glEnd_detour, g_glClear_detour, g_glEnable_detour, g_glTranslatef_detour,
	g_glVertex3fv_detour, g_glVertex3f_detour, g_glVertex2f_detour, g_glViewport_detour,
	g_wglSwapBuffers_detour, g_glBlendFunc_detour, g_glPopMatrix_detour, g_glPushMatrix_detour,
	g_glReadPixels_detour, g_BitBlt_detour;

//////////////////////////////////////////////////////////////////////////
void InstallGL()
{
	HMODULE ogl = GetModuleHandleA("opengl32.dll");
	if (ogl == nullptr)
		ogl = LoadLibraryA("opengl32.dll");

	g_glBegin_detour.Create(GetProcAddress(ogl, "glBegin"), hooked_glBegin);
	g_glEnd_detour.Create(GetProcAddress(ogl, "glEnd"), hooked_glEnd);
	g_glClear_detour.Create(GetProcAddress(ogl, "glClear"), hooked_glClear);
	g_glEnable_detour.Create(GetProcAddress(ogl, "glEnable"), hooked_glEnable);
	g_glVertex3fv_detour.Create(GetProcAddress(ogl, "glVertex3fv"), hooked_glVertex3fv);
	g_glVertex3f_detour.Create(GetProcAddress(ogl, "glVertex3f"), hooked_glVertex3f);
	g_glVertex2f_detour.Create(GetProcAddress(ogl, "glVertex2f"), hooked_glVertex2f);
	g_glViewport_detour.Create(GetProcAddress(ogl, "glViewport"), hooked_glViewport);
	g_wglSwapBuffers_detour.Create(GetProcAddress(ogl, "wglSwapBuffers"), xwglSwapBuffers);
	g_glBlendFunc_detour.Create(GetProcAddress(ogl, "glBlendFunc"), hooked_glBlendFunc);
	g_glPopMatrix_detour.Create(GetProcAddress(ogl, "glPopMatrix"), hooked_glPopMatrix);
	g_glPushMatrix_detour.Create(GetProcAddress(ogl, "glPushMatrix"), hooked_glPushMatrix);
	g_glTranslatef_detour.Create(GetProcAddress(ogl, "glTranslatef"), hooked_glTranslatef);
	g_glReadPixels_detour.Create(GetProcAddress(ogl, "glReadPixels"), hooked_glReadPixels);
	g_BitBlt_detour.Create(BitBlt, hooked_BitBlt);
	
	glBegin_detour = (FnglBegin)g_glBegin_detour.GetTrampoline();
	glEnd_detour = (FnglEnd)g_glEnd_detour.GetTrampoline();
	glClear_detour = (FnglClear)g_glClear_detour.GetTrampoline();
	glVertex3fv_detour = (FnglVertex3fv)g_glVertex3fv_detour.GetTrampoline();
	glVertex3f_detour = (FnglVertex3f)g_glVertex3f_detour.GetTrampoline();
	glEnable_detour = (FnglEnable)g_glEnable_detour.GetTrampoline();
	glVertex2f_detour = (FnglVertex2f)g_glVertex2f_detour.GetTrampoline();
	glViewport_detour = (FnglViewport)g_glViewport_detour.GetTrampoline();
	wglSwapBuffers_detour = (FnwglSwapBuffers)g_wglSwapBuffers_detour.GetTrampoline();
	glBlendFunc_detour = (FnglBlendFunc)g_glBlendFunc_detour.GetTrampoline();
	glPopMatrix_detour = (FnglPopMatrix)g_glPopMatrix_detour.GetTrampoline();
	glTranslatef_detour = (FnglTranslatef)g_glTranslatef_detour.GetTrampoline();
	glPushMatrix_detour = (FnglPushMatrix)g_glPushMatrix_detour.GetTrampoline();
	glReadPixels_detour = (FnglReadPixels)g_glReadPixels_detour.GetTrampoline();
	BitBlt_detour = (FnBitBlt)g_BitBlt_detour.GetTrampoline();

	/*
	DetourFunctionWithTrampoline((PBYTE)glBegin_detour, (PBYTE)hooked_glBegin);
	DetourFunctionWithTrampoline((PBYTE)glEnd_detour, (PBYTE)hooked_glEnd);
	DetourFunctionWithTrampoline((PBYTE)glClear_detour, (PBYTE)hooked_glClear);
	DetourFunctionWithTrampoline((PBYTE)glEnable_detour, (PBYTE)hooked_glEnable);
	DetourFunctionWithTrampoline((PBYTE)glVertex3fv_detour, (PBYTE)hooked_glVertex3fv);
	DetourFunctionWithTrampoline((PBYTE)glVertex3f_detour, (PBYTE)hooked_glVertex3f);
	DetourFunctionWithTrampoline((PBYTE)glVertex2f_detour, (PBYTE)hooked_glVertex2f);
	DetourFunctionWithTrampoline((PBYTE)glViewport_detour, (PBYTE)hooked_glViewport);
	DetourFunctionWithTrampoline((PBYTE)wglSwapBuffers_detour, (PBYTE)xwglSwapBuffers);
	DetourFunctionWithTrampoline((PBYTE)glBlendFunc_detour, (PBYTE)hooked_glBlendFunc);
	DetourFunctionWithTrampoline((PBYTE)glPopMatrix_detour, (PBYTE)hooked_glPopMatrix);
	DetourFunctionWithTrampoline((PBYTE)glPushMatrix_detour, (PBYTE)hooked_glPushMatrix);
	*/

	/*
	SpliceHookFunction(glBegin_detour, hooked_glBegin);
	SpliceHookFunction(glEnd_detour, hooked_glEnd);
	SpliceHookFunction(glClear_detour, hooked_glClear);
	SpliceHookFunction(glEnable_detour, hooked_glEnable);
	SpliceHookFunction(glVertex3fv_detour, hooked_glVertex3fv);
	SpliceHookFunction(glVertex3f_detour, hooked_glVertex3f);
	SpliceHookFunction(glVertex2f_detour, hooked_glVertex2f);
	SpliceHookFunction(glViewport_detour, hooked_glViewport);
	SpliceHookFunction(wglSwapBuffers_detour, xwglSwapBuffers);
	SpliceHookFunction(glBlendFunc_detour, hooked_glBlendFunc);
	SpliceHookFunction(glPopMatrix_detour, hooked_glPopMatrix);
	SpliceHookFunction(glPushMatrix_detour, hooked_glPushMatrix);
	*/

	g_Engine.pfnConsolePrint("OpenGL Hooked\n");
}
//////////////////////////////////////////////////////////////////////////
void UninstallGL()
{
	if (g_glBegin_detour.Created())
		g_glBegin_detour.Destroy();
	if (g_glEnd_detour.Created())
		g_glEnd_detour.Destroy();
	if (g_glClear_detour.Created())
		g_glClear_detour.Destroy();
	if (g_glEnable_detour.Created())
		g_glEnable_detour.Destroy();
	if (g_glVertex3fv_detour.Created())
		g_glVertex3fv_detour.Destroy();
	if (g_glVertex3f_detour.Created())
		g_glVertex3f_detour.Destroy();
	if (g_glVertex2f_detour.Created())
		g_glVertex2f_detour.Destroy();
	if (g_glViewport_detour.Created())
		g_glViewport_detour.Destroy();
	if (g_wglSwapBuffers_detour.Created())
		g_wglSwapBuffers_detour.Destroy();
	if (g_glBlendFunc_detour.Created())
		g_glBlendFunc_detour.Destroy();
	if (g_glPopMatrix_detour.Created())
		g_glPopMatrix_detour.Destroy();
	if (g_glPushMatrix_detour.Created())
		g_glPushMatrix_detour.Destroy();
	if (g_glReadPixels_detour.Created())
		g_glReadPixels_detour.Destroy();
	if (g_BitBlt_detour.Created())
		g_BitBlt_detour.Destroy();
	
	/*
	DetourRemove((PBYTE)hooked_glBegin, (PBYTE)glBegin_detour);
	DetourRemove((PBYTE)hooked_glEnd, (PBYTE)glEnd_detour);
	DetourRemove((PBYTE)hooked_glClear, (PBYTE)glClear_detour);
	DetourRemove((PBYTE)hooked_glEnable, (PBYTE)glEnable_detour);
	DetourRemove((PBYTE)hooked_glVertex3fv, (PBYTE)glVertex3fv_detour);
	DetourRemove((PBYTE)hooked_glVertex3f, (PBYTE)glVertex3f_detour);
	DetourRemove((PBYTE)hooked_glVertex2f, (PBYTE)glVertex2f_detour);
	DetourRemove((PBYTE)hooked_glViewport, (PBYTE)glViewport_detour);
	DetourRemove((PBYTE)xwglSwapBuffers, (PBYTE)wglSwapBuffers_detour);
	DetourRemove((PBYTE)hooked_glBlendFunc, (PBYTE)glBlendFunc_detour);
	DetourRemove((PBYTE)hooked_glPopMatrix, (PBYTE)glPopMatrix_detour);
	DetourRemove((PBYTE)hooked_glPushMatrix, (PBYTE)glPushMatrix_detour);
	*/
}