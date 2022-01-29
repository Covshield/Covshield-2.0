#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>
#include <Wire.h>
#include <VL53L1X.h>
VL53L1X sensor;
VL53L1X sensor2;
int Czujnik1 = 0;
int Czujnik2 = 0;
bool czujnik1 = false;
bool czujnik2 = false;
int i = 0;

Adafruit_ILI9341 tft = Adafruit_ILI9341(6, 7, 8, 9, 5, 10);


void setup()
{
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);

  Wire.begin();
  Wire.beginTransmission(0x29);
  Serial.begin(9600);
  tft.begin();
  tft.fillScreen(ILI9341_MAGENTA);
  tft.setCursor(30, 30);
  tft.setTextSize(3);
  tft.setRotation(1);
  tft.println("Liczba osob:");

  digitalWrite(3, HIGH);
  sensor2.init();
  Serial.println("01");
  sensor2.setAddress(0x33);
  digitalWrite(2, HIGH);
  Serial.println("02");
  sensor.init();

  sensor.setROISize(4, 4);
  sensor.setDistanceMode(VL53L1X::Long);
  sensor.setMeasurementTimingBudget(35000);
  sensor.startContinuous(15);
  sensor.setTimeout(35);
  sensor2.setROISize(4, 4);
  sensor2.setDistanceMode(VL53L1X::Long);
  sensor2.setMeasurementTimingBudget(35000);
  sensor2.startContinuous(15);
  sensor2.setTimeout(35);

  delay(150);
  Serial.println("addresses set");
  Serial.println ("I2C scanner. Scanning ...");
  byte count = 0;
  for (byte i = 1; i < 120; i++)
  {
    Wire.beginTransmission (i);
    if (Wire.endTransmission () == 0)
    {
      Serial.print ("Found address: ");
      Serial.print (i, DEC);
      Serial.print (" (0x");
      Serial.print (i, HEX);
      Serial.println (")");
      count++;
      delay (1);
    }
  }
  Serial.println ("Done.");
  Serial.print ("Found ");
  Serial.print (count, DEC);
  Serial.println (" device(s).");
}

void displayEntries()
{
  tft.setCursor(250, 30);
  tft.setTextSize(3);
  tft.setRotation(3);
  tft.setTextColor(ILI9341_WHITE, ILI9341_MAGENTA);
  tft.println(i);
}

void loop()
{
  sensor.read();
  if (sensor.read() < 250) Czujnik1 = 1;
  else Czujnik1 = 0;
  
  sensor2.read();
  if (sensor2.read() < 250) Czujnik2 = 1;
  else Czujnik2 = 0;


  if (Czujnik1 == 1 && Czujnik2 == 0)
  {
    if (czujnik2 == true)
    {
      if ( i > 0)
      {
        --i;
      }
      czujnik1 = false;
      czujnik2 = false;
      Serial.print("Wczesniej wykryty byl czujnik2, inkrementacja i: ");
      Serial.println(i);
    }
    else
    {
      czujnik1 = true;
      Serial.println("Ustawienie flagi czujnika 1");
    }
  }
  else if (Czujnik1 == 0 && Czujnik2 == 1)
  {
    if (czujnik1 == true)
    {
      if (i < 100)
      {
        ++i;
      }
      czujnik1 = false;
      czujnik2 = false;
      Serial.print("Wczesniej wykryty byl czujnik1, dekrementacja i: ");
      Serial.println(i);
    }
    else
    {
      czujnik2 = true;
      Serial.println("Ustawienie flagi czujnika 2");
    }
  }
  else if (Czujnik1 == 0 && Czujnik2 == 0)
  {
    czujnik1 = false;
    czujnik2 = false;
  }

  displayEntries();

  Serial.print("Czujnik1: ");
  Serial.print(Czujnik1);
  Serial.print("   Czujnik2: ");
  Serial.println(Czujnik2);
}
