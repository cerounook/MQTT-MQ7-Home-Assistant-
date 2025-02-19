/*
Código generado para trabajar con un pin analógico y 2 digitales
- analógico para el sensor MQ7
- digitales para un led en D7
- y un buzzer pasivo en D6

-- documentacion --
datasheet
  https://www.sparkfun.com/datasheets/Sensors/Biometric/MQ-7.pdf
  https://www.pololu.com/file/0J313/MQ7.pdf#:~:text=MQ-7%20Semiconductor%20Sensor%20for%20Carbon%20Monoxide%20Sensitive%20material,detect%20CO%20when%20low%20temperature%20%28heated%20by%201.5V%29.

ejemplos:
  https://www.luisllamas.es/arduino-detector-gas-mq/

código original para el RO y lectura de CO2
  https://www.teachmemicro.com/use-mq-7-carbon-monoxide-sensor/   

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Credenciales de la red wifi.
const char* ssid = "TOPA-LINK";
const char* password = "30330234";

//ip del servidor mqtt en Home Assistant
const char* mqtt_server = "192.168.0.200";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];

//se crea una variable para 
int value = 0;

/* la lectura se realiza desde el pin analogico, de 1 a 1024, 
 *  para obtener los valores en unidades correspondientes a la medición del gas, 
 *  necesitamos escalar el valor leído, según lo mostrado en el datasheet 
*/

//variables necesarias para escalar el valor del pin analógico

// resistencia del módulo en el monóxido
float RS_gas = 0;
float ratio = 0;
float sensorValue = 0;
float sensor_volt = 0;

// resistencia del sensor en el aire limpio
float R0 = 42521.74;
  
String estado = "Conectado";

void setup_wifi() {


  //pin D7 para el led
  pinMode(13, OUTPUT);
  //pin D6 para el buzzer pasivo
  pinMode(12, OUTPUT);
  
    
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Conectando a la red ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("Conexión exitosa");
  Serial.print("dirección IP: ");
  Serial.println(WiFi.localIP());
}

//respuesta del servidor mqtt
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // ... en caso de que necesite reconectarse
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    // Se crea un cliente ID con las credenciales del servidor de Home Assistant
    String clientId = "ESP8266Client";

    // se toman los parámetros de la configuración del complemento de MQTT en Home assistant
    String serverId = "homeassistant";
    String passId = "hojoothaetiedeeThoe4Jo6aepaizoophoo6ahfuyee5lahphieleiloolooyeir";
    
//    String serverId = "hassio";
//    String passId = "home357";
    
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), serverId.c_str(), passId.c_str())) {
      Serial.println("conectado");
      // Una vez conectado, se publica un mensaje
      client.publish("/sensor/monoxido_carbono/estado", estado.c_str());
      // ... y se resuscribe
      client.subscribe("/sensor/monoxido_carbono");
    } else {
      Serial.print("falló la conexión, reconectando=");      
      Serial.print(client.state());
      Serial.println(
        " intente en 5 segundos");
      // espera de 5 segundo para reintentar
      delay(5000);
    }
  }
}
    
void setup() {

  // se inicializa el pin BUILTIN_LED del arduino como salida
  pinMode(BUILTIN_LED, OUTPUT);
  
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
}

void loop() {

   sensorValue = analogRead(A0);
   sensor_volt = sensorValue/1024*5.0;
   RS_gas = (5.0-sensor_volt)/sensor_volt;
   ratio = RS_gas/R0; //Replace R0 with the value found using the sketch above
   float x = 1538.46 * ratio;
   float ppm = pow(x,-1.709);
//   Serial.print("PPM: ");
//   Serial.println(ppm);
//   delay(1000);

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
 // si la lectura el sensor es mayor a 50 ppm
 // se dispara una alarma
 
 if(ppm >= 50){
  
  // se publica el mensaje de alerta
  client.publish("/sensor/monoxido_carbono", "PELIGRO");
  
  // se apaga el led
  digitalWrite(13, LOW);
  // se enciende el buzzer
  //digitalWrite(12, HIGH);
  tone(12, 440);
  delay(500);
  client.publish("/sensor/monoxido_carbono", String(ppm).c_str());
  // se apaga el buzzer  
  digitalWrite(13, HIGH);
  noTone(12);
  //digitalWrite(12, LOW);
  delay(500);
 }else{
  //caso contrario, se muestra la lectura del sensor
  digitalWrite(13, LOW);
  client.publish("/sensor/monoxido_carbono", String(ppm).c_str());  
 }
 //Serial.print("Sensor: ");
 //Serial.println(analogRead(0)); 
 delay(1000);
}
