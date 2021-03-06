#pragma once

#include "script.h"

namespace Demo {
	void plotPedTrajectory();
	void pedVarianceTest();
	void carVarianceTest();
	void plotCarTrajectory();
	void spawn();
	void carMarker();
	void toggleCamera(bool& value);
	void drawFullTrajectReplay();
	void startEndDemo(SimulationData& data);
	void startSimulation(SimulationData& data);
	void pedStartEndDemo(SimulationData& data);
	void setCameraDemo(SimulationData& data);
}