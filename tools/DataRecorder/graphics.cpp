#include "graphics.h"

static unsigned char colormap[COLORNUM * 3];

void drawRectangle(float A_0, float A_1, float A_2, float A_3, int A_4, int A_5, int A_6, int A_7)
{
	GRAPHICS::DRAW_RECT((A_0 + (A_2 * 0.5f)), (A_1 + (A_3 * 0.5f)), A_2, A_3, A_4, A_5, A_6, A_7);  // x, y, width, height, rgb, alpha
}

void drawLine(Vector3d p1, Vector3d p2, const Color& color) {
	GRAPHICS::DRAW_LINE(p1.x, p1.y, p1.z, p2.x, p2.y, p2.z, color.red, color.green, color.blue, color.alpha);
}

void drawCircle(Vector3d center, float radius, const Color& color) {
	const int SIDES = 16;
	float degree = 0;
	Vector3d p1 = center;
	p1.x += sin(degree) * radius;
	p1.y += cos(degree) * radius;
	for (int i = 0; i < SIDES; i++) {
		degree += M_PI * 2 / SIDES;
		Vector3d p2 = center;
		p2.x += sin(degree) * radius;
		p2.y += cos(degree) * radius;
		drawLine(p1, p2, color);
		p1 = p2;
	}
}

void drawTextLine(std::string caption, float lineWidth, float lineHeight, float lineTop, float lineLeft, float textLeft, Color textColor, Color backgroundColor, float textScale, int font)
{
	int screen_w, screen_h;
	GRAPHICS::GET_SCREEN_RESOLUTION(&screen_w, &screen_h);

	textLeft += lineLeft;

	float lineWidthScaled = lineWidth / (float)screen_w; // line width
	float lineTopScaled = lineTop / (float)screen_h; // line top offset
	float textLeftScaled = textLeft / (float)screen_w; // text left offset
	float lineHeightScaled = lineHeight / (float)screen_h; // line height

	float lineLeftScaled = lineLeft / (float)screen_w;

	// this is how it's done in original scripts

	// text upper part
	UI::SET_TEXT_FONT(font);
	UI::SET_TEXT_SCALE(0.0, textScale);
	UI::SET_TEXT_COLOUR(textColor.red, textColor.green, textColor.blue, textColor.alpha);
	UI::SET_TEXT_CENTRE(0);
	UI::SET_TEXT_DROPSHADOW(0, 0, 0, 0, 0);
	UI::SET_TEXT_EDGE(0, 0, 0, 0, 0);
	UI::_SET_TEXT_ENTRY("STRING");
	UI::_ADD_TEXT_COMPONENT_STRING((LPSTR)caption.c_str());
	UI::_DRAW_TEXT(textLeftScaled, (((lineTopScaled + 0.00278f) + lineHeightScaled) - 0.005f));

	// text lower part
	UI::SET_TEXT_FONT(font);
	UI::SET_TEXT_SCALE(0.0, textScale);
	UI::SET_TEXT_COLOUR(textColor.red, textColor.green, textColor.blue, textColor.alpha);
	UI::SET_TEXT_CENTRE(0);
	UI::SET_TEXT_DROPSHADOW(0, 0, 0, 0, 0);
	UI::SET_TEXT_EDGE(0, 0, 0, 0, 0);
	UI::_SET_TEXT_GXT_ENTRY("STRING");
	UI::_ADD_TEXT_COMPONENT_STRING((LPSTR)caption.c_str());
	int num25 = UI::_0x9040DFB09BE75706(textLeftScaled, (((lineTopScaled + 0.00278f) + lineHeightScaled) - 0.005f));

	// rect
	drawRectangle(lineLeftScaled, lineTopScaled + (0.00278f),
		lineWidthScaled, ((((float)(num25)* UI::_0xDB88A37483346780(textScale, 0)) + (lineHeightScaled * 2.0f)) + 0.005f),
		backgroundColor.red, backgroundColor.green, backgroundColor.blue, backgroundColor.alpha);
}

void drawMarker(int type, Vector3d position, float scale, Color color) {
	float groundHeight;
	GAMEPLAY::GET_GROUND_Z_FOR_3D_COORD(position.x, position.y, position.z, &groundHeight, 0);
	Vector3d direction(0, 0, 0);
	Vector3d rotation(180, 0, 0);
	GRAPHICS::DRAW_MARKER(type, position.x, position.y, groundHeight+0.4*scale, direction.x, direction.y, direction.z, rotation.x, rotation.y, rotation.z, scale, scale, scale, color.red, color.green, color.blue, color.alpha, false, true, 2, false, false, false, false);
}

void drawHelpText(std::string text)
{
	UI::_SET_TEXT_COMPONENT_FORMAT("STRING");
	UI::_ADD_TEXT_COMPONENT_STRING((LPSTR)(text.c_str()));
	UI::_DISPLAY_HELP_TEXT_FROM_STRING_LABEL(0, 0, 1, -1);
}


void drawLinePositive(Vector3 initV, float unit_x_length, float unit_bar_length, int num_interp, float k, int colorOffset)
{
	for (int i = 0; i < num_interp; i++)
	{
		Vector3 currentV, nextV;	// relative
		currentV.x = unit_x_length * i; currentV.y = k * currentV.x; currentV.z = 0;
		nextV.x = unit_x_length * (i + 1); nextV.y = k * nextV.x; nextV.z = 0;
		currentV.x += initV.x; currentV.y += initV.y; currentV.z += initV.z;
		nextV.x += initV.x; nextV.y += initV.y; nextV.z += initV.z;
		float theta = atan(k);
		for (int j = colorOffset; j < COLORNUM; j++)
		{
			int drawPos = j - colorOffset;
			GRAPHICS::DRAW_POLY(nextV.x + unit_bar_length * drawPos*sin(theta), nextV.y - unit_bar_length * drawPos*cos(theta), nextV.z, currentV.x + unit_bar_length * drawPos*sin(theta), currentV.y - unit_bar_length * drawPos*cos(theta), currentV.z, currentV.x + unit_bar_length * (drawPos + 1)*sin(theta), currentV.y - unit_bar_length * (drawPos + 1)*cos(theta), currentV.z, colormap[3 * j], colormap[3 * j + 1], colormap[3 * j + 2], 100);
			GRAPHICS::DRAW_POLY(currentV.x + unit_bar_length * (drawPos + 1)*sin(theta), currentV.y - unit_bar_length * (drawPos + 1)*cos(theta), currentV.z, nextV.x + unit_bar_length * (drawPos + 1)*sin(theta), nextV.y - unit_bar_length * (drawPos + 1)*cos(theta), nextV.z, nextV.x + unit_bar_length * drawPos*sin(theta), nextV.y - unit_bar_length * drawPos*cos(theta), nextV.z, colormap[3 * j], colormap[3 * j + 1], colormap[3 * j + 2], 100);
			GRAPHICS::DRAW_POLY(currentV.x - unit_bar_length * drawPos*sin(theta), currentV.y + unit_bar_length * drawPos*cos(theta), currentV.z, nextV.x - unit_bar_length * drawPos*sin(theta), nextV.y + unit_bar_length * drawPos*cos(theta), nextV.z, nextV.x - unit_bar_length * (drawPos + 1)*sin(theta), nextV.y + unit_bar_length * (drawPos + 1)*cos(theta), nextV.z, colormap[3 * j], colormap[3 * j + 1], colormap[3 * j + 2], 100);
			GRAPHICS::DRAW_POLY(nextV.x - unit_bar_length * (drawPos + 1)*sin(theta), nextV.y + unit_bar_length * (drawPos + 1)*cos(theta), nextV.z, currentV.x - unit_bar_length * (drawPos + 1)*sin(theta), currentV.y + unit_bar_length * (drawPos + 1)*cos(theta), currentV.z, currentV.x - unit_bar_length * drawPos*sin(theta), currentV.y + unit_bar_length * drawPos*cos(theta), currentV.z, colormap[3 * j], colormap[3 * j + 1], colormap[3 * j + 2], 100);
		}
	}
}

void drawLineNegative(Vector3 initV, float unit_x_length, float unit_bar_length, int num_interp, float k, int colorOffset)
{
	for (int i = num_interp - 1; i >= 0; i--)
	{
		Vector3 currentV, nextV;	// relative
		currentV.x = unit_x_length * (i + 1); currentV.y = k * currentV.x; currentV.z = 0;
		nextV.x = unit_x_length * i; nextV.y = k * nextV.x; nextV.z = 0;
		currentV.x += initV.x; currentV.y += initV.y; currentV.z += initV.z;
		nextV.x += initV.x; nextV.y += initV.y; nextV.z += initV.z;
		float theta = atan(k);
		for (int j = colorOffset; j < COLORNUM; j++)
		{
			int drawPos = j - colorOffset;
			GRAPHICS::DRAW_POLY(nextV.x + unit_bar_length * drawPos*sin(theta), nextV.y - unit_bar_length * drawPos*cos(theta), nextV.z, currentV.x + unit_bar_length * drawPos*sin(theta), currentV.y - unit_bar_length * drawPos*cos(theta), currentV.z, currentV.x + unit_bar_length * (drawPos + 1)*sin(theta), currentV.y - unit_bar_length * (drawPos + 1)*cos(theta), currentV.z, colormap[3 * j], colormap[3 * j + 1], colormap[3 * j + 2], 100);
			GRAPHICS::DRAW_POLY(currentV.x + unit_bar_length * (drawPos + 1)*sin(theta), currentV.y - unit_bar_length * (drawPos + 1)*cos(theta), currentV.z, nextV.x + unit_bar_length * (drawPos + 1)*sin(theta), nextV.y - unit_bar_length * (drawPos + 1)*cos(theta), nextV.z, nextV.x + unit_bar_length * drawPos*sin(theta), nextV.y - unit_bar_length * drawPos*cos(theta), nextV.z, colormap[3 * j], colormap[3 * j + 1], colormap[3 * j + 2], 100);
			GRAPHICS::DRAW_POLY(currentV.x - unit_bar_length * drawPos*sin(theta), currentV.y + unit_bar_length * drawPos*cos(theta), currentV.z, nextV.x - unit_bar_length * drawPos*sin(theta), nextV.y + unit_bar_length * drawPos*cos(theta), nextV.z, nextV.x - unit_bar_length * (drawPos + 1)*sin(theta), nextV.y + unit_bar_length * (drawPos + 1)*cos(theta), nextV.z, colormap[3 * j], colormap[3 * j + 1], colormap[3 * j + 2], 100);
			GRAPHICS::DRAW_POLY(nextV.x - unit_bar_length * (drawPos + 1)*sin(theta), nextV.y + unit_bar_length * (drawPos + 1)*cos(theta), nextV.z, currentV.x - unit_bar_length * (drawPos + 1)*sin(theta), currentV.y + unit_bar_length * (drawPos + 1)*cos(theta), currentV.z, currentV.x - unit_bar_length * drawPos*sin(theta), currentV.y + unit_bar_length * drawPos*cos(theta), currentV.z, colormap[3 * j], colormap[3 * j + 1], colormap[3 * j + 2], 100);
		}
	}
}

void drawParaboaPositiveX(Vector3 initV, float unit_x_length, float unit_bar_length, int num_interp, float a, float b, int colorOffset)
{
	for (int i = 0; i < num_interp; i++)
	{
		Vector3 currentV, nextV;	// relative
		currentV.x = unit_x_length * i; currentV.y = a * currentV.x*currentV.x + b * currentV.x; currentV.z = 0;
		nextV.x = unit_x_length * (i + 1); nextV.y = a * nextV.x*nextV.x + b * nextV.x; nextV.z = 0;
		currentV.x += initV.x; currentV.y += initV.y; currentV.z += initV.z;
		nextV.x += initV.x; nextV.y += initV.y; nextV.z += initV.z;
		float theta = atan((nextV.y - currentV.y) / (nextV.x - currentV.x));
		for (int j = colorOffset; j < COLORNUM; j++)
		{
			GRAPHICS::DRAW_POLY(nextV.x + unit_bar_length * j*sin(theta), nextV.y - unit_bar_length * j*cos(theta), nextV.z, currentV.x + unit_bar_length * j*sin(theta), currentV.y - unit_bar_length * j*cos(theta), currentV.z, currentV.x + unit_bar_length * (j + 1)*sin(theta), currentV.y - unit_bar_length * (j + 1)*cos(theta), currentV.z, colormap[3 * j], colormap[3 * j + 1], colormap[3 * j + 2], 100);
			GRAPHICS::DRAW_POLY(currentV.x + unit_bar_length * (j + 1)*sin(theta), currentV.y - unit_bar_length * (j + 1)*cos(theta), currentV.z, nextV.x + unit_bar_length * (j + 1)*sin(theta), nextV.y - unit_bar_length * (j + 1)*cos(theta), nextV.z, nextV.x + unit_bar_length * j*sin(theta), nextV.y - unit_bar_length * j*cos(theta), nextV.z, colormap[3 * j], colormap[3 * j + 1], colormap[3 * j + 2], 100);
			GRAPHICS::DRAW_POLY(currentV.x - unit_bar_length * j*sin(theta), currentV.y + unit_bar_length * j*cos(theta), currentV.z, nextV.x - unit_bar_length * j*sin(theta), nextV.y + unit_bar_length * j*cos(theta), nextV.z, nextV.x - unit_bar_length * (j + 1)*sin(theta), nextV.y + unit_bar_length * (j + 1)*cos(theta), nextV.z, colormap[3 * j], colormap[3 * j + 1], colormap[3 * j + 2], 100);
			GRAPHICS::DRAW_POLY(nextV.x - unit_bar_length * (j + 1)*sin(theta), nextV.y + unit_bar_length * (j + 1)*cos(theta), nextV.z, currentV.x - unit_bar_length * (j + 1)*sin(theta), currentV.y + unit_bar_length * (j + 1)*cos(theta), currentV.z, currentV.x - unit_bar_length * j*sin(theta), currentV.y + unit_bar_length * j*cos(theta), currentV.z, colormap[3 * j], colormap[3 * j + 1], colormap[3 * j + 2], 100);
		}
	}
}

void drawParaboaNegativeX(Vector3 initV, float unit_x_length, float unit_bar_length, int num_interp, float a, float b, int colorOffset)
{
	for (int i = num_interp - 1; i >= 0; i--)
	{
		Vector3 currentV, nextV;	// relative
		currentV.x = unit_x_length * (i + 1); currentV.y = a * currentV.x*currentV.x + b * currentV.x; currentV.z = 0;
		nextV.x = unit_x_length * i; nextV.y = a * nextV.x*nextV.x + b * nextV.x; nextV.z = 0;
		currentV.x += initV.x; currentV.y += initV.y; currentV.z += initV.z;
		nextV.x += initV.x; nextV.y += initV.y; nextV.z += initV.z;
		float theta = atan((nextV.y - currentV.y) / (nextV.x - currentV.x));
		for (int j = colorOffset; j < COLORNUM; j++)
		{
			GRAPHICS::DRAW_POLY(nextV.x + unit_bar_length * j*sin(theta), nextV.y - unit_bar_length * j*cos(theta), nextV.z, currentV.x + unit_bar_length * j*sin(theta), currentV.y - unit_bar_length * j*cos(theta), currentV.z, currentV.x + unit_bar_length * (j + 1)*sin(theta), currentV.y - unit_bar_length * (j + 1)*cos(theta), currentV.z, colormap[3 * j], colormap[3 * j + 1], colormap[3 * j + 2], 100);
			GRAPHICS::DRAW_POLY(currentV.x + unit_bar_length * (j + 1)*sin(theta), currentV.y - unit_bar_length * (j + 1)*cos(theta), currentV.z, nextV.x + unit_bar_length * (j + 1)*sin(theta), nextV.y - unit_bar_length * (j + 1)*cos(theta), nextV.z, nextV.x + unit_bar_length * j*sin(theta), nextV.y - unit_bar_length * j*cos(theta), nextV.z, colormap[3 * j], colormap[3 * j + 1], colormap[3 * j + 2], 100);
			GRAPHICS::DRAW_POLY(currentV.x - unit_bar_length * j*sin(theta), currentV.y + unit_bar_length * j*cos(theta), currentV.z, nextV.x - unit_bar_length * j*sin(theta), nextV.y + unit_bar_length * j*cos(theta), nextV.z, nextV.x - unit_bar_length * (j + 1)*sin(theta), nextV.y + unit_bar_length * (j + 1)*cos(theta), nextV.z, colormap[3 * j], colormap[3 * j + 1], colormap[3 * j + 2], 100);
			GRAPHICS::DRAW_POLY(nextV.x - unit_bar_length * (j + 1)*sin(theta), nextV.y + unit_bar_length * (j + 1)*cos(theta), nextV.z, currentV.x - unit_bar_length * (j + 1)*sin(theta), currentV.y + unit_bar_length * (j + 1)*cos(theta), currentV.z, currentV.x - unit_bar_length * j*sin(theta), currentV.y + unit_bar_length * j*cos(theta), currentV.z, colormap[3 * j], colormap[3 * j + 1], colormap[3 * j + 2], 100);
		}
	}
}

void drawParaboaPositiveY(Vector3 initV, float unit_y_length, float unit_bar_length, int num_interp, float a, float b, int colorOffset)
{
	for (int i = 0; i < num_interp; i++)
	{
		Vector3 currentV, nextV;
		currentV.y = unit_y_length * i; currentV.x = a * currentV.y*currentV.y + b * currentV.y; currentV.z = 0;
		nextV.y = unit_y_length * (i + 1); nextV.x = a * nextV.y*nextV.y + b * nextV.y; nextV.z = 0;
		currentV.x += initV.x; currentV.y += initV.y; currentV.z += initV.z;
		nextV.x += initV.x; nextV.y += initV.y; nextV.z += initV.z;
		float theta = atan((nextV.x - currentV.x) / (nextV.y - currentV.y));
		for (int j = colorOffset; j < COLORNUM; j++)
		{
			GRAPHICS::DRAW_POLY(nextV.x + unit_bar_length * j*cos(theta), nextV.y - unit_bar_length * j*sin(theta), nextV.z, currentV.x + unit_bar_length * j*cos(theta), currentV.y - unit_bar_length * j*sin(theta), currentV.z, currentV.x + unit_bar_length * (j + 1)*cos(theta), currentV.y - unit_bar_length * (j + 1)*sin(theta), currentV.z, colormap[3 * j], colormap[3 * j + 1], colormap[3 * j + 2], 100);
			GRAPHICS::DRAW_POLY(currentV.x + unit_bar_length * (j + 1)*cos(theta), currentV.y - unit_bar_length * (j + 1)*sin(theta), currentV.z, nextV.x + unit_bar_length * (j + 1)*cos(theta), nextV.y - unit_bar_length * (j + 1)*sin(theta), nextV.z, nextV.x + unit_bar_length * j*cos(theta), nextV.y - unit_bar_length * j*sin(theta), nextV.z, colormap[3 * j], colormap[3 * j + 1], colormap[3 * j + 2], 100);
			GRAPHICS::DRAW_POLY(currentV.x - unit_bar_length * j*cos(theta), currentV.y + unit_bar_length * j*sin(theta), currentV.z, nextV.x - unit_bar_length * j*cos(theta), nextV.y + unit_bar_length * j*sin(theta), nextV.z, nextV.x - unit_bar_length * (j + 1)*cos(theta), nextV.y + unit_bar_length * (j + 1)*sin(theta), nextV.z, colormap[3 * j], colormap[3 * j + 1], colormap[3 * j + 2], 100);
			GRAPHICS::DRAW_POLY(nextV.x - unit_bar_length * (j + 1)*cos(theta), nextV.y + unit_bar_length * (j + 1)*sin(theta), nextV.z, currentV.x - unit_bar_length * (j + 1)*cos(theta), currentV.y + unit_bar_length * (j + 1)*sin(theta), currentV.z, currentV.x - unit_bar_length * j*cos(theta), currentV.y + unit_bar_length * j*sin(theta), currentV.z, colormap[3 * j], colormap[3 * j + 1], colormap[3 * j + 2], 100);
		}
	}
}

void drawParaboaNegativeY(Vector3 initV, float unit_y_length, float unit_bar_length, int num_interp, float a, float b, int colorOffset)
{
	for (int i = num_interp - 1; i >= 0; i--)
	{
		Vector3 currentV, nextV;
		currentV.y = unit_y_length * (i + 1); currentV.x = a * currentV.y*currentV.y + b * currentV.y; currentV.z = 0;
		nextV.y = unit_y_length * i; nextV.x = a * nextV.y*nextV.y + b * nextV.y; nextV.z = 0;
		currentV.x += initV.x; currentV.y += initV.y; currentV.z += initV.z;
		nextV.x += initV.x; nextV.y += initV.y; nextV.z += initV.z;
		float theta = atan((nextV.x - currentV.x) / (nextV.y - currentV.y));
		for (int j = colorOffset; j < COLORNUM; j++)
		{
			GRAPHICS::DRAW_POLY(nextV.x + unit_bar_length * j*cos(theta), nextV.y - unit_bar_length * j*sin(theta), nextV.z, currentV.x + unit_bar_length * j*cos(theta), currentV.y - unit_bar_length * j*sin(theta), currentV.z, currentV.x + unit_bar_length * (j + 1)*cos(theta), currentV.y - unit_bar_length * (j + 1)*sin(theta), currentV.z, colormap[3 * j], colormap[3 * j + 1], colormap[3 * j + 2], 100);
			GRAPHICS::DRAW_POLY(currentV.x + unit_bar_length * (j + 1)*cos(theta), currentV.y - unit_bar_length * (j + 1)*sin(theta), currentV.z, nextV.x + unit_bar_length * (j + 1)*cos(theta), nextV.y - unit_bar_length * (j + 1)*sin(theta), nextV.z, nextV.x + unit_bar_length * j*cos(theta), nextV.y - unit_bar_length * j*sin(theta), nextV.z, colormap[3 * j], colormap[3 * j + 1], colormap[3 * j + 2], 100);
			GRAPHICS::DRAW_POLY(currentV.x - unit_bar_length * j*cos(theta), currentV.y + unit_bar_length * j*sin(theta), currentV.z, nextV.x - unit_bar_length * j*cos(theta), nextV.y + unit_bar_length * j*sin(theta), nextV.z, nextV.x - unit_bar_length * (j + 1)*cos(theta), nextV.y + unit_bar_length * (j + 1)*sin(theta), nextV.z, colormap[3 * j], colormap[3 * j + 1], colormap[3 * j + 2], 100);
			GRAPHICS::DRAW_POLY(nextV.x - unit_bar_length * (j + 1)*cos(theta), nextV.y + unit_bar_length * (j + 1)*sin(theta), nextV.z, currentV.x - unit_bar_length * (j + 1)*cos(theta), currentV.y + unit_bar_length * (j + 1)*sin(theta), currentV.z, currentV.x - unit_bar_length * j*cos(theta), currentV.y + unit_bar_length * j*sin(theta), currentV.z, colormap[3 * j], colormap[3 * j + 1], colormap[3 * j + 2], 100);
		}
	}
}

void drawQuadrant(Vector3 forwardVec, float x0, float y0, Vector3 initV, int num_interp, int colorOffset)
{
	if (forwardVec.x*x0 < 0 && forwardVec.y*y0 < 0)
		return;

	float unit_x_length = x0 / num_interp;
	float unit_y_length = y0 / num_interp;
	float unit_bar_length = .002;
	if (forwardVec.x * x0 < 0 && forwardVec.y * y0 > 0)
	{
		float a = -y0 / (x0*x0);
		float b = 2 * y0 / x0;
		if (x0 > 0)
			drawParaboaPositiveX(initV, unit_x_length, unit_bar_length, num_interp, a, b, colorOffset);
		else
			drawParaboaNegativeX(initV, unit_x_length, unit_bar_length, num_interp, a, b, colorOffset);
	}

	if (forwardVec.x * x0 > 0 && forwardVec.y * y0 < 0)
	{
		float a = -x0 / (y0*y0);
		float b = 2 * x0 / y0;
		if (y0 > 0)
			drawParaboaPositiveY(initV, unit_y_length, unit_bar_length, num_interp, a, b, colorOffset);
		else
			drawParaboaNegativeY(initV, unit_y_length, unit_bar_length, num_interp, a, b, colorOffset);
	}

	if (forwardVec.x * x0 > 0 && forwardVec.y * y0 > 0)
	{
		float tg_xtype = 2 * y0 / x0;
		float tg_ytype = y0 / (2 * x0);
		float tg_player = forwardVec.y / forwardVec.x;

		if ((tg_player > tg_xtype && tg_player > tg_ytype) || (tg_player < tg_xtype && tg_player < tg_ytype))
		{
			float cos_xtype = (1 + tg_player * tg_xtype) / (sqrt(1 + tg_player * tg_player)*sqrt(1 + tg_xtype * tg_xtype));
			float cos_ytype = (1 + tg_player * tg_ytype) / (sqrt(1 + tg_player * tg_player)*sqrt(1 + tg_ytype * tg_ytype));

			if (cos_xtype > cos_ytype)
			{
				float a = -y0 / (x0*x0);
				float b = 2 * y0 / x0;
				if (x0 > 0)
					drawParaboaPositiveX(initV, unit_x_length, unit_bar_length, num_interp, a, b, colorOffset);
				else
					drawParaboaNegativeX(initV, unit_x_length, unit_bar_length, num_interp, a, b, colorOffset);
			}
			else
			{
				float a = -x0 / (y0*y0);
				float b = 2 * x0 / y0;
				if (y0 > 0)
					drawParaboaPositiveY(initV, unit_y_length, unit_bar_length, num_interp, a, b, colorOffset);
				else
					drawParaboaNegativeY(initV, unit_y_length, unit_bar_length, num_interp, a, b, colorOffset);
			}
		}
		else
		{
			float k = y0 / x0;
			if (x0 > 0)
				drawLinePositive(initV, unit_x_length, unit_bar_length, num_interp, k, colorOffset);
			else
				drawLineNegative(initV, unit_x_length, unit_bar_length, num_interp, k, colorOffset);
		}
	}
}

void loadColormap() {
	FILE* color_fid = fopen("jetcolor", "rb");
	fread(colormap, sizeof(unsigned char), COLORNUM * 3, color_fid);
	fclose(color_fid);
}