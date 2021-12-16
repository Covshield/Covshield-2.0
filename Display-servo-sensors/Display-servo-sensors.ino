#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>
#include <SparkFunMLX90614.h>
#include <Wire.h>
#include <vl53lx_class.h>                 // Libraries   
#include <Servo.h>
#include <VL53L1X.h>
#include <Fonts/FreeSansBold12pt7b.h>


IRTherm therm;
VL53LX VL53L3CX(&Wire, A1);
Servo servo;
VL53L1X sensor1;                                               // Constructors
VL53L1X sensor2;
Adafruit_ILI9341 tft = Adafruit_ILI9341(6, 7, 8, 9, 5, 10);


VL53LX_MultiRangingData_t MultiRangingData;
VL53LX_MultiRangingData_t *pMultiRangingData = &MultiRangingData;


char report[64];
bool block = false;
bool detector1 = false;
bool detector2 = false;
int i = 0;                                                                               // Variables
int measurer1 = 0;
int measurer2 = 0;
int  SingleDistance, NumberOfObject, status, j, blockade = 0, pos = 0, Return = 0;


void setup()
{
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  digitalWrite(2, LOW);     // Pin setting
  digitalWrite(3, LOW);

  Wire.begin();
  Wire.beginTransmission(0x29);     // SPI adress set

  Serial.begin (9600);     // Serial port setting

  tft.begin();
  tft.fillScreen(ILI9341_MAGENTA);
  tft.setFont(&FreeSansBold12pt7b);
  tft.setTextSize(2);
  tft.setRotation(3);
  tft.setCursor(17, 130);
  tft.println("COVSHIELD");
  delay(3000);
  tft.fillScreen(ILI9341_MAGENTA);     // Display setup
  tft.setCursor(17, 45);
  tft.println("COVSHIELD");
  tft.setCursor(17, 120);
  tft.setTextSize(1);
  tft.println("Number of people:");

  servo.attach(4);    // Servo set input pin

  therm.begin();
  therm.setUnit(TEMP_C);     // Temperature unit setting

  VL53L3CX.InitSensor(0x12);
  VL53L3CX.VL53LX_StartMeasurement();
  digitalWrite(3, HIGH);
  sensor2.init();
  sensor2.setAddress(0x33);
  digitalWrite(2, HIGH);
  sensor1.init();
  sensor1.setROISize(4, 4);
  sensor1.setDistanceMode(VL53L1X::Long);
  sensor1.setMeasurementTimingBudget(35000);     // TOF sensor setting
  sensor1.startContinuous(15);
  sensor1.setTimeout(35);
  sensor2.setROISize(4, 4);
  sensor2.setDistanceMode(VL53L1X::Long);
  sensor2.setMeasurementTimingBudget(35000);
  sensor2.startContinuous(15);
  sensor2.setTimeout(35);
}



void displayEntries()      // Displaying the current number of persons
{
  if (i = 9)
  {
    tft.setCursor(265, 30);
    tft.println("  ");
  }
  else
  {
    tft.setCursor(250, 30);
    tft.setTextSize(3);
    tft.setRotation(3);
    tft.setTextColor(ILI9341_WHITE, ILI9341_MAGENTA);
    tft.println(i);
  }
}


void temp()     // Temperature check and result of measurment
{
  therm.read();
  Serial.println("Object: " + String(therm.object(), 2) + " C");
  tft.println(String(therm.object(), 2) + " C");
  if (therm.object() >= 37 && therm.object() < 43)
  {
    tft.fillScreen(ILI9341_MAGENTA);
    tft.setCursor(20, 120);
    tft.println("Measurment succesfull!");
    tft.setCursor(20, 150);
    tft.println("Unfortunately, you cannot enter");
    blockade = 3;
  }
  else if (therm.object() < 37 && therm.object() >= 34)
  {
    tft.fillScreen(ILI9341_MAGENTA);
    tft.setCursor(20, 120);
    tft.println("Measurment succesfull!");
    tft.setCursor(20, 150);
    tft.println("Come on in!");
    blockade = 3;
  }
  else
  {
    tft.fillScreen(ILI9341_MAGENTA);
    tft.setCursor(20, 120);
    tft.println("Measurment failed!");
    tft.setCursor(20, 150);
    tft.println("Try again");
    blockade = 0;
  }
}


void distance() // person position sensor
{
  int no_of_object_found = 0;
  uint8_t NewDataReady = 0;
  do
  {
    status = VL53L3CX.VL53LX_GetMeasurementDataReady(&NewDataReady);
  } while (!NewDataReady);
  if ((!status) && (NewDataReady != 0))
  {
    status = VL53L3CX.VL53LX_GetMultiRangingData(pMultiRangingData);
    no_of_object_found = pMultiRangingData->NumberOfObjectsFound;
    snprintf(report, sizeof(report), "VL53LX Satellite: Count=%d, #Objs=%1d ", pMultiRangingData->StreamCount, no_of_object_found);
    Serial.print(report);
    for (j = 0; j < no_of_object_found; j++)
    {
      if (j != 0)Serial.print("\r\n                                     ");
      Serial.print("status=" + String(pMultiRangingData->RangeData[j].RangeStatus));
      Serial.print(", D=" + String(pMultiRangingData->RangeData[j].RangeMilliMeter / 10) + "cm");
      if (j <= 1) SingleDistance = pMultiRangingData->RangeData[0].RangeMilliMeter / 10;
      NumberOfObject = no_of_object_found;
      if (SingleDistance > 50) NumberOfObject = 0;
    }
    Serial.println("");
    Serial.println(NumberOfObject);
    Serial.println(SingleDistance);
  }
  delay(50);
}


void checkTemperature()     // Temperature measurement process
{
  sensor1.read();
  sensor2.read();
  tft.fillScreen(ILI9341_MAGENTA);
  tft.setCursor(17, 100);
  tft.println("Stop and check your temperature");
  while (sensor1.read() < 500 && blockade == 0)
  {
    distance();
    while (NumberOfObject != 0 && blockade == 0 && pos <= 90 && sensor1.read() < 500 && sensor2.read() < 500)
    {
      sensor1.read();
      sensor2.read();
      ++pos;
      servo.write(pos);
      delay(10);
      tft.setCursor(17, 120);
      tft.println("Wait, he's aiming at your forehead...");
      SingleDistance = 0;
      NumberOfObject = 0;
      status = VL53L3CX.VL53LX_ClearInterruptAndStartMeasurement();
      distance();
      if (NumberOfObject == 0)
      {
        blockade = 1;
        while (Return <= 15 && pos > 0 && sensor1.read() < 500 && sensor2.read() < 500)
        {
          ++Return;
          --pos;
          sensor1.read();
          sensor2.read();
          delay(30);
          servo.write(pos);
        }
        Return = 0;
      }
    }

    while (NumberOfObject == 0 && blockade == 0 && pos >= 0 && sensor1.read() < 500 && sensor2.read() < 500)
    {
      sensor1.read();
      sensor2.read();
      --pos;
      servo.write(pos);
      delay(10);
      tft.setCursor(17, 120);
      tft.println("Wait, he's aiming at your forehead...");
      SingleDistance = 0;
      NumberOfObject = 0;
      status = VL53L3CX.VL53LX_ClearInterruptAndStartMeasurement();
      distance();
      if (NumberOfObject != 0)
      {
        blockade = 2;
        while (Return <= 15 && pos <= 90 && sensor1.read() < 500 && sensor2.read() < 500)
        {
          ++Return;
          --pos;
          sensor1.read();
          sensor2.read();
          delay(30);
          servo.write(pos);
        }
        Return = 0;
      }
    }

    while (blockade == 1 || blockade == 2)
    {
      tft.fillScreen(ILI9341_MAGENTA);
      tft.setCursor(17, 120);
      tft.println("Bring your forehead 10 cm away from the sensor");
      SingleDistance = 0;
      NumberOfObject = 0;
      status = VL53L3CX.VL53LX_ClearInterruptAndStartMeasurement();
      distance();
      if (SingleDistance < 9) //tft.println("Move away from the sensor");
      if (SingleDistance > 11) //tft.println("Move closer to the sensor");

      if (SingleDistance >= 9 && SingleDistance <= 11) temp();
      sensor1.read();
      sensor2.read();
      while (sensor1.read() < 500 && sensor1.read() < 500 && blockade == 3)
      {
        sensor1.read();       // Wait and display
        sensor2.read();
      }
      if (sensor1.read() > 500) blockade = 4;
    }

    SingleDistance = 0;
    NumberOfObject = 0;
    status = VL53L3CX.VL53LX_ClearInterruptAndStartMeasurement();
    sensor1.read();
    sensor2.read();
  }
}


void loop()
{
  sensor1.read();
  if (sensor1.read() < 500) measurer1 = 1;
  else measurer1 = 0;

  sensor2.read();
  if (sensor2.read() < 500) measurer2 = 1;
  else measurer2 = 0;


  if (measurer1 == 1 && measurer2 == 0)
  {
    if (block == false)
    {
      checkTemperature();
      blockade = 0;
    }

    if (detector2 == true)
    {
      if ( i > 0) --i;
      detector1 = false;
      detector2 = false;
      Serial.println(i);
    }
    else detector1 = true;
  }
  else if (measurer1 == 0 && measurer2 == 1)
  {
    if (detector1 == true)
    {
      if (i < 100) ++i;

      detector1 = false;
      detector2 = false;
      Serial.println(i);
    }
    else
    {
      block = true;
      detector2 = true;
    }
  }
  else if (measurer1 == 0 && measurer2 == 0)
  {
    block = false;
    detector1 = false;
    detector2 = false;
  }


  displayEntries();
  Serial.print("Sensor1: ");
  Serial.print(measurer1);
  Serial.print("   Sensor2: ");
  Serial.println(measurer2);
}
