#include "NowireOS.h"


// Basic parameters
bool trigger=0;
unsigned long lastMillis = millis();
unsigned long timestep = MINTSTEP*1000;

void setup() {
  
  // Initialize serial port
  DEBUG_SERIAL.begin(DBG_SPEED);

  initPixel();
  jsonClear();
  mqttInit();
}

void loop() {

  uint32_t tnow=millis();

  alivePixel();
  mqttPoll();
  checkReboot();

  // Checks devices poll...
  if (tnow - lastMillis < timestep && !trigger) return;
  
  // publish a message roughly every TIMESTEP
  trigger=false;
  lastMillis = tnow;

  // launch data -- one message for each datatype whith some delay between
  sendModbus();
  sendStatus();

}
