#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <Arduino.h>
#include <Preferences.h>
#include "config.h"
#include "sensors.h"

class SystemController {
private:
  Preferences _prefs;
  bool _autoMode;
  bool _pumpOn;
  bool _alarmActive;
  bool _buzzerToneState;
  unsigned long _lastBeepMs;

  int _soilDryPct;
  int _soilWetPct;
  int _waterMinPct;
  const char* _status;

  const int _ledPins[4] = {PIN_LED_1, PIN_LED_2, PIN_LED_3, PIN_LED_4};

  // Terapkan output ke pin Relay (GPIO19 Active-HIGH sesuai Code_test/5.2)
  void applyRelayOutput(bool turnOn) {
    bool pinLevel = RELAY_ACTIVE_HIGH ? turnOn : !turnOn;
    digitalWrite(PIN_RELAY, pinLevel ? HIGH : LOW);
  }

  // Terapkan output ke Buzzer Pasif (GPIO13 tone sesuai Code_test/9.1)
  void applyBuzzerOutput(bool soundOn) {
    if (BUZZER_PASSIVE) {
      if (soundOn) {
        tone(PIN_BUZZER, BUZZER_FREQ_HZ);
      } else {
        noTone(PIN_BUZZER);
      }
    } else {
      digitalWrite(PIN_BUZZER, soundOn ? HIGH : LOW);
    }
  }

  // Tampilkan level air tandon pada 4 LED onboard (sesuai Code_test/1.12)
  void updateLevelLEDs(int waterPct) {
    int activeLeds = 0;
    if (waterPct >= 80)      activeLeds = 4;
    else if (waterPct >= 55) activeLeds = 3;
    else if (waterPct >= 30) activeLeds = 2;
    else if (waterPct >= _waterMinPct) activeLeds = 1;

    for (int i = 0; i < 4; i++) {
      digitalWrite(_ledPins[i], (i < activeLeds) ? HIGH : LOW);
    }
  }

public:
  SystemController()
    : _autoMode(true),
      _pumpOn(false),
      _alarmActive(false),
      _buzzerToneState(false),
      _lastBeepMs(0),
      _soilDryPct(DEFAULT_SOIL_DRY_PCT),
      _soilWetPct(DEFAULT_SOIL_WET_PCT),
      _waterMinPct(DEFAULT_WATER_MIN_PCT),
      _status("NORMAL") {}

  // Inisialisasi pin aktuator dan muat konfigurasi dari Preferences (NVS)
  void begin() {
    pinMode(PIN_RELAY, OUTPUT);
    pinMode(PIN_BUZZER, OUTPUT);
    for (int i = 0; i < 4; i++) {
      pinMode(_ledPins[i], OUTPUT);
      digitalWrite(_ledPins[i], LOW);
    }
    applyRelayOutput(false);
    applyBuzzerOutput(false);

    // Baca ambang batas yang tersimpan di NVS
    _prefs.begin("sgg_cfg", false);
    _soilDryPct  = _prefs.getInt("soil_dry", DEFAULT_SOIL_DRY_PCT);
    _soilWetPct  = _prefs.getInt("soil_wet", DEFAULT_SOIL_WET_PCT);
    _waterMinPct = _prefs.getInt("water_min", DEFAULT_WATER_MIN_PCT);
    _autoMode    = _prefs.getBool("auto_mode", true);
  }

  // Evaluasi logika kontrol berdasarkan data sensor terbaru
  void evaluate(const SensorData& d) {
    updateLevelLEDs(d.water_level_pct);

    // Prioritas 1: Proteksi jika air tandon habis / di bawah batas minimum
    if (d.water_level_pct < _waterMinPct) {
      _pumpOn = false;
      _alarmActive = true;
      _status = "AIR TANDON HABIS";
      applyRelayOutput(false);
      return;
    }

    // Air tandon cukup -> matikan alarm buzzer
    if (_alarmActive) {
      _alarmActive = false;
      _buzzerToneState = false;
      applyBuzzerOutput(false);
    }

    // Prioritas 2: Mode AUTO dengan Hysteresis
    if (_autoMode) {
      if (d.soil_pct < _soilDryPct) {
        _pumpOn = true;   // Mulai menyiram saat tanah di bawah ambang kering
      } else if (d.soil_pct >= _soilWetPct) {
        _pumpOn = false;  // Berhenti menyiram setelah melewati ambang basah
      }
      // Di antara _soilDryPct dan _soilWetPct, status _pumpOn dipertahankan (hysteresis)
      _status = _pumpOn ? "MENYIRAM" : "NORMAL";
    } else {
      // Prioritas 3: Mode MANUAL
      if (_pumpOn) {
        _status = "MENYIRAM";
      } else if (d.soil_pct < _soilDryPct) {
        _status = "TANAH KERING";
      } else {
        _status = "NORMAL";
      }
    }

    applyRelayOutput(_pumpOn);
  }

  // Handler buzzer non-blocking (dipanggil terus di loop utama)
  void updateBuzzerNonBlocking(unsigned long nowMs) {
    if (!_alarmActive) return;
    if (nowMs - _lastBeepMs >= BUZZER_BEEP_MS) {
      _lastBeepMs = nowMs;
      _buzzerToneState = !_buzzerToneState;
      applyBuzzerOutput(_buzzerToneState);
    }
  }

  // Ubah mode kerja (AUTO / MANUAL) dan simpan ke NVS
  void setMode(bool isAuto, const SensorData& d) {
    _autoMode = isAuto;
    _prefs.putBool("auto_mode", _autoMode);
    evaluate(d);
  }

  // Kontrol pompa manual dari dashboard (hanya diizinkan jika mode MANUAL & air cukup)
  bool setManualPump(bool turnOn, const SensorData& d) {
    if (_autoMode || d.water_level_pct < _waterMinPct) {
      return false;
    }
    _pumpOn = turnOn;
    evaluate(d);
    return true;
  }

  // Simpan ambang batas baru ke Preferences (NVS)
  bool setThresholds(int soilDry, int soilWet, int waterMin, const SensorData& d) {
    if (soilDry < 5 || soilWet > 95 || soilDry >= soilWet || waterMin < 5 || waterMin > 80) {
      return false;
    }
    _soilDryPct  = soilDry;
    _soilWetPct  = soilWet;
    _waterMinPct = waterMin;
    _prefs.putInt("soil_dry", _soilDryPct);
    _prefs.putInt("soil_wet", _soilWetPct);
    _prefs.putInt("water_min", _waterMinPct);
    evaluate(d);
    return true;
  }

  bool isPumpOn() const     { return _pumpOn; }
  bool isAutoMode() const   { return _autoMode; }
  bool isAlarmOn() const    { return _alarmActive; }
  const char* getStatus() const { return _status; }
  int getSoilDry() const    { return _soilDryPct; }
  int getSoilWet() const    { return _soilWetPct; }
  int getWaterMin() const   { return _waterMinPct; }
};

#endif // CONTROLLER_H

