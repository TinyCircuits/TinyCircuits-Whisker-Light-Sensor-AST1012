/*************************************************************************
 * TSL2572 Ambient Light Sensor Wireling Tutorial:
 * This program will print the lux value read from the sensor to  the 
 * Serial Monitor
 * 
 * Hardware by: TinyCircuits
 * Code by: Ken Burns & Laveréna Wienclaw for TinyCircuits
 *
 * Initiated: 11/29/2017 
 * Updated: 12/04/2019
 ************************************************************************/

#include <Wire.h>         // For I2C communication with sensor
#include <Wireling.h>     // For interfacing with Wirelings

// Communication address with the sensor
#define TSL2572_I2CADDR     0x39

// Sets the gain
#define   GAIN_1X 0
#define   GAIN_8X 1
#define  GAIN_16X 2
#define GAIN_120X 3

//only use this with 1x and 8x gain settings
#define GAIN_DIVIDE_6 true

// Global variable for gain value used to Read the sensor
int gain_val = 0;

// Make Serial Monitor compatible for all TinyCircuits processors
#if defined(ARDUINO_ARCH_AVR)
  #define SerialMonitorInterface Serial
#elif defined(ARDUINO_ARCH_SAMD)
  #define SerialMonitorInterface SerialUSB
#endif

void setup() {
  SerialMonitorInterface.begin(115200);
  Wire.begin();

  // Initialize Wireling
  Wireling.begin();
  Wireling.selectPort(0); // Port #'s correspond to backside of the Adapter TinyShield

//***************************************
// SETTINGS & ADJUSTMENTS 
//***************************************
//  TSL2572Init(GAIN_1X);
//  TSL2572Init(GAIN_8X);
  TSL2572Init(GAIN_16X);
  //TSL2572Init(GAIN_120X);
}

void loop() {
  SerialMonitorInterface.begin(9600);
  float AmbientLightLux = Tsl2572ReadAmbientLight();

  // Print lux value to Serial Monitor
  SerialMonitorInterface.print("Lux: ");
  SerialMonitorInterface.println(AmbientLightLux);
}

// Used to interface with the sensor by writing to its registers directly 
void Tsl2572RegisterWrite(byte regAddr, byte regData) {
  Wire.beginTransmission(TSL2572_I2CADDR);
  Wire.write(0x80 | regAddr);
  Wire.write(regData);
  Wire.endTransmission();
}

// Initializes the light sensor to be ready for output
void TSL2572Init(uint8_t gain) {
  Tsl2572RegisterWrite( 0x0F, gain );//set gain
  Tsl2572RegisterWrite( 0x01, 0xED );//51.87 ms
  Tsl2572RegisterWrite( 0x00, 0x03 );//turn on
  if (GAIN_DIVIDE_6)
    Tsl2572RegisterWrite( 0x0D, 0x04 );//scale gain by 0.16
  if (gain == GAIN_1X)gain_val = 1;
  else if (gain == GAIN_8X)gain_val = 8;
  else if (gain == GAIN_16X)gain_val = 16;
  else if (gain == GAIN_120X)gain_val = 120;
}

// Read the lux value from the light sensor so we can print it out
float Tsl2572ReadAmbientLight() {
  uint8_t data[4];
  int c0, c1;
  float lux1, lux2, cpl;

  Wire.beginTransmission(TSL2572_I2CADDR);
  Wire.write(0xA0 | 0x14);
  Wire.endTransmission();
  Wire.requestFrom(TSL2572_I2CADDR, 4);
  for (uint8_t i = 0; i < 4; i++)
    data[i] = Wire.read();

  c0 = data[1] << 8 | data[0];
  c1 = data[3] << 8 | data[2];

  //see TSL2572 datasheet: https://www.mouser.com/ds/2/588/TSL2672_Datasheet_EN_v1-255424.pdf
  cpl = 51.87 * (float)gain_val / 60.0;
  if (GAIN_DIVIDE_6) cpl /= 6.0;
  lux1 = ((float)c0 - (1.87 * (float)c1)) / cpl;
  lux2 = ((0.63 * (float)c0) - (float)c1) / cpl;
  cpl = max(lux1, lux2);
  return max(cpl, 0.0);
}
