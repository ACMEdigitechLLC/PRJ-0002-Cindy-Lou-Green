#include <stdlib.h>
#include <SoftwareSerial.h>
//
// This is the CinD_LOU_Roll_3.ino sketch for CinD-LOU
// Version 3 adds serial output
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
int    valL;  // the Left motor command value
int    valR;  // the Right motor command value
// Attach the serial display's RX line to digital pin 2
SoftwareSerial mySerial(3,2); // pin 2 = TX, pin 3 = RX (unused)
char valRstring[10], valLstring[10]; // create string arrays


void setup() {
  // initialize serial:
  Serial.begin(9600);
  Serial.println("Power On");   // a start-up message
  Serial.println("Enter a valid motion command in the form n, n, n, n <SEND>");
  Serial.println("Enter 255, 0, 0, 0 for the HELP command table");
  Serial.println();
  // initialize software serial
  mySerial.begin(9600);  // set up serial port for 9600 baud
  delay(500);            // wait for display to boot up

  mySerial.write(254);   // cursor to beginning of first line
  mySerial.write(128);
  mySerial.write("RIGHT:          "); // clear display + legends
  mySerial.write("LEFT:           ");
}

void loop() {
  // set all command inputs to zero
  cmd = 0;
  amt = 0;
  spd = 0;
  rad = 0;
  
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
      if (cmd == 0) {
         Serial.println("Command interpreted as: STOP! ");
         valL = 0;
         valR = 0;
         Serial.print("  Motor values will be: valL = ");
         Serial.print(valL);
         Serial.print(", valR = ");
         Serial.println(valR);
         Serial.println();       
      }
      else if (cmd == 1) {
         Serial.print("Command interpreted as: FORWARD, ");
         Serial.print("amt = ");
         Serial.print(amt);
         Serial.print(", spd = ");      
         Serial.println(spd);
         valL =  64 + spd;
         valR = 192 + spd;
         Serial.print("  Motor values will be: valL = ");
         Serial.print(valL);
         Serial.print(", valR = ");
         Serial.print(valR);
         Serial.println(", distance in feet");
         Serial.println();
      }
      
      else if (cmd == 2) {
         Serial.print("Command interpreted as: REVERSE, ");         
         Serial.print("amt = ");
         Serial.print(amt);
         Serial.print(", spd = ");      
         Serial.println(spd);
         valL =  64 - spd;
         valR = 192 - spd;
         Serial.print("  Motor values will be: valL = ");
         Serial.print(valL);
         Serial.print(", valR = ");
         Serial.print(valR);
         Serial.println(", distance in feet");         
         Serial.println();
      }
                
      else if (cmd == 3) {
         Serial.print("Command interpreted as: ROTATE LEFT, ");
         Serial.print("amt = ");
         Serial.print(amt);
         Serial.print(", spd = ");      
         Serial.println(spd);
         valL =  64 - spd;
         valR = 192 + spd;
         Serial.print("  Motor values will be: valL = ");
         Serial.print(valL);
         Serial.print(", valR = ");
         Serial.print(valR);
         Serial.println(", rotation in degrees");
         Serial.println();         
      }
                
      else if (cmd == 4) {
         Serial.print("Command interpreted as: ROTATE RIGHT, ");
         Serial.print("amt = ");
         Serial.print(amt);
         Serial.print(", spd = ");      
         Serial.println(spd);
         valL =  64 + spd;
         valR = 192 - spd;
         Serial.print("  Motor values will be: valL = ");
         Serial.print(valL);
         Serial.print(", valR = ");
         Serial.print(valR);
         Serial.println(", rotation in degrees");         
         Serial.println();  
      }
                
      else if (cmd == 5) {
         Serial.print("Command interpreted as: TURN LEFT, ");
         Serial.print("amt = ");
         Serial.print(amt);
         Serial.print(", spd = ");
         Serial.print(spd);
         Serial.print(", rad = ");
         Serial.println(rad);
         valL =  64;
         valR = 192 + spd;
         Serial.print("  Motor values will be: valL = ");
         Serial.print(valL);
         Serial.print(", valR = ");
         Serial.print(valR);
         Serial.println(", turn in degrees at radius");         
         Serial.println();
      }
                
      else if (cmd == 6) {
         Serial.print("Command interpreted as: TURN RIGHT, ");
         Serial.print("amt = ");
         Serial.print(amt);
         Serial.print(", spd = ");
         Serial.print(spd);
         Serial.print(", rad = ");
         Serial.println(rad);
         valL =  64 + spd;
         valR = 192;
         Serial.print("  Motor values will be: valL = ");
         Serial.print(valL);
         Serial.print(", valR = ");
         Serial.print(valR);
         Serial.println(", turn in degrees at radius");         
         Serial.println();
      }
        
      else if (cmd == 255) {
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
        
        else {
           Serial.println("Not a valid command.  Try again...");
           Serial.println("Valid cmd values are (0, 1, 2, 3, 4, 5, 6 and 255)");
        }
      
    }
  }
  // outside while loop
  mySerial.write(254);   // cursor to beginning of first line
  mySerial.write(128);
  mySerial.write("RIGHT:          "); // clear display + legends
  mySerial.write("LEFT:           ");

  sprintf(valRstring,"%4d",valR); // create strings from the numbers
  sprintf(valLstring,"%4d",valL); // right-justify to 4 spaces

  mySerial.write(254); // cursor to 7th position on first line
  mySerial.write(134);
 
  mySerial.write(valRstring); // write out the valR value

  mySerial.write(254); // cursor to 7th position on second line
  mySerial.write(198);

  mySerial.write(valLstring); // write out the valL value
  
  delay(1000); // short delay
  
}
