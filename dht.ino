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
#define AIO_KEY         "aio_LpAq40MfdgkfPL3hgS9k9sFsZMKo" //change when is necessary
#define PORT  D2
#define DHTTYPE   DHT11
/************ Global State (you don't need to change this!) ******************/

DHT dht(PORT, DHTTYPE);

WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);

/****************************** Feeds for Publishing***************************************/

Adafruit_MQTT_Publish infrarrojo = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/infrarrojo");
Adafruit_MQTT_Publish tempdht = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temp");
Adafruit_MQTT_Publish dhtHumedad = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/dht");

//*********************************************** SetUp *****************************/
void setup() {
  // pinMode(D0, INPUT);
  Serial.begin(9600);
  
  // delay(10);
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
  //********************************************* Callback Function
  dht.begin();
}

uint32_t x=0;
//***************************************************** Loop ********************************/
void loop() {
  MQTT_connect();
  mqtt.processPackets(10000);
  if(! mqtt.ping()) {   // ping the server to keep the mqtt connection alive
    mqtt.disconnect();}

  delay(3000);
// Now we can publish stuff!
  // Para infrarrojo // x = map(digitalRead(D0),0,1,0,100); 
  float humedad = dht.readHumidity();
  float temp = dht.readTemperature();

  if(isnan(humedad) || isnan(temp)) {
    Serial.println("Is NAN humedad or temp");
    return;
  } 
  
  Serial.print(F("\nMandando valores de humedad"));
  Serial.print(humedad);
  Serial.print("...");
  if (!dhtHumedad.publish(humedad)) {
    Serial.println(F("Failed humedad"));
  } else {
    Serial.println(F("OK! humedad"));
  }

  Serial.print(F("\nMandando valores de temperatura"));
  Serial.print(temp);
  Serial.print("...");
  if (!tempdht.publish(temp)) {
    Serial.println(F("Failed temp"));
  } else {
    Serial.println(F("OK! temp"));
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
