//tft display
#include<TFT_eSPI.h>
#include<SPI.h>
TFT_eSPI tft = TFT_eSPI();

//libarry gambar
#include"font2.h"
#include "icon1.h"
#include "icon2.h"
#include "icon3.h"
#include <Wire.h>

//temperature
#include <OneWire.h>
#include <DallasTemperature.h>
OneWire pin_DS18B20(17);
DallasTemperature DS18B20(&pin_DS18B20);
#define REPORTING_PERIOD_MS2 2000
uint32_t tsLastReport2 = 0;
//kalibrasi ds18b20

float suhu;

//define gsr
#define InPin5 15
int sensorValue;
int temp;

//membuat char dengan 11 karakter
char _buffer[11];

void setup()
{
    tft.init();
    tft.fillScreen(TFT_BLACK);
    Serial.begin(115200);
    //start temperature
    DS18B20.begin();

    //convert to byte
    tft.setSwapBytes(true);
    //memanggil drawimage
    drawimage();
    delay(100);

    tft.setSwapBytes(true);
    drawimage();
}
 
void loop()
{
  
  long sum=0;
  for(int i=0;i<10;i++)
  {
    sensorValue=analogRead(InPin5);
    sum += sensorValue;
    delay(5);
    }
  int volt = map(sensorValue,0, 2416,0,7);


  float temperature = DS18B20.getTempCByIndex(0);
  temperature = temperature;
  DS18B20.setWaitForConversion(false);
  DS18B20.requestTemperatures();
  DS18B20.setWaitForConversion(true);

    Serial.print("sensorValue : ");
    Serial.println(sensorValue);
    Serial.print("Mapping : ");
    Serial.println(volt);

    sprintf( _buffer, " %02u.%02u", (int)volt, (int)(volt * 100) % 100 );
    tft.setTextColor(TFT_BLUE, TFT_BLACK);
    tft.setCursor(66, 203);
    tft.print(_buffer);
    tft.setTextSize(3);

    Serial.print("Temperature : ");
    Serial.print(temperature);

    sprintf( _buffer, " %02u.%02u", (int)temperature, (int)(temperature * 100) % 100 );
    tft.setTextColor(TFT_BLUE, TFT_BLACK);
    tft.setCursor(66, 128);
    tft.print(_buffer);
    tft.setTextSize(3);

}

void drawimage(){
  // tft.drawString("Denyut Jantung", 10, 50, 4);
  tft.pushImage(5,8, 64,64,icon1);
  tft.setFreeFont(&DejaVu_Serif_Bold_20);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("BPM", 80, 23);
  tft.setFreeFont();

  tft.pushImage(5,88, 64,64,icon2);
  tft.setFreeFont(&DejaVu_Serif_Bold_20);
  tft.drawString("SUHU", 80, 98);
  tft.setFreeFont();

  tft.pushImage(5,163, 64,64,icon3);
  tft.setFreeFont(&DejaVu_Serif_Bold_20);
  tft.drawString("GSR", 80, 173);
  tft.setFreeFont();
}
