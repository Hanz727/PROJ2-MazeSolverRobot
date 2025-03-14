#include "RangeFinder.h"
#include "MazeSolver.h"
#include "bluetooth.h"
#include "ShieldMotor.h"
#include "MotionControl.h"

const uint8_t rangeFinderpins[6] = {22, 23, 24, 25, 28, 29};

RangeFinder rangeFinder;
MazeSolver mazeSolver;

ShieldMotor motorFrontLeft(3);
ShieldMotor motorFrontRight(4);
ShieldMotor motorBackLeft(2);
ShieldMotor motorBackRight(1);

MotionControl motionController;

int8_t gMazeWidth = 7;
int8_t gMazeHeight = 7;
bool gStart = false;

const double gCarWidth = 15.8; // cm
const double gCarLength = 26.; // cm

bool initialized = false;

void setup() {
  rangeFinder.init(rangeFinderpins, 3, ULTRASONIC_HC_SR04);
  Serial.begin(9600);
  Bluetooth.begin(9600);

}

void markWalls() {
  double carRotation = motionController.getCarRotation();
  vec2<double> carPos = mazeSolver.getCurrPos(); 

  double sinR = sin(carRotation);
  double cosR = cos(carRotation);

  double w = gCarWidth / 2.;
  double h = gCarLength / 2.;
  
  // Maybe add aditional conditions for distance correctness!

  double left = rangeFinder.getDistance(0);
  if (left < 330.) {
    mazeSolver.markWall(carPos - vec2<double>{w*cosR, w*sinR}, left, carRotation);
  }
  
  double right = rangeFinder.getDistance(1); 
  if (right < 330.) {
    mazeSolver.markWall(carPos + vec2<double>{w*cosR, w*sinR}, right, carRotation);
  }
  
  double center = rangeFinder.getDistance(2);
  if (center < 330.) {
    mazeSolver.markWall(carPos + vec2<double>{h*cosR, h*sinR}, center, carRotation);
  }

}

void loop() {
  // CMD LIST:
  // GET DIM -> sends dimensions as WxH
  // SET DIM WxH -> Sets dimensions
  // START -> starts the program (and initializes)
  // STOP  -> stops the program
  // RESET -> reset button on arduino
  //
  // IMPORTANT! If you don't want to use bluetooth now, just comment out the line below and set gStart to true.
  handleBluetoothCmds(gMazeWidth, gMazeHeight, gStart);
  // gStart = true;

  // wait until started
  if (!gStart)
    return;

  if (!initialized) {
    // Init after starting from bluetooth
    
    mazeSolver.init(
        4.0,
        28.0,
        28.0, 
        gMazeWidth*2-1, 
        gMazeHeight*2-1, 
        {(int8_t)(gMazeWidth-1), (int8_t)(gMazeHeight-1)}, 
        {-1,-1}, 
        true // blind mode on
        );

    motionController.init(
        28.0, 
        4.0, 
        &motorFrontLeft,
        &motorFrontRight,
        &motorBackLeft,
        &motorBackRight
        );

    initialized = true;
  }

  // update ultrasonic distances
  rangeFinder.update();
  double leftCm = rangeFinder.getDistance(0);
  double rightCm = rangeFinder.getDistance(1);
  double centerCm = rangeFinder.getDistance(2);

  // TODO: 
  // 1. update current position
  // mazeSolver.setCurrPos({x, y});

  vec2<double> carPos = mazeSolver.getCurrPos();

  // 2. Ensure car is relatively parallel to the walls before calling markWalls()
  if (true) {
    markWalls();
  }

  static vec2<int8_t> nextMove = mazeSolver.getNextMove(motionController.getHeading());

  // 3. Make this function (move from A to B)
  bool arrived = motionController.drive(carPos.x, carPos.y, nextMove.x, nextMove.y);

  if (arrived) {
    nextMove = mazeSolver.getNextMove(motionController.getHeading());
  }

}
