#pragma once

#include <utility>
#include <string>
#include <vector>
#include <fstream>
#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include "inc\natives.h"
#include "inc\types.h"
#include "inc\enums.h"
#include "inc\main.h"

#include "myTypes.h"
#include "system.h"

extern const std::string SIM_DATA_DIRECTORY;

// manages the data to control replay/simulation
class SimulationData {
public:
	struct {
		Vector3d recordCenter;
		float recordRadius;
		int numSimulations;
		DWORD recordInterval; // interval between two recordings in ms
		DWORD recordTime;
		bool stopWhenNoVehicles;
		// if enabled, individual vehicle number settings will be disabled
		// for no continuously generated groups
		bool useTotalVehicleNumber;
		// controls the total number of vehicles that are generated only once
		// override to individual vehicle number settings
		NumericalRange<int> totalVehicleNumber;
		bool useTotalPedNumber;
		NumericalRange<int> totalPedNumber;
		std::string recordDirectory;
	} recording;

	struct {
		DWORD replayInterval;
		std::string replayFile;
		std::string predictionFile;
	} replay;

	struct {
		CameraParams params;
		bool enabled;
	} camera;

	struct VehSettings {
		inline VehSettings() : wandering(false), stopRange(2.0), driveStyle(786475), speed(9.0,12.0), number(1,1), startTime(0),continuousGeneration(false), vehModel("Adder") {}
		bool wandering;
		std::vector<CarLane> starts;
		// Goals are single points since small variations of goals will be corrected by the navigation
		std::vector<Vector3d> goals; // disabled if wandering
		std::string vehModel;
		// std::string driverModel;
		float stopRange; // disabled if wandering
		int driveStyle;
		NumericalRange<float> speed;
		NumericalRange<float> carInterval;
		NumericalRange<int> number;
		DWORD startTime;
		bool continuousGeneration;
		NumericalRange<DWORD> generationInterval; // ms, disabled if not continuous generation
	};

	struct PedSettings {
		inline PedSettings() : wandering(false), running(false), number(1, 1), startTime(0), continuousGeneration(false), model("a_f_m_bevhills_01") {}
		bool wandering;
		std::vector<UnitArea> starts;
		std::vector<UnitArea> goals; // disabled if wandering. Leave blank for goals = starts
		std::string model;
		bool running; // disabled if wandering
		NumericalRange<int> number;
		DWORD startTime;
		bool continuousGeneration;
		NumericalRange<DWORD> generationInterval;
	};

	std::vector<VehSettings> vehicles;
	std::vector<PedSettings> peds;

	void initialize();
	SimulationData(); // create new settings
	void loadData(std::string fileName);
	void saveData(std::string fileName = "");
	std::string getSettingsFileName();

private:
	std::string settingsFile;
};