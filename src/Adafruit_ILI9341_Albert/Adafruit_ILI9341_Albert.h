#ifndef Adafruit_ILI9341_Albert_H
#define Adafruit_ILI9341_Albert_H

#include "Adafruit_ILI9341.h" // https://github.com/adafruit/Adafruit_ILI9341

class Adafruit_ILI9341_Albert: public Adafruit_ILI9341
{ 
public: 
  Adafruit_ILI9341_Albert(int8_t cs, int8_t dc, int8_t rst);
  void printNew(const long value, const int chCount);
  void printNew(const float value, const unsigned decimals, const int chCount);
  void printNew(const String &newString, const String &oldString); 

protected:
  void getNumberBounds(const int chCount);
  int16_t x, y, OriginalCursor_x, OriginalCursor_y;
  uint16_t w, h, charWidth, char2Width;
};
#endif
