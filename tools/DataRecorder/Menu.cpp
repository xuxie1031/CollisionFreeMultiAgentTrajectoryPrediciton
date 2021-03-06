#include "Menu.h"

const int Menu::NUMBER_OF_LINES_SHOW = 12;
const float Menu::ITEM_CHAR_WIDTH = 6;
const float Menu::CAPTION_CHAR_WIDTH = 10;
const float Menu::DESCRIPTION_EXTRA_WIDTH = 100;
const int MENU_CAPTION_FONT = 2;
const int MENU_ITEM_LINE_FONT = 6;

std::string Menu::makeLine(std::string text, bool *pState)
{
	return text + (pState ? (*pState ? "   [ON]" : "  [OFF]") : "");
}

bool Menu::isItemActive(int i) {
	return (items[i].active == NULL) || *(items[i].active) == !items[i].negateActive;
}

float Menu::lineWidth(const std::string& line, bool isCaption, bool isDescription = false) {
	if (isCaption) {
		return line.length() * CAPTION_CHAR_WIDTH;
	}
	else {
		if (isDescription) {
			return max(line.length() * ITEM_CHAR_WIDTH, maxWidth + DESCRIPTION_EXTRA_WIDTH);
		}
		else {
			return line.length() * ITEM_CHAR_WIDTH;
		}
	}
}

Menu::Menu(const std::string& CAPTION, const std::vector<MenuItem>& ITEMS, bool SINGLE_USE) {
	caption = CAPTION;
	items = ITEMS;
	maxWidth = 250;
	singleUse = false;
	size_t lineLen;
	lineActive = 0;

	lineLen = lineWidth(caption, true);
	if (lineLen > maxWidth) {
		maxWidth = lineLen;
	}

	for (int i = 0; i < lineCount(); i++) {
		lineLen = lineWidth(items[i].line, false) + (items[i].state ? ITEM_CHAR_WIDTH * 10 : 0);
		if (lineLen > maxWidth) {
			maxWidth = lineLen;
		}
	}
}

size_t Menu::lineCount() {
	return items.size();
}

void Menu::drawVertical(int lineActive) {
	int end = lineCount();
	int start = 0;
	lineActive++;
	if (end > NUMBER_OF_LINES_SHOW) {
		if (lineActive > NUMBER_OF_LINES_SHOW / 2) {
			start = min(end - NUMBER_OF_LINES_SHOW, lineActive - NUMBER_OF_LINES_SHOW / 2);
		}
		end = start + NUMBER_OF_LINES_SHOW;
	}
	lineActive--;
	drawTextLine(caption, maxWidth, 8.0, 18.0, 5.0, 5.0, DefaultColor::white, DefaultColor::blue.makeTransparent(150), 0.7, MENU_CAPTION_FONT);
	for (int i = start; i < end; i++) {
		if (i != lineActive)
		{
			if (isItemActive(i)) {
				drawTextLine(makeLine(items[i].line, items[i].state),
					maxWidth, 9.0, 65.0 + (i - start) * 40.0, 5.0, 9.0, DefaultColor::white.makeTransparent(150), DefaultColor::black.makeTransparent(150), 0.5, MENU_ITEM_LINE_FONT);
			}
			else {
				drawTextLine(makeLine(items[i].line, items[i].state),
					maxWidth, 9.0, 65.0 + (i - start) * 40.0, 5.0, 9.0, DefaultColor::grey.makeTransparent(150), DefaultColor::black.makeTransparent(150), 0.5, MENU_ITEM_LINE_FONT);
			}
		}
	}
	if (lineActive < lineCount()) {
		drawTextLine(makeLine(">" + items[lineActive].line, items[lineActive].state),
			maxWidth, 9.0, 65.0 + (lineActive - start) * 40.0, 5.0, 9.0, DefaultColor::white, DefaultColor::black.makeTransparent(200), 0.5, MENU_ITEM_LINE_FONT);
		if (items[lineActive].description != "") {
			drawTextLine(items[lineActive].description,
				lineWidth(items[lineActive].description, false, true), 9.0,
				65.0 + (end - start) * 40.0 + 15.0, 5.0, 9.0, DefaultColor::white, DefaultColor::black.makeTransparent(200), 0.5, MENU_ITEM_LINE_FONT);
		}
	}
}

bool Menu::isMenuActive() {
	for (int i = 0; i < lineCount(); i++) {
		if (isItemActive(i)) {
			return true;
		}
	}
	return false;
}

void Menu::addMenuItem(const MenuItem& newItem) {
	items.push_back(newItem);

	size_t 	lineLen = lineWidth(newItem.line, false) + (newItem.state ? ITEM_CHAR_WIDTH * 10 : 0);
	if (lineLen > maxWidth) {
		maxWidth = lineLen;
	}
}

bool Menu::onTick() {
	drawVertical(lineActive);

	bool bSelect, bBack, bUp, bDown;
	getButtonState(&bSelect, &bBack, &bUp, &bDown, NULL, NULL);
	if (bSelect)
	{
		if (isItemActive(lineActive)) {
			items[lineActive].function();
			if (singleUse) {
				return false;
			}
		}
	}
	else if (bBack) {
		return false;
	}
	else if (bUp) {
		if (isMenuActive()) {
			do {
				if (lineActive == 0)
					lineActive = lineCount();
				lineActive--;
			} while (!isItemActive(lineActive));
		}
	}
	else if (bDown)
	{
		if (isMenuActive()) {
			do {
				lineActive++;
				if (lineActive == lineCount())
					lineActive = 0;
			} while (!isItemActive(lineActive));
		}
	}

	return true;
}

void Menu::deleteItem(int i) {
	items.erase(items.begin() + i);
}

void Menu::processMenu() {
	lineActive = 0;

	while (true) {
		WAIT(0);
		if (!Menu::onTick())
			break;
	}
}