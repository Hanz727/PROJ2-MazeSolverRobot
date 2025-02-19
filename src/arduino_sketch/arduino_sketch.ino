#include "RangeFinder.h"
#include "MazeSolver.h"
#include "bluetooth.h"
#include "ShieldMotor.h"

const uint8_t rangeFinderpins[6] = {22, 23, 24, 25, 28, 29};

RangeFinder rangeFinder;
MazeSolver mazeSolver;

ShieldMotor motorFrontLeft(3);
ShieldMotor motorFrontRight(4);
ShieldMotor motorBackLeft(2);
ShieldMotor motorBackRight(1);

uint8_t gMazeWidth = 7;
uint8_t gMazeHeight = 7;
bool gStart = false;

bool initialized = false;

void setup() {
  // put your setup code here, to run once:
  rangeFinder.init(rangeFinderpins, 3, ULTRASONIC_HC_SR04);
  Serial.begin(9600);
  Bluetooth.begin(9600);
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
  // gStart = true;
  handleBluetoothCmds(gMazeWidth, gMazeHeight, gStart);

  // wait until started
  if (!gStart)
    return;

  if (!initialized) {
    // Init after starting from bluetooth
    mazeSolver.init(4.0,  // 4cm walls (MEASURED!)
                    20.0, // cell width NOT MEASURED YET
                    20.0, // cell height NOT MEASURED YET
                    gMazeWidth*2-1, 
                    gMazeHeight*2-1, 
                    {gMazeWidth-1, gMazeHeight-1}, 
                    {-1,-1}, 
                    true // blind mode on
    );
    initialized = true;
  }

  // update ultrasonic distances
  rangeFinder.update();
  double leftCm = rangeFinder.getDistance(0);
  double rightCm = rangeFinder.getDistance(1);
  double centerCm = rangeFinder.getDistance(2);

}
