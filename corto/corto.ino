#include "DHT.h"
#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define DHT_PIN 33
#define DHT_TYPE DHT11


#define i2c_Address 0x3c //initialize with the I2C addr 0x3C Typically eBay OLED's
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1   //   QT-PY / XIAO
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* ssid = "rodrick";
const char* password = "123456789";

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define IO_USERNAME  "WilmerSorto"
#define IO_KEY       "aio_aDfi31T2hrehEMQt1D3CmG2mPWFJ"

DHT dht(DHT_PIN, DHT_TYPE);
const int TRIG = 26;
const int ECHO = 25;

float duration = 0.0, distance = 0.0;

WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, IO_USERNAME, IO_KEY);

Adafruit_MQTT_Publish humedadPubli = Adafruit_MQTT_Publish(&mqtt, IO_USERNAME "/feeds/humedad");
Adafruit_MQTT_Publish temperaturaPubli = Adafruit_MQTT_Publish(&mqtt, IO_USERNAME "/feeds/temp");
Adafruit_MQTT_Publish nivelPubli = Adafruit_MQTT_Publish(&mqtt, IO_USERNAME "/feeds/Nivel");


void setup() {
  pinMode(TRIG, OUTPUT); // Sets the trigPin as an Output
  pinMode(ECHO, INPUT); // Sets the echoPin as an Input
  Serial.begin(115200); // Starts the serial communication


  initWiFi();
  Serial.print("RRSI: ");
  Serial.println(WiFi.RSSI());

  dht.begin();

  delay(250); // wait for the OLED to power up
  display.begin(i2c_Address, true); // Address 0x3C default
  display.setContrast (255); // dim display
  // Clear the buffer.
  display.clearDisplay();
  display.display();
}
void loop() {
  MQTT_connect();
  mqtt.processPackets(10000);
  if (!mqtt.ping()) {
    mqtt.disconnect();
  }

  delay(2000);
  display.clearDisplay();
  float humedad = dht.readHumidity();
  float temperatura = dht.readTemperature();

  // Clears the trigPin
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);

  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  display.setTextSize(1); //set text size
  display.setTextColor(SH110X_WHITE); //set text color
  display.setCursor(20, 0);   // set cursor

  if (isnan(humedad) || isnan(temperatura)) {
    Serial.println("Error obteniendo los datos del sensor DHT11");
    return;
  }

  duration = pulseIn(ECHO, HIGH);
  // Calculating the distance
  distance = duration/58;
 
  //publicar en adafruit
  if(!humedadPubli.publish(humedad)) {
    Serial.println("Nimodo fallo humedad");
  } else {
    Serial.print("Humedad: ");
    Serial.print(humedad);
    Serial.println("");
  }

  if(!nivelPubli.publish(distance)) {
    Serial.println("Nimodo fallo distancia");
  } else {
    Serial.print("Nivel de agua: ");
    Serial.print(distance);
    Serial.println("");
  }

  if(!temperaturaPubli.publish(temperatura)) {
    Serial.println("Nimodo fallo temperatura");
  } else {
    Serial.print("Temperatura: ");
    Serial.print(temperatura);
    Serial.println("");
  }
  
  // Reads the echoPin, returns the sound wave travel time in microseconds


  display.print("Nivel de agua: ");  //to print
  display.print(distance);

  display.setTextSize(1); //set text size
  display.setTextColor(SH110X_WHITE); //set text color
  display.setCursor(20, 20);   // set cursor
  
  if(distance == 0.0) {
    display.print("TANQUE VACIO!!!");  //to print 
  }
  display.display();
}

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

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  Serial.println(WiFi.localIP());
}
