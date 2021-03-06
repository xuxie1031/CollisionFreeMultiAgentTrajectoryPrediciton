#include "gamePlay.h"

namespace GamePlay {
	Vector3d rotToUnitVec(Vector3d rot) {
		float alpha = rot.z * 0.0174532924f;
		float beta = rot.x * 0.0174532924f;
		Vector3d result;
		result.z = sin(beta);
		result.y = cos(beta) * cos(alpha);
		result.x = cos(beta) * sin(alpha) * -1;
		return result;
	}

	void rayTestEntity(Cam cam, BOOL* hit, Vector3d* entCoords, Entity* hitEnt) {
		const float RAY_LENGTH = 200.0;
		Vector3d rot = CAM::GET_CAM_ROT(cam, 2);
		Vector3d rotVec = rotToUnitVec(rot);
		Vector3d pos = CAM::GET_CAM_COORD(cam);
		Vector3d rayEnd = pos + rotVec * RAY_LENGTH;
		Player p = PLAYER::PLAYER_PED_ID();

		int ray = WORLDPROBE::_CAST_RAY_POINT_TO_POINT(pos.x, pos.y, pos.z, rayEnd.x, rayEnd.y, rayEnd.z, IntersectPeds | IntersectPedsSimpleCollision | IntersectVehicles, p, 7);

		BOOL _hit;
		Vector3 _entCoords;
		Vector3 _surfaceNormal;
		Entity _hitEnt;

		WORLDPROBE::_GET_RAYCAST_RESULT(ray, &_hit, &_entCoords, &_surfaceNormal, &_hitEnt);

		if (_hit) {
			_entCoords = ENTITY::GET_ENTITY_COORDS(_hitEnt, !ENTITY::IS_ENTITY_DEAD(_hitEnt));
		}
		if (hit) {
			*hit = _hit;
		}
		if (entCoords) {
			*entCoords = _entCoords;
		}
		if (hitEnt) {
			*hitEnt = _hitEnt;
		}

	}

	void rayTestEveryThing(Cam cam, BOOL* hit, Vector3d* endCoords, Entity* hitEnt) {
		const float RAY_LENGTH = 100.0;
		Vector3d rot = CAM::GET_CAM_ROT(cam, 2);
		Vector3d rotVec = rotToUnitVec(rot);
		Vector3d pos = CAM::GET_CAM_COORD(cam);
		Vector3d rayEnd = pos + rotVec * RAY_LENGTH;
		Player p = PLAYER::PLAYER_PED_ID();

		int ray = WORLDPROBE::_CAST_RAY_POINT_TO_POINT(pos.x, pos.y, pos.z, rayEnd.x, rayEnd.y, rayEnd.z, IntersectEverything, p, 7);

		BOOL _hit;
		Vector3 _endCoords;
		Vector3 _surfaceNormal;
		Entity _hitEnt;

		WORLDPROBE::_GET_RAYCAST_RESULT(ray, &_hit, &_endCoords, &_surfaceNormal, &_hitEnt);

		if (!_hit) {
			_endCoords = pos + rotVec * RAY_LENGTH;
		}
		if (hit) {
			*hit = _hit;
		}
		if (endCoords) {
			*endCoords = _endCoords;
		}
		if (hitEnt) {
			*hitEnt = _hitEnt;
		}
	}


	void teleportPlayer(Vector3d coords, bool withZOffset) {
		Ped p = PLAYER::PLAYER_PED_ID();
		ENTITY::SET_ENTITY_COORDS(p, coords.x, coords.y, coords.z + withZOffset * 5.0, 1, 0, 0, 1);
	}

	void clearArea() {
		GAMEPLAY::CLEAR_AREA_OF_PEDS(0, 0, 0, 10000, 1);
		GAMEPLAY::CLEAR_AREA_OF_VEHICLES(0, 0, 0, 10000, false, false, false, false, false);
	}

	void setPlayerDisappear(bool toggle) {
		Ped player = PLAYER::PLAYER_PED_ID();
		Vector3d playerCoords = ENTITY::GET_ENTITY_COORDS(player, !ENTITY::IS_ENTITY_DEAD(player));
		float groundHeight;
		GAMEPLAY::GET_GROUND_Z_FOR_3D_COORD(playerCoords.x, playerCoords.y, playerCoords.z, &groundHeight, 0);
		if (toggle) {
			teleportPlayer(playerCoords, true);
			ENTITY::FREEZE_ENTITY_POSITION(player, true);
			ENTITY::SET_ENTITY_COLLISION(player, false, false);
			ENTITY::SET_ENTITY_VISIBLE(player, false, 0);
			PED::SET_PED_CONFIG_FLAG(player, PED_FLAG_FREEZE, true);
			PLAYER::SET_POLICE_IGNORE_PLAYER(player, true);
		}
		else {
			teleportPlayer(Vector3d(playerCoords.x, playerCoords.y, groundHeight));
			ENTITY::SET_ENTITY_COLLISION(player, 1, 1);
			ENTITY::FREEZE_ENTITY_POSITION(player, false);
			ENTITY::SET_ENTITY_VISIBLE(player, true, true);
			PED::SET_PED_CONFIG_FLAG(player, PED_FLAG_FREEZE, false);
			PLAYER::SET_POLICE_IGNORE_PLAYER(player, false);
		}
	}

	float getModelLength(LPCSTR modelName) {
		Vector3d min, max;
		Hash model = GAMEPLAY::GET_HASH_KEY((char *)modelName);
		STREAMING::REQUEST_MODEL(model);
		while (!STREAMING::HAS_MODEL_LOADED(model)) WAIT(0);
		GAMEPLAY::GET_MODEL_DIMENSIONS(model, &min, &max);
		return max.y - min.y;
	}

	std::string getTextInput() {
		std::string res;
		// Invoke keyboard
		GAMEPLAY::DISPLAY_ONSCREEN_KEYBOARD(true, "", "", "", "", "", "", 140);
		// Wait for the user to edit
		while (GAMEPLAY::UPDATE_ONSCREEN_KEYBOARD() == 0) WAIT(0);
		// Make sure they didn't exit without confirming their change
		if (!GAMEPLAY::GET_ONSCREEN_KEYBOARD_RESULT()) {
			return res;
		}
		else {
			return GAMEPLAY::GET_ONSCREEN_KEYBOARD_RESULT();
		}
	}

	void getEntityMotion(Entity e, Vector3d* coords3D, Vector2* coords2D, float* heading, float* speed) {

		Vector3d coords = ENTITY::GET_ENTITY_COORDS(e, TRUE);

		if (coords3D != NULL) {
			*coords3D = coords;
		}

		if (coords2D != NULL) {
			GRAPHICS::_WORLD3D_TO_SCREEN2D(coords.x, coords.y, coords.z, &(coords2D->x), &(coords2D->y));
			int screen_w, screen_h;
			GRAPHICS::GET_SCREEN_RESOLUTION(&screen_w, &screen_h);
			coords2D->x *= screen_w;
			coords2D->y *= screen_h;
		}

		if (heading != NULL) {
			*heading = ENTITY::GET_ENTITY_HEADING(e);
		}

		if (speed != NULL) {
			*speed = ENTITY::GET_ENTITY_SPEED(e);
		}
	}

	Camera activateCamera(CameraParams params) {
		// setup camera
		Camera cam = CAM::CREATE_CAM("DEFAULT_SCRIPTED_CAMERA", true);
		CAM::SET_CINEMATIC_MODE_ACTIVE(false);


		if (params.fov > 0) {
			CAM::SET_CAM_FOV(cam, params.fov);
			CAM::SET_CAM_COORD(cam, params.position.x, params.position.y, params.position.z);
			CAM::SET_CAM_ROT(cam, params.rotation.x, params.rotation.y, params.rotation.z, 2);
		}
		else {
			CAM::SET_CAM_FOV(cam, CAM::GET_GAMEPLAY_CAM_FOV());
			Vector3d coord = CAM::GET_GAMEPLAY_CAM_COORD();
			Vector3d rot = CAM::GET_GAMEPLAY_CAM_ROT(2);
			CAM::SET_CAM_COORD(cam, coord.x, coord.y, coord.z);
			CAM::SET_CAM_ROT(cam, rot.x, rot.y, rot.z, 2);
		}
		CAM::SET_CAM_ACTIVE(cam, true);
		CAM::RENDER_SCRIPT_CAMS(true, false, 3000, true, false);

		return cam;
	}

	void destroyCamera(Camera cam) {
		CAM::SET_CAM_ACTIVE(cam, false);
		CAM::RENDER_SCRIPT_CAMS(false, false, 3000, true, false);
		CAM::DESTROY_CAM(cam, true);
	}
}