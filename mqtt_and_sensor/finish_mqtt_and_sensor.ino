#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

#include<TFT_eSPI.h>
#include<SPI.h>
#include"font.h"
#include"font3.h"
#include "heart.h"
#include"rate.h"
#include"font2.h"
#include"font4.h"
#include "kolesterol.h"
#include "frame2.h"
#include "logo.h"
#include "glukosa_new.h"
#include "asam_urat.h"
TFT_eSPI tft = TFT_eSPI();

//mqtt
#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <ArduinoJson.h>

const char *ssid = "Redmi 10 2022";
const char *password = "123456789";

// Your MQTT broker ID
const char *mqttBroker = "192.168.206.71";
const int mqttPort = 1883;
// MQTT topics
const char *publishTopic = "sensorReadings";
const char *subscribeTopic = "sampletopic";

WiFiClient espClient;
PubSubClient client(espClient);

// Connect to Wifi
void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    start();
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Callback function whenever an MQTT message is received
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String message;
  for (int i = 0; i < length; i++)
  {
    Serial.print(message += (char)payload[i]);
  }
  Serial.println();
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");

    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    // Attempt to connect
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      // Subscribe to topic
      // client.subscribe(subscribeTopic);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//touch sensor
#define touchsensor 15
int touch_now = 0;
int touch_before = 0;

//max30100
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

char buffer[256];
char _buffer[11];

float kolesterol_check = 0;
float glukosa_check = 0;
float asam_urat_check =0;

void setup()
{
    tft.init();
    tft.setSwapBytes(true);
    tft.fillScreen(TFT_BLACK);
    Serial.begin(115200);
    delay(500);
    pinMode(touchsensor,INPUT);
    // Setup the wifi
    setup_wifi();
    // setup the mqtt server and callback
    client.setServer(mqttBroker, mqttPort);
    client.setCallback(callback);
    delay(500);

    while(!pox.begin()){
      vTaskDelay(1000); // delay
      Serial.println("Initializing pulse oximeter..");
    }

    pox.setIRLedCurrent(MAX30100_LED_CURR_24MA);
    pox.setOnBeatDetectedCallback(onBeatDetected);   
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

  delay(200);
  // Listen for mqtt message and reconnect if disconnected
  if (!client.connected())
  {
    reconnect();
    }
  client.loop();
  touch_now =digitalRead(touchsensor);
  pox.update();
    // Asynchronously dump heart rate and oxidation levels to the serial
    // For both, a value of 0 means "invalid"
    uint8_t tmp_counter = t_counter;
    if(buffer_filled){
      tmp_counter = Duration;
    }
    if (millis() - tsLastReport > SamplingPeriodMS) {
      tft.fillScreen(TFT_BLACK);
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

        float kolesterol_check = 0.131783995 + (0.92798323*tmp_spo2)+(1.021408315*tmp_heartRate);
        float glukosa_check = (-0.01173) + (1.21315278*tmp_spo2)+(-0.2712208*tmp_heartRate);
        float asam_urat_check = -0.00209127 + (0.099815406*tmp_spo2) -(0.04174689*tmp_heartRate);

        Serial.print("Kolesterol: ");
        Serial.print(kolesterol_check);
        Serial.print("Glukosa: ");
        Serial.println(glukosa_check);
        Serial.print("Asam Urat: ");
        Serial.println(asam_urat_check);


        if(touch_now == HIGH && touch_before == LOW){
          drawimage2();
          sprintf( _buffer, " %02u.%02u", (int)kolesterol_check, (int)(kolesterol_check * 100) % 100 );
          tft.setTextColor(TFT_RED, TFT_BLACK);  // set text color to yellow and black background
          tft.setFreeFont(&Tinos_Bold_35);
          tft.drawString(_buffer,66, 53);
          tft.setFreeFont();

          sprintf( _buffer, " %02u.%02u", (int)glukosa_check, (int)(glukosa_check * 100) % 100 );
          tft.setTextColor(TFT_BLUE, TFT_BLACK);
          tft.setFreeFont(&Tinos_Bold_35);
          tft.drawString(_buffer,66, 128);
          tft.setFreeFont();

          sprintf( _buffer, " %02u.%02u", (int)asam_urat_check, (int)(asam_urat_check * 100) % 100 );
          tft.setTextColor(TFT_GREEN, TFT_BLACK);
          tft.setFreeFont(&Tinos_Bold_35);
          tft.drawString(_buffer,66, 203);
          tft.setFreeFont();
        }
        else{
          top();
          drawimage();
          sprintf( _buffer, " %02u.%02u", (int)tmp_heartRate, (int)(tmp_heartRate * 100) % 100 );
          tft.setTextColor(TFT_RED, TFT_BLACK);  // set text color to yellow and black background

          tft.setFreeFont(&Tinos_Bold_40);
          tft.drawString(_buffer,65,95);
          tft.setFreeFont();

          sprintf( _buffer, " %02u.%02u", (int)tmp_spo2, (int)(tmp_spo2 * 100) % 100 );
          tft.setTextColor(TFT_GREEN, TFT_BLACK);  // set text color to yellow and black background

          tft.setFreeFont(&Tinos_Bold_40);
          tft.drawString(_buffer,65,185);
          tft.setFreeFont();
      }

  StaticJsonDocument<256> doc;
  doc["bpm"] = tmp_heartRate;
  doc["spo2"] = tmp_spo2;
  doc["kolesterol"] = kolesterol_check;
  doc["glukosa"] = glukosa_check;
  doc["asamurat"] = asam_urat_check;

  size_t n = serializeJson(doc, buffer);
  client.publish(publishTopic, buffer, n);
  tsLastReport = millis();
 }
}
void drawimage(){
  tft.setTextColor(TFT_WHITE);
  tft.drawString("Denyut Jantung", 10, 50, 4);
  tft.pushImage(10,80, 52,52,heart);
  tft.drawString("Saturasi Oksigen", 10, 140, 4);
  tft.pushImage(10,170, 52,52,rate);
}
void drawimage2(){
  tft.pushImage(5,8,64,64,kolesterol);
  tft.setTextColor(TFT_WHITE);
  tft.setFreeFont(&DejaVu_Serif_Bold_20);
  tft.drawString("Kolesterol", 80, 23);
  tft.setFreeFont();

  tft.pushImage(5,88, 64,64,glukosa);
  tft.setTextColor(TFT_WHITE);
  tft.setFreeFont(&DejaVu_Serif_Bold_20);
  tft.drawString("Glukosa", 80, 98);
  tft.setFreeFont();

  tft.pushImage(5,163, 64,64,asam_urat);
  tft.setTextColor(TFT_WHITE);
  tft.setFreeFont(&DejaVu_Serif_Bold_20);
  tft.drawString("Asam Urat", 80, 173);
  tft.setFreeFont();
}

void top(){
  tft.setFreeFont(&Coming_Soon_Regular_22);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("Vitaglise",67,10);
  tft.setFreeFont();
  tft.fillRect(0,40,240, 2, TFT_RED);
  tft.fillRect(0,238,240, 2, TFT_RED);
  tft.fillRect(0,0,240, 2, TFT_RED);
  tft.fillRect(0,0,2, 240, TFT_RED);
  tft.fillRect(238,0,2, 240, TFT_RED);
}

void start(){
for(int i=0;i<frames;i++)
{
  delay(40);
  tft.pushImage(35, 80,animation_width  , animation_height, frame2[i]);
  }
}

void first(){
  tft.pushImage(70,70,100,100,logo);
}