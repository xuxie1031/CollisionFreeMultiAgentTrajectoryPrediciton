#include "demo.h"


namespace Demo {
	void plotPedTrajectory() {
		Vector3d pedStart(
			-1375.465087890625,
			-367.1982421875,
			36.615692138671875);
		Vector3d pedEnd(
			-1356.875,
			-392.15545654296875,
			36.57622528076172
		);
		GamePlay::teleportPlayer(pedStart, true);
		Ped player = PLAYER::PLAYER_PED_ID();
		ENTITY::SET_ENTITY_COLLISION(player, false, false);
		PED::SET_PED_GRAVITY(player, false);
		ENTITY::FREEZE_ENTITY_POSITION(player, true);
		ENTITY::SET_ENTITY_VISIBLE(player, false, 0);

		std::vector<Vector3d> markersGreen;
		std::vector<Vector3d> markersRed;
		std::vector<Vector3d> markersBlue;

		GamePlay::clearArea();

		Ped p = PED::CREATE_RANDOM_PED(pedStart.x, pedStart.y, pedStart.z);
		WAIT(200);

		Camera customCam = CAM::CREATE_CAM("DEFAULT_SCRIPTED_CAMERA", true);
		CAM::SET_CINEMATIC_MODE_ACTIVE(false);

		CAM::ATTACH_CAM_TO_ENTITY(customCam, p, 4.0, 4.0, 2.0, true);

		CAM::SET_CAM_ACTIVE(customCam, true);
		CAM::RENDER_SCRIPT_CAMS(true, false, 3000, true, false);

		TaskSequence sequence = 0;
		AI::OPEN_SEQUENCE_TASK(&sequence);
		AI::TASK_GO_STRAIGHT_TO_COORD(0, pedEnd.x, pedEnd.y, pedEnd.z, 1.0, 200000, 0.0, 0.2);
		AI::CLOSE_SEQUENCE_TASK(sequence);
		AI::TASK_PERFORM_SEQUENCE(p, sequence);
		WAIT(200);


		while (true) {
			GamePlay::clearArea();
			Vector3d coords = ENTITY::GET_ENTITY_COORDS(p, !ENTITY::IS_ENTITY_DEAD(p));
			markersGreen.push_back(coords);


			if (AI::GET_SEQUENCE_PROGRESS(p) == -1) {
				AI::CLEAR_SEQUENCE_TASK(&sequence);
				ENTITY::DELETE_ENTITY(&p);
				break;
			}

			DWORD maxTickCount = GetTickCount() + 2000;
			do {
				for (auto c : markersGreen) {
					drawMarker(MarkerTypeThickChevronUp, c, 0.5, DefaultColor::green);
				}
				WAIT(50);
			} while (GetTickCount() < maxTickCount);
		}

		// ---------------------------- 2nd person ------------------------------ //

		p = PED::CREATE_RANDOM_PED(pedStart.x, pedStart.y, pedStart.z);

		CAM::ATTACH_CAM_TO_ENTITY(customCam, p, 4.0, 4.0, 2.0, true);

		AI::OPEN_SEQUENCE_TASK(&sequence);
		AI::TASK_GO_STRAIGHT_TO_COORD(0, pedEnd.x, pedEnd.y, pedEnd.z, 2.0, 200000, 0.0, 0.2);
		AI::CLOSE_SEQUENCE_TASK(sequence);
		AI::TASK_PERFORM_SEQUENCE(p, sequence);
		WAIT(50);


		while (true) {
			GamePlay::clearArea();
			Vector3d coords = ENTITY::GET_ENTITY_COORDS(p, !ENTITY::IS_ENTITY_DEAD(p));
			markersRed.push_back(coords);

			if (AI::GET_SEQUENCE_PROGRESS(p) == -1) {
				AI::CLEAR_SEQUENCE_TASK(&sequence);
				ENTITY::DELETE_ENTITY(&p);
				break;
			}

			DWORD maxTickCount = GetTickCount() + 2000;
			do {
				for (auto c : markersGreen) {
					drawMarker(MarkerTypeThickChevronUp, c, 0.5, DefaultColor::green);
				}
				for (auto c : markersRed) {
					drawMarker(MarkerTypeThickChevronUp, c, 0.5, DefaultColor::red);
				}
				WAIT(50);
			} while (GetTickCount() < maxTickCount);
		}

		// -------------------------------- 3rd person ---------------------------- //

		p = PED::CREATE_RANDOM_PED(pedStart.x, pedStart.y, pedStart.z);

		CAM::ATTACH_CAM_TO_ENTITY(customCam, p, 2.0, 2.0, 2.0, true);
		CAM::POINT_CAM_AT_ENTITY(customCam, p, 0, 0, 0, true);

		AI::OPEN_SEQUENCE_TASK(&sequence);
		AI::TASK_WANDER_IN_AREA(0, -1382.03, -390.19, 36.69, 20.0, 0.0, 0.0);
		AI::CLOSE_SEQUENCE_TASK(sequence);
		AI::TASK_PERFORM_SEQUENCE(p, sequence);
		WAIT(50);

		DWORD stopTime = GetTickCount() + 120000;
		while (true) {
			GamePlay::clearArea();
			Vector3d coords = ENTITY::GET_ENTITY_COORDS(p, !ENTITY::IS_ENTITY_DEAD(p));
			markersBlue.push_back(coords);

			if (GetTickCount() >= stopTime) {
				break;
			}

			DWORD maxTickCount = GetTickCount() + 2000;
			do {
				for (auto c : markersGreen) {
					drawMarker(MarkerTypeThickChevronUp, c, 0.5, DefaultColor::green);
				}
				for (auto c : markersRed) {
					drawMarker(MarkerTypeThickChevronUp, c, 0.5, DefaultColor::red);
				}
				for (auto c : markersBlue) {
					drawMarker(MarkerTypeThickChevronUp, c, 0.5, DefaultColor::blue);
				}
				WAIT(50);
			} while (GetTickCount() < maxTickCount);
		}

		AI::CLEAR_SEQUENCE_TASK(&sequence);
		ENTITY::DELETE_ENTITY(&p);

		CAM::DETACH_CAM(customCam);
		CAM::STOP_CAM_POINTING(customCam);
		CAM::SET_CAM_COORD(customCam, -1382.03, -390.19, 36.69 + 40.0);
		CAM::POINT_CAM_AT_COORD(customCam, -1382.03, -390.19, 36.69);

		stopTime = GetTickCount() + 15000;
		while (GetTickCount() < stopTime) {
			for (auto c : markersGreen) {
				drawMarker(MarkerTypeThickChevronUp, c, 2.0, DefaultColor::green);
			}
			for (auto c : markersRed) {
				drawMarker(MarkerTypeThickChevronUp, c, 2.0, DefaultColor::red);
			}
			for (auto c : markersBlue) {
				drawMarker(MarkerTypeThickChevronUp, c, 2.0, DefaultColor::blue);
			}
			WAIT(50);
		}

		CAM::SET_CAM_ACTIVE(customCam, false);
		CAM::RENDER_SCRIPT_CAMS(false, false, 3000, true, false);
		CAM::DESTROY_CAM(customCam, true);


		ENTITY::SET_ENTITY_COLLISION(player, 1, 1);
		PED::SET_PED_GRAVITY(player, 1);
		ENTITY::FREEZE_ENTITY_POSITION(player, false);
		ENTITY::SET_ENTITY_VISIBLE(player, true, true);
	}

	void pedVarianceTest() {
		Vector3d pedStart(
			-1375.465087890625,
			-367.1982421875,
			36.615692138671875);
		Vector3d pedEnd(
			-1356.875,
			-392.15545654296875,
			36.57622528076172
		);
		GamePlay::teleportPlayer(pedStart, true);

		GamePlay::setPlayerDisappear(true);

		DWORD model = GAMEPLAY::GET_HASH_KEY("a_m_m_business_01");
		STREAMING::REQUEST_MODEL(model);
		while (!STREAMING::HAS_MODEL_LOADED(model)) WAIT(0);
		CreateDirectory("PedVarianceRec", NULL);


		for (int speed = 1; speed <= 2; speed++) {
			std::string speedDirName = "PedVarianceRec/speed" + std::to_string(speed);
			CreateDirectory(speedDirName.c_str(), NULL);
			for (int slideDistance = 0; slideDistance <= 5; slideDistance++) {
				std::string distanceDirName = speedDirName + "/slideDistance" + std::to_string(slideDistance);
				CreateDirectory(distanceDirName.c_str(), NULL);
				GamePlay::clearArea();
				for (int i = 0; i < 10; i++) {
					std::ofstream record;
					record.open(distanceDirName + "/record" + std::to_string(i));
					Ped p = PED::CREATE_PED(PED_TYPE_MISSION, model, pedStart.x, pedStart.y, pedStart.z, 0.0, FALSE, FALSE);
					WAIT(200);

					TaskSequence sequence = 0;
					AI::OPEN_SEQUENCE_TASK(&sequence);
					AI::TASK_GO_STRAIGHT_TO_COORD(0, pedEnd.x, pedEnd.y, pedEnd.z, speed, 200000, 0.0, slideDistance);
					AI::CLOSE_SEQUENCE_TASK(sequence);
					AI::TASK_PERFORM_SEQUENCE(p, sequence);
					WAIT(200);

					int counter = 0;
					while (true) {
						GamePlay::clearArea();
						Vector3d coords = ENTITY::GET_ENTITY_COORDS(p, !ENTITY::IS_ENTITY_DEAD(p));

						if (AI::GET_SEQUENCE_PROGRESS(p) == -1) {
							AI::CLEAR_SEQUENCE_TASK(&sequence);
							ENTITY::DELETE_ENTITY(&p);
							break;
						}

						record << GetTickCount() << "," << counter++ << "," << coords.x << "," << coords.y << "," << coords.z << std::endl;

						WAIT(100);
					}
					record.close();
				}
			}
		}
		GamePlay::setPlayerDisappear(false);

	}


	void carVarianceTest() {
		Vector3d carStart(
			-1418.6588134765625,
			-356.1112365722656,
			40.589717864990234);
		Vector3d carEnd(
			-1318.8980712890625,
			-366.9628601074219,
			36.69498825073242
		);
		GamePlay::teleportPlayer(carStart, true);
		Ped player = PLAYER::PLAYER_PED_ID();
		ENTITY::SET_ENTITY_COLLISION(player, false, false);
		PED::SET_PED_GRAVITY(player, false);
		ENTITY::FREEZE_ENTITY_POSITION(player, true);
		ENTITY::SET_ENTITY_VISIBLE(player, false, 0);

		DWORD carModel = GAMEPLAY::GET_HASH_KEY("Adder");
		STREAMING::REQUEST_MODEL(carModel);
		while (!STREAMING::HAS_MODEL_LOADED(carModel)) WAIT(0);

		DWORD pedModel = GAMEPLAY::GET_HASH_KEY("a_m_m_business_01");
		STREAMING::REQUEST_MODEL(pedModel);
		while (!STREAMING::HAS_MODEL_LOADED(pedModel)) WAIT(0);

		CreateDirectory("CarVarianceRec", NULL);
		for (int i = 6; i <= 12; i += 3) {
			std::string speedDirName = "CarVarianceRec/speed" + std::to_string(i);
			CreateDirectory(speedDirName.c_str(), NULL);

			for (int j = 1; j <= 10; j++) {
				std::ofstream record;
				record.open(speedDirName + "/range" + std::to_string(j));

				for (int k = 0; k < 50; k++) {
					GamePlay::clearArea();

					Vehicle v = VEHICLE::CREATE_VEHICLE(carModel, carStart.x, carStart.y, carStart.z, 219.39024353027344, 1, 1);
					VEHICLE::SET_VEHICLE_ON_GROUND_PROPERLY(v);
					Ped p = PED::CREATE_PED_INSIDE_VEHICLE(v, 4, pedModel, -1, 1, 1);
					WAIT(200);
					TaskSequence sequence = 0;
					AI::OPEN_SEQUENCE_TASK(&sequence);
					AI::TASK_VEHICLE_DRIVE_TO_COORD(0, v, carEnd.x, carEnd.y, carEnd.z, i, 0, carModel, 786475, j, 1.0);
					AI::CLOSE_SEQUENCE_TASK(sequence);
					AI::TASK_PERFORM_SEQUENCE(p, sequence);
					WAIT(200);

					while (true) {
						GamePlay::clearArea();

						if (ENTITY::GET_ENTITY_SPEED(v) == 0) {
							AI::CLEAR_SEQUENCE_TASK(&sequence);
							Vector3d coords = ENTITY::GET_ENTITY_COORDS(p, !ENTITY::IS_ENTITY_DEAD(p));
							record << k << "," << coords.x << "," << coords.y << "," << coords.z << std::endl;
							ENTITY::DELETE_ENTITY(&p);
							ENTITY::DELETE_ENTITY(&v);
							break;
						}
						WAIT(100);
					}
				}
				record.close();
			}
		}
	}


	void plotCarTrajectory() {

		Vector3d carStart(
			-1402.01, -366.20, 37.58);
		Vector3d carEnd(
			-1336.89, -376.23, 36.68
		);
		GamePlay::teleportPlayer(Vector3d(-1382.03, -390.19, 36.69), true);
		Ped player = PLAYER::PLAYER_PED_ID();
		ENTITY::SET_ENTITY_COLLISION(player, false, false);
		PED::SET_PED_GRAVITY(player, false);
		ENTITY::FREEZE_ENTITY_POSITION(player, true);
		ENTITY::SET_ENTITY_VISIBLE(player, false, 0);

		Camera customCam = CAM::CREATE_CAM("DEFAULT_SCRIPTED_CAMERA", true);
		CAM::SET_CINEMATIC_MODE_ACTIVE(false);
		CAM::SET_CAM_COORD(customCam, -1382.03, -390.19, 36.69 + 40.0);
		CAM::POINT_CAM_AT_COORD(customCam, -1382.03, -390.19, 36.69);
		CAM::SET_CAM_ACTIVE(customCam, true);
		CAM::RENDER_SCRIPT_CAMS(true, false, 3000, true, false);

		DWORD carModel = GAMEPLAY::GET_HASH_KEY("Adder");
		STREAMING::REQUEST_MODEL(carModel);
		while (!STREAMING::HAS_MODEL_LOADED(carModel)) WAIT(0);

		DWORD pedModel = GAMEPLAY::GET_HASH_KEY("a_m_m_business_01");
		STREAMING::REQUEST_MODEL(pedModel);
		while (!STREAMING::HAS_MODEL_LOADED(pedModel)) WAIT(0);

		std::vector<Color> TrajectCols = { DefaultColor::red, DefaultColor::green, DefaultColor::blue };

		std::vector<std::vector<Vector3d>> trajects;

		Menu hintLine("car still running", {}, {});

		const float boxSize = 1.0;
		while (trajects.size() < 3) {
			GamePlay::clearArea();

			Vehicle v = VEHICLE::CREATE_VEHICLE(carModel, carStart.x, carStart.y, carStart.z, 219.39024353027344, 1, 1);
			VEHICLE::SET_VEHICLE_ON_GROUND_PROPERLY(v);
			Ped p = PED::CREATE_PED_INSIDE_VEHICLE(v, 4, pedModel, -1, 1, 1);
			WAIT(200);
			TaskSequence sequence = 0;
			AI::OPEN_SEQUENCE_TASK(&sequence);
			AI::TASK_VEHICLE_DRIVE_TO_COORD(0, v, carEnd.x, carEnd.y, carEnd.z, 12.0, 0, carModel, 786475, 10.0, 1.0);
			AI::CLOSE_SEQUENCE_TASK(sequence);
			AI::TASK_PERFORM_SEQUENCE(p, sequence);
			WAIT(1000);

			trajects.push_back({});

			while (true) {
				GamePlay::clearArea();
				Vector3d coords = ENTITY::GET_ENTITY_COORDS(p, !ENTITY::IS_ENTITY_DEAD(p));
				trajects.back().push_back(coords);

				if (AI::GET_SEQUENCE_PROGRESS(p) == -1) {
					AI::CLEAR_SEQUENCE_TASK(&sequence);
					ENTITY::DELETE_ENTITY(&p);
					ENTITY::DELETE_ENTITY(&v);
					break;
				}

				DWORD maxTickCount = GetTickCount() + 500;
				do {
					hintLine.drawVertical(0);
					for (int i = 0; i < trajects.size(); i++) {
						for (auto c : trajects[i]) {
							//drawMarker(MarkerTypeThickChevronUp, c, 3.0, TrajectCols[i]);
							GRAPHICS::DRAW_BOX(c.x, c.y, c.z, c.x + boxSize, c.y + boxSize, c.z + boxSize, TrajectCols[i].red, TrajectCols[i].green, TrajectCols[i].blue, TrajectCols[i].alpha);
						}
					}
					WAIT(50);
				} while (GetTickCount() < maxTickCount);
			}
			while (true) {
				GamePlay::clearArea();
				for (int i = 0; i < trajects.size(); i++) {
					for (auto c : trajects[i]) {
						//drawMarker(MarkerTypeThickChevronUp, c, 3.0, TrajectCols[i]);
						GRAPHICS::DRAW_BOX(c.x, c.y, c.z, c.x + boxSize, c.y + boxSize, c.z + boxSize, TrajectCols[i].red, TrajectCols[i].green, TrajectCols[i].blue, TrajectCols[i].alpha);
					}
				}
				if (IsKeyJustUp(VK_NUMPAD5)) {
					break;
				}
				else if (IsKeyJustUp(VK_NUMPAD0)) {
					trajects.pop_back();
					break;
				}
				WAIT(0);
			}
		}
		GamePlay::setPlayerDisappear(false);
	}

	void spawn() {
		LPCSTR modelName = "Adder";
		DWORD model = GAMEPLAY::GET_HASH_KEY((char *)modelName);
		if (STREAMING::IS_MODEL_IN_CDIMAGE(model) && STREAMING::IS_MODEL_A_VEHICLE(model))
		{
			STREAMING::REQUEST_MODEL(model);
			while (!STREAMING::HAS_MODEL_LOADED(model)) WAIT(0);
			Vector3 coords = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(PLAYER::PLAYER_PED_ID(), 0.0, 5.0, 0.0);
			Vehicle veh = VEHICLE::CREATE_VEHICLE(model, coords.x, coords.y, coords.z, 0.0, 1, 1);
		}
	}

	void carMarker() {
		Vector3d carStart(-1397.03, -372.517, 36.483);
		float startHeading(220.096);
		Vector3d carMid(-1383.56, -388.297, 36.2723);
		float midHeading(244.246);
		Vector3d carEnd(-1353.58, -383.6, 36.3197);
		float endHeading(297.594);
		Vector3d carStop(-1323.49, -370.212, 36.2419);
		float stopHeading(288.161);

		Camera customCam = CAM::CREATE_CAM("DEFAULT_SCRIPTED_CAMERA", true);
		CAM::SET_CINEMATIC_MODE_ACTIVE(false);
		CAM::SET_CAM_COORD(customCam, -1382.03, -390.19, 36.69 + 40.0);
		CAM::SET_CAM_ROT(customCam, 270.0, 360.0 - startHeading, 0.0, 2);

		CAM::SET_CAM_ACTIVE(customCam, true);
		CAM::RENDER_SCRIPT_CAMS(true, false, 3000, true, false);

		DWORD carModel = GAMEPLAY::GET_HASH_KEY("Adder");
		STREAMING::REQUEST_MODEL(carModel);
		while (!STREAMING::HAS_MODEL_LOADED(carModel)) WAIT(0);

		DWORD pedModel = GAMEPLAY::GET_HASH_KEY("a_m_m_business_01");
		STREAMING::REQUEST_MODEL(pedModel);
		while (!STREAMING::HAS_MODEL_LOADED(pedModel)) WAIT(0);

		Vehicle v = VEHICLE::CREATE_VEHICLE(carModel, carStart.x, carStart.y, carStart.z, startHeading, 1, 1);
		VEHICLE::SET_VEHICLE_ON_GROUND_PROPERLY(v);
		Ped p = PED::CREATE_PED_INSIDE_VEHICLE(v, 4, pedModel, -1, 1, 1);


		bool startMark = false;
		bool midMark = false;
		bool endMark = false;
		bool stopMark = false;
		float boxSize = 2.0;

		while (true) {
			GamePlay::clearArea();
			if (IsKeyJustUp(VK_NUMPAD1)) {
				ENTITY::SET_ENTITY_COORDS(v, carStart.x, carStart.y, carStart.z, 1, 0, 0, 1);
				ENTITY::SET_ENTITY_HEADING(v, startHeading);
			}
			if (IsKeyJustUp(VK_NUMPAD2)) {
				ENTITY::SET_ENTITY_COORDS(v, carMid.x, carMid.y, carMid.z, 1, 0, 0, 1);
				ENTITY::SET_ENTITY_HEADING(v, midHeading);
			}
			if (IsKeyJustUp(VK_NUMPAD3)) {
				ENTITY::SET_ENTITY_COORDS(v, carEnd.x, carEnd.y, carEnd.z, 1, 0, 0, 1);
				ENTITY::SET_ENTITY_HEADING(v, endHeading);
			}

			if (IsKeyJustUp(VK_NUMPAD7)) {
				ENTITY::SET_ENTITY_COORDS(v, carStop.x, carStop.y, carStop.z, 1, 0, 0, 1);
				ENTITY::SET_ENTITY_HEADING(v, endHeading);
			}

			if (IsKeyJustUp(VK_NUMPAD4)) {
				startMark = !startMark;
			}

			if (IsKeyJustUp(VK_NUMPAD5)) {
				midMark = !midMark;
			}

			if (IsKeyJustUp(VK_NUMPAD6)) {
				endMark = !endMark;
			}

			if (IsKeyJustUp(VK_NUMPAD0)) {
				break;
			}


			if (startMark) {
				GRAPHICS::DRAW_BOX(carStart.x, carStart.y, carStart.z, carStart.x + boxSize, carStart.y + boxSize, carStart.z + boxSize, 255, 0, 0, 255);
			}

			if (midMark) {
				GRAPHICS::DRAW_BOX(carMid.x, carMid.y, carMid.z, carMid.x + boxSize, carMid.y + boxSize, carMid.z + boxSize, 255, 0, 0, 255);
			}

			if (endMark) {
				GRAPHICS::DRAW_BOX(carEnd.x, carEnd.y, carEnd.z, carEnd.x + boxSize, carEnd.y + boxSize, carEnd.z + boxSize, 255, 0, 0, 255);
			}

			if (stopMark) {
				GRAPHICS::DRAW_BOX(carStop.x, carStop.y, carStop.z, carStop.x + boxSize, carStop.y + boxSize, carStop.z + boxSize, 255, 0, 0, 255);
			}
			WAIT(0);
		}

	}

	void toggleCamera(bool& value) {
		static Camera customCam;
		value = !value;
		if (value) {
			Vector3d coords = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), !ENTITY::IS_ENTITY_DEAD(PLAYER::PLAYER_PED_ID()));
			customCam = CAM::CREATE_CAM("DEFAULT_SCRIPTED_CAMERA", true);
			CAM::SET_CINEMATIC_MODE_ACTIVE(false);
			CAM::SET_CAM_COORD(customCam, coords.x, coords.y, coords.z + 30.0);
			CAM::POINT_CAM_AT_COORD(customCam, coords.x, coords.y, coords.z);

			CAM::SET_CAM_ACTIVE(customCam, true);
			CAM::RENDER_SCRIPT_CAMS(true, false, 3000, true, false);
		}
		else {
			CAM::SET_CAM_ACTIVE(customCam, false);
			CAM::RENDER_SCRIPT_CAMS(false, false, 3000, true, false);
			CAM::DESTROY_ALL_CAMS(true);
		}
	}

	void loadReplay(std::string filename, std::unordered_map<int, std::unordered_map <int, std::pair<Vector3d, float>>>& coordsMap) {
		std::ifstream replay(filename);
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

			coordsMap[id][timestamp] = std::make_pair(coords3D, heading);
		}

		replay.close();

	}

	void drawFullTrajectReplay() {
		std::string replayFile = "2ped2car/record0";
		float boxSize = 0.5;
		std::unordered_map<int, std::unordered_map <int, std::pair<Vector3d, float>>> replayCoords;
		loadReplay(replayFile, replayCoords);
		Color colors[] = { DefaultColor::blue, DefaultColor::green, DefaultColor::red, DefaultColor::yellow };

		while (true) {
			GamePlay::clearArea();
			int counter = 0;
			bool withLine = true;
			for (auto& traject : replayCoords) {
				int now = 0;
				Vector3d prev;
				while (!traject.second.count(now)) {
					now++;
				}
				while (traject.second.count(now)) {
					Vector3d coords = traject.second[now].first;
					GRAPHICS::DRAW_BOX(coords.x, coords.y, coords.z, coords.x + 0.5, coords.y + 0.5, coords.z + 0.5, colors[counter].red, colors[counter].green, colors[counter].blue, 255);
					if (prev.x != 0 && withLine) {
						GRAPHICS::DRAW_LINE(coords.x, coords.y, coords.z, prev.x, prev.y, prev.z, colors[counter].red, colors[counter].green, colors[counter].blue, 255);
					}
					prev = coords;
					now++;
				}
				counter++;
			}

			if (IsKeyJustUp(VK_NUMPAD5)) {
				withLine = !withLine;
			}

			if (IsKeyJustUp(VK_NUMPAD0)) {
				break;
			}

			WAIT(0);
		}
	}

	void startSimulation(SimulationData& data) {

		if (!data.vehicle.goal.empty()) {

			Vector3d startCoords = (data.vehicle.start[0] + data.vehicle.start[1] + data.vehicle.start[2] + data.vehicle.start[3]) / 4;
			Vector3d goalCoords = (data.vehicle.goal[0] + data.vehicle.goal[1] + data.vehicle.goal[2] + data.vehicle.goal[3]) / 4;

			VehicleWtihMission& v = GameResources::spawnVehicleAtCoords(data.vehicle.model.c_str(), startCoords, data.vehicle.heading);

			GameResources::spawnDriver(v, "");

			GameResources::createVehTask(v, [&]() {
				AI::TASK_VEHICLE_DRIVE_TO_COORD(0, v.veh, goalCoords.x, goalCoords.y, goalCoords.z, data.vehicle.speed, 0, GAMEPLAY::GET_HASH_KEY((char *)(data.vehicle.model.c_str())), 786475, 2.0, 1.0);
			});
		}

		if (!data.ped.goal.empty()) {

			Vector3d startCoords = (data.ped.start[0] + data.ped.start[1] + data.ped.start[2] + data.ped.start[3]) / 4;
			Vector3d goalCoords = (data.ped.goal[0] + data.ped.goal[1] + data.ped.goal[2] + data.ped.goal[3]) / 4;

			PedWithMission& p = GameResources::spawnPedAtCoords("", startCoords);

			float speed = data.ped.running ? 2.0 : 1.0;

			if (!data.ped.wandering) {
				GameResources::createPedTask(p, [&]() {
					AI::TASK_GO_STRAIGHT_TO_COORD(0, goalCoords.x, goalCoords.y, goalCoords.z, speed, 200000, 0.0, 0.2);
				});
			}
			else {
				Vector3d centerCoords = (startCoords + goalCoords) / 2;
				float walkRadius = GAMEPLAY::GET_DISTANCE_BETWEEN_COORDS(goalCoords.x, goalCoords.y, goalCoords.z, startCoords.x, startCoords.y, startCoords.z, true);
				GameResources::createPedTask(p, [&]() {
					AI::TASK_WANDER_IN_AREA(0, centerCoords.x, centerCoords.y, centerCoords.z, walkRadius, 0.0, 0.0);
				});
			}
		}

		WAIT(0);

		Camera cam;
		if (data.camera.enabled) {
			GamePlay::setPlayerDisappear(true);
			cam = GamePlay::activateCamera(data.camera.params);
		}

		while (true) {
			GamePlay::clearArea();
			if (!GameResources::createdVehicles.empty()) {
				if (AI::GET_SEQUENCE_PROGRESS(GameResources::createdVehicles[0].driver) == -1) {
					outputDebugMessage("Driver " + std::to_string(GameResources::createdVehicles[0].driver) + " has completed task.");
					GameResources::deleteVeh(0);
				}
			}
			 
			if (!GameResources::createdPeds.empty()) {
				if (AI::GET_SEQUENCE_PROGRESS(GameResources::createdPeds[0].ped) == -1) {
					outputDebugMessage(std::to_string(GameResources::createdPeds[0].ped) + " has completed task.");
					GameResources::deletePed(0);
				}
			}

			if (GameResources::createdPeds.size() == 0 && GameResources::createdVehicles.size() == 0) {
				break;
			}
			WAIT(0);
		}
		outputDebugMessage("Before Exiting.");
		if (data.camera.enabled) {
			GamePlay::setPlayerDisappear(false);
			GamePlay::destroyCamera(cam);
		}

		outputDebugMessage("Exiting.");

	}

	void startEndDemo(SimulationData& data) {
		Menu vehicleList("Select A Vehicle Setting",
			{});


		auto setVehicle = [&](int i) {
			Menu vehicleSettings("Vehicle 0", {});

			auto setVehicleStart = [&]() {
				data.vehicle.start = Sampling::samplePoints(4, "Please mark the start area of the vehicle.", DefaultColor::green);
				if (!data.vehicle.start.empty()) {
					vehicleSettings.items[0].description = "(" + data.vehicle.start[0].to_string() + "), ...";
				}
			};

			auto setVehicleGoal = [&]() {
				std::vector<Marker> referencePoints;
				for (auto& p : data.vehicle.start) {
					referencePoints.push_back({ p, DefaultColor::green });
				}
				data.vehicle.goal = Sampling::samplePoints(4, "Please mark the goal area of the vehicle.", DefaultColor::blue, referencePoints);
				if (!data.vehicle.goal.empty()) {
					vehicleSettings.items[1].description = "(" + data.vehicle.goal[0].to_string() + "), ...";
				}
			};

			auto setVehicleModel = [&]() {
				std::vector<std::string> modelList = {
					"oracle",
					"cheetah",
					"bus",
					"speedo"
				};

				Vector3d coords = (data.vehicle.start[0] + data.vehicle.start[1] + data.vehicle.start[2] + data.vehicle.start[3]) / 4;
				coords.z += 2.0;
				float rotation = 0.0;
				Menu modelMenu("Pick a Model", {});
				modelMenu.singleUse = true;
				for (int i = 0; i < 4; i++) {
					modelMenu.addMenuItem({ modelList[i], std::bind([&](int index) {
						data.vehicle.model = modelList[index];
						data.vehicle.heading = rotation;
						vehicleSettings.items[2].description = modelList[index];
					}, i)});
				}

				Camera customCam = CAM::CREATE_CAM("DEFAULT_SCRIPTED_CAMERA", true);
				CAM::SET_CINEMATIC_MODE_ACTIVE(false);
				CAM::SET_CAM_COORD(customCam, coords.x + 3.0, coords.y + 3.0, coords.z + 3.0);
				CAM::POINT_CAM_AT_COORD(customCam, coords.x, coords.y, coords.z);

				CAM::SET_CAM_ACTIVE(customCam, true);
				CAM::RENDER_SCRIPT_CAMS(true, false, 3000, true, false);

				GamePlay::setPlayerDisappear(true);

				VehicleWtihMission v;
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
					}
					else {
						ENTITY::SET_ENTITY_HEADING(v.veh, rotation);
					}
					if (!modelMenu.onTick()) {
						GameResources::deleteAllCreated();
						break;
					}
				}

				CAM::SET_CAM_ACTIVE(customCam, false);
				CAM::RENDER_SCRIPT_CAMS(false, false, 3000, true, false);
				CAM::DESTROY_ALL_CAMS(true);
				GamePlay::setPlayerDisappear(false);

			};

			//auto setVehicleHeading = [&]() {};

			auto setVechicleSpeed = [&]() {
				std::string speed = GamePlay::getTextInput();
				if (speed != "") {
					data.vehicle.speed = std::stof(speed);
					vehicleSettings.items[3].description = speed;
				}
			};

			vehicleSettings.addMenuItem({ "Start Area", setVehicleStart, data.vehicle.start.empty() ? "" : "(" + data.vehicle.start[0].to_string() + "), ..." });
			vehicleSettings.addMenuItem({ "Goal Area", setVehicleGoal, data.vehicle.goal.empty() ? "" : "(" + data.vehicle.goal[0].to_string() + "), ..." });
			vehicleSettings.addMenuItem({ "Vehicle Model", setVehicleModel, data.vehicle.model });
			//vehicleSettings.addMenuItem({ "Vehicle Heading", setVehicleHeading });
			vehicleSettings.addMenuItem({ "Vehicle Speed", setVechicleSpeed, data.vehicle.speed ? std::to_string(data.vehicle.speed) : "Enter the speed of the vehicle in m/s."});
			vehicleSettings.addMenuItem({ "Delete Vehicle", setVechicleSpeed });

			vehicleSettings.processMenu();

			return true;
		};

		auto createNewVehicle = [&]() {

			if (setVehicle(0)) {
				vehicleList.addMenuItem({ "Vehicle 0", std::bind(setVehicle, 0) });
			}
		};

		vehicleList.addMenuItem({ "Create New Vehicle Setting", createNewVehicle });

		//data.vehicle.goal = {};
		//data.vehicle.heading = 0.0;
		//data.vehicle.model = "";
		//data.vehicle.start = {};
		//data.vehicle.speed = 0.0;

		vehicleList.processMenu();
	}


	void pedStartEndDemo(SimulationData& data) {
		Menu pedList("Select A Pedestrian Setting",
			{});


		auto setPed = [&](int i) {
			Menu pedSettings("Ped 0", {});

			auto setPedStart = [&]() {
				data.ped.start = Sampling::samplePoints(4, "Please mark the start area of the ped.", DefaultColor::green);
				if (!data.ped.start.empty()) {
					pedSettings.items[0].description = "(" + data.ped.start[0].to_string() + "), ...";
				}
			};

			auto setPedGoal = [&]() {
				std::vector<Marker> referencePoints;
				for (auto& p : data.ped.start) {
					referencePoints.push_back({ p, DefaultColor::green });
				}
				data.ped.goal = Sampling::samplePoints(4, "Please mark the goal area of the ped.", DefaultColor::blue, referencePoints);
				if (!data.ped.goal.empty()) {
					pedSettings.items[1].description = "(" + data.ped.goal[0].to_string() + "), ...";
				}
			};

			//auto setVehicleHeading = [&]() {};

			auto setPedSpeed = [&]() {
				data.ped.running = !data.ped.running;
			};

			auto setPedWandering = [&]() {
				data.ped.wandering = !data.ped.wandering;
			};

			pedSettings.addMenuItem({ "Start Area", setPedStart, data.ped.start.empty() ? "" : "(" + data.ped.start[0].to_string() + "), ..." });
			pedSettings.addMenuItem({ "Goal Area", setPedGoal, data.ped.goal.empty() ? "" : "(" + data.ped.goal[0].to_string() + "), ..." });
			pedSettings.addMenuItem({ "Ped Running", setPedSpeed, "", &data.ped.running });
			pedSettings.addMenuItem({ "Ped Wandering", setPedWandering, "", &data.ped.wandering });

			pedSettings.processMenu();

			return true;
		};

		auto createNewPed = [&]() {

			if (setPed(0)) {
				pedList.addMenuItem({ "Ped 0", std::bind(setPed, 0) });
			}
		};

		pedList.addMenuItem({ "Create New Ped Setting", createNewPed });

		//data.ped.goal = {};
		//data.ped.start = {};
		//data.ped.running = false;

		pedList.processMenu();
	}

	void setCameraDemo(SimulationData& data) {
		Menu cameraMenu("Camera Settings",
			{});

		auto adjustCamera = [&]() {
			std::vector<Marker> referencePoints;
			for (auto& p : data.vehicle.start) {
				referencePoints.push_back({ p, DefaultColor::green });
			}
			for (auto& p : data.vehicle.goal) {
				referencePoints.push_back({ p, DefaultColor::green });
			}
			for (auto& p : data.ped.start) {
				referencePoints.push_back({ p, DefaultColor::blue });
			}
			for (auto& p : data.ped.goal) {
				referencePoints.push_back({ p, DefaultColor::blue });
			}
			data.camera.params = Sampling::sampleCameraParams("Please adjust camera.", referencePoints);
		};

		auto setCameraEnable = [&]() {
			data.camera.enabled = !data.camera.enabled;
		};


		cameraMenu.addMenuItem({ "Adjust Camera", adjustCamera });
		cameraMenu.addMenuItem({ "Enable Camera", setCameraEnable, "", &data.camera.enabled});

		cameraMenu.processMenu();
	}

}

