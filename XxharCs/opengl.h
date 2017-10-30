#ifndef __OPENGL_H__
#define __OPENGL_H__
#include <gl\gl.h>
#include <mutex>

void InstallGL();
void UninstallGL();
void DrawMenu(int x, int y);

typedef void(APIENTRY* FnglBegin)(GLenum);
typedef void(APIENTRY* FnglEnd)();
typedef void(APIENTRY* FnglBlendFunc)(GLenum, GLenum);
typedef void(APIENTRY* FnglEnable)(GLenum);
typedef void(APIENTRY* FnglPopMatrix)();
typedef void(APIENTRY* FnglPushMatrix)();
typedef void(APIENTRY* FnglReadPixels)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid*);
typedef BOOL(__stdcall* FnBitBlt)(HDC, int, int, int, int, HDC, int, int, DWORD);
extern FnglReadPixels glReadPixels_detour;
extern FnBitBlt BitBlt_detour;
extern FnglBegin glBegin_detour;
extern FnglEnd glEnd_detour;
extern FnglBlendFunc glBlendFunc_detour;
extern FnglEnable glEnable_detour;
extern FnglPopMatrix glPopMatrix_detour;
extern FnglPushMatrix glPushMatrix_detour;

extern byte* g_pOglFakeScreenShot;
extern size_t g_iFakeScreenShotLen;
extern std::mutex g_mScreenShotLock;

#endif