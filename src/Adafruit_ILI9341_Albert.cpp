/* 
Overwrite text and numbers on the ILI9341 TFT
The Adafruit_ILI9341 library cannot print new text over other text correctly. 
The old text is not deleted first as it should.  
With this library extension we can overwrite text and numbers correctly.

info print functions: https://github.com/esp8266/Arduino/blob/master/cores/esp8266/Print.h

7-10-2019 version v1.0.0
20-10-2019 version v1.0.1 also for float
*/

#include "Adafruit_ILI9341_Albert.h"
 
Adafruit_ILI9341_Albert::Adafruit_ILI9341_Albert(int8_t cs, int8_t dc, int8_t rst):
Adafruit_ILI9341(cs, dc, rst)
{  
}

void Adafruit_ILI9341_Albert::printNew(const long value, const int chCount)
{ getNumberBounds(chCount);
  print(value); 
  setCursor(OriginalCursor_x, OriginalCursor_y);  
}

void Adafruit_ILI9341_Albert::printNew(const float value, const unsigned decimals, const int chCount)
{ getNumberBounds(chCount);
  print(value, decimals); 
  setCursor(OriginalCursor_x, OriginalCursor_y);  
}

void Adafruit_ILI9341_Albert::getNumberBounds(const int chCount)
{ OriginalCursor_x = cursor_x; 
  OriginalCursor_y = cursor_y;
  getTextBounds("3",  OriginalCursor_x, OriginalCursor_y, &x, &y, &charWidth, &h); // THIS CODE IS UGLY 
  getTextBounds("33", OriginalCursor_x, OriginalCursor_y, &x, &y, &char2Width, &h); // font of 3 is wider than 0
  int space = char2Width-2*charWidth;
  w = chCount*charWidth + (chCount-1)*space;
  // Serial << x, y, w, h; // 0: 22 64 22 34 // 00: 22 64 48 34
  // textbgcolor = ILI9341_GREEN; // testing with green background
  fillRect(x, y, w, h, textbgcolor); // textbgcolor is protected in Adafruit_GFX.h 
}

void Adafruit_ILI9341_Albert::printNew(const String &newString, const String &oldString) // overloading needs const here
{ OriginalCursor_x = cursor_x;
  OriginalCursor_y = cursor_y;
  getTextBounds(oldString.c_str(), OriginalCursor_x, OriginalCursor_y, &x, &y, &w, &h);
  fillRect(x, y, w, h, textbgcolor); // textbgcolor is protected in Adafruit_GFX.h
  print(newString.c_str()); 
  setCursor(OriginalCursor_x, OriginalCursor_y);   
}
