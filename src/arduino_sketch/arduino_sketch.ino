#include "RangeFinder.h"
#include "MazeSolver.h"
#include "bluetooth.h"
#include "ShieldMotor.h"
#include "MotionControl.h"

#define IR_L 33
#define IR_R 35

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

double gCellWidthCm = 26.;   
double gCellHeightCm = 26.;
double gWallWidthCm = 4.;
const double gCarWidthCm = 18.; 
const double gCarLengthCm = 26.;

bool gStart = false;
bool gInitialized = false;
vec2<double> gCarPos = {6,6};
vec2<int8_t> gNextMove = {6,5};

bool gShouldMarkWall = true;

void setup() {
    rangeFinder.init(rangeFinderpins, 3, ULTRASONIC_HC_SR04);
    Serial.begin(9600);
    Bluetooth.begin(9600);

    pinMode(IR_L, INPUT);
    pinMode(IR_R, INPUT);
}

void markWalls(double leftCm, double rightCm, double centerCm) {
    if (!gShouldMarkWall)
        return;

    double carRotation = motionController.getCarRotation();
    vec2<double> carPos = mazeSolver.getCurrPos(); 

    double w = gCarWidthCm / 2.;
    double h = gCarLengthCm / 2.;

    if (leftCm < 40.) {
        if (mazeSolver.markWall(carPos, leftCm+w, carRotation-0.5*PI)) {
            Bluetooth.println("marked left");
        }
    }

    if (rightCm < 40.) {
        if (mazeSolver.markWall(carPos, rightCm+w, carRotation+0.5*PI)) {
            Bluetooth.println("marked right");
        }
    }

    if (centerCm < 40.) {
        if (mazeSolver.markWall(carPos, centerCm+h, carRotation)) {
            Bluetooth.println("marked center");
        }
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
        carPos.y = (int)carPos.y + offx;
        return;
    }
    carPos.x = (int)carPos.x + offx;
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

    delay(450*4);
    motorFrontLeft.run(RELEASE);
    motorFrontRight.run(RELEASE);
    motorBackLeft.run(RELEASE);
    motorBackRight.run(RELEASE);
}

void printDists(double left, double right, double center) {
    Serial2.print("left: " + String(left));
    Serial2.print(" right: " + String(right));
    Serial2.print(" center: " + String(center) + "\n");
}

void handle_timers() {
    static unsigned long t1 = millis();

    gShouldMarkWall = false;
    bool left = !digitalRead(IR_L);
    bool right = !digitalRead(IR_R);

    static bool lastLeft = left;
    static bool lastRight = right;
    static bool t1started = false;

    // delays aanpassen!
    int delayTime = 200;
    if (motionController.m_driveDir == BACKWARD)
        delayTime = 0;

    if (millis() - t1 >= delayTime && t1started) {
        gCarPos = gNextMove;
        //gShouldMarkWall = true;
        Bluetooth.println("pos updated to: " + String(gCarPos.x) + " " + String(gCarPos.y) + " after: " + String(millis() - t1) + "ms");
        t1started = false;
    }

    if ((lastLeft != left) || (lastRight != right)) {
        //Bluetooth.println("T1 started with: " + String(millis() - t1));
        t1 = millis();
        t1started = true;
    }

    lastLeft = left;
    lastRight = right;
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

    rangeFinder.update();
    double leftCm = rangeFinder.getDistance(0) - 2.5;
    if (leftCm < 0)
        leftCm = 0;
    double rightCm = rangeFinder.getDistance(1) - 2.5;
    if (rightCm < 0)
        rightCm = 0;

    double centerCm = rangeFinder.getDistance(2) - 4;
    if (centerCm < 0)
        centerCm = 0;

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

        markWalls(leftCm, rightCm, centerCm);
        gNextMove = mazeSolver.getNextMove(motionController.getHeading());

        gInitialized = true;
    }

    handle_timers();
    //accuratePos(gCarPos, leftCm, rightCm, centerCm);
    mazeSolver.setCurrPos(gCarPos);

    if (gCarPos == gNextMove) {
        Bluetooth.println("braking");
        motionController.goBrake(1000);
        
        gShouldMarkWall = true;
        printDists(leftCm, rightCm, centerCm);
        markWalls(leftCm, rightCm, centerCm);

        gNextMove = mazeSolver.getNextMove(motionController.getHeading());
        Serial2.println("nm: " + String(gNextMove.x) + " " + String(gNextMove.y) );
    }

    motionController.drive(gCarPos.x, gCarPos.y, gNextMove.x, gNextMove.y, leftCm, rightCm, centerCm);
}

