/*
		THIS FILE IS A PART OF GTA V SCRIPT HOOK SDK
					http://dev-c.com
				(C) Alexander Blade 2015
*/

#include "keyboard.h"

const int KEYS_SIZE = 255;
const eControl MENU_KEY = ControlSelectCharacterFranklin;

struct {
	DWORD time;
	BOOL isWithAlt;
	BOOL wasDownBefore;
	BOOL isUpNow;
} keyStates[KEYS_SIZE];

void OnKeyboardMessage(DWORD key, WORD repeats, BYTE scanCode, BOOL isExtended, BOOL isWithAlt, BOOL wasDownBefore, BOOL isUpNow)
{
	if (key < KEYS_SIZE)
	{
		keyStates[key].time = GetTickCount();
		keyStates[key].isWithAlt = isWithAlt;
		keyStates[key].wasDownBefore = wasDownBefore;
		keyStates[key].isUpNow = isUpNow;
	}
}

const int NOW_PERIOD = 100, MAX_DOWN = 5000; // ms

bool IsKeyDown(DWORD key)
{
	return (key < KEYS_SIZE) ? ((GetTickCount() < keyStates[key].time + MAX_DOWN) && !keyStates[key].isUpNow) : false;
}

bool IsKeyJustUp(DWORD key, bool exclusive)
{
	bool b = (key < KEYS_SIZE) ? (GetTickCount() < keyStates[key].time + NOW_PERIOD && keyStates[key].isUpNow) : false;
	if (b && exclusive)
		ResetKeyState(key);
	return b;
}

void ResetKeyState(DWORD key)
{
	if (key < KEYS_SIZE)
		memset(&keyStates[key], 0, sizeof(keyStates[0]));
}

bool isMenuKeyJustUp() {
	return CONTROLS::IS_CONTROL_JUST_RELEASED(0, MENU_KEY);
}

void getButtonState(bool *a, bool *b, bool *up, bool *down, bool *l, bool *r)
{
	if (a) *a = IsKeyJustUp(VK_NUMPAD5);
	if (b) *b = IsKeyJustUp(VK_NUMPAD0) || isMenuKeyJustUp() || IsKeyJustUp(VK_BACK);
	if (up) *up = IsKeyJustUp(VK_NUMPAD8);
	if (down) *down = IsKeyJustUp(VK_NUMPAD2);
	if (r) *r = IsKeyJustUp(VK_NUMPAD6);
	if (l) *l = IsKeyJustUp(VK_NUMPAD4);
}