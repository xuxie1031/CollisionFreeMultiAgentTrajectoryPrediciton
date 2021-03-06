#pragma once
#include "inc\natives.h"
#include "inc\types.h"
#include "inc\enums.h"
#include "inc\main.h"

#include "myEnums.h"
#include "myTypes.h"

#include "SimulationData.h"
#include "Menu.h"
#include "sampling.h"

void vehicleMenu(SimulationData& data);
void pedMenu(SimulationData& data);
void cameraMenu(SimulationData& data);
void recordingMenu(SimulationData& data);
void replayMenu(SimulationData& data);