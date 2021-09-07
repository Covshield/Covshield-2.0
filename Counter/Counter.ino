#include <SH1106.h>
#include <Wire.h>
#include <VL53L1X.h>
VL53L1X sensor;
VL53L1X sensor2;
int Czujnik1 = 0;
int Czujnik2 = 0;
bool czujnik1 = false;
bool czujnik2 = false;
int i = 0;
uint8_t oled_buf[WIDTH * HEIGHT / 8];

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

  digitalWrite(3, HIGH);
  delay(150);
  sensor2.init();
  Serial.println("01");
  delay(100);
  sensor2.setAddress(0x33);
  digitalWrite(2, HIGH);
  delay(150);
  Serial.println("02");
  sensor.init();
  delay(100);

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
  SH1106_begin();
  SH1106_string(0, 0, "Liczba os:", 16, 1, oled_buf);
  SH1106_display(oled_buf);
}

void displayEntries()
{
  SH1106_char(86, 0, 48 + i / 10, 16, 1 , oled_buf);
  SH1106_char(97, 0, 48 + i % 10, 16, 1 , oled_buf);
  SH1106_display(oled_buf);
}

void loop()
{
  sensor.read();
  if (sensor.read() < 1000)
  {
    Czujnik1 = 1;
  }
  else
  {
    Czujnik1 = 0;
  }
  sensor2.read();
  if (sensor2.read() < 1000)
  {
    Czujnik2 = 1;
  }
  else
  {
    Czujnik2 = 0;
  }

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
