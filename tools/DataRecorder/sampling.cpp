#include "sampling.h"

namespace Sampling {
	const float HEIGHT_LIMIT = 2000.0;
	const float ROT_X_ANGLE_LIMIT = 88.0;
	const float GROUND_OFFSET = 1.0;
	const DWORD HELP_TEXT_TIMEOUT = 3000;
	const int MAX_MARKER_NUM = 64;

	Camera initialize() {
		Vector3d position = CAM::GET_GAMEPLAY_CAM_COORD();
		Vector3d rotation = CAM::GET_GAMEPLAY_CAM_ROT(2);
		float fov = CAM::GET_GAMEPLAY_CAM_FOV();
		position.z += 5.0;

		GamePlay::setPlayerDisappear(true);

		Camera cam = GamePlay::activateCamera({position, rotation, fov });
		return cam;
	}

	void finalize(Camera cam) {
		GamePlay::setPlayerDisappear(false);
		GamePlay::destroyCamera(cam);
	}

	void changeControlThisFrame() {
		UI::HIDE_HUD_AND_RADAR_THIS_FRAME();
		CONTROLS::DISABLE_ALL_CONTROL_ACTIONS(0);
		CONTROLS::ENABLE_CONTROL_ACTION(0, ControlLookLeftRight, 1);
		CONTROLS::ENABLE_CONTROL_ACTION(0, ControlLookUpDown, 1);
		CONTROLS::ENABLE_CONTROL_ACTION(0, ControlFrontendPauseAlternate, 1);
	}

	void moveCameraOnKeyboardInput(Camera cam, bool useLeftRight) {
		float moveUpDown = CONTROLS::GET_DISABLED_CONTROL_NORMAL(0, ControlMoveUpDown);
		float moveLeftRight = CONTROLS::GET_DISABLED_CONTROL_NORMAL(0, ControlMoveLeftRight);

		Vector3d pos = CAM::GET_CAM_COORD(cam);
		Vector3d rot = CAM::GET_CAM_ROT(cam, 2);
		Vector3d rotVec = GamePlay::rotToUnitVec(rot);
		pos = pos - rotVec * moveUpDown;

		if (useLeftRight) {
			Vector3d rotPerp = rot;
			rotPerp.z += 90;
			Vector3d rotPerpVec = GamePlay::rotToUnitVec(rotPerp);
			pos = pos - rotPerpVec * moveLeftRight;
		}

		if (pos.z > HEIGHT_LIMIT) {
			pos.z = HEIGHT_LIMIT;
		}

		CAM::SET_CAM_COORD(cam, pos.x, pos.y, pos.z);
	}

	void rotateCameraOnMouseMove(Camera cam) {

		Vector3d rot = CAM::GET_CAM_ROT(cam, 2);

		float mouseX = CONTROLS::GET_CONTROL_NORMAL(0, ControlLookLeftRight);
		float mouseY = CONTROLS::GET_CONTROL_NORMAL(0, ControlLookUpDown);

		mouseX *= -15;
		mouseY *= -15;
		rot.x += mouseY;
		rot.z += mouseX;

		if (rot.x > ROT_X_ANGLE_LIMIT) {
			rot.x = ROT_X_ANGLE_LIMIT;
		}
		if (rot.x < -ROT_X_ANGLE_LIMIT) {
			rot.x = -ROT_X_ANGLE_LIMIT;
		}
		CAM::SET_CAM_ROT(cam, rot.x, rot.y, rot.z, 2);
	}

	void drawCrossFromTexture(Color color) {
		static const Texture cross = createTexture("DataRecorder/cross.png");
		static float aspectRatio = GRAPHICS::_GET_SCREEN_ASPECT_RATIO(false);
		int screen_w, screen_h;
		GRAPHICS::GET_SCREEN_RESOLUTION(&screen_w, &screen_h);
		drawTexture(cross, 0, 0, 100, 15.0f / screen_w, 15.0f / screen_h / aspectRatio, 0.5, 0.5, 0.5, 0.5, 0.0, aspectRatio, (float)color.red/255, (float)color.green / 255, (float)color.blue / 255, (float)color.alpha / 255);
	}

	void pullPlayerCloseIfTooFar(Camera cam) {
		Player p = PLAYER::PLAYER_PED_ID();
		Vector3d playerCoords = ENTITY::GET_ENTITY_COORDS(p, !ENTITY::IS_ENTITY_DEAD(p));
		Vector3d cameraCoords = CAM::GET_CAM_COORD(cam);

		if (GAMEPLAY::GET_DISTANCE_BETWEEN_COORDS(playerCoords.x, playerCoords.y, playerCoords.z, cameraCoords.x, cameraCoords.y, cameraCoords.z,false) > 100) {
			GamePlay::teleportPlayer(cameraCoords);
		}
	}

	float sampleRotation(std::string helpText, Entity referenceEntity) {

		float referenceRot = ENTITY::GET_ENTITY_HEADING(referenceEntity);

		Camera editingCam = initialize();

		std::string tempHelp;
		DWORD tempHelpTimeout = 0;

		InstructionButtons::getInstance()->loadButtonList({
			{ControlLookLeftRight, "Look Around"},
			{ControlMoveUpDown, "Move Camera"},
			{ControlMoveLeftRight, "Rotate Entity"},
			{ControlEnter, "Confirm Selection"},
			{ControlFrontendPause, "Cancel Selection"}
			});

		while (true) {
			GamePlay::clearArea();
			InstructionButtons::getInstance()->render();

			if (GetTickCount() < tempHelpTimeout) {
				drawHelpText(tempHelp);
			}
			else {
				drawHelpText(helpText);
			}

			changeControlThisFrame();

			moveCameraOnKeyboardInput(editingCam, false);
			rotateCameraOnMouseMove(editingCam);

			float moveLeftRight = CONTROLS::GET_DISABLED_CONTROL_NORMAL(0, ControlMoveLeftRight);

			float rotation = ENTITY::GET_ENTITY_HEADING(referenceEntity);

			rotation -= moveLeftRight * 2;
			if (rotation < 0) {
				rotation += 360;
			}
			if (rotation > 360) {
				rotation -= 360;
			}
			ENTITY::SET_ENTITY_HEADING(referenceEntity, rotation);

			if (moveLeftRight) {
				tempHelp = "Current Rotation: " + std::to_string(rotation);
				tempHelpTimeout = GetTickCount() + HELP_TEXT_TIMEOUT;
			}

			pullPlayerCloseIfTooFar(editingCam);

			if (CONTROLS::IS_DISABLED_CONTROL_JUST_RELEASED(0, ControlFrontendPause)) {
				ENTITY::SET_ENTITY_HEADING(referenceEntity, referenceRot);
				finalize(editingCam);
				return referenceRot;
			}

			if (CONTROLS::IS_DISABLED_CONTROL_JUST_RELEASED(0, ControlEnter)) {
				finalize(editingCam);
				return ENTITY::GET_ENTITY_HEADING(referenceEntity);
			}

			WAIT(0);
		}
	}

	std::vector<Vector3d> samplePoints(int number, std::string helpText, const Color markerColor, const std::vector<Marker>& referencePoints, const std::vector<Vector3d>& originalSet) {

		std::vector<Vector3d> points = originalSet;

		Camera editingCam = initialize();

		std::string tempHelp;
		DWORD tempHelpTimeout = 0;

		InstructionButtons::getInstance()->loadButtonList({
			{ControlLookLeftRight, "Look Around"},
			{ControlMoveUpDown, ""},
			{ControlMoveLeftRight, "Move Camera"},
			{ControlAttack, "Select Point" },
			{ControlAim, "Delete Point"},
			{ControlEnter, "Confirm Selection"},
			{ControlFrontendPause, "Cancel Selection"}
			});

		while (true) {

			GamePlay::clearArea();

			InstructionButtons::getInstance()->render();

			if (GetTickCount() < tempHelpTimeout) {
				drawHelpText(tempHelp);
			}
			else {
				drawHelpText(helpText);
			}

			for (auto& p : points) {
				drawMarker(MarkerTypeThickChevronUp, p, 0.5, markerColor);
			}

			for (auto& p : referencePoints) {
				drawMarker(MarkerTypeThickChevronUp, p.coords, 0.5, p.color);
			}

			changeControlThisFrame();

			moveCameraOnKeyboardInput(editingCam, true);
			rotateCameraOnMouseMove(editingCam);

			BOOL hit = false;
			Vector3d endCoords;
			GamePlay::rayTestEntity(editingCam, &hit, &endCoords, NULL);
			if (hit) {
				drawCrossFromTexture(DefaultColor::red.makeTransparent(150));
			}
			else {
				GamePlay::rayTestEveryThing(editingCam, &hit, &endCoords, NULL);
				drawCrossFromTexture(DefaultColor::blue.makeTransparent(150));
			}

			endCoords.z += GROUND_OFFSET;

			if (CONTROLS::IS_DISABLED_CONTROL_JUST_RELEASED(0, ControlAttack)) {
				if (hit == false) {
					tempHelp = "Not selecting anything.";
					tempHelpTimeout = GetTickCount() + HELP_TEXT_TIMEOUT;
				}
				else {
					if ((number == -1 || points.size() < number) && points.size() < 64) {
						tempHelp = "Placing marker at " + endCoords.to_string();
						tempHelpTimeout = GetTickCount() + HELP_TEXT_TIMEOUT;
						points.push_back(endCoords);
					}
					else if (number == 1) {
						tempHelp = "replacing marker by placing at " + endCoords.to_string();
						tempHelpTimeout = GetTickCount() + HELP_TEXT_TIMEOUT;
						points = { { endCoords } };
					}
					else {
						tempHelp = "Too many markers.";
						tempHelpTimeout = GetTickCount() + HELP_TEXT_TIMEOUT;
					}
				}
			}

			if (CONTROLS::IS_DISABLED_CONTROL_JUST_RELEASED(0, ControlAim)) {
				bool found = false;
				for (auto it = points.begin(); it != points.end(); it++) {
					if (SYSTEM::VDIST(endCoords.x, endCoords.y, endCoords.z, it->x, it->y, it->z) <= 2) {
						found = true;
						tempHelp = "Erasing marker at " + it->to_string();
						tempHelpTimeout = GetTickCount() + HELP_TEXT_TIMEOUT;
						points.erase(it);
						break;
					}
				}
				if (!found) {
					tempHelp = "No markers nearby to delete.";
					tempHelpTimeout = GetTickCount() + HELP_TEXT_TIMEOUT;
				}
			}

			pullPlayerCloseIfTooFar(editingCam);

			if (CONTROLS::IS_DISABLED_CONTROL_JUST_RELEASED(0, ControlFrontendPause)) {
				finalize(editingCam);
				return originalSet;
			}

			if (CONTROLS::IS_DISABLED_CONTROL_JUST_RELEASED(0, ControlEnter)) {
				if (points.size() == number || number == -1) {
					finalize(editingCam);
					break;
				}
				else {
					tempHelp = "Points recorded are not enough, " + std::to_string(number) + " needed.";
					tempHelpTimeout = GetTickCount() + HELP_TEXT_TIMEOUT;
				}
			}

			WAIT(0);
		}
		return points;
	}


	CameraParams sampleCameraParams(std::string helpText, const std::vector<Marker>& referencePoints, CameraParams original) {

		CameraParams params = original;

		Camera editingCam;
		if (params.fov < 0) {
			editingCam = initialize();
		}
		else {
			GamePlay::teleportPlayer(params.position);
			GamePlay::setPlayerDisappear(true);
			editingCam = GamePlay::activateCamera(params);
		}
		std::string tempHelp;
		DWORD tempHelpTimeout = 0;

		InstructionButtons::getInstance()->loadButtonList({
			{ControlLookLeftRight, "Rotate Camera"},
			{ControlMoveUpDown, ""},
			{ControlMoveLeftRight, "Move Camera"},
			{ControlEnter, "Confirm Selection"},
			{ControlFrontendPause, "Cancel Selection"}
			});

		while (true) {

			GamePlay::clearArea();

			InstructionButtons::getInstance()->render();

			if (GetTickCount() < tempHelpTimeout) {
				drawHelpText(tempHelp);
			}
			else {
				drawHelpText(helpText);
			}

			for (auto& p : referencePoints) {
				drawMarker(MarkerTypeThickChevronUp, p.coords, 0.5, p.color);
			}

			changeControlThisFrame();

			moveCameraOnKeyboardInput(editingCam, true);
			rotateCameraOnMouseMove(editingCam);
			pullPlayerCloseIfTooFar(editingCam);

			if (CONTROLS::IS_DISABLED_CONTROL_JUST_RELEASED(0, ControlFrontendPause)) {
				break;
			}

			if (CONTROLS::IS_DISABLED_CONTROL_JUST_RELEASED(0, ControlEnter)) {
				params.fov = CAM::GET_CAM_FOV(editingCam);
				params.position = CAM::GET_CAM_COORD(editingCam);
				params.rotation = CAM::GET_CAM_ROT(editingCam, 2);
				break;
			}

			WAIT(0);
		}
		finalize(editingCam);
		return params;
	}

}