#include "main.h"

// Software Serial for Modbus RTU, can be used for debugging or other serial communication needs, can be expanded later to include multiple software serial ports or more complex communication handling as needed for robustness in real-world applications
#ifdef CUBE_CELL
softSerial sSerial(GPIO5,GPIO6);
#else
// SoftwareSerial for ESP8266, can be used for debugging or other serial communication needs, can be expanded later to include multiple software serial ports or more complex communication handling as needed for robustness in real-world applications
SoftwareSerial sSerial(D5,D6); // RX, TX pins for software serial, can be adjusted as needed for different hardware configurations or additional serial communication needs in real-world applications
#endif


