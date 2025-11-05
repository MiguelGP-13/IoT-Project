#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// --------- WiFi ---------- 
const char* SSID = "SBC";
const char* PASSWORD = "SBCwifi$";

// --------- Broker MQTT ---------- 
const char* MQTT_HOST = "iot.etsisi.upm.es";
const uint16_t MQTT_PORT = 1883;
const char* GROUP_NAME = "grupo_06";  // Sustituir nn por numero de grupo. P. Ej: grupo_00

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// --------- Temporizador ----------
const unsigned long REFRESH_PERIOD = 1000;  // 1 segundo
unsigned long lastRefresh = 0;

// Contador para el ejemplo
unsigned int contador = 0;

// ------- WiFi STA -------
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Conectándose a la red Wi-Fi ...");
    delay(1000);
  }
  Serial.println("\n# Conexión Wi-Fi exitosa #");

  Serial.println("IP address: " + WiFi.localIP().toString());
}

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

// ------- MQTT -------
void initMqtt() {
  Serial.println("# Conectándose al broker MQTT #");
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);

  mqttClient.connect(GROUP_NAME);
  while (!mqttClient.connected()) {
    Serial.println("Connecting to MQTT Broker ...");
    delay(1000);
  }
  Serial.println("# Conectado al broker de MQTT #");
  Serial.println(String("# host: ") + MQTT_HOST);
  Serial.println(String("# port: ") + String(MQTT_PORT));
  Serial.println("# # # # # # # # # # # # # # # #");

  // Declaración de los Topics a los que se va a suscribir
  mqttClient.subscribe((String("iot/") + GROUP_NAME + "/in").c_str());
}

void setup() {
  Serial.begin(115200);
  Serial.println("Configurando Sketch");

  initWiFi();
  initMqtt();

  Serial.println("Iniciando Sketch");
}

unsigned long now;
void loop() {
  mqttClient.loop();
  
  now = millis();
  if (now - lastRefresh >= REFRESH_PERIOD) {
    lastRefresh = now;

    String payload;
    JsonDocument json;

    // Poblar el JSON
    json["sensor"] = "Contador";
    json["value"] = contador++;

    // Enviar el JSON por MQTT
    serializeJson(json, payload);
    mqttClient.publish((String("iot/") + GROUP_NAME + "/out").c_str(), payload.c_str());
  }
}
