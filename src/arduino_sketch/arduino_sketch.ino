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
ShieldMotor motorBackRight(1);
ShieldMotor motorBackLeft(2);

MotionControl motionController;

int8_t gMazeWidth = 7;
int8_t gMazeHeight = 7;

double gCellWidthCm = 28.;
double gCellHeightCm = 28.;
double gWallWidthCm = 4.2;
const double gCarWidthCm = 15.8; 
const double gCarLengthCm = 26.;

bool gStart = false;
bool gInitialized = false;
vec2<double> gCarPos = {6,6};

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

    double w = gCarWidthCm / 2.;
    double h = gCarLengthCm / 2.;

    double left = rangeFinder.getDistance(0);
    if (left < 45.) {
        mazeSolver.markWall(carPos - vec2<double>{w*cosR, w*sinR}, left, carRotation);
    }

    double right = rangeFinder.getDistance(1); 
    if (right < 45.) {
        mazeSolver.markWall(carPos + vec2<double>{w*cosR, w*sinR}, right, carRotation);
    }

    double center = rangeFinder.getDistance(2);
    if (center < 45.) {
        mazeSolver.markWall(carPos + vec2<double>{h*cosR, h*sinR}, center, carRotation);
    }

}

void accuratePos(vec2<double>& carPos, double leftCm, double rightCm, double centerCm) {
    int offx = 0;

    if (leftCm > 0 && leftCm <= (gCellWidthCm - gCarWidthCm)) {
        offx = -1 + (leftCm + gCarWidthCm/2)/(gCellWidthCm/2);
    } else if (rightCm > 0 && rightCm <= (gCellWidthCm - gCarWidthCm)){
        offx = 1 - (rightCm + gCarWidthCm/2)/(gCellWidthCm/2);
    }

    CompassDir rot = mazeSolver.radiansToDirection(motionController.getCarRotation());
    if (rot == South || rot == West) {
        offx = -offx;
    }

    if (rot == East || rot == West) {
        carPos.y = carPos.y + offx;
        return;
    }
    carPos.x = carPos.x + offx;
}

void demo_forward() {
    motorFrontLeft.run(FORWARD);
    motorFrontRight.run(FORWARD);
    motorBackLeft.run(FORWARD);
    motorBackRight.run(FORWARD);
    motorFrontLeft.setSpeed(120);
    motorFrontRight.setSpeed(120);
    motorBackLeft.setSpeed(120);
    motorBackRight.setSpeed(120);
    delay(1000);
    motorFrontLeft.setSpeed(255);
    motorFrontRight.setSpeed(255);
    motorBackLeft.setSpeed(255);
    motorBackRight.setSpeed(255);
    delay(1000);
    motorFrontLeft.run(RELEASE);
    motorFrontRight.run(RELEASE);
    motorBackLeft.run(RELEASE);
    motorBackRight.run(RELEASE);
}

void demo_backward() {
    motorFrontLeft.run(BACKWARD);
    motorFrontRight.run(BACKWARD);
    motorBackLeft.run(BACKWARD);
    motorBackRight.run(BACKWARD);
    motorFrontLeft.setSpeed(255);
    motorFrontRight.setSpeed(255);
    motorBackLeft.setSpeed(255);
    motorBackRight.setSpeed(255);
    delay(1000);
    motorFrontLeft.run(RELEASE);
    motorFrontRight.run(RELEASE);
    motorBackLeft.run(RELEASE);
    motorBackRight.run(RELEASE);
}

void demo_left() {
    motorFrontLeft.run(BACKWARD);
    motorFrontRight.run(FORWARD);
    motorBackLeft.run(FORWARD);
    motorBackRight.run(BACKWARD);
    motorFrontLeft.setSpeed(255);
    motorFrontRight.setSpeed(255);
    motorBackLeft.setSpeed(255);
    motorBackRight.setSpeed(255);
    delay(1000);
    motorFrontLeft.run(RELEASE);
    motorFrontRight.run(RELEASE);
    motorBackLeft.run(RELEASE);
    motorBackRight.run(RELEASE);
}

void demo_right() {
    motorFrontLeft.run(FORWARD);
    motorFrontRight.run(BACKWARD);
    motorBackLeft.run(BACKWARD);
    motorBackRight.run(FORWARD);
    motorFrontLeft.setSpeed(255);
    motorFrontRight.setSpeed(255);
    motorBackLeft.setSpeed(255);
    motorBackRight.setSpeed(255);
    delay(1000);
    motorFrontLeft.run(RELEASE);
    motorFrontRight.run(RELEASE);
    motorBackLeft.run(RELEASE);
    motorBackRight.run(RELEASE);
}

void demo_spin() {
    motorFrontLeft.run(FORWARD);
    motorFrontRight.run(BACKWARD);
    motorBackLeft.run(FORWARD);
    motorBackRight.run(BACKWARD);
    motorFrontLeft.setSpeed(255);
    motorFrontRight.setSpeed(255);
    motorBackLeft.setSpeed(255);
    motorBackRight.setSpeed(255);

    delay(1000);
    motorFrontLeft.run(RELEASE);
    motorFrontRight.run(RELEASE);
    motorBackLeft.run(RELEASE);
    motorBackRight.run(RELEASE);
}

void loop() {
    // CMD LIST:
    // GET DIM -> sends dimensions as WxH
    // SET DIM WxH -> Sets dimensions
    // START -> starts the program (and initializes)
    // STOP  -> stops the program
    // RESET -> reset button on arduino
    // FORWARD -> calls demo_forward func
    // BACKWARD
    // LEFT
    // RIGHT
    // SPIN
    //
    // IMPORTANT! If you don't want to use bluetooth now, just comment out the line below and set gStart to true.
    handleBluetoothCmds(gMazeWidth, gMazeHeight, gStart, demo_forward, demo_backward, demo_left, demo_right, demo_spin);
    //gStart = true;

    // wait until START cmd 
    if (!gStart)
        return;

    if (!gInitialized) {
        // Init after starting from bluetooth
        mazeSolver.init(
                gWallWidthCm,
                gCellWidthCm,
                gCellHeightCm, 
                gMazeWidth*2-1, 
                gMazeHeight*2-1, 
                {(int8_t)(gMazeWidth-1), (int8_t)(gMazeHeight-1)}, 
                {-1,-1}, 
                true // blind mode on
            );
        gCarPos = mazeSolver.getCurrPos();

        motionController.init(
                gCellWidthCm, 
                gWallWidthCm, 
                &motorFrontLeft,
                &motorFrontRight,
                &motorBackLeft,
                &motorBackRight
            );

        gInitialized = true;
    }

    rangeFinder.update();
    double leftCm = rangeFinder.getDistance(0);
    double rightCm = rangeFinder.getDistance(1);
    double centerCm = rangeFinder.getDistance(2);

    accuratePos(gCarPos, leftCm, rightCm, centerCm);
    mazeSolver.setCurrPos(gCarPos);
    markWalls();

    static vec2<int8_t> nextMove = mazeSolver.getNextMove(motionController.getHeading());
    if (motionController.drive(gCarPos.x, gCarPos.y, nextMove.x, nextMove.y, leftCm, rightCm, centerCm)) {
        gCarPos = nextMove;
        nextMove = mazeSolver.getNextMove(motionController.getHeading());
    }

}
