/*
  ============================================================================
  SMART GARDEN GUARD — Sistem Penyiraman Tanaman Otomatis Berbasis ESP32
  ============================================================================
  Konfigurasi Hardware (Disesuaikan dengan Board di Folder IoT/Code_test):
  - Soil Moisture (Analog) : GPIO34 (ADC1_CH6, sesuai 5.2-Soil_And_Relay.ino)
  - DHT11 (Suhu & Lembap)  : GPIO32 (sesuai 7.3-Weather_Station_With_Relay.ino)
  - Ultrasonic HC-SR04     : TRIG GPIO5, ECHO GPIO18 (sesuai 9.1 & 1.12)
  - Relay Pompa            : GPIO19 Active-HIGH (sesuai 5.2-Soil_And_Relay.ino)
  - Buzzer Pasif           : GPIO13 dengan fungsi tone() (sesuai 9.1)
  - LED Indikator Air 1..4 : GPIO27, GPIO26, GPIO2, GPIO4 (sesuai 1.12)
  ============================================================================
*/

#include "config.h"
#include "sensors.h"
#include "controller.h"
#include "web_server.h"

SensorManager    sensors;
SystemController controller;
GardenWebServer  webServer(sensors, controller);

unsigned long lastSensorReadMs = 0;

void setup() {
  Serial.begin(115200);
  Serial.println(F("\n=== SMART GARDEN GUARD STARTING ==="));

  // Inisialisasi sensor, aktuator + Preferences NVS, dan web server
  sensors.begin();
  controller.begin();

  // Pembacaan awal saat boot
  sensors.update(controller.isPumpOn());
  controller.evaluate(sensors.getData());

  webServer.begin();
}

void loop() {
  unsigned long nowMs = millis();

  // 1. Layani permintaan HTTP dari browser secara responsif
  webServer.handleClient();

  // 2. Jalankan bunyi putus-putus buzzer secara non-blocking jika air tandon habis
  controller.updateBuzzerNonBlocking(nowMs);

  // 3. Baca semua sensor dan evaluasi logika kontrol setiap 2 detik (tanpa delay)
  if (nowMs - lastSensorReadMs >= SENSOR_INTERVAL_MS) {
    lastSensorReadMs = nowMs;

    sensors.update(controller.isPumpOn());
    const SensorData& d = sensors.getData();
    controller.evaluate(d);

    // Tampilkan ringkasan telemetri di Serial Monitor untuk kebutuhan demo
    Serial.printf(
      "[DATA] Tanah:%d%% (ADC:%d) | Suhu:%.1fC | Lembap:%.1f%% | Air:%d%% (%.1fcm) | Mode:%s | Pompa:%s | Status:%s\n",
      d.soil_pct,
      d.soil_raw,
      d.temp_c,
      d.humidity_pct,
      d.water_level_pct,
      d.distance_cm,
      controller.isAutoMode() ? "AUTO" : "MANUAL",
      controller.isPumpOn() ? "ON" : "OFF",
      controller.getStatus()
    );
  }
}

