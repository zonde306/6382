#pragma once
#include <imgui.h>

#define GET_BYTE(_v)					(_v&0xFF)
#define COLOR_ARGB(_a,_r,_g,_b)			((GET_BYTE(_a)<<24)|(GET_BYTE(_r)<<16)|(GET_BYTE(_g)<<8)|(GET_BYTE(_b)))
#define COLOR_RGBA(_r,_g,_b,_a)			COLOR_ARGB(_a,_r,_g,_b)
