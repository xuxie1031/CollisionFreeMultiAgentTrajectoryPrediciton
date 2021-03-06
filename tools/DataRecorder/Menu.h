#pragma once

#include <string>
#include <vector>
#include <functional>
#include <cmath>

#include "inc\natives.h"
#include "inc\types.h"
#include "inc\enums.h"
#include "inc\main.h"

#include "myEnums.h"
#include "myTypes.h"

#include "graphics.h"
#include "keyboard.h"

class Menu {
public:
	Menu(const std::string& CAPTION, const std::vector<MenuItem>& ITEMS, bool SINGLE_USE = false);
	size_t lineCount();
	// draw the menu as a standard vertical menu on the upper left corner.
	// a horizontal menu for listing is planned, but may not be available,
	// as the vertical menu can mostly do the job.
	void drawVertical(int lineActive);
	void addMenuItem(const MenuItem& newItem);
	// open a menu interface
	// dedicate the program to menu processing
	void processMenu();
	// activate menu functions on this tick
	// can be combined with other real-time functionalities
	bool Menu::onTick();
	void Menu::deleteItem(int i);

	// the current menu button that is highlighted
	int lineActive;
	bool singleUse;
	std::string caption;
	// there should be a better way to hide the items but some functionalities are heavily used
	std::vector<MenuItem> items;

private:
	static const int NUMBER_OF_LINES_SHOW;
	static const float ITEM_CHAR_WIDTH;
	static const float CAPTION_CHAR_WIDTH;
	static const float DESCRIPTION_EXTRA_WIDTH;
	float maxWidth;

	std::string makeLine(std::string text, bool *pState);
	bool isItemActive(int i);
	bool isMenuActive();
	float lineWidth(const std::string& line, bool isCaption, bool isDescription);

};