//
//  D-O Receiver firmware for Arduino Uno.
//  Created by Michael Baddeley.
//  Facebook group: https://www.facebook.com/groups/MrBaddeley/
//  Modified by Diego J. Arévalo.
//  D-O Builders group: https://www.facebook.com/groups/2468594199841880/
//  2019 v 1.0.
//

#include <EasyTransfer.h>
#include <Adafruit_PWMServoDriver.h>
#include <Wire.h>
#include <ServoEasing.h>
#include <DFPlayerMini.h>
#include "SoundFxManager.h"

//SoundFxManager constants.
const int STORED_SOUNDS = 6;
const int DFPLAYER_DEFAULT_VOLUME = 100; //1-100

//Pin in/out numbers.
const int DFPLAYER_BUSY_PIN = 2;
const int DFPLAYER_RX_PIN = 3;
const int DFPLAYER_TX_PIN = 4;
const int HEAD_PAN_SERVO_PIN = 9;
const int HEAD_BEND_SERVO_PIN = 10;

//Head animation timers & angles.
const int HEAD_PAN_SERVO_MIN_ANGLE = 20;   //0-180
const int HEAD_PAN_SERVO_MAX_ANGLE = 160;  //0-180
const int HEAD_PAN_SERVO_MIN_TIME = 1000;  //[ms]
const int HEAD_PAN_SERVO_MAX_TIME = 2000;  //[ms]
const int HEAD_BEND_SERVO_MIN_ANGLE = 70;  //0-180
const int HEAD_BEND_SERVO_MAX_ANGLE = 110; //0-180
const int HEAD_BEND_SERVO_MIN_TIME = 1000; //[ms]
const int HEAD_BEND_SERVO_MAX_TIME = 1500; //[ms]

//Stop head animation timers & angles.
const int HEAD_PAN_SERVO_STOP_ANGLE = 90;  //0-180
const int HEAD_PAN_SERVO_STOP_TIME = 500;  //[ms]
const int HEAD_BEND_SERVO_STOP_ANGLE = 90; //0-180
const int HEAD_BEND_SERVO_STOP_TIME = 500; //[ms]

//EasyTransfer structure definitions.
struct RX_DATA_STRUCTURE
{
  int MainBarAngle;     //Main bar
  int NodBarAngle;      //Nod bar
  int LeftWheelSignal;  //Left Wheel
  int RightWheelSignal; //Right Wheel
  int HeadState;        //HeadState (0, Front, 1, Animation)
  int PushButtonState1; // LOW/HIGH.
  int PushButtonState2; // LOW/HIGH.
  int PushButtonState3; // LOW/HIGH.
};

struct TX_DATA_STRUCTURE
{
  int RobotStatus;
};

RX_DATA_STRUCTURE rxDataStructure;
TX_DATA_STRUCTURE txDataStructure;

EasyTransfer easyTransferRx, easyTransferTX;

DFPlayerMini dFPlayerMini;
SoundFxManager soundFxManager;

Adafruit_PWMServoDriver adafruitPWMServoDriver = Adafruit_PWMServoDriver();
ServoEasing headPanServoEasing;
ServoEasing headBendServoEasing;

int currentHeadAnimationState;
int previousHeadAnimationState;

void setup()
{
  //Initializing bluetooth components.
  Serial.begin(38400);
  easyTransferRx.begin(details(rxDataStructure), &Serial);
  easyTransferTX.begin(details(txDataStructure), &Serial);
  //Initializing sound Fx player device.
  dFPlayerMini.init(DFPLAYER_BUSY_PIN, DFPLAYER_RX_PIN, DFPLAYER_TX_PIN);
  soundFxManager.initialize(&dFPlayerMini, STORED_SOUNDS, DFPLAYER_DEFAULT_VOLUME);
  //Initializing servos.
  adafruitPWMServoDriver.begin();
  adafruitPWMServoDriver.setPWMFreq(60);
  delay(10);
  headPanServoEasing.attach(HEAD_PAN_SERVO_PIN);
  headBendServoEasing.attach(HEAD_BEND_SERVO_PIN);
  //Initializing startup variables.
  rxDataStructure.HeadState = 1;
  currentHeadAnimationState = rxDataStructure.HeadState;
  previousHeadAnimationState = currentHeadAnimationState;
  headPanServoEasing.write(90);
  headBendServoEasing.write(90);
  soundFxManager.playPowerUpSound();
  delay(2000);
}

void loop()
{
  easyTransferTX.sendData();
  for (int i = 0; i < 5; i++)
  {
    easyTransferRx.receiveData();
  }

  currentHeadAnimationState = rxDataStructure.HeadState;
  if (previousHeadAnimationState != currentHeadAnimationState)
  {
    previousHeadAnimationState = rxDataStructure.HeadState;
    soundFxManager.playRandomSound();
  }

  if (currentHeadAnimationState == 1)
  {
    int angle, time;
    if (!headPanServoEasing.isMoving())
    {
      angle = random(HEAD_PAN_SERVO_MIN_ANGLE, HEAD_PAN_SERVO_MAX_ANGLE);
      time = random(HEAD_PAN_SERVO_MIN_TIME, HEAD_PAN_SERVO_MAX_TIME);
      headPanServoEasing.startEaseToD(angle, time);
    }
    if (!headBendServoEasing.isMoving())
    {
      angle = random(HEAD_BEND_SERVO_MIN_ANGLE, HEAD_BEND_SERVO_MAX_ANGLE);
      time = random(HEAD_BEND_SERVO_MIN_TIME, HEAD_BEND_SERVO_MAX_TIME);
      headBendServoEasing.startEaseToD(angle, time);
    }
  }
  if (currentHeadAnimationState == 0)
  {
    if (!headPanServoEasing.isMoving())
    {
      headPanServoEasing.startEaseToD(HEAD_PAN_SERVO_STOP_ANGLE, HEAD_PAN_SERVO_STOP_TIME);
    }
    if (!headBendServoEasing.isMoving())
    {
      headBendServoEasing.startEaseToD(HEAD_BEND_SERVO_STOP_ANGLE, HEAD_BEND_SERVO_STOP_TIME);
    }
  }

  //Reading Controller Push Buttons states.
  if (rxDataStructure.PushButtonState1 == HIGH)
  {
    soundFxManager.playRandomSound();
    headPanServoEasing.startEaseToD(0, 2000);
  }
  if (rxDataStructure.PushButtonState2 == HIGH)
  {
    soundFxManager.playRandomSound();
    headPanServoEasing.startEaseToD(90, 1000);
  }
  if (rxDataStructure.PushButtonState3 == HIGH)
  {
    soundFxManager.playRandomSound();
    headPanServoEasing.startEaseToD(180, 2000);
  }
  delay(10);
  adafruitPWMServoDriver.setPWM(0, 0, rxDataStructure.LeftWheelSignal);
  adafruitPWMServoDriver.setPWM(1, 0, rxDataStructure.RightWheelSignal);
  adafruitPWMServoDriver.setPWM(2, 0, rxDataStructure.MainBarAngle);
  adafruitPWMServoDriver.setPWM(3, 0, rxDataStructure.NodBarAngle);
}