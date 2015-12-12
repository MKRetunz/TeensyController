/*
 * ControllerTeensy 
 * We use the Nunchuk to move the mouse
 * We compute the RELATIVE mouse position, and use smoothing and dead zone to correct for sensor inaccuracy
 *
 * THIS VERSION DOES NOT SEEM TO WORK WITH ORIGINAL NUNCHUKS
 *
 * Uses Nunchuk code based on http://blog.oscarliang.net/wii-nunchuck-arduino-tutorial/
 *
 * See http://www.pjrc.com/teensy/td_libs_Wire.html for the Wire (I2C) library:
 * The SCL line is on pin 0 of the Teensy 2.0++ (pin 5 on Teensy2.0) and is conected to the "c" (last) on the Wii adapter
 * The SDA line is on pin 1 of the Teensy 2.0++ (pin 6 on Teensy2.0) and is connected to the "d" (next to last) on the Wii adapter
 * Both SDA and SCL are also connected to the +5v via a couple of 1k8 resistors. (This really helps!)
 * The power and ground pins of the Wii Adapter are connected to +5V and ground respectively.
 *
 * Author: Gidi van Liempd (gidi@geedesign.com)
 * Version: 3.0 (sep 2014)
 *
 *
 * Author: Martijn Klok (martijnklok@hotmail.nl)
 * Version: 1.9 (dec 2015)
 *
 * Mouse now instantly stops when not moving instead of slowing down.
 * Slight clean up
 *
*/
#include <Wire.h>
#include "nunchuck.h"

#define LED_PIN PIN_D6      // Use the built-in LED to signal USB ready

#define MAX_MOVE_X (60)        // maximum mouse movement per move
#define MAX_MOVE_Y (40)        // maximum mouse movement per move

#define MIN_SENSOR1 (4)    // min sensor value, depends on the individual nunchuk
#define MAX_SENSOR1 (221)   // max sensor value, depends on the individual nunchuk 
#define MIN_SENSOR2 (24)
#define MAX_SENSOR2 (243)
#define DEAD_X (10)
#define DEAD_Y (10)
#define ALPHA (0.5)

int prev_delta_x, prev_delta_y;  // used for smoothing the movements

boolean LBpressed = false;

void setup(){
  digitalWrite( LED_PIN, HIGH);  // switch on LED
  delay(10000);                  // If I don't build in a delay, the USB device is not always recognized properly
  digitalWrite( LED_PIN, LOW);   // switch off LED to signal USB is ready 
  Serial.begin(38400);        // not needed, we use the USB connection for mouse move
  
  NunC_Init(0); // initialize nunchuck
}

void loop() {
  int diff_x, diff_y;     // difference between new and recorded position
  float delta_x, delta_y; // difference used for smoothing 
  byte zbut;
  
  if (NunC_ReceiveCommand())
  {
    diff_x = ComputeNewX();
    diff_y = ComputeNewY();

    // dead zone
    if (abs(diff_x) < DEAD_X) diff_x = 0;
    if (abs(diff_y) < DEAD_Y) diff_y = 0;
    // smoothing
    delta_x = (ALPHA * diff_x) + (1 - ALPHA);
    delta_y = (ALPHA * diff_y) + (1 - ALPHA);
  
    zbut = NunC_BZ;  // use the state of the Z-button
    if ((zbut == 0) && !LBpressed) { // left button press
      LBpressed = true;
      Mouse.click();
      Mouse.set_buttons(1, 0, 0);
    };
    if (LBpressed && (zbut == 1)) { // left button released
      LBpressed = false;
      Mouse.set_buttons(0, 0, 0);    
    };
    
    // do the actual mouse movement
    Mouse.move(delta_x, delta_y);  
    // update the recorded difference
    prev_delta_x = delta_x;
    prev_delta_y = delta_y;
  }
    
  delay(50);
}

// compute virtual screen coordinate from sensor reading
int ComputeNewX(void) {
  int sensor_value;
  int result;
  
  sensor_value = NunC_AX; 
  Serial.print("\tSENSOR1\t");
  Serial.print(sensor_value);
  
  result = map(sensor_value, MIN_SENSOR1, MAX_SENSOR1, -MAX_MOVE_X, MAX_MOVE_X);
  result = constrain( result, -MAX_MOVE_X, MAX_MOVE_X);
  return result;
}  

// compute virtual screen coordinate from sensor reading
int ComputeNewY(void) {
  int sensor_value;
  int result;
  
  sensor_value = NunC_AY; 
  Serial.print("\tSENSOR2\t");
  Serial.println(sensor_value);
  
  result = map(sensor_value, MIN_SENSOR2, MAX_SENSOR2, -MAX_MOVE_Y, MAX_MOVE_Y);
  result = constrain( result, -MAX_MOVE_Y, MAX_MOVE_Y);
  return result;
}  


