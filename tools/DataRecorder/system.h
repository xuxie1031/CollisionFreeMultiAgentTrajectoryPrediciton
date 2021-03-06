#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

// helper functions that are independent from game API's

void outputDebugMessage(const std::string& s);

std::vector<std::string> getFolderFileList(std::string folder);
void createAllSubdirectories(const std::string& path);
float randomFloat(float low, float high);
int randomInt(int low, int high);
bool isFloat(std::string str);