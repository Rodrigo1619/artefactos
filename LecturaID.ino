
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#include <SPI.h>      // incluye libreria bus SPI
#include <MFRC522.h>      // incluye libreria especifica para MFRC522

#define RST_PIN  D1      // constante para referenciar pin de reset
#define SS_PIN  D2      // constante para referenciar pin de slave select

MFRC522 mfrc522(SS_PIN, RST_PIN); // crea objeto mfrc522 enviando pines de slave select y reset

byte LecturaUID[4];         // crea array para almacenar el UID leido
byte Usuario1[4]= {0x39, 0xD4, 0xF0, 0xC1} ;    // UID de tarjeta leido en programa 1
byte Usuario2[4]= {0xB3, 0xCB, 0xD2, 0x0A} ;    // UID de llavero leido en programa 1


/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "ARTEFACTOS"
#define WLAN_PASS       "ARTEFACTOS"


/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME  "Rodrick16"
#define AIO_KEY       "aio_UHVf61mWXv2nyW4doQiQwrxEo8HS"


/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiClientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);

/****************************** Feeds for Publishing***************************************/
// Setup a feed called 'photocell' for publishing.
Adafruit_MQTT_Publish rfidpubli = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/rfid");


int value = 0;
//SETUP
void setup() {
  Serial.begin(9600);     // inicializa comunicacion por monitor serie a 9600 bps
  SPI.begin();        // inicializa bus SPI
  mfrc522.PCD_Init();     // inicializa modulo lector
  Serial.println("Listo");    // Muestra texto Listo

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
  

}

//LOOP
void loop() {
  
 MQTT_connect();
  mqtt.processPackets(10000);
  if(! mqtt.ping()) {   // ping the server to keep the mqtt connection alive
    mqtt.disconnect();}

  if ( ! mfrc522.PICC_IsNewCardPresent())   // si no hay una tarjeta presente
    return;           // retorna al loop esperando por una tarjeta
  
  if ( ! mfrc522.PICC_ReadCardSerial())     // si no puede obtener datos de la tarjeta
    return;           // retorna al loop esperando por otra tarjeta
    
    Serial.print("UID:");       // muestra texto UID:
    for (byte i = 0; i < mfrc522.uid.size; i++) { // bucle recorre de a un byte por vez el UID
      if (mfrc522.uid.uidByte[i] < 0x10){   // si el byte leido es menor a 0x10
        Serial.print(" 0");       // imprime espacio en blanco y numero cero
        }
        else{           // sino
          Serial.print(" ");        // imprime un espacio en blanco
          }
          Serial.print(mfrc522.uid.uidByte[i], HEX);    // imprime el byte del UID leido en hexadecimal
          LecturaUID[i]=mfrc522.uid.uidByte[i];     // almacena en array el byte del UID leido      
          }
          
          Serial.print("\t");         // imprime un espacio de tabulacion             
                    
          if(comparaUID(LecturaUID, Usuario1)){    // llama a funcion comparaUID con Usuario1
            Serial.println("Bienvenido Usuario 1");// si retorna verdadero muestra texto bienvenida
            value = 1;
            rfidpubli.publish(value);
            } 
          else if(comparaUID(LecturaUID, Usuario2)){ // llama a funcion comparaUID con Usuario2
            Serial.println("Bienvenido Usuario 2");
            value = 1;
            rfidpubli.publish(value);
            } // si retorna verdadero muestra texto bienvenida
           else{           // si retorna falso
            Serial.println("Quien so?");
            value = 0;
            rfidpubli.publish(value);
            
            }   // muestra texto equivalente a acceso denegado          
                  
                  mfrc522.PICC_HaltA();     // detiene comunicacion con tarjeta  



}

boolean comparaUID(byte lectura[],byte usuario[]) // funcion comparaUID
{
  for (byte i=0; i < mfrc522.uid.size; i++){    // bucle recorre de a un byte por vez el UID
  if(lectura[i] != usuario[i])        // si byte de UID leido es distinto a usuario
    return(false);          // retorna falso
  }
  return(true);           // si los 4 bytes coinciden retorna verdadero
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
