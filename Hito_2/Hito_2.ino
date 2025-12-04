#include "seeed_bme680.h"
#include "Si115X.h"
#include "rgb_lcd.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>

#include "time.h"

#include <WiFi.h>  // Librería para WiFi
#include <PubSubClient.h>  // Librería para MQTT
#include <ArduinoJson.h>  // Librería para manejar JSON

#define MQTT_MAX_PACKET_SIZE 512

Seeed_BME680 sensor((uint8_t)0x76); // Dirección I2C por defecto: 0x76 o 0x77
Si115X sensorLuz;
rgb_lcd lcd;

const int colorR = 255;
const int colorG = 0;
const int colorB = 0;

// Credenciales WiFi
const char* ssid = "SBC";          // Cambia con tu SSID
const char* password = "SBCwifi$";  // Cambia con tu contraseña
// Configuración del Broker MQTT
const char* mqtt_server = "iot.etsisi.upm.es";  // Cambia por tu broker MQTT
const int mqtt_port = 1883;  // Puerto por defecto para MQTT sin cifrado

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Nombre del tópico en MQTT
const char* topic = "iot/grupo_06/weather_station/telemetry";

int t = 0;

// ------- WiFi STA -------
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Conectándose a la red Wi-Fi ...");
    delay(1000);
  }
  Serial.println("\n# Conexión Wi-Fi exitosa #");

  Serial.println("IP address: " + WiFi.localIP().toString());
}



const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;      // Offset horario (España peninsular = +1h en invierno)
const int   daylightOffset_sec = 0; // Horario de verano = +1h (3600) extra (Puesto a 0, porque es invierno)

String getTimestamp() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Error al obtener la hora");
    return "";
  }
  char buffer[30];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S%z", &timeinfo);
  return String(buffer);
}
/*
// ------- MQTT Subscription CallBacks -------
void mqttCallback(char* topic, uint8_t* payload, unsigned int length){
  Serial.println(String("Received msg on: ") + topic);

  // Parsear JSON
  JsonDocument json;
  deserializeJson(json, payload);

  // Mostrar por puerto serie el JSON
  serializeJson(json, Serial);
  Serial.println();

  // Lógica del programa
  if (String(topic) == String("iot/") + GROUP_NAME + "/in") {
    // Aquí puedes manejar el mensaje recibido
  }
}
*/

// ------- MQTT -------
void initMqtt() {
  Serial.println("# Conectándose al broker MQTT #");
  mqttClient.setServer(mqtt_server, mqtt_port);
  //mqttClient.setCallback(mqttCallback);

  mqttClient.connect("grupo_06");
  while (!mqttClient.connected()) {
    Serial.println("Connecting to MQTT Broker ...");
    delay(1000);
  }
  Serial.println("# Conectado al broker de MQTT #");
  Serial.println(String("# host: ") + mqtt_server);
  Serial.println(String("# port: ") + String(mqtt_port));
  Serial.println("# # # # # # # # # # # # # # # #");

  // Declaración de los Topics a los que se va a suscribir
  //mqttClient.subscribe((String("iot/") + GROUP_NAME + "/in").c_str());
}

/*
// Función de reconexión MQTT
void reconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Conectando al servidor MQTT...");
    if (mqttClient.connect("ESP32Client")) {
      Serial.println("Conectado al servidor MQTT");
    } else {
      Serial.print("Error de conexión, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" intentaremos nuevamente en 5 segundos.");
      delay(5000);
    }
  }
}
*/


void setup() {
  Serial.begin(9600);

  Wire.begin(); 
  sensor.init();
  sensorLuz.Begin();

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  //lcd.setRGB(colorR, colorG, colorB);

  initWiFi();
  initMqtt();
  // Configurar NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {
  mqttClient.loop();
  // Limpiar pantalla
  lcd.clear();

  // Sensor de luz (Vis - visible light, unit in lm IR - infrared light, unit in lm UV - UN index)

  Serial.println("------- MEDIDAS SENSOR LUZ -------");
  float vis = sensorLuz.ReadVisible();
  float ir = sensorLuz.ReadIR();

  Serial.print("Vis: "); Serial.println(vis);
  Serial.print("IR: "); Serial.println(ir);
 
  // Sensor genérico
  sensor.read_sensor_data();

  Serial.println("------- MEDIDAS SENSOR AMBIENTAL -------");
  float temp= sensor.sensor_result_value.temperature;
  Serial.print("Temperatura: ");
  Serial.print(temp);
  Serial.println(" °C");

  float hum= sensor.sensor_result_value.humidity;
  Serial.print("Humedad: ");
  Serial.print(hum);
  Serial.println(" %");

  float pres= sensor.sensor_result_value.pressure/100.0;
  Serial.print("Presión: ");
  Serial.print(pres);
  Serial.println(" hPa");

  float gas= sensor.sensor_result_value.gas/100.0;
  Serial.print("Gas: ");
  Serial.print(gas);
  Serial.println(" Kohms");
  Serial.println();


  // Escribir por pantalla los resultados
  
  // Temperatura
if (t == 0) {

  // ---- Fila 0 ----
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temp, 1);
  lcd.print("C");  // °C no siempre se muestra bien, por eso uso C

  lcd.setCursor(8, 0);
  lcd.print("L:");
  lcd.print(vis);
  lcd.print("lx");

  // ---- Fila 1 ----
  lcd.setCursor(0, 1);
  lcd.print("H:");
  lcd.print(hum, 1);
  lcd.print("%");

  lcd.setCursor(8, 1);
  lcd.print("I:");
  lcd.print(ir);

  t = 1; // Cambia para la siguiente pantalla
}

else if (t == 1) {

  // ---- Fila 0 ----
  lcd.setCursor(0, 0);
  lcd.print("P:");
  lcd.print(pres, 0);
  lcd.print("hPa");

  // ---- Fila 1 ----
  lcd.setCursor(0, 1);
  lcd.print("G:");
  lcd.print(gas);
  lcd.print("ppm");

  t = 0;
}



  // Crear el mensaje JSON predeterminado
  String payload;
  DynamicJsonDocument doc(1024);  // 1 KB suele bastar para tu JSON
   // tamaño fijo

    doc["n"] = "Estación meteorológica";
    doc["id"] = "grupo_06";
    //doc["description"] = "Estación meteorológica para SAT";
    doc["timestamp"] = getTimestamp();
    doc["latitude"] = 40.550;  // Ejemplo, reemplaza con valor real
    doc["longitude"] = -3.350; // Ejemplo, reemplaza con valor real
    doc["temperature"] = temp;
    doc["humidity"] = hum;
    doc["atmosphericPressure"] = pres;
    doc["illuminance"] = vis;
    doc["ir"] = ir;
    doc["gas"] = gas;

    serializeJson(doc, payload);
    Serial.print("Payload length: ");
    Serial.println(payload.length());
    Serial.println(payload);


    // Publicar en MQTT
    if (mqttClient.publish(("iot/" + String("grupo_06") + "/out").c_str(), payload.c_str())) {
      Serial.print("Publicado: ");
      Serial.println(payload);
    } else {
      Serial.println("\n\n\nError al publicar mensaje!!!!!!!\n\n\n");
    }
    delay(1000);
 }

