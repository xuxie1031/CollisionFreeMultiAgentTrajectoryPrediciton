#include "system.h"

void outputDebugMessage(const std::string& s) {

	std::string logFile = "DataRecorder.log";
	std::ofstream debug;

	static bool initialized = false;
	if (initialized == false) {
		initialized = true;
		debug.open(logFile);
	}
	else {
		debug.open(logFile, std::fstream::app);
	}
	DWORD t = GetTickCount();

	debug << t << ": " << s << std::endl;
	debug.close();
}

std::vector<std::string> getFolderFileList(std::string folder)
{
	std::vector<std::string> res;
	std::string path = folder + "\\*";
	WIN32_FIND_DATA data;
	HANDLE hFind = FindFirstFile(path.c_str(), &data);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				res.push_back(data.cFileName);
			}
		} while (FindNextFile(hFind, &data));
		FindClose(hFind);
	}
	return res;
}

void createAllSubdirectories(const std::string& path) {
	size_t start = 0;
	size_t end = 0;
	while ((end = path.find_first_of("/\\", start)) != std::string::npos) {
		CreateDirectory(path.substr(0, end).c_str(), NULL);
		start = end + 1;
	}
	CreateDirectory(path.c_str(), NULL);
}

float randomFloat(float low, float high) {
	return low + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (high - low)));
}

int randomInt(int low, int high) {
	return low + rand() % (high - low + 1);
}

bool isFloat(std::string str) {
	std::istringstream iss(str);
	float f;
	iss >> std::noskipws >> f;
	return iss.eof() && !iss.fail();
}