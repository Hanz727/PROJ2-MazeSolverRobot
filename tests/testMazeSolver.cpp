#include "../src/arduino_sketch/vec2.h"
#include "runTests.h"
#include "../src/arduino_sketch/MazeSolver.h"
#include <iostream>

int main() {
	//if (!runTests())
	//	return -1;

	MazeSolver mazeSolver(
		1.,
		20.,
		20.,
		10,
		10,
		{ 0,0 },
		{ 1,0 }
	);

	// 5.25 - 15.74
	// -4.75 / 4.74 + 1

	//std::cout << mazeSolver.posToPosEx({0,0}).x << "\n";
	mazeSolver.markWall({ 0,0 }, 10, 1.5*PI);
	mazeSolver.floodFill(mazeSolver.m_endPos);
	vec2<int> nm = mazeSolver.getNextMove();
	//std::cout << nm.x << " " << nm.y << "\n";

	//mazeSolver.markWall({ 0,0 }, 10, CompassDir::East);


	mazeSolver.printWalls();
	//while (true);;

}
