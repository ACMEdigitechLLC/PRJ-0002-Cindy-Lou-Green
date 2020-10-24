#include <stdlib.h>
#include <SoftwareSerial.h>

// **************************************************************
// This is the CinD_LOU_Roll_4.ino sketch for CinD-LOU
// **************************************************************
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
//   7 = not used
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
int    valL;   // the Left motor command value
int    valR;   // the Right motor command value
SoftwareSerial lcdSerial(3,2); // pin 2 = TX, pin 3 = RX (unused)
char dtgstring[10];  // create string array
char valLstring[10]; // create string array
char valRstring[10]; // create string array


void setup() {
  // initialize hardware serial:
  Serial.begin(9600);
  Serial.println("Power On");   // a start-up message
  Serial.println("Enter a valid motion command in the form n, n, n, n <SEND>");
  Serial.println("Enter 255, 0, 0, 0 for the HELP command table");
  Serial.println();

  // initialize software serial on pin 2
  lcdSerial.begin(9600); // set up serial port for 9600 baud
  delay(500); // wait for display to boot up

  lcdSerial.write(254); // cursor to beginning of first line
  lcdSerial.write(128);

  lcdSerial.write("CinD-LOU motion "); // clear display + legends
  lcdSerial.write(" Enter Command  ");

}

void loop() {
  // set all command inputs to zero
  cmd = 0;
  amt = 0;
  spd = 0;
  rad = 0;
  // Reset software serial 2x16 LCD display
  lcdSerial.write(254); // cursor to 1st position on 1st line
  lcdSerial.write(128);
  lcdSerial.write("CinD-LOU motion "); // clear display + legends
  lcdSerial.write(" Enter Command  ");
  
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

  valL =  64;    //  64 = zero velocity for left motor
  valR = 192;    // 192 = zero velocity for right motor

  // echo commands to serial monitor for debug
  Serial.print("Command interpreted as: FORWARD, ");
  Serial.print("amt = ");
  Serial.print(amt);
  Serial.print(", spd = ");      
  Serial.println(spd);
  Serial.println("Start Motion Profile Now");
  
  // setup 2x16 LCD display text
  lcdSerial.write(254); // cursor to 1st position on 1st line
  lcdSerial.write(128);
  lcdSerial.write("FORWARD DTG=    "); // software serial 1st line
  lcdSerial.write("LFT=        =RHT"); // software serial 2nd line
  
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
    lcdSerial.write(254); // cursor to 13th position on 1st line
    lcdSerial.write(140);
    lcdSerial.write(dtgstring); // write out valL value
    // valL
    lcdSerial.write(254); // cursor to 5th position on 2nd line
    lcdSerial.write(196);
    lcdSerial.write(valLstring); // write out valL value
    // valR
    lcdSerial.write(254); // cursor to 10th position on 2nd line
    lcdSerial.write(201);
    lcdSerial.write(valRstring); // write out valR value
    
    delay(500);
    
  }
  Serial.println("Stop Now: valL = 64, valR = 192, End of Motion");
  Serial.println();
  

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
  lcdSerial.write(254); // cursor to 1st position on 1st line
  lcdSerial.write(128);
  lcdSerial.write("REVERSE DTG=    "); // software serial 1st line
  lcdSerial.write("LFT=        =RHT"); // software serial 2nd line
  
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
    
    // set software serial print values
    sprintf(dtgstring,"%4d",DistanceToGo);   
    sprintf(valRstring,"%3d",valL); // create strings from the numbers
    sprintf(valLstring,"%3d",valR); // right-justify to 3 spaces
    // Distance to Go
    lcdSerial.write(254); // cursor to 13th position on 1st line
    lcdSerial.write(140);
    lcdSerial.write(dtgstring); // write out valL value
    // valL
    lcdSerial.write(254); // cursor to 5th position on 2nd line
    lcdSerial.write(196);
    lcdSerial.write(valLstring); // write out valL value
    // valR
    lcdSerial.write(254); // cursor to 10th position on 2nd line
    lcdSerial.write(201);
    lcdSerial.write(valRstring); // write out valR value
    
    delay(500);
    
  }
  Serial.println("Stop Now: valL = 64, valR = 192, End of Motion");
  Serial.println();

  
  
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
  lcdSerial.write(254); // cursor to 1st position on 1st line
  lcdSerial.write(128);
  lcdSerial.write("ROT-LFT DTG=    "); // software serial 1st line
  lcdSerial.write("LFT=        =RHT"); // software serial 2nd line 
  
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
    lcdSerial.write(254); // cursor to 13th position on 1st line
    lcdSerial.write(140);
    lcdSerial.write(dtgstring); // write out valL value
    // valL
    lcdSerial.write(254); // cursor to 5th position on 2nd line
    lcdSerial.write(196);
    lcdSerial.write(valLstring); // write out valL value
    // valR
    lcdSerial.write(254); // cursor to 10th position on 2nd line
    lcdSerial.write(201);
    lcdSerial.write(valRstring); // write out valR value
    
    delay(500);
    
  }
  Serial.println("Stop Now: valL = 64, valR = 192, End of Motion");
  Serial.println();
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
  lcdSerial.write(254); // cursor to 1st position on 1st line
  lcdSerial.write(128);
  lcdSerial.write("ROT-RHT DTG=    "); // software serial 1st line
  lcdSerial.write("LFT=        =RHT"); // software serial 2nd line
  
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
 
     // set software serial print values
    sprintf(dtgstring,"%4d",DistanceToGo);   
    sprintf(valRstring,"%3d",valL); // create strings from the numbers
    sprintf(valLstring,"%3d",valR); // right-justify to 3 spaces
    // Distance to Go
    lcdSerial.write(254); // cursor to 13th position on 1st line
    lcdSerial.write(140);
    lcdSerial.write(dtgstring); // write out valL value
    // valL
    lcdSerial.write(254); // cursor to 5th position on 2nd line
    lcdSerial.write(196);
    lcdSerial.write(valLstring); // write out valL value
    // valR
    lcdSerial.write(254); // cursor to 10th position on 2nd line
    lcdSerial.write(201);
    lcdSerial.write(valRstring); // write out valR value
    
    delay(500);
 
  }
  Serial.println("Stop Now: valL = 64, valR = 192, End of Motion");
  Serial.println();
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
  lcdSerial.write(254); // cursor to 1st position on 1st line
  lcdSerial.write(128);
  lcdSerial.write("TRN-LFT DTG=    "); // software serial 1st line
  lcdSerial.write("LFT=        =RHT"); // software serial 2nd line 
  
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
    lcdSerial.write(254); // cursor to 13th position on 1st line
    lcdSerial.write(140);
    lcdSerial.write(dtgstring); // write out valL value
    // valL
    lcdSerial.write(254); // cursor to 5th position on 2nd line
    lcdSerial.write(196);
    lcdSerial.write(valLstring); // write out valL value
    // valR
    lcdSerial.write(254); // cursor to 10th position on 2nd line
    lcdSerial.write(201);
    lcdSerial.write(valRstring); // write out valR value
    
    delay(500);
    
  }
  Serial.println("Stop Now: valL = 64, valR = 192, End of Motion");
  Serial.println();
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
  lcdSerial.write(254); // cursor to 1st position on 1st line
  lcdSerial.write(128);
  lcdSerial.write("TRN-RHT DTG=    "); // software serial 1st line
  lcdSerial.write("LFT=        =RHT"); // software serial 2nd line
  
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
 
     // set software serial print values
    sprintf(dtgstring,"%4d",DistanceToGo);   
    sprintf(valRstring,"%3d",valL); // create strings from the numbers
    sprintf(valLstring,"%3d",valR); // right-justify to 3 spaces
    // Distance to Go
    lcdSerial.write(254); // cursor to 13th position on 1st line
    lcdSerial.write(140);
    lcdSerial.write(dtgstring); // write out valL value
    // valL
    lcdSerial.write(254); // cursor to 5th position on 2nd line
    lcdSerial.write(196);
    lcdSerial.write(valLstring); // write out valL value
    // valR
    lcdSerial.write(254); // cursor to 10th position on 2nd line
    lcdSerial.write(201);
    lcdSerial.write(valRstring); // write out valR value
    
    delay(500);
 
  }
  Serial.println("Stop Now: valL = 64, valR = 192, End of Motion");
  Serial.println();
}

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
}

// **************************************************************
// function BadCommand
// **************************************************************
void BadCommand(){
   Serial.println("Not a valid command.  Try again...");
   Serial.println("Valid cmd values are (0, 1, 2, 3, 4, 5, 6 and 255)");
}
 
