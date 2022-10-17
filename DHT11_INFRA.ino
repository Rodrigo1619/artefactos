#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <DHT.h>


 /************************* WiFi Access Point *********************************/

#define WLAN_SSID       "ARTEFACTOS"
#define WLAN_PASS       "ARTEFACTOS"


/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "Rodrick16"
#define AIO_KEY         "aio_UHVf61mWXv2nyW4doQiQwrxEo8HS"
// Definimos el pin digital donde se conecta el sensor
#define PORT D2

// Dependiendo del tipo de sensor
#define DHTTYPE DHT11

/************ Global State (you don't need to change this!) ******************/
// Inicializamos el sensor DHT11
DHT dht(PORT, DHTTYPE);

WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);


/****************************** Feeds for Publishing***************************************/

Adafruit_MQTT_Publish infrarrojo = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/infrarrojo");
Adafruit_MQTT_Publish temp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperatura");
Adafruit_MQTT_Publish hume = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/dht11");

//variable para infrarrojo
int detector = D0;


void setup() {
  // Inicializamos comunicación serie
  Serial.begin(9600);
  //****************************************** Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

 //INFRARROJO
 pinMode(detector, INPUT);
  // Comenzamos el sensor DHT
  dht.begin();
  uint32_t x=0;
  

}
 
void loop() {
   MQTT_connect();
  mqtt.processPackets(10000);
  if(! mqtt.ping()) {   // ping the server to keep the mqtt connection alive
    mqtt.disconnect();}

  delay(3000);

 
  // Leemos la humedad relativa
  float humedad = dht.readHumidity();
  // Leemos la temperatura en grados centígrados (por defecto)
  float temperatura = dht.readTemperature();
  // Leemos la temperatura en grados Fahreheit
  float f = dht.readTemperature(true);
 
  // Comprobamos si ha habido algún error en la lectura
  if (isnan(humedad) || isnan(temperatura) || isnan(f)) {
    Serial.println("Error obteniendo los datos del sensor DHT11");
    return;
  }
 
  // Calcular el índice de calor en Fahreheit
  float hif = dht.computeHeatIndex(f, humedad);
  // Calcular el índice de calor en grados centígrados
  float hic = dht.computeHeatIndex(temperatura, humedad, false);
 
  Serial.print("Humedad: ");
  Serial.print(humedad);
  Serial.print(" %\t");
  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Índice de calor: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");

  //publicando en ada DHT
  if(!temp.publish(temperatura)){
    Serial.println("Error opbteniendo datos");
    return;
  }
  if(!hume.publish(humedad)){
    Serial.println("Error obteniendo datos");
    return;
  }


//INFRARROJO
int value = 0;
value = digitalRead(detector);

Serial.print(value);



//Publicando
if(!infrarrojo.publish(value)){
  Serial.println("no hay nada");
}

}
// Function to connect and reconnect as necessary to the MQTT server.
void MQTT_connect() {
  int8_t ret;
  // Stop if already connected.
  if (mqtt.connected()) {
    return;}
  Serial.print("Connecting to MQTT... ");
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 10 seconds...");
       mqtt.disconnect();
       delay(10000);  // wait 10 seconds
       retries--;
       if (retries == 0) {       // basically die and wait for WDT to reset me
         while (1);}}
  Serial.println("MQTT Connected!");
}
