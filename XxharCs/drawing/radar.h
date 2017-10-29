#pragma once
#include "../Engine/util_vector.h"

void overview_draw_entity(const float* origin, int radius, int r, int g, int b, int a = 255);
void overview_calcRadarPoint(const float* origin, int * screenx, int * screeny);
void overview_redraw();
void overview_cmd();
void overview_loadcurrent();
int clientopenglcalc(int number);
void drawRadarPoint(const float* origin, int r, int g, int b, int a, bool blink, int boxsize);
void DrawRadar();

extern bool oglSubtractive;
extern bool mapLoaded;
