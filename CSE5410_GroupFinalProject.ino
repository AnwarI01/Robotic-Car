//////////////////////////////////////////////
//        RemoteXY include library          //
//////////////////////////////////////////////

/*
   -- New project --
   
   This source code of graphical user interface 
   has been generated automatically by RemoteXY editor.
   To compile this code using RemoteXY library 3.1.8 or later version 
   download by link http://remotexy.com/en/library/
   To connect using RemoteXY mobile app by link http://remotexy.com/en/download/                   
     - for ANDROID 4.11.1 or later version;
     - for iOS 1.9.1 or later version;
    
   This source code is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.    
*/

//////////////////////////////////////////////
//        RemoteXY include library          //
//////////////////////////////////////////////

// RemoteXY select connection mode and include library 
#define REMOTEXY_MODE__ESP8266_HARDSERIAL_POINT

#include <RemoteXY.h>
#include <NewPing.h>

// RemoteXY connection settings 
#define REMOTEXY_SERIAL Serial
#define REMOTEXY_SERIAL_SPEED 115200
#define REMOTEXY_WIFI_SSID "RemoteXY"
#define REMOTEXY_WIFI_PASSWORD "12345678"
#define REMOTEXY_SERVER_PORT 6377


// RemoteXY configurate  
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] =   // 195 bytes
  { 255,7,0,0,0,188,0,16,30,0,1,0,22,17,12,12,2,31,0,1,
  0,12,28,12,12,2,31,0,1,0,22,39,12,12,2,31,0,1,0,32,
  28,12,12,2,31,0,129,0,21,3,54,7,6,65,114,100,117,105,110,111,
  32,82,101,109,111,116,101,0,1,0,68,16,12,12,1,31,0,1,0,81,
  48,12,12,95,31,0,1,0,65,38,12,12,12,31,0,129,0,82,45,11,
  2,6,83,112,101,101,100,68,111,119,110,0,129,0,67,34,8,2,6,83,
  112,101,101,100,85,112,0,129,0,71,12,4,2,6,83,116,111,112,0,129,
  0,22,13,11,3,6,70,111,114,119,97,114,100,0,129,0,21,53,13,3,
  6,66,97,99,107,119,97,114,100,0,129,0,47,31,7,3,6,82,105,103,
  104,116,0,129,0,4,31,5,3,6,76,101,102,116,0 };
  
// this structure defines all the variables and events of your control interface 
struct {

    // input variables
  uint8_t forward; // =1 if button pressed, else =0 
  uint8_t left; // =1 if button pressed, else =0 
  uint8_t backward; // =1 if button pressed, else =0 
  uint8_t right; // =1 if button pressed, else =0 
  uint8_t Stop; // =1 if button pressed, else =0 
  uint8_t speedDown; // =1 if button pressed, else =0 
  uint8_t speedUp; // =1 if button pressed, else =0 

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0 

} RemoteXY;
#pragma pack(pop)

/////////////////////////////////////////////
//           END RemoteXY include          //
/////////////////////////////////////////////

// DC Motor Shiled
#define Dir_A_Pin 8
#define PWM_A_Pin 9
#define Dir_B_Pin 4
#define PWM_B_Pin 11
const bool forward = HIGH;
const bool backward = LOW;
float speedA = 200;
float speedB = 200 ;
float speedBL = 0.60; // to change speed B TO 60% of speed A
float speedAR = 0.60; // to change speed A TO 60% of speed B
int speedCounter = 0;
int speedTestUp = 0;
int speedTestDown = 175;

int dir = 0;

// Two Sonar sensors
#define BTRIGGER_PIN 10 // Arduino pin tied to trigger pin, Butttom sensor
#define B_ECHO_PIN 5// Echo pin for Bottom Ultrasonic sensor
#define MAX_DISTANCE 400//max distance 9in centimeters

const int interrupt_pin =2;


//NewPing F_sonar(FTRIGGER_PIN, F_ECHO_PIN, MAX_DISTANCE);
NewPing B_sonar(BTRIGGER_PIN, B_ECHO_PIN, MAX_DISTANCE);

void setup() 
{
  RemoteXY_Init (); 
  
   //Setup Channel A
  pinMode(Dir_A_Pin, OUTPUT); //Initiates Motor Channel A pin
  //Setup Channel B
  pinMode(Dir_B_Pin, OUTPUT); //Initiates Motor Channel B pin
  
  pinMode(interrupt_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interrupt_pin), halt, FALLING);
  Serial.begin(115200);//initialize serial 
  
}

void loop() 
{ 
  RemoteXY_Handler ();

  unsigned int B_uS = B_sonar.ping(); // Send ping, get ping time in microseconds (uS).
  int B_distance = B_uS / US_ROUNDTRIP_CM;

  // step/fall detection 
  if (B_distance > 50)
  {
    stop_delay();
    //stopCar();
  }
  //crash detection
  if ((F_distance < 10) && (F_distance > 0))
  {
    stop_delay();
  }
  
  if(RemoteXY.forward == 1) //RemoteXY.joystick_1_x > 0 &
  {
      goStraight(forward, speedA);
      dir = 1;
  }
  if(RemoteXY.backward == 1) // RemoteXY.joystick_1_x < 0 &
  {
      goBackward(backward, speedA);
      dir = 2;
  }
  if(RemoteXY.right == 1)
  {
      turnR(forward, speedA, speedB);
      dir = 3;
  }
  if(RemoteXY.left == 1)
  {
      turnL(forward, speedA, speedB);
      dir = 4;
  }
  if(RemoteXY.Stop == 1)
  {
    stopCar();
    dir = 0;
  }
  if(RemoteXY.speedUp == 1)
  {
    speedUp();
    
    //RemoteXY.speedUp = 0;
  }
  if(RemoteXY.speedDown == 1)
  {
    speedDown();
    //RemoteXY.speedDown = 0;
  }
  
}

//interrupt
void halt()
{
  while(digitalRead(interrupt_pin) == LOW)
  {
    stop_delay();
  }
}

void motionA(bool directionF, float speed1)
{
  digitalWrite(Dir_A_Pin, directionF);
  analogWrite(PWM_A_Pin, speed1);  
}
void motionB(bool directionB, float speed2)
{
  digitalWrite(Dir_B_Pin, directionB);
  analogWrite(PWM_B_Pin, speed2);  
}
void goStraight(bool directionS, float speedS)
{
  motionA(directionS, speedS);
  motionB(directionS, speedS); 
}
void goBackward(bool directionB, float speedB)
{
  motionB(directionB, speedB);
  motionA(directionB, speedB);
}
void turnL(bool directionL, float speedA, float speedB)
{
  motionA(directionL, speedA);
  motionB(directionL, (speedB*speedBL)); 
}
void turnR(bool directionR, float speedA, float speedB)
{
  motionA(directionR, (speedA*speedAR));
  motionB(directionR, speedB); 
}
void stopCar()//Stops the car by setting the speed to zero using PWM for both motors
{
  analogWrite(PWM_A_Pin, 0);
  analogWrite(PWM_B_Pin, 0);
}
void stop_delay()
{
   // Serial.print("STOPED!!");
    stopCar();
    delay(1000);//delay one second
    
    goBackward(backward, speedA);
    delay(2000);
    
    stopCar();
    
}

void speedUp()
{
  if(speedA < 255)
  {
    speedA = speedA + 5;
    speedB = speedB + 5;
  }
  else
  {
     speedA = 255;
     speedB = 255;
  }
  switch(dir)
  {
    case 0:
      RemoteXY.Stop = 1;
      break;
    case 1:
      RemoteXY.forward = 1;
      break;
    case 2:
      RemoteXY.backward = 1;
      break;
    case 3:
      RemoteXY.right = 1;
      break;
    case 4:
      RemoteXY.left = 1;
      break;
    default: RemoteXY.Stop = 1;
  }
  RemoteXY.speedUp = 0;
}
void speedDown()
{
  if(speedA >= 0)
  {
    speedA = speedA - 5;
    speedB = speedB - 5;
  }
  else
  {
     speedA = 0;
     speedB = 0;
  }
  switch(dir)
  {
    case 0:
      RemoteXY.Stop = 1;
      break;
    case 1:
      RemoteXY.forward = 1;
      break;
    case 2:
      RemoteXY.backward = 1;
      break;
    case 3:
      RemoteXY.right = 1;
      break;
    case 4:
      RemoteXY.left = 1;
      break;
    default: RemoteXY.Stop = 1;
  }
  RemoteXY.speedDown = 0;
}
