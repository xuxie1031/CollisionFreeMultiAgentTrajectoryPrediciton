#pragma once

#include "keyboard.h"
#include "graphics.h"
#include "SimulationData.h"
#include "gamePlay.h"
#include "gameResources.h"

// a class that performs simulation, recording and replay
// according to the simulation data
class Simulator {
public:
	Simulator(SimulationData& data);
	void startSimulation();
	void processRecording(std::function<bool()> delegate, std::string fileName);
	// TODO: change the format of replay input to a more general one
	void processReplay(bool drawRainbow);
private:
	void setCars(SimulationData::VehSettings& settings, int totalCars = -1);
	void setPeds(SimulationData::PedSettings& settings, int totalPeds = -1);
	void loadPredictions(std::unordered_map<int, std::unordered_map<int, std::vector<std::vector<Vector2>>>>& coordsMap);
	void loadReplay(std::unordered_map<int, std::unordered_map <int, std::pair<Vector3d, float>>>& coordsMap, std::unordered_map<int, int>& lastAppear);
	SimulationData& data;
};