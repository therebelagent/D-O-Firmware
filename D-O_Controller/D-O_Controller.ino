//
//  D-O Controller firmware for Arduino Nano.
//  Created by Michael Baddeley.
//  Facebook group: https://www.facebook.com/groups/MrBaddeley/
//  Modified by Diego J. Arévalo.
//  D-O Builders group: https://www.facebook.com/groups/2468594199841880/
//  2019 v 1.0.
//

#include <EasyTransfer.h>

//Preset values.
const int MAIN_BAR_MIN_ANGLE = 200;     //Minimun angle/backwards possition in [ticks]
const int MAIN_BAR_MAX_ANGLE = 500;     //Maximun angle/forward possition in [ticks]
const int NOD_BAR_MIN_ANGLE = 240;      //Minimun angle/backwards possition in [ticks]
const int NOD_BAR_MAX_ANGLE = 430;      //Maximun angle/forward possition in [ticks]
const int LEFT_WHEEL_MIN_SIGNAL = 190;  //Minimun backward possition in [ticks]
const int LEFT_WHEEL_MAX_SIGNAL = 500;  //Maximun forward possition in [ticks]
const int RIGHT_WHEEL_MIN_SIGNAL = 500; //Inverted Minimun backward possition in [ticks]
const int RIGHT_WHEEL_MAX_SIGNAL = 190; //Inverted Maximun forward possition in [ticks]

//Pin in/out numbers.
const int PUSH_BUTTON_1_PIN = 2;         // Input Pin for Push Button 1.
const int PUSH_BUTTON_2_PIN = 3;         // Input Pin for Push Button 2.
const int PUSH_BUTTON_3_PIN = 4;         // Input Pin for Push Button 3.
const int MAIN_BAR_SERVO_PIN = A0;       // select the input pin for the potentiometer
const int NOD_BAR_SERVO_PIN = A1;        // select the input pin for the potentiometer
const int JOYSTICK_X_AXIS_PIN = A3;      // select the input pin for the potentiometer
const int JOYSTICK_y_AXIS_PIN = A2;      // select the input pin for the potentiometer
const int JOYSTICK_PUSH_BUTTON_PIN = A4; //select the input for the switch pin on the joystick

struct RX_DATA_STRUCTURE
{
  int RobotStatus;
};

struct TX_DATA_STRUCTURE
{
  int MainBarAngle;     //Main bar
  int NodBarAngle;      //Nod bar
  int LeftWheelSignal;  //Left Wheel
  int RightWheelSignal; //Right Wheel
  int HeadState;        //Head State (0, Front, 1, Animation)
  int PushButtonState1; // LOW/HIGH.
  int PushButtonState2; // LOW/HIGH.
  int PushButtonState3; // LOW/HIGH.
};

RX_DATA_STRUCTURE rxDataStructure;
TX_DATA_STRUCTURE txDataStructure;

EasyTransfer easyTransferRx, easyTransferTX;

//Variables to store final positions / speed
int mainBarAngle = 0;     // variable to store the servo angle
int nodBarAngle = 0;      // variable to store the servo angle
int leftWheelSignal = 0;  // variable to store the Wheel1 speed
int rightWheelSignal = 0; // variable to store the Wheel2 speed
float Steerpos = 0;       //Variable to adjust the steering
float Steeradjust = 0;

//Variables  to store actual controller values
int Headservo1Value = 0; // variable to store the value coming from the pot sensor
int Headservo2Value = 0; // variable to store the value coming from the pot sensor
int JoyXValue = 0;       // variable to store the value coming from the pot sensor
int JoyYValue = 0;       // variable to store the value coming from the pot sensor
int SwitchState = 0;     // The switch pin value
int LastSwitchState = 0; // records the last switch state so we can compare the two.

//Variables for steer logic
//Variables for Ramp up / down (Speed)

int Targetspeed1 = 0;            //variable to store target speed
int Targetspeed2 = 0;            //variable to store target speed
int CurrentWheel1 = 344;         // variable to store current speed for wheel 1
int CurrentWheel2 = 344;         // variable to store current speed for wheel 2
int SpeedChange = 5;             // variable to store speed step change (lower, more lag / smoother)
int DrivePeriod = 50;            //increase speed every x milliseconds, (higher, more lag / smoother)
unsigned long Drivetime_now = 0; // Drivetime current,
int DirectionState = 0;          // 0 for Stationary, 1 for forward, 2 for backwards
int SteerState = 0;              // 0 for Stationary, 1 for left, 2 for right

void setup()
{
  Serial.begin(38400);
  easyTransferRx.begin(details(rxDataStructure), &Serial);
  easyTransferTX.begin(details(txDataStructure), &Serial);
  pinMode(PUSH_BUTTON_1_PIN, INPUT_PULLUP);
  pinMode(PUSH_BUTTON_2_PIN, INPUT_PULLUP);
  pinMode(PUSH_BUTTON_3_PIN, INPUT_PULLUP);
}

void loop()
{
  //Reading Push Buttons
  txDataStructure.PushButtonState1 = !digitalRead(PUSH_BUTTON_1_PIN);
  txDataStructure.PushButtonState2 = !digitalRead(PUSH_BUTTON_2_PIN);
  txDataStructure.PushButtonState3 = !digitalRead(PUSH_BUTTON_3_PIN);

  //Read Potentiometer for the two head pots
  Headservo1Value = analogRead(MAIN_BAR_SERVO_PIN);
  Headservo2Value = analogRead(NOD_BAR_SERVO_PIN);
  JoyXValue = analogRead(JOYSTICK_X_AXIS_PIN);
  JoyYValue = analogRead(JOYSTICK_y_AXIS_PIN);
  SwitchState = analogRead(JOYSTICK_PUSH_BUTTON_PIN);

  //Map Potentiometer values to Servo values
  mainBarAngle = map(Headservo1Value, 0, 1023, MAIN_BAR_MIN_ANGLE, MAIN_BAR_MAX_ANGLE);
  nodBarAngle = map(Headservo2Value, 0, 1023, NOD_BAR_MIN_ANGLE, NOD_BAR_MAX_ANGLE);
  leftWheelSignal = map(JoyYValue, 1023, 0, LEFT_WHEEL_MIN_SIGNAL, LEFT_WHEEL_MAX_SIGNAL);
  rightWheelSignal = map(JoyYValue, 1023, 0, RIGHT_WHEEL_MIN_SIGNAL, RIGHT_WHEEL_MAX_SIGNAL);
  Steerpos = map(JoyXValue, 0, 1023, -146, 155);

  //Create Steeradjust/SteerState value
  if (Steerpos > 3)
  {
    SteerState = 1;
  }
  else if (Steerpos < -3)
  {
    SteerState = 2;
  }
  else if (Steerpos > -3 && Steerpos < 3)
  {
    SteerState = 0;
  }
  Steeradjust = Steerpos / 155;
  if (Steeradjust < 0)
  {
    Steeradjust = -Steeradjust;
  }
  if ((LastSwitchState - SwitchState) > 200)
  {
    if (txDataStructure.HeadState == 0)
    {
      txDataStructure.HeadState = 1;
    }
    else if (txDataStructure.HeadState == 1)
    {
      txDataStructure.HeadState = 0;
    }
  }

  //Set Variable for EasyTransfer
  txDataStructure.MainBarAngle = mainBarAngle;
  txDataStructure.NodBarAngle = nodBarAngle;

  //Ramp up / Ramp down
  Targetspeed1 = leftWheelSignal;
  Targetspeed2 = rightWheelSignal;
  if (millis() > Drivetime_now + DrivePeriod)
  {
    Drivetime_now = millis();
    if (Targetspeed1 > CurrentWheel1)
    {
      CurrentWheel1 = CurrentWheel1 + SpeedChange;
    }
    else if (Targetspeed1 < CurrentWheel1)
    {
      CurrentWheel1 = CurrentWheel1 - SpeedChange;
    }
    if (Targetspeed2 > CurrentWheel2)
    {
      CurrentWheel2 = CurrentWheel2 + SpeedChange;
    }
    else if (Targetspeed2 < CurrentWheel2)
    {
      CurrentWheel2 = CurrentWheel2 - SpeedChange;
    }
  }
  txDataStructure.LeftWheelSignal = CurrentWheel1;
  txDataStructure.RightWheelSignal = CurrentWheel2;

  //Kill wheel movement if centred (stops creep).
  if (CurrentWheel1 > 332 && CurrentWheel1 < 350)
  {
    txDataStructure.LeftWheelSignal = 4096;
  }
  if (CurrentWheel2 > 332 && CurrentWheel2 < 350)
  {
    txDataStructure.RightWheelSignal = 4096;
  }

  //Check directionstate and move head ready for movement.
  if (CurrentWheel1 > 350 && CurrentWheel2 < 325)
  {
    DirectionState = 1;
    //txDataStructure.MainBarAngle = 275;
  }
  else if (CurrentWheel1 < 325 && CurrentWheel2 > 350)
  {
    DirectionState = 2;
    //txDataStructure.MainBarAngle = 390;
  }
  else if (CurrentWheel1 > 325 && CurrentWheel1 < 350 && CurrentWheel2 > 325 && CurrentWheel2 < 350)
  {
    DirectionState = 0;
  }

  //Steering modifier
  if (CurrentWheel1 > 325 && CurrentWheel1 < 350 && CurrentWheel2 > 325 && CurrentWheel2 < 350 && (Steerpos > 20 || Steerpos < 0))
  {
    txDataStructure.LeftWheelSignal = CurrentWheel1 + (-Steerpos / 3);
    txDataStructure.RightWheelSignal = CurrentWheel2 + (-Steerpos / 3);
  }

  //Moving steering
  if (DirectionState == 1)
  {
    txDataStructure.LeftWheelSignal = CurrentWheel1 + (-Steerpos / 3);
    txDataStructure.RightWheelSignal = CurrentWheel2 + (-Steerpos / 3);
  }
  if (DirectionState == 2)
  {
    txDataStructure.LeftWheelSignal = CurrentWheel1 + (-Steerpos / 3);
    txDataStructure.RightWheelSignal = CurrentWheel2 + (-Steerpos / 3);
  }

  easyTransferTX.sendData();
  for (int i = 0; i < 4; i++)
  {
    easyTransferRx.receiveData();
    delay(10);
  }
  LastSwitchState = SwitchState;
}