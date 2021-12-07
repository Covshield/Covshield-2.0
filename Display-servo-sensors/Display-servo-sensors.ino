#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>
#include <SparkFunMLX90614.h>
#include <Wire.h>
#include <vl53lx_class.h>
#include <Servo.h>
#include <VL53L1X.h>
IRTherm therm;
VL53LX VL53L3CX(&Wire, A1);
Servo servo;
VL53L1X sensor1;
VL53L1X sensor2;
int measurer1 = 0;
int measurer2 = 0;
bool detector1 = false;
bool detector2 = false;
int i = 0;
bool block = false;
Adafruit_ILI9341 tft = Adafruit_ILI9341(6, 7, 8, 9, 5, 10);
char report[64];
int  SingleDistance, NumberOfObject, status, j, blockade = 0, pos = 0, Return = 0;
VL53LX_MultiRangingData_t MultiRangingData;
VL53LX_MultiRangingData_t *pMultiRangingData = &MultiRangingData;

void setup()
{
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  delay(500);
  Wire.begin();
  Wire.beginTransmission(0x29);
  Serial.begin (9600);
  tft.begin();
  tft.fillScreen(ILI9341_MAGENTA);
  tft.setCursor(30, 30);
  tft.setTextSize(3);
  tft.setRotation(3);
  tft.println("Liczba osob:");
  servo.attach(4);
  therm.begin();
  therm.setUnit(TEMP_C);
  VL53L3CX.InitSensor(0x12);
  VL53L3CX.VL53LX_StartMeasurement();

  digitalWrite(3, HIGH);
  sensor2.init();
  sensor2.setAddress(0x33);
  digitalWrite(2, HIGH);
  sensor1.init();
  sensor1.setROISize(4, 4);
  sensor1.setDistanceMode(VL53L1X::Long);
  sensor1.setMeasurementTimingBudget(35000);
  sensor1.startContinuous(15);
  sensor1.setTimeout(35);
  sensor2.setROISize(4, 4);
  sensor2.setDistanceMode(VL53L1X::Long);
  sensor2.setMeasurementTimingBudget(35000);
  sensor2.startContinuous(15);
  sensor2.setTimeout(35);
}

void displayEntries()
{
  tft.setCursor(250, 30);
  tft.setTextSize(3);
  tft.setRotation(3);
  tft.setTextColor(ILI9341_WHITE, ILI9341_MAGENTA);
  tft.println(i);
}

void temp()
{
  therm.read();
  Serial.println("Object: " + String(therm.object(), 2) + " C");
  //tft.println(String(therm.object(), 2) + " C");
  if (therm.object() >= 37 && therm.object() < 43)
  {
    //tft.println("The measurement is successful"); orange
    //tft.println("You should go out!");    red
    blockade = 3;
  }
  else if (therm.object() < 37 && therm.object() >= 34)
  {
    //tft.println("The measurement is successful");  orange
    //tft.println("You can go inside");   green
    blockade = 3;
  }
  else
  {
    //tft.println("The measurement is failed!");   red
    //tft.println("You have to do it again");     orange
    blockade = 0;
  }
}

void distance()
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

void checkTemperature()
{
  sensor1.read();
  sensor2.read();
  //tft.println("Stop and check your temperature");
  while (sensor1.read() < 500 && sensor2.read() < 500 && blockade == 0 ||
         sensor1.read() < 500 && sensor2.read() > 500 && blockade == 0)
  {
    Serial.println("Jest");
    distance();
    while (NumberOfObject != 0 && blockade == 0 && pos <= 90 && sensor1.read() < 500 && sensor2.read() < 500)
    {
      sensor1.read();
      sensor2.read();
      ++pos;
      servo.write(pos);
      delay(10);
      Serial.println("Silnik obraca w górę  ");
      //tft.println("Wait for the setting of the pyrometer...");
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
          Serial.println(sensor1.read());
          sensor2.read();
          Serial.println(sensor2.read());
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
      delay(20);
      Serial.println("Silnik obraca w dół  ");
      //tft.println("Wait for the setting of the pyrometer...");
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
          Serial.println(sensor1.read());
          sensor2.read();
          Serial.println(sensor2.read());
          delay(30);
          servo.write(pos);
        }
        Return = 0;
      }
    }

    while (blockade == 1 || blockade == 2)
    {
      //tft.println("Bring your forehead 10 cm away from the sensor");
      SingleDistance = 0;
      NumberOfObject = 0;
      status = VL53L3CX.VL53LX_ClearInterruptAndStartMeasurement();
      distance();
      if (SingleDistance < 9) //tft.println("Move away from the sensor");
        if (SingleDistance > 11) //tft.println("Move closer to the sensor");
          if (SingleDistance >= 9 && SingleDistance <= 11) temp();
      sensor1.read();
      sensor2.read();
      if (blockade == 3)
      {
        while (sensor1.read() < 500 && sensor1.read() < 500)
        {
          sensor1.read();   // Wait and display
          sensor2.read();
        }
      }
      else
      {
        if (sensor1.read() > 500 ) blockade = 4;
      }
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
  if (sensor1.read() < 250) measurer1 = 1;
  else measurer1 = 0;

  sensor2.read();
  if (sensor2.read() < 250) measurer2 = 1;
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
