float sensor_volt;
// resistencia del módulo en el monóxido
float RS_gas; 
// resistencia del sensor en el aire limpio
float R0;
int R2 = 2000;
  
void setup() {
 Serial.begin(9600);
}
 
void loop() {
  int sensorValue = analogRead(A0);
  sensor_volt=(float)sensorValue/1024*5.0;
  RS_gas = ((5.0 * R2)/sensor_volt) - R2;
  R0 = RS_gas / 1;
  Serial.print("R0: ");
  Serial.println(R0);
}
