#pragma once
#include "inc\types.h"
#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include "system.h"


// extended class for Vector3
struct Vector3d : Vector3
{
	inline Vector3d& operator=(const Vector3d& other) // copy assignment
	{
		if (this != &other) { // self-assignment check expected
			this->x = other.x;
			this->y = other.y;
			this->z = other.z;
		}
		return *this;
	}

	inline Vector3d& operator=(const Vector3& other) // copy assignment
	{
		if (this != &other) { // self-assignment check expected
			this->x = other.x;
			this->y = other.y;
			this->z = other.z;
		}
		return *this;
	}

	inline Vector3d() {
		x = 0;
		y = 0;
		z = 0;
	}

	inline Vector3d(const float X, const float Y, const float Z) {
		x = X;
		y = Y;
		z = Z;
	}

	inline Vector3d(const Vector3& other) {
		operator=(other);
	}

	inline Vector3d operator=(const json& jsonVector3) {
		if (!jsonVector3.count("x") || !jsonVector3.count("y") || !jsonVector3.count("z")) {
			throw std::invalid_argument("Invalid json formatting for Vector3.");
		}
		x = jsonVector3["x"];
		y = jsonVector3["y"];
		z = jsonVector3["z"];
		return *this;
	}

	inline json to_json() {
		json res;
		res["x"] = x;
		res["y"] = y;
		res["z"] = z;
		return res;
	}

	inline std::string to_string() {
		return std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(z);
	}

	inline bool operator==(const Vector3d& other) {
		return x == other.x && y == other.y && z == other.z;
	}

	inline Vector3d operator+(const Vector3d& other) const
	{
		return Vector3d(x + other.x, y + other.y, z + other.z);
	}

	inline Vector3d operator-(const Vector3d& A) const
	{
		return Vector3d(x - A.x, y - A.y, z - A.z);
	}

	inline Vector3d operator/(const float A) const
	{
		return Vector3d(x / A, y / A, z / A);
	}

	inline Vector3d operator*(const float A) const
	{
		return Vector3d(x * A, y * A, z * A);
	}
};

struct Vector2 {
	float x;
	float y;
};

struct Color {
	inline Color() {};
	inline Color(int r, int g, int b) : red(r), green(g), blue(b) {}
	inline Color(int r, int g, int b, int alph) : red(r), green(g), blue(b), alpha(alph) {}

	inline Color& operator=(const Color& other) // copy assignment
	{
		if (this != &other) { // self-assignment check expected
			this->red = other.red;
			this->green = other.green;
			this->blue = other.blue;
			this->alpha = other.alpha;
		}
		return *this;
	}

	inline Color makeTransparent(int alph)  const {
		Color newColor(*this);
		newColor.alpha = alph;
		return newColor;
	}

	inline Color operator+(const Color& other) const
	{
		int r = red + other.red;
		int g = green + other.green;
		int b = blue + other.blue;

		if (r > 255 || g > 255 || b > 255) {
			float m = max(max(r, g), b) / 255.0f;
			r = (int)(r / m);
			g = (int)(g / m);
			b = (int)(b / m);
		}

		return Color( r, g, b, (alpha + other.alpha)/2 );
	}


	int red;
	int green;
	int blue;
	int alpha;
};


struct Marker {
	Vector3d coords;
	Color color;
};

// TODO: get a better name for this
struct PedWithMission {
	Ped ped;
	TaskSequence task;
	bool scripted;
	inline PedWithMission() : ped(0), task(0) {}
};

// to be precise, it is a vehicle with its driver with mission
struct VehicleWithMission {
	Ped driver;
	Vehicle veh;
	TaskSequence task;
	bool scripted;
	inline VehicleWithMission() : veh(0), driver(0), task(0), scripted(true) {}
};

struct CameraParams {
	Vector3d position;
	Vector3d rotation;
	float fov;

	inline CameraParams() : fov(-1.0) {}
	inline CameraParams(Vector3d POSITION, Vector3d ROTATION, float FOV) : position(POSITION), rotation(ROTATION), fov(FOV) {}

	inline CameraParams operator=(const json& jsonParams) {
		if (jsonParams.count("fov")) {
			fov = jsonParams["fov"];
		}
		if (jsonParams.count("position")) {
			position = jsonParams["position"];
		}
		if (jsonParams.count("rotation")) {
			rotation = jsonParams["rotation"];
		}

		return *this;
	}

	inline json to_json() {
		json res;
		res["position"] = position.to_json();
		res["rotation"] = rotation.to_json();
		res["fov"] = fov;
		return res;
	}

};

struct MenuItem {
	inline MenuItem(const std::string& LINE, const std::function<void()>& FUNCTION, const std::string& DESCRIPTION = "", bool* STATE = NULL, bool* ACTIVE = NULL, bool NEGATE_ACTIVE = false)
		: line(LINE), function(FUNCTION), description(DESCRIPTION), state(STATE), active(ACTIVE), negateActive(NEGATE_ACTIVE) {}

	// the text on the menu button
	std::string line;
	// the associated function
	std::function<void()> function;
	// the text shown as complementary description in the bottom
	std::string description;
	// reference to ON/OFF state of the menu item, if it can be specified in this way
	bool* state;
	// reference to a variable that controls whether the item can be used or not
	bool* active;
	// whether to negate the above mentioned reference to get item's activeness
	bool negateActive;
};

struct UnitArea {
	Vector3d center;
	Vector3d xUnit;
	Vector3d yUnit;

	inline UnitArea operator=(const json& jsonArea) {
		if (jsonArea.count("center")) {
			center = jsonArea["center"];
		}
		if (jsonArea.count("xUnit")) {
			xUnit = jsonArea["xUnit"];
		}
		if (jsonArea.count("yUnit")) {
			yUnit = jsonArea["yUnit"];
		}

		return *this;
	}

	inline json to_json() {
		json res;
		res["center"] = center.to_json();
		res["xUnit"] = xUnit.to_json();
		res["yUnit"] = yUnit.to_json();
		return res;
	}

	inline Vector3d sample() {
		float ratio1 = randomFloat(0.0f, 1.0f);
		float ratio2 = randomFloat(0.0f, 1.0f);
		return (xUnit - center) * ratio1 + (yUnit - center) * ratio2 + center;
	}
};

struct CarLane {
	Vector3d front;
	Vector3d back;
	float heading;
	inline CarLane() : heading(0) {}

	inline CarLane operator=(const json& jsonArea) {
		if (jsonArea.count("front")) {
			front = jsonArea["front"];
		}
		if (jsonArea.count("back")) {
			back = jsonArea["back"];
		}
		if (jsonArea.count("heading")) {
			heading = jsonArea["heading"];
		}
		return *this;
	}

	inline json to_json() {
		json res;
		res["front"] = front.to_json();
		res["back"] = back.to_json();
		res["heading"] = heading;
		return res;
	}

	inline Vector3d sample() {
		float ratio = randomFloat(0.0f, 1.0f);
		return (back - front) * ratio + front;
	}
};


// define a range between two numbers (int, float, double, etc.)
template <
	typename T,
	typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type
>
struct NumericalRange {
	T minVal;
	T maxVal;
	inline NumericalRange() : minVal(0), maxVal(0) {}
	inline NumericalRange(T minVal, T maxVal): minVal(minVal), maxVal(maxVal) {
		if (minVal > maxVal) {
			outputDebugMessage("Invalid Range detected. Continue.");
		}
	}
	inline T sample() {
		if (std::is_floating_point<T>::value) {
			return randomFloat(minVal, maxVal);
		}
		else {
			return randomInt(minVal, maxVal);
		}
	}
	inline NumericalRange operator=(const json& range) {
		if (range.count("minVal")) {
			minVal = range["minVal"];
		}
		if (range.count("maxVal")) {
			maxVal = range["maxVal"];
		}
		return *this;
	}

	inline NumericalRange operator=(const std::string& range) {
		int division = range.find('-');
		if (isFloat(range)) {
			minVal = std::stof(range);
			maxVal = minVal;
		}
		else if (division == std::string::npos || division == range.size() - 1) {
			outputDebugMessage("Invalid range input.");
		}
		else {
			std::string minStr = range.substr(0, division);
			std::string maxStr = range.substr(division + 1);
			if (!isFloat(minStr) || !isFloat(maxStr)) {
				outputDebugMessage("Invalid range input.");
			}
			else {
				minVal = std::stof(minStr);
				maxVal = std::stof(maxStr);
			}
		}
		return *this;
	}

	inline std::string to_string() {
		return std::to_string(minVal) + "-" + std::to_string(maxVal);
	}

	inline json to_json() {
		json res;
		res["minVal"] = minVal;
		res["maxVal"] = maxVal;
		return res;
	}
};
