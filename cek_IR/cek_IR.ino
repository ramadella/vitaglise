#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

// Create a MAX30100 object
MAX30100 sensor;

void setup() {
    Serial.begin(115200);

    Serial.print("Initializing MAX30100..");

    // Initialize sensor
      if (!sensor.begin()) {
          Serial.println("FAILED");
          for(;;);
      } else {
          Serial.println("SUCCESS");
      }

    // Configure sensor
    configureMax30100();
}

void loop() {
    uint16_t ir, red;

    sensor.update();

    delay(100);
    while (sensor.getRawValues(&ir, &red)) {

      float sample = (3.3*ir)/4095;
      float sample2 = (3.3*red)/4095;
		Serial.print("R[");
		Serial.print(sample);
		Serial.print("] IR[");
		Serial.print(sample2);
		Serial.println("]");
    }
    delay(10);
}

void configureMax30100() {
  sensor.setMode(MAX30100_MODE_SPO2_HR);
  sensor.setLedsCurrent(MAX30100_LED_CURR_24MA, MAX30100_LED_CURR_24MA);
  sensor.setLedsPulseWidth(MAX30100_SPC_PW_1600US_16BITS);
  sensor.setSamplingRate(MAX30100_SAMPRATE_100HZ);
  // sensor.setHighresModeEnabled(true);
}