#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// --------- WiFi ---------- 
const char* SSID = "SBC";
const char* PASSWORD = "SBCwifi$";

// --------- Web ----------
WebServer server(80);  // Puerto: 80 (http)

// --------- Página web ----------
const char WEB_PAGE[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html>
<body>
  <h1>Telemetries</h1>
  <p id="telemetry" class="value"></p>

  <script>
    const telemetry = document.getElementById('telemetry');

    async function getTelemetry() {
      try {
        const res = await fetch('/api/telemetries', { cache: 'no-store' });
        if (!res.ok) throw new Error(`HTTP ${res.status}`);
        telemetry.innerHTML = await res.text();
      } catch (e) {
        console.error(e);
      }
    }
    
    setInterval(getTelemetry, 1000);  // Auto refresh (1 seg)
    getTelemetry();  // First time
  </script>
</body>
</html>
)HTML";

// --------- Temporizador ----------
const unsigned long REFRESH_PERIOD = 1000;  // 1 segundo
unsigned long lastRefresh = 0;

// --------- Telemetrías (variables globales) ----------
float telemetry_1 = 0.0;
float telemetry_2;

// ------- Logica de sensores para obtención de telemetrias -------
void getTelemetries () {
  telemetry_1++;
  telemetry_2 = telemetry_1 * 2.0;
}

// ------- Handlers HTTP ------
void sendTelemetries() {
  server.send(200, "text/html", (String) 
    "Telemetry 1: " + telemetry_1 + "<br>" + 
    "Telemetry 2: " + telemetry_2
  );
}

void initWebServer() {
  server.on("/", []() {server.send(200, "text/html; charset=utf-8", WEB_PAGE);});
  server.on("/api/telemetries", HTTP_GET, sendTelemetries);

  server.begin();
}

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

void setup() {
  Serial.begin(115200);

  initWiFi();
  initWebServer();
}

unsigned long now;
void loop() {
  server.handleClient();  // Atender peticiones web en cada iteracion

  // Temporizador no bloqueante para actualizar telemetrias
  now = millis();
  if (now - lastRefresh >= REFRESH_PERIOD) {
    lastRefresh = now;
    getTelemetries();
  }
}