#include "samplerMenu.h"

std::vector<Marker> createReferencePoints(SimulationData& data) {
	std::vector<Marker> res;
	for (auto& veh : data.vehicles) {
		for (auto& lane : veh.starts) {
			res.push_back({ lane.front, DefaultColor::blue.makeTransparent(50) });
			res.push_back({ lane.back, DefaultColor::green.makeTransparent(50) });
		}
		for (auto& point : veh.goals) {
			res.push_back({ point, DefaultColor::yellow.makeTransparent(50) });
		}
	}
	WAIT(0);
	for (auto& ped : data.peds) {
		for (auto& area : ped.starts) {
			res.push_back({ area.center, DefaultColor::red.makeTransparent(50) });
			res.push_back({ area.xUnit, DefaultColor::red.makeTransparent(50) });
			res.push_back({ area.yUnit, DefaultColor::red.makeTransparent(50) });
			res.push_back({ area.xUnit + area.yUnit - area.center, DefaultColor::red.makeTransparent(50) });
		}
		for (auto& area : ped.goals) {
			res.push_back({ area.center, DefaultColor::black.makeTransparent(50) });
			res.push_back({ area.xUnit, DefaultColor::black.makeTransparent(50) });
			res.push_back({ area.yUnit, DefaultColor::black.makeTransparent(50) });
			res.push_back({ area.xUnit + area.yUnit - area.center, DefaultColor::black.makeTransparent(50) });
		}
	}
	WAIT(0);
	if (res.size() > 64) {
		outputDebugMessage("Too many reference points for sampling. Truncating to 64.");
		res.resize(64);
	}

	return res;
}

// TODO: merge similar functions (e.g. resizeable lists)
template <
	typename T,
	typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type
>
void setNumber(T& value, std::string& description) {
	std::string text = GamePlay::getTextInput();
	if (text != "" && isFloat(text)) {
		value = std::stof(text);
		description = text;
	}
}

void setString(std::string& value, std::string& description) {
	std::string text = GamePlay::getTextInput();
	if (text != "") {
		value = text;
		description = value;
	}
}

void setVector3d(SimulationData& data, Vector3d& value, std::string& description, std::string helpText, const Color& markerColor) {
	auto reference = createReferencePoints(data);
	auto res = Sampling::samplePoints(1, helpText, markerColor, reference, { value });
	if (!res.empty()) {
		value = res[0];
		description = value.to_string();
	}
};

bool carLaneMenu(SimulationData& data, CarLane& lane, LPCSTR model) {
	Menu laneMenu("Select a property to change", {});
	bool deleting = false;
	auto sampleFront = [&]() {
		setVector3d(data, lane.front, laneMenu.items[0].description, "Mark the front of the lane.", DefaultColor::blue);
	};
	auto sampleBack = [&]() {
		setVector3d(data, lane.back, laneMenu.items[1].description, "Mark the back of the lane.", DefaultColor::green);
	};

	auto sampleHeading = [&]() {
		VehicleWithMission& v = GameResources::spawnVehicleAtCoords(model, lane.sample(), lane.heading);
		ENTITY::FREEZE_ENTITY_POSITION(v.veh, true);
		ENTITY::SET_ENTITY_COLLISION(v.veh, false, false);
		lane.heading = Sampling::sampleRotation("Rotate Object to desired heading at start.", v.veh);
		laneMenu.items[2].description = std::to_string(lane.heading);
		GameResources::deleteAllCreated();
	};

	auto deleteLane = [&]() {
		deleting = true;
		laneMenu.singleUse = true;
	};

	laneMenu.addMenuItem({ "Front", sampleFront, lane.front.to_string() });
	laneMenu.addMenuItem({ "Back", sampleBack, lane.back.to_string() });
	laneMenu.addMenuItem({ "Heading", sampleHeading, std::to_string(lane.heading) });
	laneMenu.addMenuItem({ "Delete Lane Setting", deleteLane });

	laneMenu.processMenu();

	return !deleting;
}

bool individualVehicleMenu(SimulationData& data, SimulationData::VehSettings& vehSettings, int index) {
	Menu vehicleSettingsMenu("Vehicle Group " + std::to_string(index), {});
	bool deleting = false;

	auto switchWandering = [&]() {
		vehSettings.wandering = !vehSettings.wandering;
	};

	// There are some duplicates of these "group list" functions
	// I keep these to avoid using template
	// TODO:: refactor if you want
	auto setVehicleStarts = [&]() {
		Menu vehicleStartsList("Select a start lane configuration", {});
		int count = 0;

		auto carLaneBinder = [&](int i) {
			if (!carLaneMenu(data, vehSettings.starts[i], vehSettings.vehModel.c_str())) {
				// prevent deleting self before finishing execution
				std::function<void()> self;
				if (i == vehicleStartsList.lineCount() - 2) {
					self = std::move(vehicleStartsList.items[i].function);
				}
				vehSettings.starts.erase(vehSettings.starts.begin() + i);
				vehicleStartsList.deleteItem(vehicleStartsList.lineCount() - 2);
			}
		};

		for (int i = 0; i < vehSettings.starts.size(); i++) {
			vehicleStartsList.addMenuItem({ "Lane " + std::to_string(i), std::bind(carLaneBinder , i) });
		}
		vehicleStartsList.addMenuItem({ "Add New Lane", [&]() {
			vehSettings.starts.push_back(CarLane());
			if (!carLaneMenu(data, vehSettings.starts.back(), vehSettings.vehModel.c_str())) {
				vehSettings.starts.pop_back();
			}
			else {
				vehicleStartsList.items.insert(vehicleStartsList.items.end() - 1, { "Lane " + std::to_string(vehSettings.starts.size() - 1), std::bind(carLaneBinder , vehSettings.starts.size() - 1) });
			}
		} });
		vehicleStartsList.processMenu();
	};

	auto setVehicleGoals = [&]() {
		auto reference = createReferencePoints(data);
		vehSettings.goals = Sampling::samplePoints(-1, "Edit the goals.", DefaultColor::yellow, reference, vehSettings.goals);
	};

	auto deleteVeh = [&]() {
		deleting = true;
		vehicleSettingsMenu.singleUse = true;
	};

	auto setVehicleModel = [&]() {
		std::vector<std::string> modelList = {
			"adder",
			"oracle",
			"cheetah",
			"bus",
			"speedo"
		};
		Vector3d coords = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), ENTITY::IS_ENTITY_DEAD(PLAYER::PLAYER_PED_ID()));
		if (!vehSettings.starts.empty()) {
			coords = vehSettings.starts[0].sample();
		}
		coords.z += 2.0;
		float rotation = 0.0;
		Menu modelMenu("Pick a Model", {});
		modelMenu.singleUse = true;
		for (int i = 0; i < modelList.size(); i++) {
			modelMenu.addMenuItem({ modelList[i], std::bind([&](int index) {
				vehSettings.vehModel = modelList[index];
				vehicleSettingsMenu.items[3].description = modelList[index];
			}, i) });
		}

		Camera customCam = GamePlay::activateCamera(CameraParams(Vector3d(coords.x + 3.0, coords.y + 3.0, coords.z + 3.0), Vector3d(), CAM::GET_GAMEPLAY_CAM_FOV()));
		CAM::POINT_CAM_AT_COORD(customCam, coords.x, coords.y, coords.z);

		GamePlay::setPlayerDisappear(true);

		VehicleWithMission v;
		int viewing = -1;
		while (true) {
			WAIT(0);
			GamePlay::clearArea();
			rotation += 2.0;
			if (viewing != modelMenu.lineActive) {
				viewing = modelMenu.lineActive;
				GameResources::deleteAllCreated();
				v = GameResources::spawnVehicleAtCoords((char *)(modelList[viewing]).c_str(), coords, rotation);
				ENTITY::FREEZE_ENTITY_POSITION(v.veh, true);
				ENTITY::SET_ENTITY_COLLISION(v.veh, false, false);
			}
			else {
				ENTITY::SET_ENTITY_HEADING(v.veh, rotation);
			}
			if (!modelMenu.onTick()) {
				GameResources::deleteAllCreated();
				break;
			}
		}

		GamePlay::destroyCamera(customCam);
		GamePlay::setPlayerDisappear(false);

	};

	auto setVehicleStopRange = [&]() {
		std::string stopRange = GamePlay::getTextInput();
		if (stopRange != "" && isFloat(stopRange)) {
			vehSettings.stopRange = std::stof(stopRange);
			vehicleSettingsMenu.items[4].description = stopRange;
		}
	};

	auto setVehicleSpeed = [&]() {
		std::string speed = GamePlay::getTextInput();
		if (speed != "") {
			vehSettings.speed = speed;
			vehicleSettingsMenu.items[5].description = vehSettings.speed.to_string();
		}
	};

	auto setCarInterval = [&]() {
		std::string carInterval = GamePlay::getTextInput();
		if (carInterval != "") {
			vehSettings.carInterval = carInterval;
			vehicleSettingsMenu.items[6].description = vehSettings.carInterval.to_string();
		}
	};

	auto setVehicleNumber = [&]() {
		std::string number = GamePlay::getTextInput();
		if (number != "") {
			vehSettings.number = number;
			vehicleSettingsMenu.items[7].description = vehSettings.number.to_string();
		}
	};

	auto setVehicleStartTime = [&]() {
		std::string startTime = GamePlay::getTextInput();
		if (startTime != "" && isFloat(startTime)) {
			vehSettings.startTime = std::stof(startTime);
			vehicleSettingsMenu.items[8].description = startTime;
		}
	};

	auto switchContinuousGeneration = [&]() {
		vehSettings.continuousGeneration = !vehSettings.continuousGeneration;
	};

	auto setVehicleGenerationInterval = [&]() {
		std::string generationInterval = GamePlay::getTextInput();
		if (generationInterval != "") {
			vehSettings.generationInterval = generationInterval;
			vehicleSettingsMenu.items[10].description = vehSettings.generationInterval.to_string();
		}
	};


	vehicleSettingsMenu.addMenuItem({"Wandering", switchWandering, "", &(vehSettings.wandering) });
	vehicleSettingsMenu.addMenuItem({"Starts", setVehicleStarts });
	vehicleSettingsMenu.addMenuItem({"Goals", setVehicleGoals, "",  NULL, &(vehSettings.wandering) , true});
	vehicleSettingsMenu.addMenuItem({"Model", setVehicleModel , vehSettings.vehModel });
	vehicleSettingsMenu.addMenuItem({"Stop Range", setVehicleStopRange, std::to_string(vehSettings.stopRange) ,  NULL, &(vehSettings.wandering), true });
	vehicleSettingsMenu.addMenuItem({"Speed", setVehicleSpeed, vehSettings.speed.to_string() });
	vehicleSettingsMenu.addMenuItem({ "Car Interval", setCarInterval, vehSettings.carInterval.to_string() });
	if (data.recording.useTotalVehicleNumber) {
		vehicleSettingsMenu.addMenuItem({ "Number", setVehicleNumber, vehSettings.number.to_string(), NULL, &(vehSettings.continuousGeneration)});
	}
	else {
		vehicleSettingsMenu.addMenuItem({ "Number", setVehicleNumber, vehSettings.number.to_string() });
	}
	vehicleSettingsMenu.addMenuItem({ "Start Time", setVehicleStartTime, std::to_string(vehSettings.startTime) });
	vehicleSettingsMenu.addMenuItem({ "Continuous Generation", switchContinuousGeneration, "", &(vehSettings.continuousGeneration) });
	vehicleSettingsMenu.addMenuItem({ "Generation Interval", setVehicleGenerationInterval, vehSettings.generationInterval.to_string(),  NULL, &(vehSettings.continuousGeneration) });

	vehicleSettingsMenu.addMenuItem({ "Delete Vehicle", deleteVeh });

	vehicleSettingsMenu.processMenu();

	return !deleting;
}

void vehicleMenu(SimulationData& data) {
		Menu vehicleGroupList("Select a vehicle group configuration", {});
		int count = 0;

		auto individualVehBinder = [&](int i) {
			if (!individualVehicleMenu(data, data.vehicles[i], i)) {
				std::function<void()> self;
				if (i == vehicleGroupList.lineCount() - 2) {
					self = std::move(vehicleGroupList.items[i].function);
				}
				data.vehicles.erase(data.vehicles.begin() + i);
				vehicleGroupList.deleteItem(vehicleGroupList.lineCount() - 2);
			}
		};

		for (int i = 0; i < data.vehicles.size(); i++) {
			vehicleGroupList.addMenuItem({ "Vehicle Group " + std::to_string(i), std::bind(individualVehBinder, i) });
		}
		vehicleGroupList.addMenuItem({ "Add New Vehicle Setting", [&]() {
			data.vehicles.push_back(SimulationData::VehSettings());
			if (!individualVehicleMenu(data, data.vehicles.back(), data.vehicles.size()-1)) {
				data.vehicles.pop_back();
			}
			else {
				vehicleGroupList.items.insert(vehicleGroupList.items.end() - 1, { "Vehicle Group " + std::to_string(data.vehicles.size() - 1), std::bind(individualVehBinder , data.vehicles.size() - 1) });
			}
		} });
		vehicleGroupList.processMenu();
}

bool unitAreaMenu(SimulationData& data, UnitArea& area, bool isGoal) {
	Menu areaMenu("Select a property to change", {});
	bool deleting = false;
	auto sampleCenter = [&]() {
		auto reference = createReferencePoints(data);
		auto res = Sampling::samplePoints(1, "Mark the central corner of the lane.", isGoal ? DefaultColor::black : DefaultColor::red, reference);
		if (!res.empty()) {
			area.center = res[0];
			areaMenu.items[0].description = area.center.to_string(); // TODO: add change description interface
		}
	};

	auto sampleCorner = [&](bool isX) {
		auto reference = createReferencePoints(data);
		auto res = Sampling::samplePoints(1, "Mark an adjacent corner to the center.", isGoal ? DefaultColor::black : DefaultColor::red, reference);
		if (!res.empty()) {
			if (isX) {
				area.xUnit = res[0];
				areaMenu.items[1].description = area.xUnit.to_string();
			}
			else {
				area.yUnit = res[0];
				areaMenu.items[2].description = area.yUnit.to_string();
			}
		}
	};

	auto deleteArea = [&]() {
		deleting = true;
		areaMenu.singleUse = true;
	};

	areaMenu.addMenuItem({ "Center", sampleCenter, area.center.to_string() });
	areaMenu.addMenuItem({ "Corner 1", std::bind(sampleCorner, true), area.xUnit.to_string() });
	areaMenu.addMenuItem({ "Corner 2", std::bind(sampleCorner, false), area.yUnit.to_string() });
	areaMenu.addMenuItem({ "Delete Area Setting", deleteArea });

	areaMenu.processMenu();

	return !deleting;

}

bool individualPedMenu(SimulationData& data, SimulationData::PedSettings& pedSettings, int index) {
	Menu pedSettingsMenu("Pedestrian Group " + std::to_string(index), {});
	bool deleting = false;

	auto switchWandering = [&]() {
		pedSettings.wandering = !pedSettings.wandering;
	};

	auto setPedArea = [&](bool isGoal) {
		std::string title = isGoal ? "Select a goal area configuration" : "Select a start area configuration";
		auto& areaData = isGoal ? pedSettings.goals : pedSettings.starts;
		Menu pedAreaList(title, {});
		int count = 0;

		auto unitAreaBinder = [&](int i) {
			if (!unitAreaMenu(data, areaData[i], isGoal)) {
				std::function<void()> self;
				if (i == pedAreaList.lineCount() - 2) {
					self = std::move(pedAreaList.items[i].function);
				}
				areaData.erase(areaData.begin() + i);
				pedAreaList.deleteItem(pedAreaList.lineCount() - 2);
			}
		};

		for (int i = 0; i < areaData.size(); i++) {
			pedAreaList.addMenuItem({ "Area " + std::to_string(i), std::bind(unitAreaBinder, i) });
		}
		pedAreaList.addMenuItem({ "Add New Area", [&]() {
			areaData.push_back(UnitArea());
			if (!unitAreaMenu(data, areaData.back(), isGoal)) {
				areaData.pop_back();
			}
			else {
				pedAreaList.items.insert(pedAreaList.items.end() - 1, { "Area " + std::to_string(areaData.size() - 1), std::bind(unitAreaBinder , areaData.size() - 1) });
			}
		} });
		pedAreaList.processMenu();
	};


	auto deletePed = [&]() {
		deleting = true;
		pedSettingsMenu.singleUse = true;
	};

	auto setPedModel = [&]() {
		std::vector<std::string> modelList = {
			"a_f_m_bevhills_01",
			"a_f_m_business_02",
			"a_f_y_runner_01",
			"a_m_y_vinewood_01"
		};
		Vector3d coords = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), ENTITY::IS_ENTITY_DEAD(PLAYER::PLAYER_PED_ID()));
		if (!pedSettings.starts.empty()) {
			coords = pedSettings.starts[0].sample();
		}
		float rotation = 0.0;
		Menu modelMenu("Pick a Model", {});
		modelMenu.singleUse = true;
		for (int i = 0; i < modelList.size(); i++) {
			modelMenu.addMenuItem({ modelList[i], std::bind([&](int index) {
				pedSettings.model = modelList[index];
				pedSettingsMenu.items[3].description = modelList[index];
			}, i) });
		}

		Camera customCam = GamePlay::activateCamera(CameraParams(Vector3d(coords.x + 3.0, coords.y + 3.0, coords.z + 3.0), Vector3d(), CAM::GET_GAMEPLAY_CAM_FOV()));
		CAM::POINT_CAM_AT_COORD(customCam, coords.x, coords.y, coords.z);

		GamePlay::setPlayerDisappear(true);

		PedWithMission p;
		int viewing = -1;
		while (true) {
			WAIT(0);
			GamePlay::clearArea();
			rotation += 2.0;
			if (viewing != modelMenu.lineActive) {
				viewing = modelMenu.lineActive;
				GameResources::deleteAllCreated();
				p = GameResources::spawnPedAtCoords((char *)(modelList[viewing]).c_str(), coords, rotation);
				ENTITY::FREEZE_ENTITY_POSITION(p.ped, true);
				ENTITY::SET_ENTITY_COLLISION(p.ped, false, false);
			}
			else {
				ENTITY::SET_ENTITY_HEADING(p.ped, rotation);
			}
			if (!modelMenu.onTick()) {
				GameResources::deleteAllCreated();
				break;
			}
		}

		GamePlay::destroyCamera(customCam);
		GamePlay::setPlayerDisappear(false);

	};

	auto switchPedRunning = [&]() {
		pedSettings.running = !pedSettings.running;
	};

	auto setPedNumber = [&]() {
		std::string number = GamePlay::getTextInput();
		if (number != "") {
			pedSettings.number = number;
			pedSettingsMenu.items[5].description = pedSettings.number.to_string();
		}
	};

	auto setPedStartTime = [&]() {
		std::string startTime = GamePlay::getTextInput();
		if (startTime != "" && isFloat(startTime)) {
			pedSettings.startTime = std::stof(startTime);
			pedSettingsMenu.items[6].description = startTime;
		}
	};

	auto switchContinuousGeneration = [&]() {
		pedSettings.continuousGeneration = !pedSettings.continuousGeneration;
	};

	auto setPedGenerationInterval = [&]() {
		std::string generationInterval = GamePlay::getTextInput();
		if (generationInterval != "") {
			pedSettings.generationInterval = generationInterval;
			pedSettingsMenu.items[8].description = pedSettings.generationInterval.to_string();
		}
	};


	pedSettingsMenu.addMenuItem({ "Wandering", switchWandering, "", &(pedSettings.wandering) });
	pedSettingsMenu.addMenuItem({ "Starts", std::bind(setPedArea, false) });
	pedSettingsMenu.addMenuItem({ "Goals", std::bind(setPedArea, true), "",  NULL, &(pedSettings.wandering), true });
	pedSettingsMenu.addMenuItem({ "Model", setPedModel , pedSettings.model });
	pedSettingsMenu.addMenuItem({ "Running", switchPedRunning, "", &(pedSettings.running), &(pedSettings.wandering), true });
	if (data.recording.useTotalPedNumber) {
		pedSettingsMenu.addMenuItem({ "Number", setPedNumber, pedSettings.number.to_string(),  NULL, &(pedSettings.continuousGeneration) });
	}
	else {
		pedSettingsMenu.addMenuItem({ "Number", setPedNumber, pedSettings.number.to_string() });
	}
	pedSettingsMenu.addMenuItem({ "Start Time", setPedStartTime, std::to_string(pedSettings.startTime) });
	pedSettingsMenu.addMenuItem({ "Continuous Generation", switchContinuousGeneration, "", &(pedSettings.continuousGeneration) });
	pedSettingsMenu.addMenuItem({ "Generation Interval", setPedGenerationInterval, pedSettings.generationInterval.to_string(),  NULL, &(pedSettings.continuousGeneration) });

	pedSettingsMenu.addMenuItem({ "Delete Ped", deletePed });

	pedSettingsMenu.processMenu();

	return !deleting;
}

void pedMenu(SimulationData& data) {
	Menu pedGroupList("Select a pedestrian group configuration", {});
	int count = 0;

	auto individualPedBinder = [&](int i) {
		if (!individualPedMenu(data, data.peds[i], i)) {
			std::function<void()> self;
			if (i == pedGroupList.lineCount() - 2) {
				self = std::move(pedGroupList.items[i].function);
			}
			data.peds.erase(data.peds.begin() + i);
			pedGroupList.deleteItem(pedGroupList.lineCount() - 2);
		}
	};

	for (int i = 0; i < data.peds.size(); i++) {
		pedGroupList.addMenuItem({ "Pedestrian Group " + std::to_string(i), std::bind(individualPedBinder, i) });
	}
	pedGroupList.addMenuItem({ "Add New Pedestrian Setting", [&]() {
		data.peds.push_back(SimulationData::PedSettings());
		if (!individualPedMenu(data, data.peds.back(), data.peds.size() - 1)) {
			data.peds.pop_back();
		}
		else {
			pedGroupList.items.insert(pedGroupList.items.end() - 1, { "Pedestrian Group " + std::to_string(data.peds.size() - 1), std::bind(individualPedBinder , data.peds.size() - 1) });
		}
	} });
	pedGroupList.processMenu();
}

void recordingMenu(SimulationData& data) {
	Menu recordingOptions("Recording Options", {});

	auto setRecordCenter = [&]() {
		setVector3d(data, data.recording.recordCenter, recordingOptions.items[0].description, "Mark the recording center.", DefaultColor::grey);
	};

	auto teleportPlayerToCenter = [&]() {
		GamePlay::teleportPlayer(data.recording.recordCenter);
	};

	auto setRecordRadius = [&]() {
		setNumber(data.recording.recordRadius, recordingOptions.items[2].description);
		DWORD maxTickCount = GetTickCount() + 5000;
		do {
			drawCircle(data.recording.recordCenter, data.recording.recordRadius, DefaultColor::red);
			WAIT(0);
		} while (GetTickCount() < maxTickCount);
	};

	auto setNumSimulations = [&]() {
		setNumber(data.recording.numSimulations, recordingOptions.items[3].description);
	};

	auto setRecordInterval = [&]() {
		setNumber(data.recording.recordInterval, recordingOptions.items[4].description);
	};

	auto setRecordTime = [&]() {
		setNumber(data.recording.recordTime, recordingOptions.items[5].description);
	};

	auto switchStopWhenNoVehicles = [&]() {
		data.recording.stopWhenNoVehicles = !data.recording.stopWhenNoVehicles;
	};

	auto switchUseTotalVehicleNumber = [&]() {
		data.recording.useTotalVehicleNumber = !data.recording.useTotalVehicleNumber;
	};
	auto setTotalVehicleNumber = [&]() {
		std::string totalVehicleNumber = GamePlay::getTextInput();
		if (totalVehicleNumber != "") {
			data.recording.totalVehicleNumber = totalVehicleNumber;
			recordingOptions.items[8].description = data.recording.totalVehicleNumber.to_string();
		}
	};
	auto switchUseTotalPedNumber = [&]() {
		data.recording.useTotalPedNumber = !data.recording.useTotalPedNumber;
	};
	auto setTotalPedNumber = [&]() {
		std::string totalPedNumber = GamePlay::getTextInput();
		if (totalPedNumber != "") {
			data.recording.totalPedNumber = totalPedNumber;
			recordingOptions.items[10].description = data.recording.totalPedNumber.to_string();
		}
	};

	auto setRecordDirectory = [&]() {
		setString(data.recording.recordDirectory, recordingOptions.items[11].description);
	};

	recordingOptions.addMenuItem({ "Record Center", setRecordCenter, data.recording.recordCenter.to_string() });
	recordingOptions.addMenuItem({ "Teleport To Record Center", teleportPlayerToCenter });
	recordingOptions.addMenuItem({ "Record Radius", setRecordRadius, std::to_string(data.recording.recordRadius) });
	recordingOptions.addMenuItem({ "Num Simulations", setNumSimulations, std::to_string(data.recording.numSimulations) });
	recordingOptions.addMenuItem({ "Record Interval", setRecordInterval, std::to_string(data.recording.recordInterval) });
	recordingOptions.addMenuItem({ "Record Time", setRecordTime, std::to_string(data.recording.recordTime) });
	recordingOptions.addMenuItem({ "Stop When No Vehicles", switchStopWhenNoVehicles, "", &(data.recording.stopWhenNoVehicles) });

	recordingOptions.addMenuItem({ "Use Total Vehicle Number", switchUseTotalVehicleNumber, "", &(data.recording.useTotalVehicleNumber) });
	recordingOptions.addMenuItem({ "Total Vehicle Number", setTotalVehicleNumber, data.recording.totalVehicleNumber.to_string(), NULL, &(data.recording.useTotalVehicleNumber) });
	recordingOptions.addMenuItem({ "Use Total Ped Number", switchUseTotalPedNumber, "", &(data.recording.useTotalPedNumber) });
	recordingOptions.addMenuItem({ "Total Ped Number", setTotalPedNumber, data.recording.totalPedNumber.to_string(), NULL, &(data.recording.useTotalPedNumber) });

	recordingOptions.addMenuItem({ "Record Directory", setRecordDirectory, data.recording.recordDirectory });

	recordingOptions.processMenu();
}

void replayMenu(SimulationData& data) {
	Menu replayOptions("Replay Options", {});

	auto setReplayInterval = [&]() {
		setNumber(data.replay.replayInterval, replayOptions.items[0].description);
	};

	auto setReplayFile = [&]() {
		setString(data.replay.replayFile, replayOptions.items[1].description);
	};

	auto setPredictionFile = [&]() {
		setString(data.replay.predictionFile, replayOptions.items[2].description);
	};

	replayOptions.addMenuItem({ "Replay Interval", setReplayInterval, std::to_string(data.replay.replayInterval) });
	replayOptions.addMenuItem({ "Replay File", setReplayFile, data.replay.replayFile });
	replayOptions.addMenuItem({ "Prediction File", setPredictionFile, data.replay.predictionFile });

	replayOptions.processMenu();
}

void cameraMenu(SimulationData& data) {
	Menu cameraOptions("Camera Options", {});


	auto switchEnabled = [&]() {
		data.camera.enabled = !data.camera.enabled;
	};
	auto setCameraParams = [&]() {
		auto referece = createReferencePoints(data);
		CameraParams params = Sampling::sampleCameraParams("Rotate and move camera to the desired location.", referece, data.camera.params);
		if (params.fov > 0) {
			data.camera.params = params;
			cameraOptions.items[1].description = data.camera.params.position.to_string();
		}
	};

	cameraOptions.addMenuItem({ "Enabled", switchEnabled, "", &(data.camera.enabled) });
	cameraOptions.addMenuItem({ "Camera Location", setCameraParams, data.camera.params.position.to_string(), NULL, &(data.camera.enabled) });

	cameraOptions.processMenu();
}
