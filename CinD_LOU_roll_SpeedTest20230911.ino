#include <stdlib.h>
#include <SoftwareSerial.h>

// **************************************************************
// CinD_LOU_roll_SpeedTest  20230911
// **************************************************************

// serial to serial monitor uses pin 1 for TX and pin 0 for RX
// serial1 to LCD-DISPLAY uses pin 18 for TX and pin 19 for RX is not used
// serial2 to CTL-0001 SABERTOOTH  uses pin 16 for TX and pin 17 for RX is not used


// Declarations
String str;            // holds the serial text that is read in
int    len;            // the character length of the text string
int    cmd;            // the command
int    amt;            // the amount
int    spd;            // the speed
int    dir    = 1;     // dir = 1 -> clockwise, dir = -1 -> counter-clockwise
int    curspd = spd;
int    rad;            // the radius
int    num;            // the number
byte   valL   = 0;     // the Left motor command value
byte   valR   = 0;     // the Right motor command value
int    i      = 1;     // used as a loop counter
int    doWhat    = 1;  //
char   dtgstring[10];  // create string array
char   valLstring[10]; // create string array 
char   valRstring[10]; // create string array

// declarations for Encoder 1 (Left)
volatile int pinEncoder1A = 2;                                   // Encoder1 Pin A
int pinEncoder1B = 3;                                            // Encoder1 Pin B
volatile int Encoder1AstateCurrent = LOW;                        // Current state of Encoder1 Pin A
volatile int Encoder1AStateLast = Encoder1AstateCurrent;         // Last read value of Encoder1 Pin A
int Encoder1ticks = 0;                                           // Encoder1 inital tick count value is zero

// declarations for Encoder 2 (Right)
volatile int pinEncoder2A = 20;                                   // Encoder2 Pin A
int pinEncoder2B = 21;                                            // Encoder2 Pin B
volatile int Encoder2AstateCurrent = LOW;                         // Current state of Encoder2 Pin A
volatile int Encoder2AStateLast = Encoder2AstateCurrent;          // Last read value of Encoder2 Pin A
int Encoder2ticks = 0;                                            // Encoder2 inital tick count value is zero

void setup() {
  // initialize hardware serial:
  Serial.begin(9600);
  Serial.println("Power On");   // a start-up message
  Serial.println("Enter a valid motion command in the form n, n, n, n <SEND>");
  Serial.println("Enter 255, 0, 0, 0 for the HELP command table");
  Serial.println();

  // initialize hardware serial1 <-ONE! for LCD-Display on pin 18(TX) and pin19(RX)
  Serial1.begin(9600); // set up serial port for 9600 baud
  //delay(500); // wait for display to boot up
  Serial1.write(254); // cursor to beginning of first line
  Serial1.write(128);
  Serial1.write("CinD-LOU motion "); // clear display + legends
  Serial1.write(" Enter Command  ");

  // initialize hardware serial2 <-TWO! for CTL-0001 SABERTOOTH on pin 16(TX) and pin17(RX)
  Serial2.begin(9600,SERIAL_8N1);
  Serial2.write(valL);
  Serial2.write(valR);

  // declarations for Encoder1 and Encoder2
  pinMode (pinEncoder1A, INPUT_PULLUP);                          // Set Encoder1 Pin A as input
  pinMode (pinEncoder1B, INPUT_PULLUP);                          // Set Encoder1 Pin B as input

  pinMode (pinEncoder2A, INPUT_PULLUP);                          // Set Encoder2 Pin A as input
  pinMode (pinEncoder2B, INPUT_PULLUP);                          // Set Encoder2 Pin B as input

  // Atach a CHANGE interrupt to two Ecoder PinB and exectute the update function when this change occurs.
  attachInterrupt(digitalPinToInterrupt(pinEncoder1B), updateEncoder1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pinEncoder2B), updateEncoder2, CHANGE);

  i             = 1;
  amt           = 6;
  dir           = 1;      // dir = 1 -> clockwise, dir = -1 -> counter-clockwise
  curspd        = spd;
  Encoder1ticks = 0;
  Encoder2ticks = 0;
  valL          =  64;    //  64 = zero velocity for left motor
  valR          =  192;    // 192 = zero velocity for right motor
  doWhat        = 1;
}

void loop() {

  // echo commands to serial monitor for debug
  Serial.print("Step = ");
  Serial.print(i);
  Serial.print(", doWhat = ");
  Serial.print(doWhat);
  Serial.print(", valL =  ");
  Serial.print(valL);

  // command step motion and see motion
  Serial2.write(valL);
  delay(1000);

  if (dir == 1 && valL == 64){
    doWhat = 1;         //  at ZERO velocity, switch to Forward direction 
    Serial.println(", at ZERO velocity, switch to Forward direction");

  }
  
  else if (dir == 1 && valL == 69){
    doWhat = 2;         // starting Forward acceleration 
    Serial.println(", starting Forward acceleration");
  }

  else if (dir == 1 && valL > 69 && valL < 119){
    doWhat = 3;         // more Forward acceleration 
    Serial.println(", more Forward acceleration");
  }

  else if (dir == 1 && valL == 119){
    doWhat = 4;         //  ending Forward acceleration
    Serial.println(", ending Forward acceleration");
  }

  else if (dir == 1 && valL == 124){
    doWhat = 5;         // at Maximum Forward velocity 
    Serial.println(", at Maximum Forward velocity");
  }

  else if (dir == -1 && valL == 119){
    doWhat = 6;         // starting Forward deceleration 
    Serial.println(", starting Forward deceleration");
  }

  else if (dir == -1 && valL < 119 && valL > 69){
    doWhat = 7;         // less Forward acceleration 
    Serial.println(", less Forward acceleration");
  }

  else if (dir == -1 && valL == 69){
    doWhat = 8;         // ending Forward acceleration 
    Serial.println(", ending Forward acceleration");
  }
    
  else if (dir == -1 && valL == 64){
    doWhat = 9;         // at ZERO velocity, switch to Reverse direction
    Serial.println(", at ZERO velocity, switch to Reverse direction");
  }
    
  else if (dir == -1 && valL == 59){
    doWhat = 10;         // starting acceleration in Reverse 
    Serial.println(", starting acceleration in Reverse");
  }

  else if (dir == -1 && valL < 59 && valL > 9){
    doWhat = 11;         // more acceleration in Reverse 
    Serial.println(", more acceleration in Reverse");
  }
  
  else if (dir == -1 && valL == 9){
    doWhat = 12;         // ending acceleration in Reverse 
    Serial.println(", ending acceleration in Reverse");
  }

  else if (dir == -1 && valL == 4){
    doWhat = 13;         // at maximum velocity in Reverse 
    Serial.println(", at maximum velocity in Reverse");
  }

  else if (dir == 1 && valL == 9){
    doWhat = 14;         // starting deceleration in Reverse 
    Serial.println(", starting deceleration in Reverse");
  }

  else if (dir == 1 && valL > 9 && valL < 59){
    doWhat = 15;         // more deceleration in Reverse 
    Serial.println(", more decelleration in Reverse");
  }

  else if (dir == 1 && valL == 59){
    doWhat = 16;         // ending deceleration in Reverse 
    Serial.println(", ending deceleration in Reverse");
  }

  else {
    // Should never happen
    valL = 0;
    Serial2.write(0);
    Serial.println(", STOP because doWhat = WTF?");
  }

  i = i + 1;

  //  use switch case statements set up next step values 
  switch (doWhat) {
  case 1:
    // at ZERO velocity, switch to Forward direction
    valL = valL +5;
    break;

  case 2:
    // starting Forward acceleration
    valL = valL + 5;
    break;

  case 3:
    // more Forward acceleration
    valL = valL + 5;
    break;

  case 4:
    // ending Forward acceleration
    valL = valL + 5;
    break;
    
  case 5:
    // at Maximum Forward velocity
    dir = -1;
    valL = valL - 5;
    break;
  
  case 6:
    // starting Forward deceleration
    valL = valL - 5;
    break;

  case 7:
    // more Forward deceleration
    valL = valL - 5;
    break;
  
  case 8:
    // Ending Forward acceleration
    valL = valL - 5;
    break;
  
  case 9:
    // At ZERO velocity, switch to Reverse direction
    valL = valL - 5;
    break;
  
  case 10:
    // Starting acceleration in Reverse
    valL = valL - 5;
    break;

  case 11:
    // more acceleration in Reverse
    valL = valL - 5;
    break;
  
  case 12:
    // Ending acceleration im Reverse
    valL = valL - 5;
    break;
  
  case 13:
    // At maximum velocity in Reverse
    dir = 1;
    valL = valL + 5;
    break;
  
  case 14:
    // Starting decelleration in Reverse
    valL = valL + 5;
    break;

  case 15:
    // more decelleration in Reverse
    valL = valL + 5;
    break;

  case 16:
    // Ending decelleration in Reverse
    valL = valL + 5;
    break;
  
  default:
    // if nothing else matches, do the default
    Serial.println("case doWhat = WTF?");
    break;
  }
 
}  //end main loop


// **************************************************************
// function ShowEncoderTicks
// **************************************************************
void ShowEncoderTicks(){
  Serial.print("Encoder1 ticks = ");
  Serial.print(Encoder1ticks);
  Serial.print(", Encoder2 ticks = ");
  Serial.println(Encoder2ticks);
}  //end function ShowEncoderTicks

// ********************************************
// Function updateEncoder1
// ********************************************
void updateEncoder1() {

  // ROTATION DIRECTION
  Encoder1AstateCurrent = digitalRead(pinEncoder1A);    // Read the current state of Pin A
  
  // If there is a minimal movement of 1 tick
  if ((Encoder1AStateLast == LOW) && (Encoder1AstateCurrent == HIGH)) {
    
    if (digitalRead(pinEncoder1B) == HIGH) {      // If Pin B is HIGH
      Encoder1ticks--;           // minus(-) is forward and plus (+) is reverse
    } else {
      Encoder1ticks++;
    }
  }
  Encoder1AStateLast = Encoder1AstateCurrent;        // Store the latest read value in the currect state variable
}  //end function updateEncoder1


// ********************************************
// Function updateEncoder2
// ********************************************
void updateEncoder2() {

  // ROTATION DIRECTION
  Encoder2AstateCurrent = digitalRead(pinEncoder2A);    // Read the current state of Pin A
  
  // If there is a minimal movement of 1 tick
  if ((Encoder2AStateLast == LOW) && (Encoder2AstateCurrent == HIGH)) {
    
    if (digitalRead(pinEncoder2B) == HIGH) {      // If Pin B is HIGH
      Encoder2ticks--;           // minus(-) is forward and plus (+) is reverse  
    } else {
      Encoder2ticks++;
    }
  }
  Encoder2AStateLast = Encoder2AstateCurrent;        // Store the latest read value in the currect state variable
}  //end function updateEncoder2


// ********************************************
// Function LevelUpTicks
// ********************************************
void LevelUpTicks() {

  // if 
    if(Encoder1ticks > Encoder2ticks) {
    valL = valL+1;
    Serial.println("***** valL < valR *****");
  }
  else if(Encoder1ticks < Encoder2ticks) {
    valR = valL+1;
    Serial.println("***** valL < valR *****");
  }
  else {  
  }
}  //end function LevelUpTicks
