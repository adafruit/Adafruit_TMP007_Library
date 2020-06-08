/***************************************************
  This is a library for the TMP007 Temp Sensor

  Designed specifically to work with the Adafruit TMP007 Breakout
  ----> https://www.adafruit.com/products/2023

  These displays use I2C to communicate, 2 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include "Adafruit_TMP007.h"

//#define TESTDIE 0x0C78
//#define TESTVOLT 0xFEED

/**************************************************************************/
/*!
    @brief Constructor for TMP007 sensor object
    @param i2caddr The I2C sensor address to use
*/
/**************************************************************************/
Adafruit_TMP007::Adafruit_TMP007(uint8_t i2caddr) {
  i2c_dev = new Adafruit_I2CDevice(i2caddr);
}

/**************************************************************************/
/*!
    @brief Initialize I2C and connect to the TMP007 sensor
    @param samplerate The value written to TMP007_CONFIG
    @returns True if sensor found
*/
/**************************************************************************/
bool Adafruit_TMP007::begin(uint16_t samplerate) {
  if (!i2c_dev || !i2c_dev->begin()) {
    return false;
  }

  Adafruit_BusIO_Register config_reg =
      Adafruit_BusIO_Register(i2c_dev, TMP007_CONFIG, 2, MSBFIRST);
  config_reg.write(TMP007_CFG_RESET);
  delay(10);
  config_reg.write(TMP007_CFG_MODEON | TMP007_CFG_ALERTEN | TMP007_CFG_TRANSC |
                   samplerate);

  Adafruit_BusIO_Register stat_reg =
      Adafruit_BusIO_Register(i2c_dev, TMP007_STATMASK, 2, MSBFIRST);
  stat_reg.write(TMP007_STAT_ALERTEN | TMP007_STAT_CRTEN);
  // enable conversion ready alert

  Adafruit_BusIO_Register did_reg =
      Adafruit_BusIO_Register(i2c_dev, TMP007_DEVID, 2, MSBFIRST);
  uint16_t did = did_reg.read();
#ifdef TMP007_DEBUG
  Serial.print("did = 0x");
  Serial.println(did, HEX);
#endif
  if (did != 0x78)
    return false;

  return true;
}

/**************************************************************************/
/*!
    @brief  Read the calibrated die temperature
    @returns double The calculated temperature of the sensor itself
*/
/**************************************************************************/
double Adafruit_TMP007::readDieTempC(void) {
  double Tdie = readRawDieTemperature();
  Tdie *= 0.03125; // convert to celsius
#ifdef TMP007_DEBUG
  Serial.print("Tdie = ");
  Serial.print(Tdie);
  Serial.println(" C");
#endif
  return Tdie;
}

/**************************************************************************/
/*!
    @brief  Read the calibrated object temperature
    @returns double The calculated temperature of the object in front of sensor
*/
/**************************************************************************/
double Adafruit_TMP007::readObjTempC(void) {
  Adafruit_BusIO_Register tobj_reg =
      Adafruit_BusIO_Register(i2c_dev, TMP007_TOBJ, 2, MSBFIRST);

  int16_t raw = tobj_reg.read();
  // invalid
  if (raw & 0x1)
    return NAN;
  raw >>= 2;

  double Tobj = raw;
  Tobj *= 0.03125; // convert to celsius
#ifdef TMP007_DEBUG
  Serial.print("Tobj = ");
  Serial.print(Tobj);
  Serial.println(" C");
#endif
  return Tobj;
}

/**************************************************************************/
/*!
    @brief  Read the raw, uncalibrated die temperature
    @returns int16_t The raw data read from the TMP007_TDIE register
*/
/**************************************************************************/
int16_t Adafruit_TMP007::readRawDieTemperature(void) {
  Adafruit_BusIO_Register tdie_reg =
      Adafruit_BusIO_Register(i2c_dev, TMP007_TDIE, 2, MSBFIRST);

  int16_t raw = tdie_reg.read();

#if TMP007_DEBUG == 1

#ifdef TESTDIE
  raw = TESTDIE;
#endif

  Serial.print("Raw Tambient: 0x");
  Serial.print(raw, HEX);

  float v = raw / 4;
  v *= 0.03125;
  Serial.print(" (");
  Serial.print(v);
  Serial.println(" *C)");
#endif
  raw >>= 2;
  return raw;
}

/**************************************************************************/
/*!
    @brief  Read the raw, uncalibrated thermopile voltage
    @returns int16_t The raw data read from the TMP007_VOBJ register
*/
/**************************************************************************/
int16_t Adafruit_TMP007::readRawVoltage(void) {
  int16_t raw;

  Adafruit_BusIO_Register vobj_reg =
      Adafruit_BusIO_Register(i2c_dev, TMP007_VOBJ, 2, MSBFIRST);
  raw = vobj_reg.read();

#if TMP007_DEBUG == 1

#ifdef TESTVOLT
  raw = TESTVOLT;
#endif

  Serial.print("Raw voltage: 0x");
  Serial.print(raw, HEX);
  float v = raw;
  v *= 156.25;
  v /= 1000;
  Serial.print(" (");
  Serial.print(v);
  Serial.println(" uV)");
#endif
  return raw;
}
