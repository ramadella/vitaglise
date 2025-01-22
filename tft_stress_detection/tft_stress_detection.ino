#include<TFT_eSPI.h>
#include<SPI.h>
#include"font2.h"
#include "kolesterol.h"
#include "glukosa_new.h"
#include "asam_urat.h"
TFT_eSPI tft = TFT_eSPI();
 
char _buffer[11];

void setup()
{
    tft.init();
    tft.fillScreen(TFT_BLACK);
    Serial.begin(115200);

    tft.setSwapBytes(true);
    drawimage();
    delay(100);
}
 
void loop()
{
        

        // sprintf( _buffer, " %02u.%02u", (int)BPM, (int)(BPM * 100) % 100 );
        // tft.setTextColor(TFT_RED, TFT_TRANSPARENT, false);  // set text color to yellow and black background
        // tft.setCursor(65, 95);
        // tft.print(_buffer);
        // tft.setTextSize(4);

        // sprintf( _buffer, " %02u.%02u", (int)SpO2, (int)(SpO2 * 100) % 100 );
        // tft.setTextColor(TFT_GREEN,TFT_WHITE);  // set text color to yellow and black background
        // tft.setCursor(65, 185);
        // tft.print(_buffer);
        // tft.setTextSize(4);
    }

void drawimage(){
  // tft.drawString("Denyut Jantung", 10, 50, 4);
  tft.pushImage(5,8, 64,64,kolesterol);
  tft.setFreeFont(&DejaVu_Serif_Bold_20);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("Kolesterol", 80, 23);
  tft.setFreeFont();

  tft.pushImage(5,88, 64,64,glukosa);
  tft.setFreeFont(&DejaVu_Serif_Bold_20);
  tft.drawString("Glukosa", 80, 98);
  tft.setFreeFont();

  tft.pushImage(5,163, 64,64,asam_urat);
  tft.setFreeFont(&DejaVu_Serif_Bold_20);
  tft.drawString("Asam Urat", 80, 173);
  tft.setFreeFont();
}
