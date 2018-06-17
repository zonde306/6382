#include <Windows.h>
#include "../../XxharCs/clientdll.h"

// OpenGL
#include <gl/gl.h>
#pragma comment(lib, "opengl32.lib")

#include "../imgui.h"
#include "imgui_impl_gl.h"

#include "../../XxharCs/Engine/keydefs.h"

// Data
HWND g_Window = NULL;
double g_Time = 0.0;
bool g_MouseJustPressed[3] = { false, false, false };
float g_MouseWheel = 0.0f;
GLuint g_FontTexture = 0;
extern cl_enginefuncs_s gEngfuncs;

// This is the main rendering function that you have to implement and provide to ImGui (via setting up 'RenderDrawListsFn' in the ImGuiIO structure)
void ImGui_ImplGL_RenderDrawLists(ImDrawData* draw_data)
{
	// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
	ImGuiIO& io = ImGui::GetIO();
	int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
	int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
	if (fb_width <= 0 || fb_height <= 0)
		return;
	draw_data->ScaleClipRects(io.DisplayFramebufferScale);

	// We are using the OpenGL fixed pipeline to make the example code simpler to read!
	// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, vertex/texcoord/color pointers, polygon fill.
	GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	GLint last_polygon_mode[2]; glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
	GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
	GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
	glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnable(GL_TEXTURE_2D);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glShadeModel(GL_SMOOTH);

	// Setup viewport, orthographic projection matrix
	glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0f, io.DisplaySize.x, io.DisplaySize.y, 0.0f, -1.0f, +1.0f);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// Render command lists
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		const ImDrawVert* vtx_buffer = cmd_list->VtxBuffer.Data;
		const ImDrawIdx* idx_buffer = cmd_list->IdxBuffer.Data;
		glVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + IM_OFFSETOF(ImDrawVert, pos)));
		glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + IM_OFFSETOF(ImDrawVert, uv)));
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + IM_OFFSETOF(ImDrawVert, col)));

		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback)
			{
				pcmd->UserCallback(cmd_list, pcmd);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
				glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
				glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer);
			}
			idx_buffer += pcmd->ElemCount;
		}
	}

	// Restore modified state
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glBindTexture(GL_TEXTURE_2D, (GLuint)last_texture);
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glPopAttrib();
	glPolygonMode(GL_FRONT, last_polygon_mode[0]); glPolygonMode(GL_BACK, last_polygon_mode[1]);
	glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
	glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
}

//=============
// -button
//    0:left-button
//    1:right-button
//    2:middle-button
// -action
//    1:down 0:up
//=============
void ImGui_ImplGL_MouseButtonCallback(int button, int action)
{
	if (button >= 0 && button < 3)
		g_MouseJustPressed[button] = action > 0 ? true : false;
}

void ImGui_ImplGL_ScrollCallback(double xoffset, double yoffset)
{
	g_MouseWheel += (float)yoffset; // Use fractional mouse wheel.
}

//=============
// -input 
//    key ( engine key )
//    action ( 1:down 0:up )
//=============
void ImGui_ImplGL_KeyCallback(int key, int action)
{
	ImGuiIO& io = ImGui::GetIO();
	if (action == 1)
		io.KeysDown[key] = true;
	if (action == 0)
		io.KeysDown[key] = false;

	io.KeyCtrl = io.KeysDown[K_CTRL];
	io.KeyShift = io.KeysDown[K_SHIFT];
	io.KeyAlt = io.KeysDown[K_ALT];
	//io.KeySuper
}

void ImGui_ImplGL_CharCallback(unsigned int c)
{
	ImGuiIO& io = ImGui::GetIO();
	if (c > 0 && c < 0x10000)
		io.AddInputCharacter((unsigned short)c);
}

bool ImGui_ImplGL_CreateDeviceObjects(void)
{
	// Build texture atlas
	ImGuiIO& io = ImGui::GetIO();
	unsigned char* pixels;
	int width, height;
	// Load as RGBA 32-bits (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	// Upload texture to graphics system
	GLint last_texture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	glGenTextures(1, &g_FontTexture);
	// g_FontTexture = 888888;	// UNDONE: create texture by surface()->CreateNewTextureId()
	glBindTexture(GL_TEXTURE_2D, g_FontTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	// Store our identifier
	io.Fonts->TexID = (void *)(intptr_t)g_FontTexture;

	// Restore state
	glBindTexture(GL_TEXTURE_2D, last_texture);

	return true;
}

void ImGui_ImplGL_InvalidateDeviceObjects(void)
{
	if (g_FontTexture)
	{
		glDeleteTextures(1, &g_FontTexture);	// !!!
		ImGui::GetIO().Fonts->TexID = 0;
		g_FontTexture = 0;
	}
}

bool ImGui_ImplGL_Init(void)
{
	g_Window = GetActiveWindow();

	ImGuiIO& io = ImGui::GetIO();
	// Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
	io.KeyMap[ImGuiKey_Tab] = K_TAB;
	io.KeyMap[ImGuiKey_LeftArrow] = K_LEFTARROW;
	io.KeyMap[ImGuiKey_RightArrow] = K_RIGHTARROW;
	io.KeyMap[ImGuiKey_UpArrow] = K_UPARROW;
	io.KeyMap[ImGuiKey_DownArrow] = K_DOWNARROW;
	io.KeyMap[ImGuiKey_PageUp] = K_PGUP;
	io.KeyMap[ImGuiKey_PageDown] = K_PGDN;
	io.KeyMap[ImGuiKey_Home] = K_HOME;
	io.KeyMap[ImGuiKey_End] = K_END;
	io.KeyMap[ImGuiKey_Delete] = K_DEL;
	io.KeyMap[ImGuiKey_Backspace] = K_BACKSPACE;
	io.KeyMap[ImGuiKey_Enter] = K_ENTER;
	io.KeyMap[ImGuiKey_Escape] = K_ESCAPE;
	io.KeyMap[ImGuiKey_A] = 'a';
	io.KeyMap[ImGuiKey_C] = 'c';
	io.KeyMap[ImGuiKey_V] = 'v';
	io.KeyMap[ImGuiKey_X] = 'x';
	io.KeyMap[ImGuiKey_Y] = 'y';
	io.KeyMap[ImGuiKey_Z] = 'z';

	// Alternatively you can set this to NULL and call ImGui::GetDrawData() after ImGui::Render() to get the same ImDrawData pointer.
	io.RenderDrawListsFn = ImGui_ImplGL_RenderDrawLists;
#ifdef _WIN32
	io.ImeWindowHandle = NULL;
#endif

	// Setup display size (assume the size of the game window is never changed)
	SCREENINFO si = { sizeof(SCREENINFO) };
	gEngfuncs.pfnGetScreenInfo(&si);
	io.DisplaySize = ImVec2((float)si.iWidth, (float)si.iHeight);
	io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

	io.Fonts->AddFontFromFileTTF("msyhul.ttf", 16, NULL, io.Fonts->GetGlyphRangesChinese());

	return true;
}

void ImGui_ImplGL_Shutdown(void)
{
	ImGui_ImplGL_InvalidateDeviceObjects();
	ImGui::Shutdown();
}

void ImGui_ImplGL_NewFrame(void)
{
	if (!g_FontTexture)
		ImGui_ImplGL_CreateDeviceObjects();

	ImGuiIO& io = ImGui::GetIO();

	// Setup display size (every frame to accommodate for window resizing)
	// NOTE: GetScreenInfo() is too slow
//	SCREENINFO si = { sizeof(SCREENINFO) };
//	gEngfuncs.pfnGetScreenInfo(&si);
//	io.DisplaySize = ImVec2((float)si.iWidth, (float)si.iHeight);
//	io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

	// Setup time step
	double current_time = gEngfuncs.GetClientTime();
	io.DeltaTime = g_Time > 0.0 ? (float)(current_time - g_Time) : (float)(1.0f / 60.0f);
	io.DeltaTime = io.DeltaTime > 0.0f ? io.DeltaTime : 0.0f;	// NOTE: ??? Must be greater than zero
	g_Time = current_time;

	if ( GetForegroundWindow() == g_Window )
	{
		// NOTE: the game window is active

		POINT pt;

		if (io.WantMoveMouse)
		{
			pt.x = (LONG)io.MousePos.x;
			pt.y = (LONG)io.MousePos.y;
			ClientToScreen(g_Window, &pt);
			SetCursorPos(pt.x, pt.y);
		}
		else
		{
			GetCursorPos(&pt);
			ScreenToClient(g_Window, &pt);
			io.MousePos = ImVec2((float)pt.x, (float)pt.y);
		}
	}
	else
	{
		// without focus
		io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
	}

	for (int i = 0; i < 3; i++)
	{
		// If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
		io.MouseDown[i] = g_MouseJustPressed[i];
	}

	io.MouseWheel = g_MouseWheel;
	g_MouseWheel = 0.0f;

	// Hide OS mouse cursor if ImGui is drawing it
	ShowCursor( io.MouseDrawCursor ? FALSE : TRUE );

	// Start the frame. This call will update the io.WantCaptureMouse, io.WantCaptureKeyboard flag that you can use to dispatch inputs (or not) to your application.
	ImGui::NewFrame();
}

void ImGui_ImplGL_Render(void)
{
	ImGui::Render();
}
