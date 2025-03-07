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

uint8_t gMazeWidth = 7;
uint8_t gMazeHeight = 7;
bool gStart = false;

const double gCarWidth = 15.8; // cm
const double gCarLength = 26.; // cm

bool initialized = false;

void setup() {
    // put your setup code here, to run once:
    rangeFinder.init(rangeFinderpins, 3, ULTRASONIC_HC_SR04);
    Serial.begin(9600);
    Bluetooth.begin(9600);
}

bool validateRange(double a, double b, double c) {
    if ( a > 330.)
        return false;
    if ( b > 330.)
        return false;
    if ( c > 330.)
        return false;
    return true;
}

void markWalls() {
    double carRotation = motionController.getCarRotation();
    vec2<double> carPos = mazeSolver.getCurrPos(); 

    double sinR = sin(carRotation);
    double cosR = cos(carRotation);

    double w = gCarWidth / 2.;
    double h = gCarLength / 2.;

    mazeSolver.markWall(carPos - vec2<double>{w*cosR, w*sinR}, rangeFinder.getDistance(0), carRotation);
    mazeSolver.markWall(carPos + vec2<double>{w*cosR, w*sinR}, rangeFinder.getDistance(1), carRotation);
    mazeSolver.markWall(carPos + vec2<double>{h*cosR, h*sinR}, rangeFinder.getDistance(2), carRotation);
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
                4.0,  // 4cm walls (MEASURED!)
                28.0, // cell width (MEASURED)
                28.0, // cell height (MEASURED) 
                gMazeWidth*2-1, 
                gMazeHeight*2-1, 
                {gMazeWidth-1, gMazeHeight-1}, 
                {-1,-1}, 
                true // blind mode on
                );

        motionController.init(
                28.0, // cell width NOT MEASURED YET
                4.0,  // 4cm walls
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

    // check if the readings are realistic
    if (!validateRange(leftCm, rightCm, centerCm))
        return;

    // TODO: 
    // 1. update current position
    // mazeSolver.setCurrPos({x, y});
    
    vec2<double> carPos = mazeSolver.getCurrPos();

    // 2. Ensure car is relatively parallel to the walls before calling markWalls()
    if (true) {
        markWalls();
    }

    static vec2<int> nextMove = mazeSolver.getNextMove(motionController.getHeading());

    // 3. Make this function (move from A to B)
    bool arrived = motionController.drive(carPos.x, carPos.y, nextMove.x, nextMove.y);

    if (arrived) {
        nextMove = mazeSolver.getNextMove(motionController.getHeading());
    }


}
