/* Text Display for the Arduino v2 for SSD1306 or SH1106 OLED modules

   David Johnson-Davies - www.technoblogy.com - 31st August 2016
   Arduino Uno or ATmega328
   
   CC BY 4.0
   Licensed under a Creative Commons Attribution 4.0 International license: 
   http://creativecommons.org/licenses/by/4.0/

   Modified by Jean-Marie Bussat (work in progress 1/4/2017)
    - Added I2C support
    - Added support for some sort of scalable font (x2, x3, x4 original size)
*/

#ifdef I2C_DISPLAY
#include <Wire.h>
#define OLED_address  0x3c
#endif

// Write a data byte to the display
void Data (uint8_t d) {
#ifdef I2C_DISPLAY
  Wire.beginTransmission(OLED_address); 
  Wire.write(0x40);//data mode
  Wire.write(d);   
  Wire.endTransmission();
#else
  PIND = 1<<cs; // cs low
  for (uint8_t bit = 0x80; bit; bit >>= 1) {
    PIND = 1<<clk; // clk low
    if (d & bit) PORTB = PORTB | (1<<data); else PORTB = PORTB & ~(1<<data);
    PIND = 1<<clk; // clk high
  }
  PIND = 1<<cs; // cs high
#endif
}

// Write a command byte to the display
void Command (uint8_t c) {
#ifdef I2C_DISPLAY
  Wire.beginTransmission(OLED_address);     
  Wire.write(0x80);                          
  Wire.write(c);
  Wire.endTransmission(); 
#else  
  PIND = 1<<dc; // dc low
  Data(c);
  PIND = 1<<dc; // dc high
#endif
}

void InitDisplay () {
#ifdef I2C_DISPLAY
#ifdef ESP8266
  Wire.begin(D1,D2); // Nodemcu board specific
#else
  Wire.begin();
#endif  
#else
  // Define pins
  pinMode(dc, OUTPUT); digitalWrite(dc,HIGH);
  pinMode(clk, OUTPUT); digitalWrite(clk,HIGH);
  pinMode(datapin, OUTPUT);
  pinMode(cs, OUTPUT); digitalWrite(cs,HIGH);
#endif
  for (uint8_t c=0; c<InitLen; c++) Command(Init[c]);
  Display(12,1,1);    // Clear display
  Command(0xAF);  // Display on
}

// Character terminal **********************************************

void ClearLine (uint8_t line, uint8_t height) {
  for (uint8_t h = 0 ; h < height ; h++){
    Command(0xB0 + line + h);
    Command(0x00); // Column start low
    Command(0x00); // Column start high
    for (uint8_t b = 0 ; b < 128 + 4*SH1106; b++) {
      Data(0);
    }
  }
}

// Clears the top line, then scrolls the display up by one line
void ScrollDisplay (uint8_t *scroll, uint8_t height) {
  ClearLine(*scroll, height);
  *scroll = (*scroll + height) & 0x07;
  Command(0xD3);
  Command(*scroll << 3);
}

// Plots a character; line = 0 to 7; column = 0 to 20
void PlotChar (char c, uint8_t line, uint8_t column) {
  column = column*6+2*SH1106;
  Command(0xB0 + (line & 0x07));
  Command(0x00 + (column & 0x0F)); // Column start low
  Command(0x10 + (column >> 4));   // Column start high
  for (uint8_t col = 0; col < 6; col++) {
    Data(pgm_read_byte(&CharMap[(c & 0x7F)-32][col]));
  }
}

void PlotCharScaled (char c, uint8_t line, uint8_t column, uint8_t height, uint8_t width) {
  uint8_t cc,mask,h,col,d,dstmask,srcmask,dst,i,j,w;
  
  cc = column*6;
  mask=1;
  for (h = 0 ; h < height ; h++){
    Command(0xB0 + ( (line + h ) & 0x07));
    Command(0x00 + (cc & 0x0F)); // Column start low
    Command(0x10 + (cc >> 4));   // Column start high 
    for (col = 0; col < 6; col++) {
      d=pgm_read_byte(&CharMap[(c & 0x7F)-32][col]); 
      dstmask=1;
      srcmask=mask;
      dst=0;
      for (i=0 ; i < int(8/height) ; i++) {
        if( (d & srcmask) == srcmask) {
          for (j=0 ; j < height ; j++) {
            dst=dst|dstmask;
            dstmask=dstmask<<1;
          }  
        } else {
          dstmask=dstmask<<height;
        }
        srcmask=srcmask<<1;
      }
      for (w=0 ; w < width ; w++) { 
        Data(dst);
      }
    }
    mask=mask<<(int(8/height));
  }
}

// Prints a character to display, handling control characters
void Display(char c, uint8_t height, uint8_t width) {
  static uint8_t Line = 0, Column = 0, Scroll = 0;
  if (c >= 32) {                   // Normal character
    PlotCharScaled(c, Line+Scroll, Column, height, width);
    if ( (Column+width) > 20) {
      Column = 0;
      if ( (Line+height) == 7) {
         ScrollDisplay(&Scroll,height); 
      } else {
        Line=Line+height;
      }
    }
  } else if (c == 12) {            // Clear display
    for (uint8_t p=0; p < 8; p++) ClearLine(p,1);
    Line = Scroll;
    Column = 0;
  } else if (c == 13) {            // Return
    Column = 0;
    if (Line == 7) {
      ScrollDisplay(&Scroll,height);
    } else {
      Line=Line+height;
    }
  } 
}

void DisplayStr (const char *s, uint8_t height, uint8_t width) {
  while (*s) Display(*s++, height, width);
}

void DisplayStrXY (const char *s,int line, int col, uint8_t height, uint8_t width) {
  const char *str=s;
  uint8_t l=line;
  uint8_t c=col;
  
  while (*str){
    if(c<20){
      PlotCharScaled(*str++,l,c,height,width);
      c=c+width;
    } else {
      c=0;
      l=l+height;
      PlotCharScaled(*str++,l,c,height,width);
      c=c+width;
    }
  }
}
