#include "gameResources.h"

namespace GameResources {

	std::vector<PedWithMission> createdPeds;
	std::vector<VehicleWithMission> createdVehicles;

	VehicleWithMission& spawnVehicleAtCoords(LPCSTR modelName, Vector3d coords, float heading) {
		DWORD model;
		if (modelName == "") {
			modelName = "adder";
		}

		model = GAMEPLAY::GET_HASH_KEY((char *)modelName);
		STREAMING::REQUEST_MODEL(model);
		while (!STREAMING::HAS_MODEL_LOADED(model)) WAIT(0);

		VehicleWithMission v;
		v.veh = VEHICLE::CREATE_VEHICLE(model, coords.x, coords.y, coords.z, heading, 1, 1);
		VEHICLE::SET_VEHICLE_ON_GROUND_PROPERLY(v.veh);
		createdVehicles.push_back(v);
		return createdVehicles.back();

	}

	PedWithMission& spawnPedAtCoords(LPCSTR modelName, Vector3d coords, float heading) {
		DWORD model;
		PedWithMission p;
		if (modelName[0] != '\0') {
			model = GAMEPLAY::GET_HASH_KEY((char *)modelName);
			STREAMING::REQUEST_MODEL(model);
			while (!STREAMING::HAS_MODEL_LOADED(model)) WAIT(0);
			p.ped = PED::CREATE_PED(PED_TYPE_MISSION, model, coords.x, coords.y, coords.z, heading, FALSE, FALSE);
		}
		else {
			p.ped = PED::CREATE_RANDOM_PED(coords.x, coords.y, coords.z);
		}
		PED::SET_PED_CAN_EVASIVE_DIVE(p.ped, false);
		createdPeds.push_back(p);

		return createdPeds.back();
	}

	VehicleWithMission& spawnDriver(VehicleWithMission& vehicle, LPCSTR modelName) {

		DWORD model;

		if (modelName[0] != '\0') {
			model = GAMEPLAY::GET_HASH_KEY((char *)modelName);
			STREAMING::REQUEST_MODEL(model);
			while (!STREAMING::HAS_MODEL_LOADED(model)) WAIT(0);
			vehicle.driver = PED::CREATE_PED_INSIDE_VEHICLE(vehicle.veh, 4, model, -1, 1, 1);
		}
		else {
			vehicle.driver = PED::CREATE_RANDOM_PED_AS_DRIVER(vehicle.veh, true);
		}
		PED::SET_PED_CAN_EVASIVE_DIVE(vehicle.driver, false);

		return vehicle;
	}

	PedWithMission& createPedTask(PedWithMission& p, std::function<void()> actionItem) {
		AI::OPEN_SEQUENCE_TASK(&p.task);
		actionItem();
		AI::CLOSE_SEQUENCE_TASK(p.task);
		AI::TASK_PERFORM_SEQUENCE(p.ped, p.task);

		return p;
	}

	VehicleWithMission& createVehTask(VehicleWithMission& v, std::function<void()> actionItem) {
		AI::OPEN_SEQUENCE_TASK(&v.task);
		actionItem();
		AI::CLOSE_SEQUENCE_TASK(v.task);
		AI::TASK_PERFORM_SEQUENCE(v.driver, v.task);
		return v;
	}

	void deleteVeh(int i) {
		if (createdVehicles[i].task != 0) AI::CLEAR_SEQUENCE_TASK(&createdVehicles[i].task);
		if (createdVehicles[i].driver != 0) ENTITY::DELETE_ENTITY(&createdVehicles[i].driver);
		if (createdVehicles[i].veh != 0) ENTITY::DELETE_ENTITY(&createdVehicles[i].veh);

		createdVehicles.erase(createdVehicles.begin() + i);
	}

	void deletePed(int i) {

		if (createdPeds[i].task != 0) AI::CLEAR_SEQUENCE_TASK(&createdPeds[i].task);
		if (createdPeds[i].ped != 0) ENTITY::DELETE_ENTITY(&createdPeds[i].ped);

		createdPeds.erase(createdPeds.begin() + i);

	}

	void deleteAllCreated() {
		while (!createdVehicles.empty()) {
			deleteVeh(0);
		}

		while (!createdPeds.empty()) {
			deletePed(0);
		}

	}

}