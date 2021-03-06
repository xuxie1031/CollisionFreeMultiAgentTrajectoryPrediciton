#pragma once

#include <vector>

#include "inc\natives.h"
#include "inc\types.h"
#include "inc\enums.h"
#include "inc\main.h"


// a singleton class as the instructionbuttons interface is shared across the script
class InstructionButtons {
public:
	static InstructionButtons* getInstance();
	// set the mapping between buttons and descriptions
	void loadButtonList(const std::vector<std::pair<eControl, std::string>>& buttons);
	// render the instruction for this frame
	void render();

private:
	InstructionButtons();
	int handle;
	static InstructionButtons* instance;
};
