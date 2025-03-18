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
bool gStart = false;

const double gCarWidth = 15.8; // cm
const double gCarLength = 26.; // cm

bool initialized = false;
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

    double w = gCarWidth / 2.;
    double h = gCarLength / 2.;

    // Maybe add aditional conditions for distance correctness!

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

    if (leftCm > 0 && leftCm <= (gCellWidthCm - gCarWidth)) {
        offx = -1 + (leftCm + gCarWidth/2)/(gCellWidthCm/2);
    } else if (rightCm > 0 && rightCm <= (gCellWidthCm - gCarWidth)){
        offx = 1 - (rightCm + gCarWidth/2)/(gCellWidthCm/2);
    }

    CompassDir rot = mazeSolver.radiansToDirection(motionController.getCarRotation());
    if (rot == South || rot == West){
        offx = -offx;
    }

    if (rot == East){
        carPos.y = carPos.y + offx;
        return;
    }
    carPos.x = carPos.x + offx;
}

void generalPos(vec2<double>& carPos) {
  
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
    delay(500);
    motorFrontLeft.setSpeed(255);
    motorFrontRight.setSpeed(255);
    motorBackLeft.setSpeed(255);
    motorBackRight.setSpeed(255);
    delay(500);
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
    //
    // IMPORTANT! If you don't want to use bluetooth now, just comment out the line below and set gStart to true.
    handleBluetoothCmds(gMazeWidth, gMazeHeight, gStart, demo_forward, demo_backward, demo_left, demo_right, demo_spin);
    //gStart = true;

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

    // TODO: update general position into the gCarPos variable
    generalPos(gCarPos);
    accuratePos(gCarPos, leftCm, rightCm, centerCm);
    mazeSolver.setCurrPos(gCarPos);

    // 2. Ensure car is relatively parallel to the walls before calling markWalls()
    if (true) {
        markWalls();
    }

    static vec2<int8_t> nextMove = mazeSolver.getNextMove(motionController.getHeading());

    // 3. Make this function (move from A to B)
    bool arrived = motionController.drive(gCarPos.x, gCarPos.y, nextMove.x, nextMove.y);

    if (arrived) {
        nextMove = mazeSolver.getNextMove(motionController.getHeading());
        motionController.drive(gCarPos.x, gCarPos.y, nextMove.x, nextMove.y);
    }

}
