#include "Adafruit_ILI9341_Albert.h"
#include <SparkFunMLX90614.h>
#include <Wire.h>
#include <vl53lx_class.h>
#include <Servo.h>                        // Libraries   
#include <VL53L1X.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <ArduinoJson.h>
#include <ArduinoHttpClient.h>
#include <WiFi101.h>

unsigned short detectionZone = 700;              // Set active zone on VL53L1X  [mm]
unsigned short positionSensor = 50;              // Set active zone on VL53L3CX [cm]
unsigned short Max = 75;                         // Set maximum value for servo [deg]
unsigned short Min = 30;                         // Set minimum value for servo [deg]
char ssid[] = "iPhone (Paweł)";                  // Enter wifi name
char pass[] = "iphonePawel";                     // Enter wifi password


char report[64];
const byte TFT_RST = 5, TFT_CS = 6, TFT_DC = 7;
bool detector1, detector2, block = false, hold = false, Stop = false;
int measurer1, measurer2, i = 0, blockade = 0, stoppage = 0, delayed = 0;
int SingleDistance, NumberOfObject, pos, j, Return = 0, stoploop = 0;           // Variables
const char serverAddress[] = "cov-shield.herokuapp.com";
int deviceId = 0, maxPeople = 0, port = 80;
int status = WL_IDLE_STATUS;
String secretKey = "123";
float maxTemperature = 0;


IRTherm therm;
VL53LX VL53L3CX(&Wire, A1);
Servo servo;
VL53L1X sensor1;                                                  // Constructors
VL53L1X sensor2;
Adafruit_ILI9341_Albert tft = Adafruit_ILI9341_Albert(6, 7, 5);
WiFiClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port);

VL53LX_MultiRangingData_t MultiRangingData;
VL53LX_MultiRangingData_t *pMultiRangingData = &MultiRangingData;


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
  tft.setRotation(1);
  tft.setCursor(17, 130);
  tft.println("COVSHIELD");
  delay(2000);
  tft.fillScreen(ILI9341_MAGENTA);     // Display setup
  tft.setCursor(17, 45);
  tft.println("COVSHIELD");
  tft.setCursor(17, 120);
  tft.setTextSize(1);
  tft.println("Number of people:");

  servo.attach(4);    // Servo set input pin
  servo.write(Min);
  pos = Min;

  therm.begin();
  therm.setUnit(TEMP_C);     // Temperature unit setting
  therm.setEmissivity(0.65);

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

  ConnectToWiFi();
  GetConfigurationData();                     //Wifi connection
  Serial.println("Setup finished.");
  SendMeasurement(i);
}



void displayEntries()      // Displaying the current number of persons
{
  tft.setTextSize(3);
  tft.setTextColor(ILI9341_WHITE, ILI9341_MAGENTA);
  tft.setFont();
  tft.setCursor(250, 100);
  tft.print(i);
  if (i == 9)
  {
    tft.setCursor(265, 100);
    tft.print("  ");
  }
}


void temp()     // Temperature check and result of measurment
{
  therm.read();
  Serial.println("Object: " + String(therm.object(), 2) + " C");
  tft.fillScreen(ILI9341_MAGENTA);
  tft.setTextSize(3);
  tft.setCursor(40, 140);
  tft.println(String(therm.object(), 2) + " C");

  if (therm.object() >= 37.5 && therm.object() < 43)
  {
    tft.setTextSize(1);
    tft.setCursor(20, 45);
    tft.println("Measurment succesfull!");
    tft.setCursor(220, 107);
    tft.println("O");
    tft.setCursor(80, 190);
    tft.println("Unfortunately,");
    tft.setCursor(60, 220);
    tft.println("you cannot enter");
    blockade = 3;
  }
  else if (therm.object() < 37.5 && therm.object() >= 34)
  {
    tft.setTextSize(1);
    tft.setCursor(20, 45);
    tft.println("Measurment succesfull!");
    tft.setCursor(220, 107);
    tft.println("O");
    tft.setCursor(85, 210);
    tft.println("Come on in!");
    blockade = 3;
  }
  else
  {
    tft.setTextSize(1);
    tft.setCursor(45, 45);
    tft.println("Measurment failed!");
    tft.setCursor(220, 107);
    tft.println("O");
    tft.setCursor(100, 210);
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
      if (SingleDistance > positionSensor) NumberOfObject = 0;
      if (SingleDistance > detectionZone / 10) NumberOfObject = 999;
    }
    Serial.println("");
    Serial.println(NumberOfObject);
    Serial.println(SingleDistance);
  }
}


void checkTemperature()     // Temperature measurement process
{
  sensor1.read();
  sensor2.read();
  SingleDistance = 0;
  NumberOfObject = 0;
  status = VL53L3CX.VL53LX_ClearInterruptAndStartMeasurement();

  while (sensor1.read() < detectionZone && blockade == 0)
  {
    distance();
    SingleDistance = 0;
    NumberOfObject = 0;
    status = VL53L3CX.VL53LX_ClearInterruptAndStartMeasurement();
    sensor1.read();
    sensor2.read();
    distance();
    while (NumberOfObject != 0 && blockade == 0 && pos <= Max && sensor1.read() < detectionZone && sensor2.read() < detectionZone)
    {
      sensor1.read();
      sensor2.read();
      ++pos;
      servo.write(pos);
      if (pos >= Max) blockade = 1;
      delay(10);
      if (stoploop == 0)
      {
        tft.fillScreen(ILI9341_MAGENTA);
        tft.setCursor(15, 40);
        tft.println("Wait, he's aiming at your");
        tft.setCursor(100, 70);
        tft.println("forehead...");
      }
      stoploop = 1;
      SingleDistance = 0;
      NumberOfObject = 0;
      status = VL53L3CX.VL53LX_ClearInterruptAndStartMeasurement();
      distance();
      if (NumberOfObject == 0 && pos < Max)
      {
        blockade = 1;
        while (Return <= 15 && pos > Min && pos <= Max && sensor1.read() < detectionZone && sensor2.read() < detectionZone)
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

    while ((NumberOfObject == 0 || 999) && blockade == 0 && pos >= Min && sensor1.read() < detectionZone && sensor2.read() < detectionZone)
    {
      sensor1.read();
      sensor2.read();
      --pos;
      servo.write(pos);
      if (pos <= Min) blockade = 2;
      delay(10);
      if (stoploop == 0)
      {
        tft.fillScreen(ILI9341_MAGENTA);
        tft.setCursor(15, 40);
        tft.println("Wait, he's aiming at your");
        tft.setCursor(100, 70);
        tft.println("forehead...");
      }
      stoploop = 1;
      SingleDistance = 0;
      NumberOfObject = 0;
      status = VL53L3CX.VL53LX_ClearInterruptAndStartMeasurement();
      distance();
      if (NumberOfObject != 0 && pos > Min)
      {
        blockade = 2;
        while (Return <= 15 && pos <= Max && pos > Min && sensor1.read() < detectionZone && sensor2.read() < detectionZone)
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
    stoploop = 0;

    while (blockade == 1 || blockade == 2)
    {
      if (stoploop == 0)
      {
        tft.setCursor(45, 140);
        tft.println("Bring your forehead");
        tft.setCursor(60, 170);
        tft.println("10 cm away from");
        tft.setCursor(95, 200);
        tft.println("the sensor");
      }
      stoploop = 1;
      SingleDistance = 0;
      NumberOfObject = 0;
      status = VL53L3CX.VL53LX_ClearInterruptAndStartMeasurement();
      distance();
      if (SingleDistance >= 9 && SingleDistance <= 11) temp();
      sensor1.read();
      sensor2.read();
      while (sensor1.read() < detectionZone && sensor2.read() < detectionZone && blockade == 3)
      {
        sensor1.read();       // Wait and display
        sensor2.read();
        if (sensor1.read() > detectionZone && sensor2.read() < detectionZone)
        {
          ++i;
          Stop = true;
        }
      }
      while (sensor1.read() < detectionZone && sensor2.read() < detectionZone && blockade == 0 && stoppage < 30)
      {
        sensor1.read();
        sensor2.read();
        ++stoppage;
      }
      stoppage = 0;
      if (sensor1.read() > detectionZone) blockade = 4;
    }
    stoploop = 0;

    SingleDistance = 0;
    NumberOfObject = 0;
    status = VL53L3CX.VL53LX_ClearInterruptAndStartMeasurement();
    sensor1.read();
    sensor2.read();

  }
  if (blockade == 1 || blockade == 2 || blockade == 3 || blockade == 4) tft.fillScreen(ILI9341_MAGENTA);
  blockade = 0;
  tft.setCursor(17, 45);
  tft.setTextSize(2);
  tft.println("COVSHIELD");
  tft.setCursor(17, 120);
  tft.setTextSize(1);
  tft.println("Number of people:");
}


void loop()
{
  sensor1.read();
  if (sensor1.read() < detectionZone) measurer1 = 1;
  else measurer1 = 0;

  sensor2.read();
  if (sensor2.read() < detectionZone) measurer2 = 1;
  else measurer2 = 0;


  if (measurer1 == 1 && measurer2 == 0)
  {
    if (block == false)
    {
      tft.setFont(&FreeSansBold12pt7b);
      tft.setTextSize(1);
      tft.setCursor(40, 170);
      tft.println("Stop and check your");
      tft.setCursor(90, 200);
      tft.println("temperature");
      hold = true;
    }
    if (detector2 == true)
    {
      if ( i > 0)
      {
        --i;
        SendMeasurement(i);
      }
      detector1 = false;
      detector2 = false;
      Serial.println(i);
    }
    else detector1 = true;
  }
  else if (measurer1 == 1 && measurer2 == 1)
  {
    if (block == false) ++delayed;
    if (delayed == 20)
    {
      tft.setFont(&FreeSansBold12pt7b);
      tft.setTextSize(1);
      checkTemperature();
    }
  }
  else if (measurer1 == 0 && measurer2 == 1)
  {
    if (detector1 == true)
    {
      if (i < 100 && Stop == false)
      {
        ++i;
        SendMeasurement(i);
      }
      Stop = false;
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
    if (hold == true)
    {
      tft.fillRect(0, 140, 320, 100, ILI9341_MAGENTA);
      hold = false;
    }
    delayed = 0;
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



void ConnectToWiFi() {
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
  }
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}

void GetConfigurationData() {
  Serial.println("Getting configuration... ");
  String path = "/api/admin/configuration/" + secretKey;
  client.get(path);
  int statusCode = client.responseStatusCode();
  Serial.print("Status code: ");
  Serial.println(statusCode);
  String response = client.responseBody();
  ParseConfigurationResponse(response);
}

void ParseConfigurationResponse(String response) {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(response);
  deviceId = root["id"];
  maxPeople = root["maxPeople"];
  maxTemperature = root["maxTemperature"];
  Serial.println("Configuration downloaded: ");
  Serial.print("Device id: ");
  Serial.println(deviceId);
  Serial.print("Max people: ");
  Serial.println(maxPeople);
  Serial.print("Max temperature: ");
  Serial.println(maxTemperature);
}

void SendMeasurement(int peopleCount) {
  client = HttpClient(wifi, serverAddress, port);
  String object = "{\"deviceId\":\"";
  object += deviceId;
  object += "\",\"peopleCount\":\"";
  object += peopleCount;
  object += "\"}";
  Serial.println("Doing post request ");
  String path = "/api/measurement";
  String contentType = "application/json";
  client.post(path, contentType, object);
  //client.endRequest();
  Serial.println("Finished ");
}
