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

const double gCellWidthCm = 26.;   
const double gCellHeightCm = 26.;
const double gWallWidthCm = 4.;
const double gCarWidthCm = 18.; 
const double gCarLengthCm = 26.;

enum mode {
    BLIND,
    PREPLANNED
};

bool gStart = false;
bool gInitialized = false;
mode gMode = BLIND;


// preplanned variables
vec2<int8_t> gStartPos = {-1,-1};
vec2<int8_t> gEndPos = {-1,-1};

vec2<double> gCarPos = {6,6};
vec2<int8_t> gNextMove = {6,5};

double gLeftCm = 0.;
double gRightCm = 0.;
double gCenterCm = 0.;

void setup() {
    rangeFinder.init(rangeFinderpins, 3, ULTRASONIC_HC_SR04);
    Serial.begin(9600);
    Bluetooth.begin(9600);

    motionController.init(
        gCellWidthCm, 
        gWallWidthCm, 
        &motorFrontLeft,
        &motorFrontRight,
        &motorBackLeft,
        &motorBackRight
        );

    pinMode(IR_L, INPUT);
    pinMode(IR_R, INPUT);
}

void updateDists() {
    rangeFinder.update();
    gLeftCm = rangeFinder.getDistance(0) - 2.5;
    gRightCm = rangeFinder.getDistance(1) - 2.5;
    gCenterCm = rangeFinder.getDistance(2) - 6.;

    if (gLeftCm < 0)
        gLeftCm = 0;
    if (gRightCm < 0)
        gRightCm = 0;
    if (gCenterCm < 0)
        gCenterCm = 0;
}

void printDists() {
    Serial2.print("left: " + String(gLeftCm));
    Serial2.print(" right: " + String(gRightCm));
    Serial2.print(" center: " + String(gCenterCm) + "\n");
}

void markWalls() {
    double carRotation = motionController.getCarRotation();
    vec2<double> carPos = mazeSolver.getCurrPos(); 

    double w = gCarWidthCm / 2.;
    double h = gCarLengthCm / 2.;

    updateDists();

    if (gCenterCm < 11.) {
        vec2<int8_t> centerWall = mazeSolver.markWall(carPos, gCenterCm+h, carRotation);

        if (centerWall != INVALID_VECTOR) {
            Bluetooth.println("{W " + String(centerWall.x) + "," + String(centerWall.y) + "}");
        }
    }

    if (gLeftCm < 12.) {
        vec2<int8_t> leftWall = mazeSolver.markWall(carPos, gLeftCm+w, carRotation-0.5*PI);

        if (leftWall != INVALID_VECTOR) {
            Bluetooth.println("{W " + String(leftWall.x) + "," + String(leftWall.y) + "}");
        }
    }

    if (gRightCm < 12.) {
        vec2<int8_t> rightWall = mazeSolver.markWall(carPos, gRightCm+w, carRotation+0.5*PI);

        if (rightWall != INVALID_VECTOR) {
            Bluetooth.println("{W " + String(rightWall.x) + "," + String(rightWall.y) + "}");
        }
    }

    Bluetooth.println("");
}

void demo_forward() {
    motionController.goForward();
    delay(1000);
    motionController.goBrake();
}

void demo_backward() {
    motionController.goBackward();
    delay(1000);
    motionController.goBrake();
}

void demo_left() {
    motionController.goLeft();
}

void demo_right() {
    motionController.goRight();
}

void updatePosition() {
    static unsigned long t1 = millis();
    static bool t1started = false;

    bool left = !digitalRead(IR_L);
    bool right = !digitalRead(IR_R);

    static bool lastLeft = left;
    static bool lastRight = right;

    int delayTime = 390;

    if ((millis() - t1 >= delayTime && t1started) || (millis() - t1 >= 4000 && !t1started)) {
        if (!t1started) {
            gNextMove = gCarPos;
            Bluetooth.println("Emergency response");
        }

        gCarPos = gNextMove;
        mazeSolver.setCurrPos(gCarPos);

        Bluetooth.println("{P " + String(gCarPos.x*2+1) + "," + String(gCarPos.y*2+1) + "}");
        
        t1started = false;
        t1 = millis();
    }

    // forward state change
    if (((lastLeft != left) || (lastRight != right)) && motionController.m_driveDir == FORWARD && !t1started) {
        t1 = millis();
        t1started = true;
    }

    // backward state change
    if ((left < lastLeft || right < lastRight) && motionController.m_driveDir == BACKWARD && !t1started) {
        t1 = millis();
        t1started = true;
    }

    lastLeft = left;
    lastRight = right;
}

void postStartInit() {
    // Init after starting from bluetooth

    if (gStartPos.x != -1 && gEndPos.x != -1) {
        gMode = PREPLANNED;
    }

    if (gMode == BLIND) {
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
    } else if (gMode == PREPLANNED) {
        mazeSolver.init(
                gWallWidthCm,
                gCellWidthCm,
                gCellHeightCm, 
                gMazeWidth, 
                gMazeHeight, 
                gStartPos, 
                gEndPos, 
                false 
                );
    } else {
        Bluetooth.println("Invalid mode");
        return;
    }

    gCarPos = mazeSolver.getCurrPos();

    markWalls();
    gNextMove = mazeSolver.getNextMove(motionController.getHeading());
    gInitialized = true;
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
    handleBluetoothCmds(
            gMazeWidth, 
            gMazeHeight, 
            gStart, 
            demo_forward, 
            demo_backward, 
            demo_left, 
            demo_right,
            gStartPos,
            gEndPos
            );

    // wait until START cmd 
    if (!gStart)
        return;

    if (!gInitialized)
        postStartInit();

    updatePosition();

    bool arrived = false;
    if (gCarPos == gNextMove) {
        motionController.goBrake(100);
        arrived = true;

        markWalls();

        if (gMode == PREPLANNED) {
            mazeSolver.floodFill(mazeSolver.getEndPos());
        }

        gNextMove = mazeSolver.getNextMove(motionController.getHeading());
        Bluetooth.println("next move");
    }

    if (!arrived)
        updateDists();

    motionController.drive(gCarPos.x, gCarPos.y, gNextMove.x, gNextMove.y, gLeftCm, gRightCm, gCenterCm);
}
