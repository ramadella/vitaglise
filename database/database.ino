#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <HTTPClient.h>
#include "MAX30100_PulseOximeter.h"

#define REPORTING_PERIOD_MS     1000

// Create a PulseOximeter object
PulseOximeter pox;

// Time at which the last beat occurred
uint32_t tsLastReport = 0;

// Callback routine is executed when a pulse is detected
void onBeatDetected() {
    Serial.println("Beat!");
}

// Replace with your SSID and Password
const char* ssid      = "Redmi 10 2022";
const char* password  = "123456789";
// REPLACE with your Domain name and URL path or IP address with path
const char* serverName = "http://192.168.1.9/database_ta/post-esp-data.php";
// Keep this API Key value to be compatible with the PHP code provided in the project page. 
// If you change the apiKeyValue value, the PHP file /post-esp-data.php also needs to have the same key 
String apiKeyValue = "tPmAT5Ab3j7F9";
String sensorName = "Vitaglise";
String sensorLocation = "Left Hand"; 
void setup() {
  Serial.begin(115200);
  if (!pox.begin()) {
    Serial.println("FAILED");
    for(;;);
    }
    else {
      Serial.println("SUCCESS");
    } 
  pox.setIRLedCurrent(MAX30100_LED_CURR_46_8MA);

    // Register a callback routine
    pox.setOnBeatDetectedCallback(onBeatDetected);
// Connect to WiFi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);
   
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
   
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void postData(float value1, float value2, float value3, float value5, float value6){
      //Check WiFi connection status
  if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;
     
    // Your Domain name with URL path or IP address with path
    http.begin(serverName);
     
    // Specify content-type header
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
     
    // Prepare your HTTP POST request data
    String httpRequestData = "api_key=" + apiKeyValue + "&value1=" + String(value1) + "&value2=" + String(value2) + "&value3=" + String(value3) + "&value5=" + String(value5) + "&value6=" + String(value6) +"";
    Serial.print("httpRequestData: ");
    Serial.println(httpRequestData);
// Send HTTP POST request
    int httpResponseCode = http.POST(httpRequestData);
     
    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();
  }
  else {
    Serial.println("WiFi Disconnected");
  }
  //Send an HTTP POST request every hour
  delay(100);
}
void loop() {
     pox.update();

    // Grab the updated heart rate and SpO2 levels
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {

      float A = pox.getHeartRate();
      float B = pox.getSpO2();
      Serial.print("Heart rate:");
      Serial.print(A);
      Serial.print("bpm / SpO2:");
      Serial.print(B);
      Serial.println("%");

        float C = 0.131783995 + (0.92798323*B)+(1.021408315*A);
        float D = (-0.01173) + (1.21315278*B)+(-0.2712208*A);
        float E = -0.00209127 + (0.099815406*B) -(0.04174689*A);

        postData(A,B,C,D,E);

        tsLastReport = millis();
    }
}