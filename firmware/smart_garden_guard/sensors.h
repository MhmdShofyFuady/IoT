#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <DHT.h>
#include "config.h"

// Struktur data hasil pembacaan seluruh sensor
struct SensorData {
  int   soil_raw;         // Nilai ADC mentah soil moisture (0 - 4095)
  int   soil_pct;         // Kelembapan tanah dalam persen (0 - 100%)
  float temp_c;           // Suhu udara dari DHT11 (°C)
  float humidity_pct;     // Kelembapan udara dari DHT11 (%)
  float distance_cm;      // Jarak permukaan air dari HC-SR04 (cm)
  int   water_level_pct;  // Level air tandon dalam persen (0 - 100%)
};

class SensorManager {
private:
  DHT _dht;
  SensorData _data;
  bool _simMode;

public:
  SensorManager() : _dht(PIN_DHT, DHT_TYPE), _simMode(SIMULATION_MODE_DEFAULT) {
    _data.soil_raw = 2048;
    _data.soil_pct = 55;
    _data.temp_c = 28.5f;
    _data.humidity_pct = 68.0f;
    _data.distance_cm = 12.0f;
    _data.water_level_pct = 72;
  }

  // Inisialisasi pin sensor sesuai konfigurasi folder IoT/Code_test
  void begin() {
    pinMode(PIN_SOIL, INPUT);
    pinMode(PIN_TRIG, OUTPUT);
    pinMode(PIN_ECHO, INPUT);
    digitalWrite(PIN_TRIG, LOW);
    _dht.begin();
  }

  // Baca nilai kelembapan tanah dari GPIO34 (sesuai 5.2-Soil_And_Relay.ino)
  void readSoil() {
    _data.soil_raw = analogRead(PIN_SOIL);
    int pct = map(_data.soil_raw, SOIL_RAW_MIN, SOIL_RAW_MAX, 100, 0);
    _data.soil_pct = constrain(pct, 0, 100);
  }

  // Baca suhu dan kelembapan udara dari DHT11 di GPIO32 (sesuai 7.3)
  void readDHT() {
    float t = _dht.readTemperature();
    float h = _dht.readHumidity();
    // Gunakan nilai sebelumnya jika pembacaan gagal (NaN)
    if (!isnan(t)) _data.temp_c = t;
    if (!isnan(h)) _data.humidity_pct = h;
  }

  // Baca jarak dan level air tandon dari HC-SR04 di GPIO5 & GPIO18 (sesuai 1.12 & 9.1)
  void readUltrasonic() {
    digitalWrite(PIN_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIG, LOW);

    long duration = pulseIn(PIN_ECHO, HIGH, ULTRASONIC_TIMEOUT_US);
    if (duration > 0) {
      _data.distance_cm = (duration * 0.0343f) / 2.0f;
    }

    // Konversi jarak (cm) menjadi persentase level air tandon (0 - 100%)
    float ratio = (TANK_EMPTY_CM - _data.distance_cm) / (TANK_EMPTY_CM - TANK_FULL_CM);
    int pct = (int)(ratio * 100.0f);
    _data.water_level_pct = constrain(pct, 0, 100);
  }

  // Pembaruan data berkala (dipanggil tiap SENSOR_INTERVAL_MS menggunakan millis)
  void update(bool pumpActive) {
    if (_simMode) {
      updateSimulation(pumpActive);
      return;
    }
    readSoil();
    readDHT();
    readUltrasonic();
  }

  // Generator data sensor dummy untuk pengujian dashboard tanpa hardware fisik
  void updateSimulation(bool pumpActive) {
    if (pumpActive) {
      // Saat pompa menyala: tanah bertambah basah, air tandon sedikit berkurang
      _data.soil_pct = constrain(_data.soil_pct + 5, 0, 100);
      _data.water_level_pct = constrain(_data.water_level_pct - 2, 0, 100);
    } else {
      // Saat pompa mati: tanah perlahan mengering
      _data.soil_pct = constrain(_data.soil_pct - 2, 15, 100);
    }
    _data.soil_raw = map(_data.soil_pct, 100, 0, SOIL_RAW_MIN, SOIL_RAW_MAX);
    _data.distance_cm = TANK_EMPTY_CM - (_data.water_level_pct / 100.0f) * (TANK_EMPTY_CM - TANK_FULL_CM);

    // Fluktuasi kecil pada suhu dan kelembapan udara
    float deltaT = ((random(-10, 11)) * 0.05f);
    _data.temp_c = constrain(_data.temp_c + deltaT, 25.0f, 34.0f);
    float deltaH = ((random(-10, 11)) * 0.1f);
    _data.humidity_pct = constrain(_data.humidity_pct + deltaH, 55.0f, 88.0f);
  }

  // Atur mode simulasi dan nilai uji secara langsung dari dashboard
  void setSimulation(bool enabled, int soilPct = -1, int waterPct = -1, float tempC = -1.0f) {
    _simMode = enabled;
    if (soilPct >= 0)  _data.soil_pct = constrain(soilPct, 0, 100);
    if (waterPct >= 0) _data.water_level_pct = constrain(waterPct, 0, 100);
    if (tempC > 0.0f)  _data.temp_c = tempC;
  }

  bool isSimulation() const { return _simMode; }
  const SensorData& getData() const { return _data; }
};

#endif // SENSORS_H

