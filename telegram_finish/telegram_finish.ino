#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
const int THRESH_MUL = 2.5;
const int REPORTING_PERIOD_MS = 1000;
const int MIN_SPO2 = 70;
const int MAX_SPO2 = 100;
const int MIN_HR = 40;
const int MAX_HR = 250;

//telegram
#define USE_CLIENTSSL false  

#include <AsyncTelegram2.h>
// Timezone definition
#include <time.h>
#define MYTZ "CET-1CEST,M3.5.0,M10.5.0/3"

#ifdef ESP8266
  #include <ESP8266WiFi.h>
  BearSSL::WiFiClientSecure client;
  BearSSL::Session   session;
  BearSSL::X509List  certificate(telegram_cert);
  
#elif defined(ESP32)
  #include <WiFi.h>
  #include <WiFiClient.h>
  #if USE_CLIENTSSL
    #include <SSLClient.h>  
    #include "tg_certificate.h"
    WiFiClient base_client;
    SSLClient client(base_client, TAs, (size_t)TAs_NUM, A0, 1, SSLClient::SSL_ERROR);
  #else
    #include <WiFiClientSecure.h>
    WiFiClientSecure client;    
  #endif
#endif


AsyncTelegram2 myBot(client);
const char* ssid  =  "Kos Arrester Lantai 2";     // SSID WiFi network
const char* pass  =  "ELEKTROONLY";     // Password  WiFi network
const char* token =  "6604934530:AAHmii4Xh4HlaJhEQ5h72N2LJmaeKADGCM0";  // Telegram token
int64_t userid = 5397600682;


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
    Serial.begin(115200);

    while(!pox.begin()){
      vTaskDelay(1000); // delay
      Serial.println("Initializing pulse oximeter..");
    }

    pox.setIRLedCurrent(MAX30100_LED_CURR_24MA);
    pox.setOnBeatDetectedCallback(onBeatDetected);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
    delay(500);
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print('.');
      delay(500);
      }
    #ifdef ESP8266
      // Sync time with NTP, to check properly Telegram certificate
      configTime(MYTZ, "time.google.com", "time.windows.com", "pool.ntp.org");
      //Set certficate, session and some other base client properies
      client.setSession(&session);
      client.setTrustAnchors(&certificate);
      client.setBufferSizes(1024, 1024);
  #elif defined(ESP32)
    // Sync time with NTP
    configTzTime(MYTZ, "time.google.com", "time.windows.com", "pool.ntp.org");
    #if USE_CLIENTSSL == false
    client.setCACert(telegram_cert);
  #endif
#endif

  // Set the Telegram bot properies
  myBot.setUpdateTime(1000);
  myBot.setTelegramToken(token);

  // Check if all things are ok
  Serial.print("\nTest Telegram connection... ");
  myBot.begin() ? Serial.println("OK") : Serial.println("NOK");

  Serial.print("Bot name: @");
  Serial.println(myBot.getBotName());
    
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
          // Serial.print("invalid:");
          // Serial.print(i);
          // Serial.print(", ");
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
  // Serial.print("t_counter:");
  // Serial.println(t_counter);

  bool bRefined = (t_counter == Duration-1);;
  if(!bRefined){
    buffer_filled = false;
  }
  return bRefined;
}

void loop()
{
  TBMessage msg;
  pox.update();
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

        String welcome_msg;
        welcome_msg = "Welcome\n " ;
        welcome_msg += tmp_heartRate;
        welcome_msg += ".segera ke rumah sakit\n ";
        myBot.sendMessage(msg, welcome_msg); 
        myBot.sendTo(userid, welcome_msg);
        
        tsLastReport = millis();
      }

 }

 
