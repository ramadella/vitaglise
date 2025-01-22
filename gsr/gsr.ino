const int GSR=34;
int sensorValue=0;
int gsr_average=0;

void setup(){
  Serial.begin(115200);
}

void loop(){
  long sum=0;
  sensorValue=analogRead(GSR);

  Serial.println(sensorValue);
  delay(5);
}