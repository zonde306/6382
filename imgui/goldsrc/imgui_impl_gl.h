#ifndef IMGUI_IMPL_GL_H
#define IMGUI_IMPL_GL_H
#include <imgui.h>

bool ImGui_ImplGL_Init(void);
void ImGui_ImplGL_Shutdown(void);
void ImGui_ImplGL_NewFrame(void);
void ImGui_ImplGL_Render(void);
void ImGui_ImplGL_RenderDrawLists(ImDrawData* draw_data);

// Use if you want to reset your rendering device without losing ImGui state.
void ImGui_ImplGL_InvalidateDeviceObjects(void);
bool ImGui_ImplGL_CreateDeviceObjects(void);

// Provided here if you want to chain callbacks yourself. You may also handle inputs yourself and use those as a reference.
void ImGui_ImplGL_MouseButtonCallback(int button, int action);
void ImGui_ImplGL_ScrollCallback(double xoffset, double yoffset);
void ImGui_ImplGL_KeyCallback(int key, int action);
void ImGui_ImplGL_CharCallback(unsigned int c);

#endif
