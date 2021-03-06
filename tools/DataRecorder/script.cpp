#include "script.h"

#pragma warning(disable : 4244 4305) // double <-> float conversions

void main()
{
	SimulationData data;
	Simulator sim(data);
	Menu mainMenu("Data Recorder", {
		{"Vehicle Settings",    std::bind(vehicleMenu, std::ref(data))},
		{"Pedestrian Settings", std::bind(pedMenu, std::ref(data))},
		{"Camera Settings",     std::bind(cameraMenu, std::ref(data))},
		{"Record Settings",     std::bind(recordingMenu, std::ref(data))},
		{"Replay Settings",     std::bind(replayMenu, std::ref(data))},
		});

	auto saveSettings = [&]() {
		std::string fileName = GamePlay::getTextInput();
		data.saveData(fileName);
		mainMenu.items[6].description = data.getSettingsFileName();
	};

	auto loadSettings = [&]() {
		Menu loadSettingsList("Choose Setting File", {});
		auto fileList = getFolderFileList(SIM_DATA_DIRECTORY);

		auto loadFile = [&](int i) {
			outputDebugMessage("Load config file " + fileList[i]);
			data.loadData(fileList[i]);
			mainMenu.items[6].description = data.getSettingsFileName();
		};

		std::vector<std::function<void()>> funcList;

		for (int i = 0; i < fileList.size(); i++) {
			loadSettingsList.addMenuItem({ fileList[i], std::bind(loadFile, i) });
		}
		loadSettingsList.singleUse = true;

		loadSettingsList.processMenu();
	};

	auto newSettings = [&]() {
		data.initialize();
		mainMenu.items[6].description = data.getSettingsFileName();
	};

	mainMenu.addMenuItem({ "Load Settings",      loadSettings });
	mainMenu.addMenuItem({ "Save Settings",      saveSettings, data.getSettingsFileName() });
	mainMenu.addMenuItem({ "New Settings",       newSettings });
	mainMenu.addMenuItem({ "Start Simulation",      std::bind(&Simulator::startSimulation, &sim) });
	mainMenu.addMenuItem({ "Start Replay", std::bind(&Simulator::processReplay, &sim, true) });
	
	std::vector<std::pair<eControl, std::string>> menuButtonList = { {MENU_KEY, "Open Data Recorder"} };

	InstructionButtons::getInstance()->loadButtonList(menuButtonList);

	while (true)
	{
		InstructionButtons::getInstance()->render();
		if (isMenuKeyJustUp()) {
			mainMenu.processMenu();
			InstructionButtons::getInstance()->loadButtonList(menuButtonList);
		}
		WAIT(0);
	}
}

void ScriptMain()
{
	srand(GetTickCount());
	main();
}

