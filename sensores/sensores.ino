#include "seeed_bme680.h"

Seeed_BME680 sensor((uint8_t)0x76); // Dirección I2C por defecto: 0x76 o 0x77

void setup() {
  Serial.begin(9600);
  sensor.init();
}

void loop() {
  sensor.read_sensor_data();

  Serial.print("Temperatura: ");
  Serial.print(sensor.read_pressure());
  Serial.println(" °C");
/*
  Serial.print("Humedad: ");
  Serial.print(sensor.rae);
  Serial.println(" %");

  Serial.print("Presión: ");
  Serial.print(sensor.sensor_result_value.pressure);
  Serial.println(" hPa");
*/
  delay(2000);
}
