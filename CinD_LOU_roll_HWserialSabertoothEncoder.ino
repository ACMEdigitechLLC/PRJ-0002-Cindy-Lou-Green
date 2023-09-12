#include <stdlib.h>
#include <SoftwareSerial.h>

// **************************************************************
// CinD_LOU_roll_HWserialSabertooth  20230903
// **************************************************************

// serial to serial monitor uses pin 1 for TX and pin 0 for RX
// serial1 to LCD-DISPLAY uses pin 18 for TX and pin 19 for RX is not used
// serial2 to CTL-0001 SABERTOOTH  uses pin 16 for TX and pin 17 for RX is not used

//  
// A text line command entered in the Serial Monitor window
// can be sent to the Arduino for deciphering and action.
//
// Generic command format:
// "cmd, amt, spd"
// Where "cmd" is a command (see table)
// Where "amt" is an amount (distance or angle see table)
// Where "spd" is the speed (0 to 60 dimensionless units)
//
// Command list:
//   0 = STP = SToP
//   1 = FWD = move FORward
//   2 = REV = move REVerse
//   3 = ROL = ROtate Left
//   4 = ROR = ROtate Right
//   5 = TNL = TurN Left
//   6 = TNR = TurN Right
//   7 = TST = Test speed control balance
// ... = not used
// 254 = not used
// 255 = HLP = HELp print this command table

// Declarations
String str;   // holds the serial text that is read in
int    len;   // the character length of the text string
int    cmd;   // the command
int    amt;   // the amount
int    spd;   // the speed
int    rad;   // the radius
int    num;   // the number
byte   valL = 0;   // the Left motor command value
byte   valR = 0;   // the Right motor command value
int       i = 1;   // used as a loop counter
//SoftwareSerial lcdSerial(2,3); // pin 2 = RX, pin 3 = TX (unused)
//SoftwareSerial xbeeSerial(6,7);//pin 6 = TX, pin 7 = RX (unused)
char dtgstring[10];  // create string array
char valLstring[10]; // create string array 
char valRstring[10]; // create string array

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
    Serial2.begin(9600);
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
}

void loop() {
  // set all command inputs to zero
  cmd = 0;
  amt = 0;
  spd = 0;
  rad = 0;
  // Reset Serial1 2x16 LCD display
  Serial1.write(254); // cursor to 1st position on 1st line
  Serial1.write(128);
  Serial1.write("CinD-LOU motion "); // clear display + legends
  Serial1.write(" Enter Command  ");
  
  // if there's any serial available, read it:
  while (Serial.available() > 0) {

    // the 1st valid integer in the incoming serial stream is the "cmd"
    int cmd = Serial.parseInt(); 
    // do it again, the 2nd valid integer is the amount (distance or degrees)
    int amt = Serial.parseInt(); 
    // do it again, the 3rd valid integer is the speed
    int spd = Serial.parseInt(); 
    // do it again, if a 4th valid integer it is the turn radius
    int rad = Serial.parseInt(); 

    // look for the newline. That's the end of your command input
    if (Serial.read() == '\n') {

      // echo the command input for validation:
      Serial.print("Command input received: cmd = ");
      Serial.print(cmd);
      Serial.print(", amt = ");      
      Serial.print(amt);
      Serial.print(", spd = ");      
      Serial.print(spd);
      Serial.print(", rad = ");      
      Serial.println(rad);
      
     
      // constrain the values to valid ranges 
      cmd = constrain(cmd, 0, 255);
      amt = constrain(amt, 0, 360);
      spd = constrain(spd, 0, 60);
      rad = constrain(rad, 0, 60);

      // logic to decipher and take action
      if      (cmd == 0)   {Stop        ();}
      else if (cmd == 1)   {Forward     (amt, spd);}
      else if (cmd == 2)   {Reverse     (amt, spd);}
      else if (cmd == 3)   {RotateLeft  (amt, spd);}
      else if (cmd == 4)   {RotateRight (amt, spd);}
      else if (cmd == 5)   {TurnLeft    (amt, spd, rad);}
      else if (cmd == 6)   {TurnRight   (amt, spd, rad);}
      else if (cmd == 7)   {Test        (amt, spd);}
      else if (cmd == 255) {Help        ();}
      else                 {BadCommand  ();}
    }
  }
}

// **************************************************************
// function Stop
// **************************************************************
void Stop(){
  Serial.println("Command interpreted as: STOP! ");
  valL = 0;
  valR = 0;
  Serial.print("  Motor values will be: valL = ");
  Serial.print(valL);
  Serial.print(", valR = ");
  Serial.println(valR);
  Serial.println(); 
    Serial2.write(valL);
    Serial2.write(valR);  
}

// **************************************************************
// function Forward
// **************************************************************
void Forward(int amt, int spd){
  // determine if motion profile has constant velocity or not
  if (2 * spd > amt) spd = amt / 2;  // if can not get to top speed in amt, reduce spd
  // special case if amt = 1
  if (amt == 1) spd = 1;
  // set start of decel point
  int startdec =  amt - spd + 2;

  // Zero Encoder ticks before motion starts
  Encoder1ticks = 0;
  Encoder2ticks = 0;
  valL =  64;    //  64 = zero velocity for left motor
  valR = 192;    // 192 = zero velocity for right motor

  // echo commands to serial monitor for debug
  Serial.print("Command interpreted as: FORWARD, ");
  Serial.print("amt = ");
  Serial.print(amt);
  Serial.print(", spd = ");      
  Serial.println(spd);
  ShowEncoderTicks();
  Serial.println("Start Motion Profile Now");
  
  // setup 2x16 LCD display text
  Serial1.write(254); // cursor to 1st position on 1st line
  Serial1.write(128);
  Serial1.write("FORWARD DTG=    "); // serial1 1st line
  Serial1.write("LFT=        =RHT"); // serial1 2nd line
  
  // loop to issue motor commands at each step in the profile
  for(int i = 1; i <= amt; i++){
    if(i <= spd){ 
      // start at 1 and accelerate up to top speed at each step
      valL = valL + 1;
      valR = valR + 1;
    }
    else if(i < startdec){
      // continue at top speed until decel start step      
    }
    else{
      // decel down from top speed to end at amt
      valL = valL - 1;
      valR = valR - 1;
    }
    LevelUpTicks();
    int DistanceToGo = amt - i + 1;
    Serial.print("Step ");
    Serial.print(i);
    Serial.print(": valL = ");
    Serial.print(valL);
    Serial.print(", valR = ");
    Serial.print(valR);
    Serial.print(", distance to go = ");
    Serial.println(DistanceToGo);
    ShowEncoderTicks();
    
    // set Serial1 print values
    sprintf(dtgstring,"%4d",DistanceToGo);   
    sprintf(valRstring,"%3d",valL); // create strings from the numbers
    sprintf(valLstring,"%3d",valR); // right-justify to 3 spaces
    // Distance to Go
    Serial1.write(254); // cursor to 13th position on 1st line
    Serial1.write(140);
    Serial1.write(dtgstring); // write out valL value
    // valL
    Serial1.write(254); // cursor to 5th position on 2nd line
    Serial1.write(196);
    Serial1.write(valLstring); // write out valL value
    // valR
    Serial1.write(254); // cursor to 10th position on 2nd line
    Serial1.write(201);
    Serial1.write(valRstring); // write out valR value
    ShowEncoderTicks();
    
    delay(50);
    Serial2.write(valL);
    Serial2.write(valR);
    
  }
  Serial.println("Stop Now: valL = 0, valR = 0, End of Motion");
  Serial.println();
  ShowEncoderTicks();
  // Reset valR and valL to zero
  Serial2.write(0);

}

// **************************************************************
// function Reverse
// **************************************************************
void Reverse(int amt, int spd){
  // determine if motion profile has constant velocity or not
  if (2 * spd > amt) spd = amt / 2;  // if can not get to top speed in amt, reduce spd
  // special case if amt = 1
  if (amt == 1) spd = 1;
  // set start of decel point
  int startdec =  amt - spd + 2;

  valL =  64;    //  64 = zero velocity for left motor
  valR = 192;    // 192 = zero velocity for right motor

  // echo commands for debug
  Serial.print("Command interpreted as: REVERSE, ");
  Serial.print("amt = ");
  Serial.print(amt);
  Serial.print(", spd = ");      
  Serial.println(spd);
  Serial.println("Start Motion Profile Now");
  
  // setup 2x16 LCD display text
  Serial1.write(254); // cursor to 1st position on 1st line
  Serial1.write(128);
  Serial1.write("REVERSE DTG=    "); // Serial1 1st line
  Serial1.write("LFT=        =RHT"); // Serial1 2nd line
  
  // loop to issue motor commands at each step in the profile
  for(int i = 1; i <= amt; i++){
    if(i <= spd){ 
      // start at 1 and accelerate up to top speed at each step
      valL = valL - 1;
      valR = valR - 1;
    }
    else if(i < startdec){
      // continue at top speed until decel start step      
    }
    else{
      // decel down from top speed to end at amt
      valL = valL + 1;
      valR = valR + 1;
    }
  
    int DistanceToGo = amt - i + 1;
    Serial.print("Step ");
    Serial.print(i);
    Serial.print(": valL = ");
    Serial.print(valL);
    Serial.print(", valR = ");
    Serial.print(valR);
    Serial.print(", distance to go = ");
    Serial.println(DistanceToGo);
    
    // set Serial1 print values
    sprintf(dtgstring,"%4d",DistanceToGo);   
    sprintf(valRstring,"%3d",valL); // create strings from the numbers
    sprintf(valLstring,"%3d",valR); // right-justify to 3 spaces
    // Distance to Go
    Serial1.write(254); // cursor to 13th position on 1st line
    Serial1.write(140);
    Serial1.write(dtgstring); // write out valL value
    // valL
    Serial1.write(254); // cursor to 5th position on 2nd line
    Serial1.write(196);
    Serial1.write(valLstring); // write out valL value
    // valR
    Serial1.write(254); // cursor to 10th position on 2nd line
    Serial1.write(201);
    Serial1.write(valRstring); // write out valR value
    
    delay(500);
    Serial2.write(valL);
    Serial2.write(valR);   
  }
  Serial.println("Stop Now: valL = 0, valR = 0, End of Motion");
  Serial.println();
    Serial2.write(0);
}

// **************************************************************
// function RotateLeft
// **************************************************************
void RotateLeft(int amt, int spd){
  // determine if motion profile has constant velocity or not
  if (2 * spd > amt) spd = amt / 2;  // if can not get to top speed in amt, reduce spd
  // special case if amt = 1
  if (amt == 1) spd = 1;
  // set start of decel point
  int startdec =  amt - spd + 2;

  valL =  64;    //  64 = zero velocity for left motor
  valR = 192;    // 192 = zero velocity for right motor

  // echo commands for debug
  Serial.print("Command interpreted as: ROTATE LEFT, ");
  Serial.print("amt = ");
  Serial.print(amt);
  Serial.print(", spd = ");      
  Serial.println(spd);
  Serial.println("Start Motion Profile Now");
 
  // setup 2x16 LCD display text
  Serial1.write(254); // cursor to 1st position on 1st line
  Serial1.write(128);
  Serial1.write("ROT-LFT DTG=    "); // Serial1 1st line
  Serial1.write("LFT=        =RHT"); // Serial1 2nd line 
  
  // loop to issue motor commands at each step in the profile
  for(int i = 1; i <= amt; i++){
    if(i <= spd){ 
      // start at 1 and accelerate up to top speed at each step
      valL = valL - 1;
      valR = valR + 1;
    }
    else if(i < startdec){
      // continue at top speed until decel start step      
    }
    else{
      // decel down from top speed to end at amt
      valL = valL + 1;
      valR = valR - 1;
    }
  
    int DistanceToGo = amt - i + 1;
    Serial.print("Step ");
    Serial.print(i);
    Serial.print(": valL = ");
    Serial.print(valL);
    Serial.print(", valR = ");
    Serial.print(valR);
    Serial.print(", distance to go = ");
    Serial.println(DistanceToGo);

    // set Serial1 print values
    sprintf(dtgstring,"%4d",DistanceToGo);   
    sprintf(valRstring,"%3d",valL); // create strings from the numbers
    sprintf(valLstring,"%3d",valR); // right-justify to 3 spaces
    // Distance to Go
    Serial1.write(254); // cursor to 13th position on 1st line
    Serial1.write(140);
    Serial1.write(dtgstring); // write out valL value
    // valL
    Serial1.write(254); // cursor to 5th position on 2nd line
    Serial1.write(196);
    Serial1.write(valLstring); // write out valL value
    // valR
    Serial1.write(254); // cursor to 10th position on 2nd line
    Serial1.write(201);
    Serial1.write(valRstring); // write out valR value
    
    delay(500);
    Serial2.write(valL);
    Serial2.write(valR);    
  }
  Serial.println("Stop Now: valL = 0, valR = 0, End of Motion");
  Serial.println();
    Serial2.write(0);
} 
  
// **************************************************************
// function RotateRight
// **************************************************************
void RotateRight(int amt, int spd){
  // determine if motion profile has constant velocity or not
  if (2 * spd > amt) spd = amt / 2;  // if can not get to top speed in amt, reduce spd
  // special case if amt = 1
  if (amt == 1) spd = 1;
  // set start of decel point
  int startdec =  amt - spd + 2;

  valL =  64;    //  64 = zero velocity for left motor
  valR = 192;    // 192 = zero velocity for right motor

  // echo commands for debug
  Serial.print("Command interpreted as: ROTATE RIGHT, ");
  Serial.print("amt = ");
  Serial.print(amt);
  Serial.print(", spd = ");      
  Serial.println(spd);
  Serial.println("Start Motion Profile Now");
  
  // setup 2x16 LCD display text
  Serial1.write(254); // cursor to 1st position on 1st line
  Serial1.write(128);
  Serial1.write("ROT-RHT DTG=    "); // Serial1 1st line
  Serial1.write("LFT=        =RHT"); // Serial1 2nd line
  
  // loop to issue motor commands at each step in the profile
  for(int i = 1; i <= amt; i++){
    if(i <= spd){ 
      // start at 1 and accelerate up to top speed at each step
      valL = valL + 1;
      valR = valR - 1;
    }
    else if(i < startdec){
      // continue at top speed until decel start step      
    }
    else{
      // decel down from top speed to end at amt
      valL = valL - 1;
      valR = valR + 1;
    }
  
    int DistanceToGo = amt - i + 1;
    Serial.print("Step ");
    Serial.print(i);
    Serial.print(": valL = ");
    Serial.print(valL);
    Serial.print(", valR = ");
    Serial.print(valR);
    Serial.print(", distance to go = ");
    Serial.println(DistanceToGo);
 
     // set Serial1 print values
    sprintf(dtgstring,"%4d",DistanceToGo);   
    sprintf(valRstring,"%3d",valL); // create strings from the numbers
    sprintf(valLstring,"%3d",valR); // right-justify to 3 spaces
    // Distance to Go
    Serial1.write(254); // cursor to 13th position on 1st line
    Serial1.write(140);
    Serial1.write(dtgstring); // write out valL value
    // valL
    Serial1.write(254); // cursor to 5th position on 2nd line
    Serial1.write(196);
    Serial1.write(valLstring); // write out valL value
    // valR
    Serial1.write(254); // cursor to 10th position on 2nd line
    Serial1.write(201);
    Serial1.write(valRstring); // write out valR value
    
    delay(500);
    Serial2.write(valL);
    Serial2.write(valR); 
  }
  Serial.println("Stop Now: valL = 0, valR = 0, End of Motion");
  Serial.println();
    Serial2.write(0);
}

// **************************************************************
// function TurnLeft
// **************************************************************
void TurnLeft(int amt, int spd, int rad){
  // determine if motion profile has constant velocity or not
  if (2 * spd > amt) spd = amt / 2;  // if can not get to top speed in amt, reduce spd
  // special case if amt = 1
  if (amt == 1) spd = 1;
  // set start of decel point
  int startdec =  amt - spd + 2;

  valL =  64;    //  64 = zero velocity for left motor
  valR = 192;    // 192 = zero velocity for right motor

  // echo commands for debug
  Serial.print("Command interpreted as: TURN LEFT, ");
  Serial.print("amt = ");
  Serial.print(amt);
  Serial.print(", spd = ");      
  Serial.println(spd);
  Serial.println("Start Motion Profile Now");
 
  // setup 2x16 LCD display text
  Serial1.write(254); // cursor to 1st position on 1st line
  Serial1.write(128);
  Serial1.write("TRN-LFT DTG=    "); // Serial1 1st line
  Serial1.write("LFT=        =RHT"); // Serial1 2nd line 
  
  // loop to issue motor commands at each step in the profile
  for(int i = 1; i <= amt; i++){
    if(i <= spd){ 
      // start at 1 and accelerate up to top speed at each step
      valL = valL - 1;
      valR = valR + 1;
    }
    else if(i < startdec){
      // continue at top speed until decel start step      
    }
    else{
      // decel down from top speed to end at amt
      valL = valL + 1;
      valR = valR - 1;
    }
  
    int DistanceToGo = amt - i + 1;
    Serial.print("Step ");
    Serial.print(i);
    Serial.print(": valL = ");
    Serial.print(valL);
    Serial.print(", valR = ");
    Serial.print(valR);
    Serial.print(", distance to go = ");
    Serial.println(DistanceToGo);

    // set software serial print values
    sprintf(dtgstring,"%4d",DistanceToGo);   
    sprintf(valRstring,"%3d",valL); // create strings from the numbers
    sprintf(valLstring,"%3d",valR); // right-justify to 3 spaces
    // Distance to Go
    Serial1.write(254); // cursor to 13th position on 1st line
    Serial1.write(140);
    Serial1.write(dtgstring); // write out valL value
    // valL
    Serial1.write(254); // cursor to 5th position on 2nd line
    Serial1.write(196);
    Serial1.write(valLstring); // write out valL value
    // valR
    Serial1.write(254); // cursor to 10th position on 2nd line
    Serial1.write(201);
    Serial1.write(valRstring); // write out valR value
    
    delay(500);
    Serial2.write(valL);
    Serial2.write(valR);   
  }
  Serial.println("Stop Now: valL = 0, valR = 0, End of Motion");
  Serial.println();
    Serial2.write(0);
}

// **************************************************************
// function TurnRight
// **************************************************************
void TurnRight(int amt, int spd, int rad){
  // determine if motion profile has constant velocity or not
  if (2 * spd > amt) spd = amt / 2;  // if can not get to top speed in amt, reduce spd
  // special case if amt = 1
  if (amt == 1) spd = 1;
  // set start of decel point
  int startdec =  amt - spd + 2;

  valL =  64;    //  64 = zero velocity for left motor
  valR = 192;    // 192 = zero velocity for right motor

  // echo commands for debug
  Serial.print("Command interpreted as: TURN RIGHT, ");
  Serial.print("amt = ");
  Serial.print(amt);
  Serial.print(", spd = ");      
  Serial.println(spd);
  Serial.println("Start Motion Profile Now");
  
  // setup 2x16 LCD display text
  Serial1.write(254); // cursor to 1st position on 1st line
  Serial1.write(128);
  Serial1.write("TRN-RHT DTG=    "); // Serial1 1st line
  Serial1.write("LFT=        =RHT"); // Serial1 2nd line
  
  // loop to issue motor commands at each step in the profile
  for(int i = 1; i <= amt; i++){
    if(i <= spd){ 
      // start at 1 and accelerate up to top speed at each step
      valL = valL + 1;
      valR = valR - 1;
    }
    else if(i < startdec){
      // continue at top speed until decel start step      
    }
    else{
      // decel down from top speed to end at amt
      valL = valL - 1;
      valR = valR + 1;
    }
  
    int DistanceToGo = amt - i + 1;
    Serial.print("Step ");
    Serial.print(i);
    Serial.print(": valL = ");
    Serial.print(valL);
    Serial.print(", valR = ");
    Serial.print(valR);
    Serial.print(", distance to go = ");
    Serial.println(DistanceToGo);
 
     // set Serial1 print values
    sprintf(dtgstring,"%4d",DistanceToGo);   
    sprintf(valRstring,"%3d",valL); // create strings from the numbers
    sprintf(valLstring,"%3d",valR); // right-justify to 3 spaces
    // Distance to Go
    Serial1.write(254); // cursor to 13th position on 1st line
    Serial1.write(140);
    Serial1.write(dtgstring); // write out valL value
    // valL
    Serial1.write(254); // cursor to 5th position on 2nd line
    Serial1.write(196);
    Serial1.write(valLstring); // write out valL value
    // valR
    Serial1.write(254); // cursor to 10th position on 2nd line
    Serial1.write(201);
    Serial1.write(valRstring); // write out valR value
    
    delay(500);
    Serial2.write(valL);
    Serial2.write(valR);
  }
  Serial.println("Stop Now: valL = 0, valR = 0, End of Motion");
  Serial.println();
    Serial2.write(0);
}


// **************************************************************
// function Test 7
// **************************************************************
void Test(int amt, int spd){

  int curamt = amt;
  int curspd = spd;
   
  
  Encoder1ticks = 0;
  Encoder2ticks = 0;
  valL =  92;    //  64 = zero velocity for left motor
  valR = 222;    // 192 = zero velocity for right motor
  
  Serial2.write(0);
  delay(1000);
  Serial2.write(valL);
  delay(1000);
  Serial2.write(0);
  delay(1000);
  Serial2.write(valR);
  delay(1000);
  Serial2.write(0);
  delay(1000);
  Serial2.write(valL);
  delay(1000);
  Serial2.write(valR);
  delay(1000);
  Serial2.write(0);

  delay(1000);
  Serial2.write(valL);
  delay(100);
  Serial2.write(valR);
  delay(5000);
  Serial2.write(0);
    
  delay(1000);
  Serial2.write(valL);
  delay(100);
  Serial2.write(valR);
  delay(10000);
  Serial2.write(0);

  delay(1000);
  Serial2.write(valR);
  delay(100);
  Serial2.write(valL);
  delay(10000);
  Serial2.write(0);

  
  ShowEncoderTicks();
  Serial.print("Step ");
  Serial.print(i);
  Serial.print(": valL = ");
  Serial.print(valL);
  Serial.print(", valR = ");
  Serial.println(valR);
  
  delay(50);
}  //end function Test 7


// **************************************************************
// function Test 8
// **************************************************************
void Test8(int amt, int spd){

  int curspd = spd;
  // determine if motion profile has constant velocity or not
  //if (2 * spd > amt) spd = amt / 2;  // if can not get to top speed in amt, reduce spd
  // special case if amt = 1
  //if (amt == 1) spd = 1;
  // set start of decel point
  //int startdec =  amt - spd + 2;

  // Zero Encoder ticks before motion starts
  Encoder1ticks = 0;
  Encoder2ticks = 0;
  valL =  64;    //  64 = zero velocity for left motor
  valR = 192;    // 192 = zero velocity for right motor

  // echo commands to serial monitor for debug
  Serial.print("Command interpreted as: TEST, ");
  Serial.print("amt = ");
  Serial.print(amt);
  Serial.print(", spd = ");      
  Serial.println(spd);
  ShowEncoderTicks();
  Serial.println("Start Acceleration up to speed");
  
  // setup 2x16 LCD display text
  Serial1.write(254); // cursor to 1st position on 1st line
  Serial1.write(128);
  Serial1.write("   TEST DTG=    "); // serial1 1st line
  Serial1.write("LFT=        =RHT"); // serial1 2nd line
  
  // loop to issue motor commands at each step in the profile
  for(curspd = 0 ; curspd < spd; curspd++){
    // start at 1 and accelerate up to top speed at each step
    valL = valL + 1;
    valR = valR + 1;
    Serial.println(curspd);
    Serial.print("Step ");
    Serial.print(i);
    Serial.print(": valL = ");
    Serial.print(valL);
    Serial.print(", valR = ");
    Serial.print(valR);
    Serial.print(", distance to go = ");
    Serial.println(curspd);
    ShowEncoderTicks();

    // set Serial1 print values
    sprintf(dtgstring,"%4d",curspd);   
    sprintf(valRstring,"%3d",valL); // create strings from the numbers
    sprintf(valLstring,"%3d",valR); // right-justify to 3 spaces
    // Distance to Go
    Serial1.write(254); // cursor to 13th position on 1st line
    Serial1.write(140);
    Serial1.write(dtgstring); // write out valL value
    // valL
    Serial1.write(254); // cursor to 5th position on 2nd line
    Serial1.write(196);
    Serial1.write(valLstring); // write out valL value
    // valR
    Serial1.write(254); // cursor to 10th position on 2nd line
    Serial1.write(201);
    Serial1.write(valRstring); // write out valR value
    //ShowEncoderTicks();
    
    delay(100);
    Serial2.write(valL);
    Serial2.write(valR);
  }
  Encoder1ticks = 0;
  Encoder2ticks = 0;

  if (curspd = spd){
    } 
  if (Encoder1ticks >= 60) {
    }    
  else if(Encoder1ticks >= 60) {
    }
 
  
  Serial.println("Stop Now: valL = 0, valR = 0, End of Motion");
  Serial.println();
  ShowEncoderTicks();
  // Reset valR and valL to zero
  //Serial2.write(0);
}  //end function Test  8

// **************************************************************
// function Help
// **************************************************************
void Help(){
   Serial.println("The command list is:");
   Serial.println("* cmd =   0 => STOP!        (ex: 0,0,0,0)");
   Serial.println("* cmd =   1 => Forward      (ex: 1,100,60,0)");
   Serial.println("* cmd =   2 => Reverse      (ex: 2,100,60,0)");
   Serial.println("* cmd =   3 => Rotate Left  (ex: 3,90,60,0)");
   Serial.println("* cmd =   4 => Rotate Right (ex: 4,270,60,0)");
   Serial.println("* cmd =   5 => Turn Left    (ex: 5,90,60,10)");
   Serial.println("* cmd =   6 => Turn Right   (ex: 6,270,60,10)");
   Serial.println("* cmd = 255 => Help         (ex: 255,0,0,0)");
   Serial.println();
}  //end function Help

// **************************************************************
// function BadCommand
// **************************************************************
void BadCommand(){
   Serial.println("Not a valid command.  Try again...");
   Serial.println("Valid cmd values are (0, 1, 2, 3, 4, 5, 6 and 255)");
}  //end function BadCommand

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
