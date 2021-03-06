#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <ctime>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <functional>
#include <unordered_map>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include "inc\natives.h"
#include "inc\types.h"
#include "inc\enums.h"
#include "inc\main.h"

#include "system.h"
#include "myEnums.h"
#include "myTypes.h"
#include "SimulationData.h"
#include "graphics.h"
#include "keyboard.h"
#include "Menu.h"
#include "gamePlay.h"
#include "InstructionButtons.h"
#include "sampling.h"
#include "gameResources.h"
#include "samplerMenu.h"
#include "Simulator.h"

void ScriptMain();