#include <DallasTemperature.h>
OneWire pin_DS18B20(17);
DallasTemperature DS18B20(&pin_DS18B20);

float suhu;
float tambah = 2.8;
float kurang = 0;

void setup(){

  Serial.begin(9600);
  DS18B20.begin();
}

void loop(){
  DS18B20.requestTemperatures();
  float celcius = DS18B20.getTempCByIndex(0);
  float suhu_celcius = celcius+tambah-kurang;
  suhu_celcius = suhu_celcius;

  Serial.print("Suhu Celcius: ");
  Serial.println(suhu_celcius); 
}

