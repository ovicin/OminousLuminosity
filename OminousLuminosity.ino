#include <Stdio.h>

#define LED 13                   //pin for the LED
#define SWITCH 0                 //input for REED SWITCH
int rim = 2170;                  //circumference in mm

int val = 0;                     // used to store input value
int previousVal = 0;             // lets not be too repetitious

int debounce = 10;               // the debounce time, increase if the output flickers
int cycles = 1;                  // total number of revolutions
float currentSpeed = 0;          // current speed in MPH
float averageSpeed = 0;          // average speed since "newRide" was true

unsigned long revTimer;          // create a timer that tells us how long we go between pulses,
unsigned long serialTimer;       // and one for how long it's been since we sent anything over serial
unsigned long rideTimer;         // total time since "newRide" was true

boolean activeRiding = false;    // is the bike moving?
boolean activityChange = true;   // just a way of keeping track of how long we're inactive
long inactivityTimer;            // millis() on which we began inactivity
long activeRidingOffset = 0;     // time subtracted from total riding time when calculating average speed
boolean newRide = true;          // true if we haven't moved in half an hour or more
float rideDistance = 0;          // total distance traveled since "newRide" was true

void setup() {
 pinMode(LED, OUTPUT);           // tell arduino LED port is an output,
 pinMode(SWITCH, INPUT);         // and SWITCH port is input
 Serial.begin(9600);             // start a serial session
 revTimer = millis();            // start pulse timer
 serialTimer = millis();         // start serial timer
 rideTimer = millis();           // start ride timer
}

void loop(){
 if(!activeRiding) {
   if(activityChange) {
     inactivityTimer = millis();
     activityChange = false;
   }
 } else {
   if(activityChange) {
     activeRidingOffset += millis() - inactivityTimer;
     activityChange = false;
   }
 }
  
 val=digitalRead(SWITCH);         // read input value and store it

 if (val==HIGH) {                 // check whether input is HIGH (magnet is NOT in range of reed switch)
   digitalWrite(LED, HIGH);       // turn LED on
   previousVal = HIGH;            // allow the next "pulse" to be counted
 } else{
   digitalWrite(LED, LOW);        // turn LED off
   if (previousVal != LOW && millis() - revTimer > debounce) { // we've got a pulse!
     pulse(); 
   } 
   previousVal = LOW;             // (in case the magnet is in range of the switch while sitting still)
 }
 
 // if it's been too long since the last pulse, assume we're not moving.
 if(millis()-revTimer > 2000) { 
  currentSpeed = 0;  
  if(activeRiding) { activityChange = true; activeRiding = false; }
 }
 
 // if it's been more than fifteen minutes...
 if (millis() - revTimer > 15*60*1000) {   
   // we'll assume it's a new riding session & zero everything out at next pulse.
   newRide = true;                         
 }
}


void pulse() {
  if(newRide) {
    cycles = 0;
    averageSpeed = 0;
    rideTimer = millis();
    rideDistance = 0;
  }
  
  cycles++;                                             // The wheel has obviously turned another revolution
  
  rideDistance = (float) rim*cycles/1000;               // distance in meters, 
  rideDistance = rideDistance * 0.000621371192;         // converted to miles
  
  currentSpeed = (float) (millis() - revTimer)*0.001;   // Convert time elapsed to milliseconds to seconds
  currentSpeed = rim/currentSpeed;                      // S = d/t: Rim circumference divided by time elapsed
  currentSpeed = currentSpeed*0.002237;                 // MPH Conversion: 1 mm/s = 0.001 m/s * 3600 s/hr * 1 mile / 1609 m = 0.002237 mi/hr

  // time ridden since "newRide", in hours, not including inactive time
  unsigned long activeRidingMillis = millis() - rideTimer - activeRidingOffset;  
  float activeRidingHours = (float) activeRidingMillis / 1000/60/60; // convert to hours
  averageSpeed = rideDistance / activeRidingHours;

  revTimer = millis();       // remember the current moment for speed calculations next time around
  sendStats();               // Tell Processing what's going on
  newRide = false;

  if(!activeRiding) { activityChange = true; activeRiding = true; }
}

void sendStats() {
                                                        
  Serial.print("1");
  /*Serial.print(rideDistance,2);

  Serial.print("&currentSpeed=");
  Serial.print(currentSpeed,2);

  Serial.print("&averageSpeed=");
  Serial.print(averageSpeed,2);
  
  // send a linefeed character, telling Processing that we're done transmitting.
  Serial.write(10);*/     
  
  serialTimer = millis();
}
