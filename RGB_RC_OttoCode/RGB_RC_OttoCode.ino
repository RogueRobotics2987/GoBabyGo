// Otto's wheelchair code // in progress

#include <Adafruit_DotStar.h>
#include <Servo.h>

enum controlMode {childControl, parentControl}; 
controlMode driveMode = childControl;
boolean SPEED_POTENTIOMETER = true;

// Invert one or two of the motors
boolean INVERT_1 = true;
boolean INVERT_2 = false;
//boolean INVERT_2 = false;
//boolean INVERT_1 = true;

// Constants
int SPEED_LIMIT = 512; // Between 0-512, recomended 256 for talon SR's and wild thing motors 

Servo servo; 

int DEADBAND = 40; //150  // changed from 60
int DEADBAND_RC = 60;
int RAMPING = 1;
int REVERSE_PULSE    = 1000; // Talon SR is 1000
int FORWARD_PULSE    = 2000; // Talon SR is 2000

// Pins
// check pins
int JOYSTICK_X = A0;
int JOYSTICK_Y = A1;
int MOTOR_1    = 10;
int MOTOR_2    = 11;     // SWITCHED FROM 3 TO 11
int SPEED_POT  = A3;
int turnPin = 4;  // ch1
int throttlePin = 5;  // ch2
int togglePin = 3;  // ch3

int invertStickX = 1; 
int invertStickY = 1;// comment

// #define NUMPIXELS 228// Number of LEDs in strip
#define ACTNUMPIXELS 144 // actual number of pixels for initializing strip
#define NUMPIXELS 100// Number of LEDs in strip
#define DATAPIN    9
#define CLOCKPIN   8
Adafruit_DotStar strip(ACTNUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BGR);

int led1 = NUMPIXELS / 2 -1; //first half of strip
int led2 = NUMPIXELS / 2; // second half of strip
int GROUPWIDTH = 6;
//double PULSEBRIGHTNESS = //HOW MUCH;

// Debug Over Serial - Requires a FTDI cable
boolean DEBUG = true;

// -----Don't Mess With Anything Past This Line-----

Servo motor1;
Servo motor2;

void lights_green ()
{
    debug("green", 1);
    strip.fill(strip.Color(0, 50, 0));
    strip.show();
  }

  void lights_red()
  {
    debug("red", 1);
    strip.fill(strip.Color(50, 0, 0));
    strip.show();
  }

  void lights_yellow()
  {
    debug("yellow", 1);
    strip.fill(strip.Color(50, 50, 0));
    strip.show();
  }

  void patternChase()
  {
    // second half of strip
    led2 = led2 + GROUPWIDTH;

    for(int i = led2 - GROUPWIDTH; i < led2; i++)
    {
      strip.setPixelColor(i, 30, 0, 0);
    }
    // first half of strip
    led1 = led1 - GROUPWIDTH;

    for(int i = led1 + GROUPWIDTH; i > led1; i--)
    {
      strip.setPixelColor(i, 30, 0, 0);
    }
    
    strip.show();
    
    if(led2 >= NUMPIXELS)
    {
      led2 = NUMPIXELS /2;
      strip.clear();
    }

    if(led1 <= 0)
    {
      led1 = NUMPIXELS /2 - 1;
      strip.clear();
    }
  }

//  void lightPulse()
//  {
//    strip.fill(strip.Color(50, 0, 0));
//
//    for(double )
//
//    //take inthe pulse brightness and subtract 1 each time till it gets to 0 the add till 50 loop
//  }

void setup() {
  while (!Serial);  // required for Flora & Micro
  delay(500);
  Serial.begin(115200);
  pinMode(JOYSTICK_X, INPUT);
  pinMode(JOYSTICK_Y, INPUT);
  pinMode(turnPin, INPUT);
  pinMode(throttlePin, INPUT);
  pinMode(togglePin, INPUT);
  motor1.attach(MOTOR_1);
  motor2.attach(MOTOR_2);
  strip.begin();
  if(SPEED_POTENTIOMETER){ pinMode(SPEED_POT, INPUT);}
  
//  if(DEBUG) Serial.begin(9600);
  Serial.println( F("OK!") );
}

void loop() {
  // int pulse_turn = pulseIn(turnPin, HIGH, 30000);
  // int pulse_throttle = pulseIn(throttlePin, HIGH, 30000);
  int pulse_toggle = pulseIn(togglePin, HIGH, 30000);

  // push values to serial monitor
  debug("Toggle", pulse_toggle);
  debug("SpeedPot_Value", analogRead(SPEED_POT));

   // chooses driveMode
   if(pulse_toggle < 1500){
    driveMode = childControl;
   }
   else{
    driveMode = parentControl;
   }
   
  if(driveMode == childControl){
    
    debug("Control Mode", driveMode);
    
    //Read from the joystick
    int x = invertStickX * analogRead(JOYSTICK_X);  
    int y = invertStickY * analogRead(JOYSTICK_Y);
    debug("RawX", x);
    debug("RawY", y);

    //Zero values within deadband
    if(abs(512-x)<DEADBAND) x = 512;
    if(abs(512-y)<DEADBAND) y = 512;

    //Map values outside deadband to inside deadband
    if(x>512) x = map(x, 512+DEADBAND, 1023, 512, 1023);
    else if (x<512) x = map(x, 0, 512-DEADBAND, 0, 512);
    if(y>512) y = map(y, 512+DEADBAND, 1023, 512, 1023);
    else if(y<512) y = map(y, 0, 512-DEADBAND, 0, 512);

    //Establish a speed limit
    int limit = SPEED_LIMIT;
    if(SPEED_POTENTIOMETER) limit = map(analogRead(SPEED_POT), 0, 1023, 0, SPEED_LIMIT);
    debug("LIMIT", limit);

    //Map speeds to within speed limit
    x = map(x, 0, 1023, 512-limit, 512+limit);
    y = map(y, 0, 1023, 512-limit, 512+limit);
    debug("Xout", x);
    debug("Yout", y);
  
    int moveValue = 0;
    moveValue = y - 512;
    
    int rotateValue = 0;
    rotateValue = x - 512;

    arcadeDrive(moveValue, rotateValue);
  
    debugF();
   
  }
  else if( driveMode == parentControl){
   
    debug("Control Mode", driveMode);
    
    //Read from the RC
    int x = pulseIn(turnPin, HIGH, 30000);  
    int y = pulseIn(throttlePin, HIGH, 30000);
    debug("RawX", x);
    debug("RawY", y);

    //Map values outside deadband to inside deadband
    if(x>1500+DEADBAND_RC) x = map(x, 1500+DEADBAND_RC, 2000, 512, 1023);
    else if (x<1500-DEADBAND_RC) { x = map(x, 950, 1500-DEADBAND_RC, 0, 512); }
    else {x = 512;}
    if(y>1500+DEADBAND_RC) y = map(y, 1500+DEADBAND_RC, 2000, 512, 1023);
    else if(y<1500-DEADBAND_RC) { y = map(y, 950, 1500-DEADBAND_RC, 0, 512); }
    else {y = 512;}

    //Establish a speed limit
    int limit = SPEED_LIMIT;
    if(SPEED_POTENTIOMETER) limit = map(analogRead(SPEED_POT), 0, 1023, 0, SPEED_LIMIT);
    debug("LIMIT", limit);

    //Map speeds to within speed limit
    x = map(x, 0, 1023, 512-limit, 512+limit);
    y = map(y, 0, 1023, 512-limit, 512+limit);
    debug("Xout", x);
    debug("Yout", y);
  
    int moveValue = 0;
    moveValue = y - 512;
    
    int rotateValue = 0;
    rotateValue = x - 512;

    arcadeDrive(moveValue, rotateValue);
  
    debugF();
  
  }

  delay(20); //Make loop run approximately 50hz

}

void arcadeDrive(int moveValue, int rotateValue) {
  int leftMotorSpeed = 0;
  int rightMotorSpeed = 0;
  
  if (moveValue > 0.0) {
      if (rotateValue > 0.0) {
        leftMotorSpeed = moveValue - rotateValue;
        rightMotorSpeed = max(moveValue, rotateValue);
      } else {
        leftMotorSpeed = max(moveValue, -rotateValue);
        rightMotorSpeed = moveValue + rotateValue;
      }
    } else {
      if (rotateValue > 0.0) {
        leftMotorSpeed = -max(-moveValue, rotateValue);
        rightMotorSpeed = moveValue + rotateValue;
      } else {
        leftMotorSpeed = moveValue - rotateValue;
        rightMotorSpeed = -max(-moveValue, -rotateValue);
      }
    }
    drive(map(leftMotorSpeed, -512, 512, 0, 1023), map(rightMotorSpeed, -512, 512, 0, 1023));
}

int prevLeft = 500;
int prevRight = 500;

void drive(int left, int right){
  int speed1 = map(left, 0, 1023, 0, FORWARD_PULSE-REVERSE_PULSE);
  if(speed1>prevLeft+RAMPING) speed1=speed1+RAMPING;
  else if(speed1<prevLeft-RAMPING) speed1=speed1-RAMPING;
  if(INVERT_1) motor1.write(FORWARD_PULSE-speed1);
  else motor1.write(REVERSE_PULSE+speed1);
  prevLeft = speed1;
  
  int speed2 = map(right, 0, 1023, 0, FORWARD_PULSE-REVERSE_PULSE);
  if(speed2>prevLeft+RAMPING) speed2=speed2+RAMPING;
  else if(speed2<prevLeft-RAMPING) speed2=speed2-RAMPING;
  if(INVERT_2) motor2.write(FORWARD_PULSE-speed2);
  else motor2.write(REVERSE_PULSE+speed2);
  prevRight = speed2;
  
  /*if(speed1 + speed2 < 0)
  {
    lights_yellow();
  }
  else if (speed1 + speed2 > 0)
  {
    lights_green();
  }
  else
  {
    lights_red();
  }*/

  if(speed1 != 0 || speed2 != 0 )
  {
    patternChase();
    //lights_red();
  }else
  {
    lights_red();
  }

  //lights_red();
}

void debug(String s, int value){
  // form of values in the serial monitor comment 
  if(DEBUG){
    Serial.print(" ");
    Serial.print(s);
    Serial.print(": ");
    Serial.print(value);
  }
}
void debugF(){
  if(DEBUG){
    Serial.println("");
  }
}
