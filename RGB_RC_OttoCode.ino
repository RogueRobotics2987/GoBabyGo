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

#define NUM_PIXELS 144// Number of LEDs in strip
#define DATA_PIN    9
#define CLOCK_PIN   8
Adafruit_DotStar strip(NUM_PIXELS, DATA_PIN, CLOCK_PIN, DOTSTAR_BGR);

const int LED_CHAIN_COUNT = 12; // How many LEDs to include
const int LED_BRIGHTNESS = 64; // brightness for the LEDs (0-255)
const uint32_t STATIONARY_COLOR = strip.Color(50, 0, 0);  // RED
const uint32_t FORWARD_COLOR = strip.Color(0, 50, 0);     // GREEN
const uint32_t REVERSE_COLOR = strip.Color(50, 50, 0);    // YELLOW
int ledIndex = 0;
int reverseLEDIndex = NUM_PIXELS - 1;
int ledCount = 1;

// Debug Over Serial - Requires a FTDI cable
boolean DEBUG = true;

// -----Don't Mess With Anything Past This Line-----

Servo motor1;
Servo motor2;

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
  strip.setBrightness(LED_BRIGHTNESS);
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
    delay(60); //Make loop run approximately 50hz
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
    delay(60); //Make loop run approximately 50hz
  
  }
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
  
  if(speed1 + speed2 < 0) {
    lightLEDs(REVERSE_COLOR);
  } 
  else if (speed1 + speed2 > 0) {
    lightLEDs(FORWARD_COLOR);
  } else {
    lightLEDs(STATIONARY_COLOR);
  }
}

void lightLEDs(uint32_t color) {
  strip.clear();
  if (ledCount > 0) { // Alow all the lights to go out since strip.fill with 0 index and 0 count lights the full strip
    strip.fill(color, ledIndex, ledCount);
    strip.fill(color, reverseLEDIndex, ledCount);
  }
  strip.show();

  if (ledIndex == NUM_PIXELS) { // We've reached the end so we reset
    ledIndex = 0;
    reverseLEDIndex = NUM_PIXELS;
    ledCount = 0;
  } else if (ledIndex + ledCount == NUM_PIXELS) { // We're getting close to the end so we lower our count
    ledIndex++;
    ledCount--;
    if (reverseLEDIndex > 0) {
      reverseLEDIndex--;
    }
  } else if (ledCount < LED_CHAIN_COUNT) { // We're just starting out so we up the count and reduce reverse index
    ledCount++;
    reverseLEDIndex--;
  } else { // We're in the middle so this part is easy
    ledIndex++;
    reverseLEDIndex--;
  }
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
