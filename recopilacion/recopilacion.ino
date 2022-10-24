/*
NOTA:Este codigo funciona con placa ESP8266 en NodeMCU 1.0, si se esta trabajando en un ESP32 ver el codigo utilizado en el
archivo de "nivel del agua" y lo mas que cambia es la conexion con internet y con adafruit, lo demas sigue la misma logica 
*/

#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* WiFi Access Point *********************************/
//entre las comillas se coloca el nombre de nuestra red de internet
#define WLAN_SSID       "......."

//entre las comillas se coloca la contraseña de nuestra red de internet
#define WLAN_PASS       "......."

/************************* Adafruit.io Setup *********************************/
/*
Esta informacion se puede ver en IO>simbolo de llave amarilla y ahi aparece tanto el usuario como la active key
*/
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL

//aqui se coloca el usuario con el que tenemos nuestra cuenta de adafruit
#define AIO_USERNAME    ".........."

//se coloca la llave que nos genera adafruit (siempre copiar y pegar para evitar problemas de conexion)
#define AIO_KEY         ".........."

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiClientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);

/****************************** Feeds for Publishing***************************************/

//El publish solo se usa cuando nosotros queremos enviar cosas a Adafruit
/*para declarar un publish debemos crear en adafruit un feed, por ejemplo el feed "humedad", este feed es el que recibira todos los
//datos que envie el sensor. Para realizar la conexion solo sustituimos en las comillas del codigo de abajo con:
"/feeds/nombre_del_feed_creado_en_adafruit", por ejemplo el de humedad se llamara: "/feeds/humedad" y sera guardado bajo el nombre
de humedad, este nombre aparece en la linea de codigo: Adafruit_MQTT_Publish humedad
NO NECESARIAMENTE TIENE QUE LLAMARSE HUMEDAD TAMBIEN, puede llevar otro nombre pero para faCilitar se le llama humedad, pero si colocamos
otro nombre la linea de codigo nos queda:
Adafruit_MQTT_Publish HUMEDADPUBLISH = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humedad");
*/

// feed/humedad
Adafruit_MQTT_Publish humedad = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humedad");

// feed/temperatura
//el feed se llama temp, pero solo se sustituye con el nombre del feed que se tiene 
Adafruit_MQTT_Publish temperatura = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temp");

// feed/distancia 
Adafruit_MQTT_Publish distancia = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/distancia");

/*
Si quisieramos un feed con otro nombre, por ejemplo el de sensacion termica, entonces nos aseguramos que en adafruit este creado
el feed que recibira los datos de la sensacion termica, por ejemplo sensacionT y lo guardamos bajo el nombre que querramos
Por lo tanto el codigo quedaria:
Adafruit_MQTT_Publish sensacionT = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/sensacionT");
*/



/****************************** Feeds for Subscribing***************************************/
// Setup a feed called 'slider' for subscribing to changes on the slider
Adafruit_MQTT_Subscribe slider = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/slider", MQTT_QOS_1);

// Setup a feed called 'onoff' for subscribing to changes to the button
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoff", MQTT_QOS_1);

/*************************** Sketch Code ************************************/

void slidercallback(double x) {
  Serial.print("Hey we're in a slider callback, the slider value is: ");
  Serial.println(x);
}

void onoffcallback(char *data, uint16_t len) {
  Serial.print("Hey we're in a onoff callback, the button value is: ");
  Serial.println(data);
  String message = String(data);
      message.trim();
      if (message == "ON") {digitalWrite(D0, LOW);}
      if (message == "OFF") {digitalWrite(D0, HIGH);}}

//*********************************************** Declaracion de variables y pines *****************************/

/*Codigo para sensor infrarrojo*/
//se declara el pin donde se conecta el sensor
const int infrarrojo = D7;

/*Codigo para sensor ultrasonico*/
//se declara el emisor (el gatillo)
const int trigPin = D5;
//se declara el receptor (el eco)
const int echoPin = D6;
//se declaran dos variables que guardaran los valores
long duration; //long porque los datos son muy grandes
int distance;


/*Codigo para sensor DHT*/
//se incluye libreria de DHT
#include <DHT.h>

//se define el pin donde se conecta el sensor
#define DHTPIN D2 //depende de la board se escribe la letra o simplemente el numero, en caso del ESP82 se escribe la D y el numero

//se define el tipo de DHT que se utiliza, en este caso es el DHT11
#define DHTTYPE DHT11

//se inicializa el sensor DHT
DHT dht(DHTPIN, DHTTYPE);

//***************************************************** setup ********************************/
void setup() {
/*Codigo para sensor infrarrojo*/
//se declara que el infrarrojo es de entrada (INPUT)
  pinMode(infrarrojo, INPUT);


/*Codigo para sensor ultrasonico*/
//se declara que el gatillo es de salida (OUTPUT)
  pinMode(trigPin, OUTPUT);
//se declara que el eco es de entrada (INPUT)
  pinMode(echoPin, INPUT);


/*Codigo para DHT*/
dht.begin();


/*Codigo para conexion a internet*/
  pinMode(D0, OUTPUT);
  Serial.begin(115200);
  digitalWrite(D0, HIGH);
  delay(10);
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
  //********************************************* Callback Functions
  slider.setCallback(slidercallback);
  onoffbutton.setCallback(onoffcallback);
  
  // Setup MQTT subscription for feed's.
  mqtt.subscribe(&slider);
  mqtt.subscribe(&onoffbutton);

}

uint32_t x=0;
//***************************************************** Loop ********************************/
void loop() {
/*Codigo para infrarrojo*/
//se declara la variable para guardar los valores (los valores que envia el infrarrojo es de 1(HIGH) y 0(LOW))
int value = 0;
//la variable toma el valor que recibe del sensor
value = digitalRead(infrarrojo);
//usamos un if para mostrar el mensaje que si value es HIGH muestre que hay un obstaculo cerca, si no, dice que no hay obstaculo
if (value == HIGH) {
      Serial.println("Detectado obstaculo");
  } else if(value == LOW){
      Serial.println("No hay obstaculo");
  }



/*Codigo para DHT*/
//hacemos esperar 5 segundos entre medidas
delay(5000);
//leemos el dato que recibe el sensor para la humedad
float h = dht.readHumidity();

//leemos el dato que recibe el sensor para la temperatura (en centigrados)
float t = dht.readTemperature();

//leemos el dato que recibe el sensor para la temperatura (en fahreheit)
float f = dht.readTemperature(true);

//leemos la sensacion termica (indice de calor) medido en fahreheit
float stf = dht.computeHeatIndex(f, h);

//leemos la sensacion termica (indice de calor) medido en centigrados
float stc = dht.computeHeatIndex(t, h, false);

// Comprobamos si ha habido algún error en la lectura
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Error obteniendo los datos del sensor DHT11");
    return;
  }



/*Codigo para sensor ultrasonico*/
//primero limpiamos el gatillo
digitalWrite(trigPin,LOW);
delayMicroseconds(2);

//hacemos que el gatillo se active por 10 micro segundos
digitalWrite(trigPin,HIGH);
delayMicroseconds(10);
digitalWrite(trigPin,LOW);

//activamos el eco para que reciba el tiempo de cada onda
duration = pulseIn(echoPin, HIGH);

//calculamos la distancia
distance = duration*0.034/2;

/*si deseamos mostrar el resultado se debe ejecutar
Serial.print(distance);
*/


/*Codigo para publicar las vainas a Adafruit*/
//subiendo humedad
//NOTA: en realidad solo se necesita el ".publish();" pero se usa un if para saber si falla o no
//para publicar en adafruit debemos colocar el nombre del publish declarado arriba (donde dice "feeds for publishing") y el .publish
//por ejemplo un publish declarado se llamada humedad, para publicar colocamos humedad.publish() y en los parentesis colocamos la
//variable que corresponde al feed, en este caso la humedad es "h" entonces queda humedad.publish(h);
//lo demas son validaciones
 if(!humedad.publish(h)) {
    Serial.println("fallo humedad");
  } else {
    Serial.print("Humedad: ");
    Serial.print(h);
  }

 if(!temperatura.publish(t)) {
    Serial.println("fallo temperatura");
  } else {
    Serial.print("Temperatura: ");
    Serial.print(t);
  }

 if(!distancia.publish(distance)) {
    Serial.println("fallo distancia");
  } else {
    Serial.print("Distancia: ");
    Serial.print(distance);
  }

/*
Siguiendo con el ejemplo de sensacion termica, para mandar las cosas a adafruit llamamos al nombre donde se guarda el publish de 
sensacion termica y le colocamos el .publish con la variable correspondiente:
if(!sensacionT.publish(stc)) {
    Serial.println("fallo sensacion termica");
  } else {
    Serial.print("sensacion termica: ");
    Serial.print(stc);
  }
*/

/*Codigo para conectar a internet*/
  MQTT_connect();
  mqtt.processPackets(10000);
  if(! mqtt.ping()) {   // ping the server to keep the mqtt connection alive
    mqtt.disconnect();}
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
