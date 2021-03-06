#pragma once
#define _USE_MATH_DEFINES
#include "inc\natives.h"
#include "inc\types.h"
#include "inc\enums.h"
#include "inc\main.h"

#include "myTypes.h"
#include <string>
#include <cmath>

#define COLORNUM 256

void drawRectangle(float A_0, float A_1, float A_2, float A_3, int A_4, int A_5, int A_6, int A_7);

void drawLine(Vector3d p1, Vector3d p2, const Color& color);

void drawCircle(Vector3d center, float radius, const Color& color);

void drawTextLine(std::string caption, float lineWidth, float lineHeight, float lineTop, float lineLeft, float textLeft, Color textColor, Color backgroundColor, float textScale, int font);

void drawMarker(int type, Vector3d position, float scale, Color color);

void drawHelpText(std::string text);

void drawQuadrant(Vector3 forwardVec, float x0, float y0, Vector3 initV, int num_interp, int colorOffset);

void loadColormap();