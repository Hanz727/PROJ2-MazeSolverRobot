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

void markWalls(double leftCm, double rightCm, double centerCm) {
    double carRotation = motionController.getCarRotation();
    vec2<double> carPos = mazeSolver.getCurrPos(); 

    double w = gCarWidthCm / 2.;
    double h = gCarLengthCm / 2.;

    if (centerCm < 11.) {
        vec2<int8_t> centerWall = mazeSolver.markWall(carPos, centerCm+h, carRotation);
        if (centerWall != INVALID_VECTOR) {
            //Bluetooth.print("center ");
            Bluetooth.println("{W " + String(centerWall.x) + "," + String(centerWall.y) + "}");
        }
    }

    if (leftCm < 12.) {
        vec2<int8_t> leftWall = mazeSolver.markWall(carPos, leftCm+w, carRotation-0.5*PI);
        if (leftWall != INVALID_VECTOR) {
            //Bluetooth.print("left ");
            Bluetooth.println("{W " + String(leftWall.x) + "," + String(leftWall.y) + "}");
        }
    }

    if (rightCm < 12.) {
        vec2<int8_t> rightWall = mazeSolver.markWall(carPos, rightCm+w, carRotation+0.5*PI);
        if (rightWall != INVALID_VECTOR) {
            Bluetooth.println("{W " + String(rightWall.x) + "," + String(rightWall.y) + "}");
        }
    }

    Bluetooth.println("");
}

void demo_forward() {
    motionController.goForward();
    delay(1000);
    motionController.goBrake(0, 80);
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

void configSetTurnTime(int16_t time) {
    motionController.setRotTime(time);
}

void printDists(double left, double right, double center) {
    Serial2.print("left: " + String(left));
    Serial2.print(" right: " + String(right));
    Serial2.print(" center: " + String(center) + "\n");
}

void updatePosition(double centerCm) {
    static unsigned long t1 = millis();
    static bool t1started = false;

    bool left = !digitalRead(IR_L);
    bool right = !digitalRead(IR_R);

    static bool lastLeft = left;
    static bool lastRight = right;

    int delayTime = 450;
    if (motionController.m_driveDir == BACKWARD) {
        delayTime = 100;
    }

    if (millis() - t1 >= delayTime && t1started) {
        gCarPos = gNextMove;
        mazeSolver.setCurrPos(gCarPos);

        Bluetooth.println("{P " + String(gCarPos.x*2+1) + "," + String(gCarPos.y*2+1) + "}");
        //Bluetooth.println("pos updated to: " + String(gCarPos.x) + " " + String(gCarPos.y) + " after: " + String(millis() - t1) + "ms");
        t1started = false;
    }

    // anti wall fucker
    if (millis() - t1 > 2000 && t1started) {
        gCarPos = gNextMove;
        mazeSolver.setCurrPos(gCarPos);
        Bluetooth.println("{P " + String(gCarPos.x*2+1) + "," + String(gCarPos.y*2+1) + "}");
        t1started = false;
    }

    if (((lastLeft != left) || (lastRight != right)) && motionController.m_driveDir != BACKWARD && motionController.m_driveDir != RELEASE && !t1started) {
        //Bluetooth.println("T1 started with: " + String(millis() - t1));
        t1 = millis();
        t1started = true;
    }

    if ((left < lastLeft || right < lastRight) && motionController.m_driveDir == BACKWARD && motionController.m_driveDir != RELEASE  && !t1started) {
        t1 = millis();
        t1started = true;
    }

    lastLeft = left;
    lastRight = right;
}

void updateDists(double& l, double& r, double& c) {
    rangeFinder.update();
    l = rangeFinder.getDistance(0) - 2.5;
    r = rangeFinder.getDistance(1) - 2.5;
    c = rangeFinder.getDistance(2) - 6.;

    if (l < 0)
        l = 0;
    if (r < 0)
        r = 0;
    if (c < 0)
        c = 0;
}

void loop() {
    static double leftCm = 0;
    static double rightCm = 0;
    static double centerCm = 0;

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
    handleBluetoothCmds(gMazeWidth, gMazeHeight, gStart, demo_forward, demo_backward, demo_left, demo_right, configSetTurnTime);
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

        updateDists(leftCm, rightCm, centerCm);
        markWalls(leftCm, rightCm, centerCm);
        gNextMove = mazeSolver.getNextMove(motionController.getHeading());
        gInitialized = true;
    }

    updateDists(leftCm, rightCm, centerCm);
    updatePosition(centerCm);

    if (gCarPos == gNextMove) {
        motionController.goBrake(100);
        // go brake does a small delay, update distances again!
        updateDists(leftCm, rightCm, centerCm);

printDists(leftCm, rightCm, centerCm); // debug print

        markWalls(leftCm, rightCm, centerCm);
        gNextMove = mazeSolver.getNextMove(motionController.getHeading());

        Bluetooth.println("nm: " + String(gNextMove.x) + " " + String(gNextMove.y) );
    }

    //updateDists(leftCm, rightCm, centerCm);
    motionController.drive(gCarPos.x, gCarPos.y, gNextMove.x, gNextMove.y, leftCm, rightCm, centerCm);
}

