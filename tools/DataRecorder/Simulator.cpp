#include "Simulator.h"

const int PED_ID_OFFSET = 100;

Simulator::Simulator(SimulationData& data): data(data) {}

void Simulator::startSimulation() {
	Cam cam;
	if (data.camera.enabled) {
		cam = GamePlay::activateCamera(data.camera.params);
		GamePlay::setPlayerDisappear(true);
	}

	// for each generation setting, use one DWORD to record the next time instance for repeated generation.
	// tick is set to 0 to indicate disabled generation.
	std::vector<DWORD> nextTickCountPed;
	std::vector<DWORD> nextTickCountVeh;
	bool isAccident;
	bool forceBreak;

	std::vector<int> numPeds;
	std::vector<int> numVehs;

	// I made this a lambda to separate the recording function from checking termination conditions and
	// making other updates
	// it controls the continuous generation and deletion of injured/useless peds/vehs
	// if ped injured or force break pressed, it will return false to interrupt the simulation
	auto delegate = [&]() {
		GamePlay::clearArea();

		DWORD currentTick = GetTickCount();

		for (int i = 0; i < data.peds.size(); i++) {
			if (nextTickCountPed[i] != 0 && nextTickCountPed[i] <= currentTick) {
				if (data.peds[i].continuousGeneration) {
					setPeds(data.peds[i]);
					nextTickCountPed[i] = currentTick + data.peds[i].generationInterval.sample();
				}
				else {
					if (data.recording.useTotalPedNumber) {
						setPeds(data.peds[i], numPeds[i]);
					}
					else {
						setPeds(data.peds[i]);
					}
					nextTickCountPed[i] = 0;

				}
			}
		}
		for (int i = 0; i < data.vehicles.size(); i++) {
			if (nextTickCountVeh[i] != 0 && nextTickCountVeh[i] <= currentTick) {
				if (data.vehicles[i].continuousGeneration) {
					setCars(data.vehicles[i]);
					nextTickCountVeh[i] = currentTick + data.vehicles[i].generationInterval.sample();
				}
				else {
					if (data.recording.useTotalVehicleNumber) {
						setCars(data.vehicles[i], numVehs[i]);
					}
					else {
						setCars(data.vehicles[i]);
					}
					nextTickCountVeh[i] = 0;
				}
			}
		}

		for (int i = 0; i < GameResources::createdVehicles.size(); i++) {
			if (PED::IS_PED_IN_MELEE_COMBAT(GameResources::createdVehicles[i].driver)) {
				outputDebugMessage("Ped fighting. Break.");
				isAccident = true;
				return false;
			}
			if (ENTITY::HAS_ENTITY_COLLIDED_WITH_ANYTHING(GameResources::createdVehicles[i].veh)) {
				outputDebugMessage("Car collided with something. Break.");
				isAccident = true;
				return false;
			}

			if (AI::GET_SEQUENCE_PROGRESS(GameResources::createdVehicles[i].driver) == -1) {
				outputDebugMessage("Deleting Car " + std::to_string(i));
				GameResources::deleteVeh(i);
				i--;
			}
		}

		for (int i = 0; i < GameResources::createdPeds.size(); i++) {
			if (PED::IS_PED_IN_MELEE_COMBAT(GameResources::createdPeds[i].ped) || PED::IS_PED_RAGDOLL(GameResources::createdPeds[i].ped)) {
				outputDebugMessage("Ped fighting. Break.");
				isAccident = true;
				return false;
			}
			if (AI::GET_SEQUENCE_PROGRESS(GameResources::createdPeds[i].ped) == -1) {
				outputDebugMessage("Deleting Ped " + std::to_string(i));
				GameResources::deletePed(i);
				i--;
			}
		}

		if (IsKeyJustUp(VK_NUMPAD0)) {
			outputDebugMessage("Force break.");
			forceBreak = true;
			return false;
		}
		return true;
	};


	for (int i = 0; i < data.recording.numSimulations; i++) {
		outputDebugMessage("start simulation " + std::to_string(i) + ".");
		GamePlay::clearArea();
		outputDebugMessage("clearArea complete.");
		DWORD startTick = GetTickCount();
		nextTickCountPed = {};
		nextTickCountVeh = {};
		for (auto& ped : data.peds) {
			nextTickCountPed.push_back(ped.startTime + startTick);
		}
		for (auto& veh : data.vehicles) {
			nextTickCountVeh.push_back(veh.startTime + startTick);
		}
		if (data.recording.useTotalPedNumber) {
			numPeds = std::vector<int>(data.peds.size(), 0);
			int total = data.recording.totalPedNumber.sample();
			int numGroups = 0;
			for (auto setting : data.peds) {
				if (!setting.continuousGeneration) {
					numGroups++;
				}
			}
			if (numGroups != 0) {
				std::vector<int> counts(numGroups, 0);
				for (int i = 0; i < total; i++) {
					int group = randomInt(0, numGroups - 1);
					counts[group]++;
				}
				for (int i = 0; i < data.peds.size(); i++) {
					if (!data.peds[i].continuousGeneration) {
						numPeds[i] = counts.back();
						counts.pop_back();
					}
				}
			}
		}
		if (data.recording.useTotalVehicleNumber) {
			numVehs = std::vector<int>(data.vehicles.size(), 0);
			int total = data.recording.totalVehicleNumber.sample();
			int numGroups = 0;
			for (auto setting : data.vehicles) {
				if (!setting.continuousGeneration) {
					numGroups++;
				}
			}
			if (numGroups != 0) {
				std::vector<int> counts(numGroups, 0);
				for (int i = 0; i < total; i++) {
					int group = randomInt(0, numGroups - 1);
					counts[group]++;
				}
				for (int i = 0; i < data.vehicles.size(); i++) {
					if (!data.vehicles[i].continuousGeneration) {
						numVehs[i] = counts.back();
						counts.pop_back();
					}
				}
			}
		}
		isAccident = false;
		forceBreak = false;
		outputDebugMessage("initialize variables complete.");
		processRecording(delegate, "record" + std::to_string(i));
		GameResources::deleteAllCreated();
		outputDebugMessage("deleteAll complete.");
		WAIT(0);
		if (isAccident) {
			i--;
		}
		if (forceBreak) {
			break;
		}
	}

	if (data.camera.enabled) {
		GamePlay::destroyCamera(cam);
		GamePlay::setPlayerDisappear(false);
	}
}

// record ped&veh states at each frame
// stop when delegate tells it to stop
// only start recording when the first vehicle appear on the screen
void Simulator::processRecording(std::function<bool()> delegate, std::string fileName) {

	std::ofstream record;
	createAllSubdirectories("DataRecorder/" + data.recording.recordDirectory);
	record.open("DataRecorder/" + data.recording.recordDirectory + "/" + fileName);
	outputDebugMessage("Logging into file DataRecorder/" + data.recording.recordDirectory + "/" + fileName);

	std::unordered_map<Entity, int> IDMap;

	DWORD startTick = GetTickCount();
	int count = 0;
	int carID = 0;
	int pedID = PED_ID_OFFSET; // constant offset

	// run delegate first to setup the initial positions of the cars 
	if (!delegate()) {
		return;
	}

	while (true) {
		bool seeACar = false;
		for (int i = 0; i < GameResources::createdVehicles.size(); i++) {
			Entity e = GameResources::createdVehicles[i].veh;
			if (!IDMap.count(e)) {
				IDMap[e] = carID;
				carID++;
			}

			Vector3d coords3D;
			Vector2 coords2D;
			float heading;
			// float speed;
			GamePlay::getEntityMotion(e, &coords3D, &coords2D, &heading, NULL);

			if (SYSTEM::VDIST(coords3D.x, coords3D.y, coords3D.z, data.recording.recordCenter.x, data.recording.recordCenter.y, data.recording.recordCenter.z) > data.recording.recordRadius) {
				continue;
			}
			seeACar = true;

			record << count << "," << IDMap[e] << "," << coords2D.x << "," << coords2D.y << "," << coords3D.x << "," << coords3D.y << "," << coords3D.z << "," << heading << std::endl;
		}
		if (seeACar) {
			for (int i = 0; i < GameResources::createdPeds.size(); i++) {
				Entity e = GameResources::createdPeds[i].ped;
				if (!IDMap.count(e)) {
					IDMap[e] = pedID;
					pedID++;
				}
				Vector3d coords3D;
				Vector2 coords2D;
				float heading;
				//float speed;
				GamePlay::getEntityMotion(e, &coords3D, &coords2D, &heading, NULL);

				if (SYSTEM::VDIST(coords3D.x, coords3D.y, coords3D.z, data.recording.recordCenter.x, data.recording.recordCenter.y, data.recording.recordCenter.z) > data.recording.recordRadius) {
					continue;
				}
				record << count << "," << IDMap[e] << "," << coords2D.x << "," << coords2D.y << "," << coords3D.x << "," << coords3D.y << "," << coords3D.z << "," << heading << std::endl;
			}
		}

		DWORD maxTickCount = GetTickCount() + data.recording.recordInterval;
		bool stopRecording = false;
		while (GetTickCount() < maxTickCount) {
			if (!delegate()) {
				stopRecording = true;
				break;
			}
			if (startTick + data.recording.recordTime <= GetTickCount()) {
				outputDebugMessage("Time limit reached. Stop recording.");
				stopRecording = true;
				break;
			}
			WAIT(0);
		}
		if (stopRecording) {
			break;
		}
		if (!seeACar && count != 0 && data.recording.stopWhenNoVehicles) {
			outputDebugMessage("No vehicles in the recording range. Break.");
			break;
		}

		// only start counting when the first vehicle appears on the recording range
		if (seeACar || count != 0)
			count++;;
		
	}
	record.close();

}

void Simulator::setCars(SimulationData::VehSettings& settings, int totalCars) {
	if (settings.starts.size() == 0) {
		outputDebugMessage("Start point for cars not specified.");
		return;
	}
	
	float carLength = GamePlay::getModelLength(settings.vehModel.c_str());

	// randomly distribute cars on each lane
	std::vector<int> numCars(settings.starts.size(), 0);
	if (totalCars == -1) {
		totalCars = settings.number.sample();
	}
	for (int i = 0; i < totalCars; i++) {
		int lane = randomInt(0, numCars.size()-1);
		numCars[lane]++;
	}
	outputDebugMessage("Assigned Lanes for " + std::to_string(totalCars) + " cars.");

	for (int i = 0; i < settings.starts.size(); i++) {
		outputDebugMessage("Spawn cars on Lane " + std::to_string(i));

		Vector3d start = settings.starts[i].sample();
		Vector3d front = Vector3d(settings.starts[i].front);
		Vector3d back = Vector3d(settings.starts[i].back);

		for (int j = 0; j < numCars[i]; j++) {
			outputDebugMessage("Spawn " + std::to_string(j) + "-th car.");

			VehicleWithMission& v = GameResources::spawnVehicleAtCoords(settings.vehModel.c_str(), start, settings.starts[i].heading);
			GameResources::spawnDriver(v, "");

			outputDebugMessage("Finished spawn car.");
			float speed = settings.speed.sample();

			if (settings.wandering) {
				v.scripted = false;
				GameResources::createVehTask(v, [&]() {
					AI::TASK_VEHICLE_DRIVE_WANDER(0, v.veh, speed, settings.driveStyle);
				});
			}
			else {
				if (settings.goals.size() == 0) {
					outputDebugMessage("Must specify at least one goal in scripted mode.");
					return;
				}

				int goalNum = randomInt(0, settings.goals.size() - 1);
				Vector3d goal = settings.goals[goalNum];
				outputDebugMessage("Going to " + std::to_string(goalNum) + "-th goal");
				GameResources::createVehTask(v, [&]() {
					AI::TASK_VEHICLE_DRIVE_TO_COORD(0, v.veh, goal.x, goal.y, goal.z, speed, 0, GAMEPLAY::GET_HASH_KEY((char *)(settings.vehModel.c_str())), settings.driveStyle, settings.stopRange, 1.0);
				});
			}

			float ratio = (settings.carInterval.sample() + carLength) / SYSTEM::VDIST(front.x, front.y, front.z, back.x, back.y, back.z);
			start = (back - front) * ratio + start;
			WAIT(0);
		}
	}

}

void Simulator::setPeds(SimulationData::PedSettings& settings, int totalPeds) {
	if (settings.starts.size() == 0) {
		outputDebugMessage("Start point for peds not specified.");
		return;
	}
	std::vector<Vector3d> pedPositions;

	// randomly distribute peds in each area
	std::vector<int> numPeds(settings.starts.size(), 0);
	if (totalPeds == -1) {
		totalPeds = settings.number.sample();
	}
	for (int i = 0; i < totalPeds; i++) {
		int area = randomInt(0, numPeds.size() - 1);
		numPeds[area]++;
	}

	for (int i = 0; i < settings.starts.size(); i++) {
		for (int j = 0; j < numPeds[i]; j++) {
			outputDebugMessage("Spawn " + std::to_string(j) + "-th ped.");

			Vector3d start = settings.starts[i].sample();
			bool tooClose = false;
			for (int k = 0; k < pedPositions.size(); k++) {
				if (SYSTEM::VDIST(pedPositions[k].x, pedPositions[k].y, pedPositions[k].z, start.x, start.y, start.z) < 0.5) {
					tooClose = true;
					break;
				}
			}
			if (tooClose) {
				j--;
				continue;
			}


			outputDebugMessage("Finished spawn ped.");

			if (settings.wandering) {
				PedWithMission& p = GameResources::spawnPedAtCoords(settings.model.c_str(), start);
				p.scripted = false;
				GameResources::createPedTask(p, [&]() {
					AI::TASK_WANDER_IN_AREA(0, data.recording.recordCenter.x, data.recording.recordCenter.y, data.recording.recordCenter.z, data.recording.recordRadius, 0.0, 0.0);
				});
			}
			else {
				Vector3d goal;
				int goalNum;
				if (!settings.goals.empty()) {
					goalNum = randomInt(0, settings.goals.size() - 1);
					goal = settings.goals[goalNum].sample();
				}
				else {
					if (settings.starts.size() < 2) {
						outputDebugMessage("Too few areas for ped generation. Stop.");
						return;
					}
					goalNum = randomInt(0, settings.starts.size() - 1);
					if (goalNum == i) {
						j--;
						continue;
					}
					goal = settings.starts[goalNum].sample();
				}
				outputDebugMessage("Going to " + std::to_string(goalNum) + "-th goal");
				PedWithMission& p = GameResources::spawnPedAtCoords(settings.model.c_str(), start);
				GameResources::createPedTask(p, [&]() {
					AI::TASK_GO_STRAIGHT_TO_COORD(0, goal.x, goal.y, goal.z, 1.0 + settings.running, -1, 0.0, 0.2);
				});
			}
			pedPositions.push_back(start);

			WAIT(0);
		}
	}

}

// specific to Project code
// TODO: change to general one
void Simulator::loadPredictions(std::unordered_map<int, std::unordered_map<int, std::vector<std::vector<Vector2>>>>& coordsMap) {
	std::ifstream prediction("DataRecorder/" + data.replay.predictionFile);
	outputDebugMessage("Loading prediction file " + data.replay.predictionFile);
	int predictionNum;
	int counter = 0;
	while (prediction >> predictionNum) {
		prediction.ignore(1000, ',');

		int timestamp;
		int id;
		Vector2 coords;

		prediction >> timestamp;
		prediction.ignore(1000, ',');

		prediction >> id;
		prediction.ignore(1000, ',');

		prediction >> coords.x;
		prediction.ignore(1000, ',');
		prediction >> coords.y;
		prediction.ignore(1000, '\n');

		if (id < PED_ID_OFFSET) {
			if (counter == 0) {
				coordsMap[timestamp][id].push_back({});
			}
			coordsMap[timestamp][id][predictionNum].push_back(coords);
			counter = (counter + 1) % 12;
		}
	}

	prediction.close();
}

void Simulator::loadReplay(std::unordered_map<int, std::unordered_map <int, std::pair<Vector3d, float>>>& coordsMap, std::unordered_map<int, int>& lastAppear) {
	std::ifstream replay("DataRecorder/" + data.replay.replayFile);
	int timestamp;
	while (replay >> timestamp) {
		replay.ignore(1000, ',');

		int id;
		Vector3d coords3D;
		float heading;

		replay >> id;
		replay.ignore(1000, ',');

		// ignore 2D coords, not needed
		replay.ignore(1000, ',');
		replay.ignore(1000, ',');

		replay >> coords3D.x;
		replay.ignore(1000, ',');
		replay >> coords3D.y;
		replay.ignore(1000, ',');
		replay >> coords3D.z;
		replay.ignore(1000, ',');

		replay >> heading;
		replay.ignore(1000, '\n');

		lastAppear[id] = timestamp;
		coordsMap[timestamp][id] = std::make_pair(coords3D, heading);
	}

	replay.close();

}

void Simulator::processReplay(bool drawRainbow) {

	Color colors[] = { DefaultColor::red, DefaultColor::green, DefaultColor::blue };
	float boxSize = 0.1;

	std::unordered_map<int, int> lastAppears;
	std::unordered_map<int, std::unordered_map <int, std::pair<Vector3d, float>>> replayCoords;
	std::unordered_map<int, std::unordered_map <int, std::vector<std::vector<Vector2>>>> predictCoords;
	loadPredictions(predictCoords);
	outputDebugMessage("Loaded Prediction.");
	loadReplay(replayCoords, lastAppears);

	std::unordered_map<int, Entity> idMap;

	int waitTime = data.replay.replayInterval;
	int now = 0;

	Cam cam;
	if (data.camera.enabled) {
		cam = GamePlay::activateCamera(data.camera.params);
	}

	bool forceBreak = false;
	while (replayCoords.count(now) || predictCoords.count(now)) {
		GamePlay::clearArea();

		if (replayCoords.count(now)) {
			// recycle unappeared entities
			for (auto& lastAppear : lastAppears) {
				int id = lastAppear.first;
				int timestamp = lastAppear.second;
				if (timestamp == now - 1) {
					if (id >= PED_ID_OFFSET) {
						for (int i = 0; i < GameResources::createdPeds.size(); i++) {
							if (GameResources::createdPeds[i].ped == idMap[id]) {
								GameResources::deletePed(i);
							}
						}
					}
					else {
						for (int i = 0; i < GameResources::createdVehicles.size(); i++) {
							if (GameResources::createdVehicles[i].veh == idMap[id]) {
								GameResources::deleteVeh(i);
							}
						}
					}
					idMap.erase(id);
				}
			}

			// update positions of appeared entities
			for (auto& entityProps : replayCoords[now]) {
				int id = entityProps.first;
				Vector3d coords3D = entityProps.second.first;
				float heading = entityProps.second.second;
				if (idMap.count(id)) {
					Entity e = idMap[id];
					ENTITY::SET_ENTITY_COORDS_NO_OFFSET(e, coords3D.x, coords3D.y, coords3D.z, 1, 0, 0);
					ENTITY::SET_ENTITY_HEADING(e, heading);
				}
				else {
					if (id >= PED_ID_OFFSET) {
						PedWithMission& p = GameResources::spawnPedAtCoords("", coords3D, heading);
						idMap[id] = p.ped;
					}
					else {
						VehicleWithMission& v = GameResources::spawnVehicleAtCoords("Adder", coords3D, heading);
						GameResources::spawnDriver(v, "");
						idMap[id] = v.veh;
					}
				}
			}
		}
		else {
			GameResources::deleteAllCreated();
		}
		DWORD maxTickCount = GetTickCount() + waitTime;
		while (GetTickCount() < maxTickCount) {
			GamePlay::clearArea();
			//	if (IsKeyJustUp(VK_NUMPAD5)) {
			//		break;
			//	}
			if (IsKeyJustUp(VK_NUMPAD0)) {
				forceBreak = true;
				break;
			}

			if (predictCoords.count(now)) {
				//int stage = 1;
				//while (true) {
				//	if (IsKeyJustUp(VK_NUMPAD5)) {
				//		break;
				//	}
				//	if (IsKeyJustUp(VK_NUMPAD0)) {
				//		if (now > 0) {
				//			now -= 2;
				//			break;
				//		}
				//	}
				//	if (IsKeyJustUp(VK_NUMPAD1)) {
				//		stage = 1;
				//	}
				//	if (IsKeyJustUp(VK_NUMPAD2)) {
				//		stage = 2;
				//	}
				//	if (IsKeyJustUp(VK_NUMPAD3)) {
				//		stage = 3;
				//	}
				for (auto& p : predictCoords[now]) {
					Entity ent = idMap[p.first];
					Vector3 entPose = replayCoords[now][p.first].first;
					Vector3 groundTruth = replayCoords[now + 12][p.first].first;
					if (drawRainbow) {
						//if (stage == 1) {
						for (auto& c : p.second) {
							Vector3 forwardVec;
							forwardVec.x = c.back().x - entPose.x;
							forwardVec.y = c.back().y - entPose.y;

							float colorOffset = 255 - 255 * exp(-0.05 * GAMEPLAY::GET_DISTANCE_BETWEEN_COORDS(c.back().x, c.back().y, groundTruth.z, groundTruth.x, groundTruth.y, groundTruth.z, false));

							drawQuadrant(forwardVec, forwardVec.x, forwardVec.y, entPose, 1, colorOffset);
						}
					}
					//else if (stage == 2) {
					else {
						// sort the predictions according to how close its last frame is to the ground truth
						std::sort(p.second.begin(), p.second.end(), [&](const std::vector<Vector2>& lhs, const std::vector<Vector2>& rhs)
						{
							float lhsDist = SYSTEM::VDIST2(lhs.back().x, lhs.back().y, groundTruth.z, groundTruth.x, groundTruth.y, groundTruth.z);
							float rhsDist = SYSTEM::VDIST2(rhs.back().x, rhs.back().y, groundTruth.z, groundTruth.x, groundTruth.y, groundTruth.z);
							return lhsDist < rhsDist;
						});
						for (int i = 0; i < p.second.size(); i++) {
							bool counter = 0;
							Vector2 prev = { entPose.x, entPose.y };
							for (auto& c : p.second[i]) {
								if (counter == 1) {
									GRAPHICS::DRAW_LINE(c.x, c.y, entPose.z, prev.x, prev.y, entPose.z, colors[i].red, colors[i].green, colors[i].blue, colors[i].alpha);
									GRAPHICS::DRAW_BOX(c.x - boxSize, c.y - boxSize, entPose.z - boxSize, c.x + boxSize, c.y + boxSize, entPose.z + boxSize, colors[i].red, colors[i].green, colors[i].blue, colors[i].alpha);
									prev = c;
								}
								counter = !counter;
							}
						}
					}
				}
			}
			WAIT(0);
		}
		now++;
		if (forceBreak) {
			break;
		}
	}
	GameResources::deleteAllCreated();
	if (data.camera.enabled) {
		GamePlay::destroyCamera(cam);
	}
}
