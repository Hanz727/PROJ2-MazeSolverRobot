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


//all for centering

const float Kp_pos = 4.5;
const float Kd_pos = 2.5;
const float Kp_yaw = 3.8;
const float Kd_yaw = 2.2;

// --- SPEED AND TIMING CONFIG ---
const int baseSpeed = 255;
const int controlInterval = 50;

// --- DEADZONE ---
float centerDeadband = 1.0;

// --- PID STATE ---
float prev_error_pos = 0;
float prev_error_yaw = 0;
float prev_dLeft = 0;
float prev_dRight = 0;
unsigned long lastTime = 0;

//

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

    delay(450*4);
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

  // === PID GAINS ===


  if(rangeFinder.getDistance(2) > 20){

    unsigned long currentTime = millis();
    if (currentTime - lastTime >= controlInterval) {
      lastTime = currentTime;

      // === GET DISTANCES FROM ULTRASONIC SENSORS ===
      float dLeft = rangeFinder.getDistance(0);
      float dRight = rangeFinder.getDistance(1);
      rangeFinder.update();

      bool trustLeft = (dLeft <= 12.0);
      bool trustRight = (dRight <= 12.0);

      float correction_pos = 0.0;
      float correction_yaw = 0.0;
      float pos_error = 0.0;
      float yaw_error = 0.0;

      if (trustLeft || trustRight) {
        // === POSITION ERROR ===
        pos_error = (dLeft - dRight) / 2.0;

        if (abs(pos_error) < centerDeadband) pos_error = 0;

        float derivative_pos = (pos_error - prev_error_pos) / (controlInterval / 1000.0);
        correction_pos = Kp_pos * pos_error + Kd_pos * derivative_pos;
        correction_pos = constrain(correction_pos, -55.0, 55.0);
        prev_error_pos = pos_error;

        // === YAW ERROR ===
        float delta_dLeft = dLeft - prev_dLeft;
        float delta_dRight = dRight - prev_dRight;
        yaw_error = delta_dLeft - delta_dRight;

        if (abs(yaw_error) < 0.5) yaw_error = 0;

        float derivative_yaw = (yaw_error - prev_error_yaw) / (controlInterval / 1000.0);
        correction_yaw = Kp_yaw * yaw_error + Kd_yaw * derivative_yaw;
        correction_yaw = constrain(correction_yaw, -55.0, 55.0);
        prev_error_yaw = yaw_error;

        prev_dLeft = dLeft;
        prev_dRight = dRight;
      } else {
        // No trustable wall → reset previous errors
        prev_error_pos = 0;
        prev_error_yaw = 0;
      }

      // === DYNAMIC SPEED ADJUSTMENT ===
      float correctionMagnitude = abs(correction_pos) + abs(correction_yaw);
      int adjustedBaseSpeed = baseSpeed - int(correctionMagnitude * 0.7);
      adjustedBaseSpeed = constrain(adjustedBaseSpeed, 160, baseSpeed);

      int speedLeft = constrain(adjustedBaseSpeed - correction_pos - correction_yaw, 0, 255);
      int speedRight = constrain(adjustedBaseSpeed + correction_pos + correction_yaw, 0, 255);

      // === APPLY MOTOR COMMANDS ===
      motorFrontLeft.run(FORWARD);
      motorBackLeft.run(FORWARD);
      motorFrontRight.run(FORWARD);
      motorBackRight.run(FORWARD);

      motorFrontLeft.setSpeed(speedLeft);
      motorBackLeft.setSpeed(speedLeft);
      motorFrontRight.setSpeed(speedRight);
      motorBackRight.setSpeed(speedRight);

      // === DEBUG LOGGING ===
      Serial2.print("L: "); Serial2.print(dLeft);
      Serial2.print(" R: "); Serial2.print(dRight);
      Serial2.print(" | PosErr: "); Serial2.print(pos_error);
      Serial2.print(" YawErr: "); Serial2.print(yaw_error);
      Serial2.print(" | CorrMag: "); Serial2.print(correctionMagnitude);
      Serial2.print(" | AdjSpd: "); Serial2.print(adjustedBaseSpeed);
      Serial2.print(" | L_Spd: "); Serial2.print(speedLeft);
      Serial2.print(" R_Spd: "); Serial2.println(speedRight);
    }
  }
  else if(rangeFinder.getDistance(0) > 12){
    motorFrontLeft.run(BACKWARD);
    motorBackLeft.run(BACKWARD);
    motorFrontRight.run(FORWARD);
    motorBackRight.run(FORWARD);
    
    motorFrontLeft.setSpeed(150);
    motorFrontRight.setSpeed(150);
    motorBackLeft.setSpeed(150); 
    motorBackRight.setSpeed(150);

    delay(450);
    motorFrontLeft.run(RELEASE);
    motorBackLeft.run(RELEASE);
    motorFrontRight.run(RELEASE);
    motorBackRight.run(RELEASE);
    rangeFinder.update();
  }
  else if(rangeFinder.getDistance(1) > 12){
    motorFrontLeft.run(FORWARD);
    motorBackLeft.run(FORWARD);
    motorFrontRight.run(BACKWARD);
    motorBackRight.run(BACKWARD);
    
    motorFrontLeft.setSpeed(150);
    motorFrontRight.setSpeed(150);
    motorBackLeft.setSpeed(150); 
    motorBackRight.setSpeed(150);

    delay(450);
    motorFrontLeft.run(RELEASE);
    motorBackLeft.run(RELEASE);
    motorFrontRight.run(RELEASE);
    motorBackRight.run(RELEASE);
    rangeFinder.update();
  }
  else{
    motorFrontLeft.run(RELEASE);
    motorBackLeft.run(RELEASE);
    motorFrontRight.run(RELEASE);
    motorBackRight.run(RELEASE);
    rangeFinder.update();
  }






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
