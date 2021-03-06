#include "SimulationData.h"

const std::string SIM_DATA_DIRECTORY = "DataRecorder/RecordSettings";
const int DEFAULT_INTERVAL = 250;

void SimulationData::initialize() {

	recording.recordCenter = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), ENTITY::IS_ENTITY_DEAD(PLAYER::PLAYER_PED_ID()));
	recording.recordRadius = 20.0;
	recording.numSimulations = 1;
	recording.recordInterval = DEFAULT_INTERVAL;
	recording.recordTime = 60000;
	recording.stopWhenNoVehicles = false;
	recording.recordDirectory = "";
	recording.useTotalVehicleNumber = false;
	recording.totalVehicleNumber = { 0, 0 };
	recording.useTotalPedNumber = false;
	recording.totalPedNumber = { 0, 0 };

	replay.replayInterval = DEFAULT_INTERVAL;
	replay.replayFile = "";
	replay.predictionFile = "";

	camera.enabled = false;
	camera.params = CameraParams();

	vehicles = {};
	peds = {};

	settingsFile = "";
}
SimulationData::SimulationData() {
	initialize();
}

void SimulationData::loadData(std::string fileName) {

	initialize();

	fileName = SIM_DATA_DIRECTORY + "/" + fileName;
	std::ifstream config(fileName);
	json data;
	config >> data;
	config.close();
	settingsFile = fileName;

	if (data.count("recording")) {
		if (data["recording"].count("recordCenter")) {
			recording.recordCenter = data["recording"]["recordCenter"];
		}
		if (data["recording"].count("recordRadius")) {
			recording.recordRadius = data["recording"]["recordRadius"];
		}
		if (data["recording"].count("numSimulations")) {
			recording.numSimulations = data["recording"]["numSimulations"];
		}
		if (data["recording"].count("recordInterval")) {
			recording.recordInterval = data["recording"]["recordInterval"];
		}
		if (data["recording"].count("recordTime")) {
			recording.recordTime = data["recording"]["recordTime"];
		}
		if (data["recording"].count("stopWhenAllCarLeaves")) {
			recording.stopWhenNoVehicles = data["recording"]["stopWhenAllCarLeaves"];
		}
		if (data["recording"].count("useTotalVehicleNumber")) {
			recording.useTotalVehicleNumber = data["recording"]["useTotalVehicleNumber"];
		}
		if (data["recording"].count("totalVehicleNumber")) {
			recording.totalVehicleNumber = data["recording"]["totalVehicleNumber"];
		}
		if (data["recording"].count("useTotalPedNumber")) {
			recording.useTotalPedNumber = data["recording"]["useTotalPedNumber"];
		}
		if (data["recording"].count("totalPedNumber")) {
			recording.totalPedNumber = data["recording"]["totalPedNumber"];
		}
		if (data["recording"].count("recordDirectory") && data["recording"]["recordDirectory"].is_string()) {
			recording.recordDirectory = data["recording"]["recordDirectory"].get<std::string>();
		}
	}

	if (data.count("replay")) {
		if (data["replay"].count("replayInterval")) {
			replay.replayInterval = data["replay"]["replayInterval"];
		}
		if (data["replay"].count("replayFile") && data["replay"]["replayFile"].is_string()) {
			replay.replayFile = data["replay"]["replayFile"].get<std::string>();
		}
		if (data["replay"].count("predictionFile") && data["replay"]["predictionFile"].is_string()) {
			replay.predictionFile = data["replay"]["predictionFile"].get<std::string>();
		}
	}

	if (data.count("camera")) {
		if (data["camera"].count("params")) {
			camera.params = data["camera"]["params"];
		}
		if (data["camera"].count("enabled")) {
			camera.enabled = data["camera"]["enabled"];
		}
	}

	if (data.count("vehicles") && data["vehicles"].is_array()) {
		for (auto& veh : data["vehicles"]) {
			VehSettings vehSettings;
			if (veh.count("wandering")) {
				vehSettings.wandering = veh["wandering"];
			}
			if (veh.count("starts") && veh["starts"].is_array()) {
				for (auto& lane : veh["starts"]) {
					CarLane laneSettings;
					laneSettings = lane;
					vehSettings.starts.push_back(laneSettings);
				}
			}
			if (veh.count("goals") && veh["goals"].is_array()) {
				for (json& goal : veh["goals"]) {
					Vector3d goalSetting;
					goalSetting = goal;
					vehSettings.goals.push_back(goalSetting);
				}
			}
			if (veh.count("vehModel") && veh["vehModel"].is_string()) {
				vehSettings.vehModel = veh["vehModel"].get<std::string>();
			}
			//if (veh.count("driverModel") && veh["driverModel"].is_string()) {
			//	vehSettings.driverModel = veh["driverModel"].get<std::string>();
			//}
			if (veh.count("stopRange")) {
				vehSettings.stopRange = veh["stopRange"];
			}
			if (veh.count("driveStyle")) {
				vehSettings.driveStyle = veh["driveStyle"];
			}
			if (veh.count("speed")) {
				vehSettings.speed = veh["speed"];
			}
			if (veh.count("carInterval")) {
				vehSettings.carInterval = veh["carInterval"];
			}
			if (veh.count("number")) {
				vehSettings.number = veh["number"];
			}
			if (veh.count("startTime")) {
				vehSettings.startTime = veh["startTime"];
			}
			if (veh.count("continuousGeneration")) {
				vehSettings.continuousGeneration = veh["continuousGeneration"];
			}
			if (veh.count("generationInterval")) {
				vehSettings.generationInterval = veh["generationInterval"];
			}
			vehicles.push_back(vehSettings);
		}
	}

	if (data.count("peds") && data["peds"].is_array()) {
		for (auto& ped : data["peds"]) {
			PedSettings pedSettings;
			if (ped.count("wandering")) {
				pedSettings.wandering = ped["wandering"];
			}
			if (ped.count("starts") && ped["starts"].is_array()) {
				for (auto& area: ped["starts"]) {
					UnitArea areaSettings;
					areaSettings = area;
					pedSettings.starts.push_back(areaSettings);
				}
			}

			if (ped.count("goals") && ped["goals"].is_array()) {
				for (auto& area : ped["goals"]) {
					UnitArea areaSettings;
					areaSettings = area;
					pedSettings.goals.push_back(areaSettings);
				}
			}
			if (ped.count("model") && ped["model"].is_string()) {
				pedSettings.model = ped["model"].get<std::string>();
			}
			if (ped.count("running")) {
				pedSettings.running = ped["running"];
			}
			if (ped.count("number")) {
					pedSettings.number= ped["number"];
			}
			if (ped.count("startTime")) {
				pedSettings.startTime = ped["startTime"];
			}
			if (ped.count("continuousGeneration")) {
				pedSettings.continuousGeneration = ped["continuousGeneration"];
			}
			if (ped.count("generationInterval")) {
				pedSettings.generationInterval = ped["generationInterval"];
			}
			peds.push_back(pedSettings);
		}
	}
}

void SimulationData::saveData(std::string fileName) {
	if (fileName == "") {
		if (settingsFile == "") {
			createAllSubdirectories(SIM_DATA_DIRECTORY);
			auto fileList = getFolderFileList(SIM_DATA_DIRECTORY);
			std::sort(fileList.begin(), fileList.end());
			int i = 0;
			auto it = std::find(fileList.begin(), fileList.end(), "settings" + std::to_string(i) + ".json");
			while (it != fileList.end() && *it == "settings" + std::to_string(i) + ".json") {
				i++;
				it++;
			}
			fileName = SIM_DATA_DIRECTORY + "/settings" + std::to_string(i) + ".json";
		}
		else {
			fileName = settingsFile;
		}
	}
	else {
		fileName = SIM_DATA_DIRECTORY + "/" + fileName;
	}
	std::ofstream config(fileName);
	json formarttedData;

	formarttedData["recording"]["recordCenter"] = recording.recordCenter.to_json();
	formarttedData["recording"]["recordRadius"] = recording.recordRadius;
	formarttedData["recording"]["numSimulations"] = recording.numSimulations;
	formarttedData["recording"]["recordInterval"] = recording.recordInterval;
	formarttedData["recording"]["recordTime"] = recording.recordTime;
	formarttedData["recording"]["stopWhenAllCarLeaves"] = recording.stopWhenNoVehicles;
	formarttedData["recording"]["recordDirectory"] = recording.recordDirectory;
	formarttedData["recording"]["useTotalVehicleNumber"] = recording.useTotalVehicleNumber;
	formarttedData["recording"]["totalVehicleNumber"] = recording.totalVehicleNumber.to_json();
	formarttedData["recording"]["useTotalPedNumber"] = recording.useTotalPedNumber;
	formarttedData["recording"]["totalPedNumber"] = recording.totalPedNumber.to_json();

	formarttedData["replay"]["replayInterval"] = replay.replayInterval;
	formarttedData["replay"]["replayFile"] = replay.replayFile;
	formarttedData["replay"]["predictionFile"] = replay.predictionFile;

	formarttedData["camera"]["params"] = camera.params.to_json();
	formarttedData["camera"]["enabled"] = camera.enabled;

	formarttedData["vehicles"] = json::array();
	for (auto& vehSettings : vehicles) {
		json formattedVeh;
		formattedVeh["wandering"] = vehSettings.wandering;
		formattedVeh["starts"] = json::array();
		for (auto& lane : vehSettings.starts) {
			formattedVeh["starts"].push_back(lane.to_json());
		}
		formattedVeh["goals"] = json::array();
		for (auto& goal : vehSettings.goals) {
			formattedVeh["goals"].push_back(goal.to_json());
		}
		formattedVeh["vehModel"] = vehSettings.vehModel;
		//formattedVeh["driverModel"] = vehSettings.driverModel;
		formattedVeh["stopRange"] = vehSettings.stopRange;
		formattedVeh["driveStyle"] = vehSettings.driveStyle;
		formattedVeh["speed"] = vehSettings.speed.to_json();
		formattedVeh["carInterval"] = vehSettings.carInterval.to_json();
		formattedVeh["number"] = vehSettings.number.to_json();
		formattedVeh["startTime"] = vehSettings.startTime;
		formattedVeh["continuousGeneration"] = vehSettings.continuousGeneration;
		formattedVeh["generationInterval"] = vehSettings.generationInterval.to_json();
		formarttedData["vehicles"].push_back(formattedVeh);
	}

	for (auto& pedSettings : peds) {
		json formattedPed;
		formattedPed["wandering"] = pedSettings.wandering;
		formattedPed["starts"] = json::array();
		for (auto& lane : pedSettings.starts) {
			formattedPed["starts"].push_back(lane.to_json());
		}
		formattedPed["goals"] = json::array();
		for (auto& area : pedSettings.goals) {
			formattedPed["goals"].push_back(area.to_json());
		}
		formattedPed["model"] = pedSettings.model;
		formattedPed["running"] = pedSettings.running;
		formattedPed["number"] = pedSettings.number.to_json();
		formattedPed["startTime"] = pedSettings.startTime;
		formattedPed["continuousGeneration"] = pedSettings.continuousGeneration;
		formattedPed["generationInterval"] = pedSettings.generationInterval.to_json();
		formarttedData["peds"].push_back(formattedPed);

	}

	config << formarttedData;
	config.close();
	settingsFile = fileName;
}

std::string SimulationData::getSettingsFileName() {
	return settingsFile;
}