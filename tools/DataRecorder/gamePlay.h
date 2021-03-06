#pragma once

#include "inc\natives.h"
#include "inc\types.h"
#include "inc\enums.h"
#include "inc\main.h"

#include "myEnums.h"
#include "myTypes.h"

namespace GamePlay {
	Vector3d rotToUnitVec(Vector3d rot);
	void rayTestEntity(Cam cam, BOOL* hit, Vector3d* entCoords, Entity* hitEnt);
	void rayTestEveryThing(Cam cam, BOOL* hit, Vector3d* endCoords, Entity* hitEnt);
	void teleportPlayer(Vector3d coords, bool withZOffset = false);
	void clearArea();
	void setPlayerDisappear(bool toggle);
	float getModelLength(LPCSTR modelName);
	std::string getTextInput();
	void getEntityMotion(Entity e, Vector3d* coords3D = NULL, Vector2* coords2D = NULL, float* heading = NULL, float* speed = NULL);
	Camera activateCamera(CameraParams params);
	void destroyCamera(Camera cam);
}