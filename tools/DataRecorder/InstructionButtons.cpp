#include "InstructionButtons.h"

InstructionButtons* InstructionButtons::instance = NULL;

InstructionButtons::InstructionButtons() {
	handle = GRAPHICS::REQUEST_SCALEFORM_MOVIE("instructional_buttons");
	while (!GRAPHICS::HAS_SCALEFORM_MOVIE_LOADED(handle)) {
		WAIT(0);
	}
}

InstructionButtons* InstructionButtons::getInstance() {
	if (!instance) {
		instance = new InstructionButtons;
	}
	return instance;
}

void InstructionButtons::loadButtonList(const std::vector<std::pair<eControl, std::string>>& buttons) {
	GRAPHICS::_PUSH_SCALEFORM_MOVIE_FUNCTION(handle, "SET_DATA_SLOT_EMPTY");
	GRAPHICS::_POP_SCALEFORM_MOVIE_FUNCTION();

	for (int i = 0; i < buttons.size(); i++) {
		GRAPHICS::_PUSH_SCALEFORM_MOVIE_FUNCTION(handle, "SET_DATA_SLOT");
		GRAPHICS::_PUSH_SCALEFORM_MOVIE_FUNCTION_PARAMETER_INT(i);
		GRAPHICS::_PUSH_SCALEFORM_MOVIE_FUNCTION_PARAMETER_STRING(CONTROLS::_GET_CONTROL_ACTION_NAME(0, buttons[i].first, 0));
		GRAPHICS::_PUSH_SCALEFORM_MOVIE_FUNCTION_PARAMETER_STRING((char*)buttons[i].second.c_str());
		GRAPHICS::_POP_SCALEFORM_MOVIE_FUNCTION();
	}

	GRAPHICS::_PUSH_SCALEFORM_MOVIE_FUNCTION(handle, "DRAW_INSTRUCTIONAL_BUTTONS");
	GRAPHICS::_PUSH_SCALEFORM_MOVIE_FUNCTION_PARAMETER_INT(-1);
	GRAPHICS::_POP_SCALEFORM_MOVIE_FUNCTION();
}

void InstructionButtons::render() {
	GRAPHICS::DRAW_SCALEFORM_MOVIE_FULLSCREEN(handle, 255, 255, 255, 255, 0);
}