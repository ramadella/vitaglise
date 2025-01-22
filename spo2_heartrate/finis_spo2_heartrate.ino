#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

#include<TFT_eSPI.h>
#include<SPI.h>
#include"font.h"
#include "heart.h"
#include"rate.h"
TFT_eSPI tft = TFT_eSPI();


const int THRESH_MUL = 2.5;
const int REPORTING_PERIOD_MS = 1000;
const int MIN_SPO2 = 70;
const int MAX_SPO2 = 100;
const int MIN_HR = 40;
const int MAX_HR = 250;

int SamplingPeriodMS = REPORTING_PERIOD_MS;
uint32_t tsLastReport = 0;

const uint8_t Duration = 30; //[samplings]
uint8_t t_counter = 0;
uint8_t spo2[Duration]={0};
float heartRate[Duration]={0};
bool buffer_filled = false;

PulseOximeter pox;


uint8_t getNewCounter(){
  uint8_t counter = t_counter;
  t_counter++;
  if(t_counter>=Duration){
    t_counter = 0;
    buffer_filled = true;
  }
  return counter;
}

void resetBuffer(){
  t_counter = 0;
  memset(spo2, 0x00, Duration);
  memset(heartRate, 0x00, sizeof(float)*Duration);
  buffer_filled = false;
  SamplingPeriodMS = REPORTING_PERIOD_MS;

  // センサー再起動
  while(!pox.begin()){
    vTaskDelay(1000); // delay
  }
}

// Callback (registered below) fired when a pulse is detected
void onBeatDetected()
{
    Serial.println("Heart");
    vTaskDelay(100);
}

char _buffer[11];

void setup()
{
    tft.init();
    tft.fillScreen(TFT_BLACK);
    Serial.begin(115200);
    tft.setFreeFont(&Coming_Soon_Regular_22);
    tft.drawString("Vitaglise",67,10);
    tft.setFreeFont();
    tft.fillRect(0,40,240, 2, TFT_RED);
    tft.fillRect(0,238,240, 2, TFT_RED);
    tft.fillRect(0,0,240, 2, TFT_RED);
    tft.fillRect(0,0,2, 240, TFT_RED);
    tft.fillRect(238,0,2, 240, TFT_RED);

    while(!pox.begin()){
      vTaskDelay(1000); // delay
      Serial.println("Initializing pulse oximeter..");
    }

    pox.setIRLedCurrent(MAX30100_LED_CURR_24MA);
    pox.setOnBeatDetectedCallback(onBeatDetected);
    tft.setSwapBytes(true);
    drawimage();
    
}

float aveSpO2(){
    float aveSPO2 = 0;
    for(int i=0; i<Duration; i++){
      aveSPO2 += spo2[i];
    }
    aveSPO2 /= Duration;
    return aveSPO2;
}

float stdSpO2(){
    float ave = aveSpO2();
    float sumDiff2 = 0, diff=0; 
    for(int i=0; i<Duration; i++){
      diff = (spo2[i]-ave);
      sumDiff2 += diff*diff;
    }
    float std = sumDiff2/(Duration-1);
    std = sqrt(std);
    return std;
}

float aveHR(){
    float aveHR = 0;
    for(int i=0; i<Duration; i++){
      aveHR += heartRate[i];
    }
    aveHR /= Duration;
    return aveHR;
}

float stdHR(){
    float ave = aveHR();
    float sumDiff2 = 0, diff=0; 
    for(int i=0; i<Duration; i++){
      diff = (heartRate[i]-ave);
      sumDiff2 += diff*diff;
    }
    float std = sumDiff2/(Duration-1);
    std = sqrt(std);
    return std;
}

bool refineData()
{  
  float stdHRv = stdHR();
  float aveHRv = aveHR();
  float stdSpO2v = stdSpO2();
  float aveSpO2v = aveSpO2();
  Serial.print("aveHRv:");
  Serial.print(aveHRv);
  Serial.print(", stdHRv:");
  Serial.print(stdHRv);
  Serial.print(", aveSpO2v:");
  Serial.print(aveSpO2v);
  Serial.print(", stdSpO2v:");
  Serial.print(stdSpO2v);
  
  int8_t validMaxIdxDst = 0;
  if(stdSpO2v>10 || stdHRv>30){
    validMaxIdxDst = 0;
  }
  else{
    int8_t validIdx[Duration]={-1};
    for(int i=0; i<Duration; i++){
      if( (abs(heartRate[i]-aveHRv)<=stdHRv*THRESH_MUL)
         && (abs((float)spo2[i]-aveSpO2v)<=stdSpO2v*THRESH_MUL) ){
          validIdx[i] = i;
       }
       else{
          validIdx[i] = -1;      
          Serial.print("invalid:");
          Serial.print(i);
          Serial.print(", ");
       }
    }
    Serial.println("");
    

    int8_t validMaxIdxSrc = 0;
    bool finishMoving = false;
    for(int i=0; i<Duration; i++){
      finishMoving = false;
      for(int j=validMaxIdxSrc; j<Duration; j++){
        if(validIdx[j]>=0){
          heartRate[i] = heartRate[j];
          spo2[i] = spo2[j];
          validMaxIdxDst = i;
          if(validMaxIdxSrc<Duration-1){
            validMaxIdxSrc = j+1;          
          }
          else{
            finishMoving = true;
          }
          break;//j
        }
        else{
          continue;
        }
      }
      if(finishMoving){
        break;
      }
    }
  }
  t_counter = validMaxIdxDst;
  Serial.print("t_counter:");
  Serial.println(t_counter);

  bool bRefined = (t_counter == Duration-1);;
  if(!bRefined){
    buffer_filled = false;
  }
  return bRefined;
}

void loop()
{
  pox.update();

    // Asynchronously dump heart rate and oxidation levels to the serial
    // For both, a value of 0 means "invalid"
    uint8_t tmp_counter = t_counter;
    if(buffer_filled){
      tmp_counter = Duration;
    }
    if (millis() - tsLastReport > SamplingPeriodMS) {
      if(buffer_filled && refineData() ){
        float aveHRv = aveHR();
        float aveSpO2v = aveSpO2();

      }
        // pox.shutdown();
        uint8_t tmp_spo2 = pox.getSpO2();
        float tmp_heartRate = pox.getHeartRate();

        
        uint8_t cidx = getNewCounter();
        spo2[cidx] = tmp_spo2;
        heartRate[cidx] = tmp_heartRate;

        Serial.print("Heart rate:");
        Serial.print(tmp_heartRate);
        Serial.print("bpm / SpO2:");
        Serial.print(tmp_spo2);
        Serial.println("%");
        


        sprintf( _buffer, " %02u.%02u", (int)tmp_heartRate, (int)(tmp_heartRate * 100) % 100 );
        tft.setTextColor(TFT_RED, TFT_BLACK);  // set text color to yellow and black background
        tft.setCursor(65, 95);
        tft.print(_buffer);
        tft.setTextSize(4);

        sprintf( _buffer, " %02u.%02u", (int)tmp_spo2, (int)(tmp_spo2 * 100) % 100 );
        tft.setTextColor(TFT_GREEN, TFT_BLACK);  // set text color to yellow and black background
        tft.setCursor(65, 185);
        tft.print(_buffer);
        tft.setTextSize(4);
        tsLastReport = millis();
      }
    // M5.update();
    // if(M5.BtnA.wasPressed()){
    //   resetBuffer();
    // }

 }

 void drawimage(){
  tft.drawString("Denyut Jantung", 10, 50, 4);
  tft.pushImage(10,80, 52,52,heart);
  tft.drawString("Saturasi Oksigen", 10, 140, 4);
  tft.pushImage(10,170, 52,52,rate);
}
 
