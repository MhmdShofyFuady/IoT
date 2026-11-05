#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include "config.h"
#include "sensors.h"
#include "controller.h"
#include "web_fallback.h"

class GardenWebServer {
private:
  WebServer _server;
  SensorManager& _sensors;
  SystemController& _ctrl;
  bool _fsMounted;

  // Helper ringan untuk membaca angka bulat dari JSON body tanpa library eksternal
  static int parseJsonInt(const String& body, const char* key, int defaultVal) {
    String pattern = String("\"") + key + "\":";
    int idx = body.indexOf(pattern);
    if (idx < 0) return defaultVal;
    idx += pattern.length();
    while (idx < (int)body.length() && (body[idx] == ' ' || body[idx] == '"')) idx++;
    return body.substring(idx).toInt();
  }

  // Helper ringan untuk membaca boolean dari JSON body
  static bool parseJsonBool(const String& body, const char* key, bool defaultVal) {
    String pattern = String("\"") + key + "\":";
    int idx = body.indexOf(pattern);
    if (idx < 0) return defaultVal;
    String sub = body.substring(idx + pattern.length());
    sub.trim();
    if (sub.startsWith("true") || sub.startsWith("1")) return true;
    if (sub.startsWith("false") || sub.startsWith("0")) return false;
    return defaultVal;
  }

  // Format detik uptime ke string HH:MM:SS
  static void formatUptime(unsigned long totalSec, char* buf, size_t len) {
    unsigned long h = totalSec / 3600UL;
    unsigned long m = (totalSec % 3600UL) / 60UL;
    unsigned long s = totalSec % 60UL;
    snprintf(buf, len, "%02lu:%02lu:%02lu", h, m, s);
  }

  // Layani file statis dari LittleFS, atau gunakan fallback PROGMEM jika belum di-upload
  void serveStaticOrFallback(const char* path, const char* mime, const char* fallbackProgmem) {
    if (_fsMounted && LittleFS.exists(path)) {
      File f = LittleFS.open(path, "r");
      if (f) {
        _server.streamFile(f, mime);
        f.close();
        return;
      }
    }
    _server.send_P(200, mime, fallbackProgmem);
  }

  // Endpoint GET /api/data
  void handleGetData() {
    const SensorData& d = _sensors.getData();
    unsigned long uptimeSec = millis() / 1000UL;
    char timeBuf[16];
    formatUptime(uptimeSec, timeBuf, sizeof(timeBuf));

    char json[512];
    snprintf(
      json,
      sizeof(json),
      "{\"soil_pct\":%d,\"soil_raw\":%d,\"temp_c\":%.1f,\"humidity_pct\":%.1f,"
      "\"water_level_pct\":%d,\"distance_cm\":%.1f,\"pump\":%s,\"mode\":\"%s\","
      "\"status\":\"%s\",\"uptime\":%lu,\"timestamp\":\"%s\",\"sim\":%s,"
      "\"thresholds\":{\"soil_dry\":%d,\"soil_wet\":%d,\"water_min\":%d}}",
      d.soil_pct,
      d.soil_raw,
      d.temp_c,
      d.humidity_pct,
      d.water_level_pct,
      d.distance_cm,
      _ctrl.isPumpOn() ? "true" : "false",
      _ctrl.isAutoMode() ? "AUTO" : "MANUAL",
      _ctrl.getStatus(),
      uptimeSec,
      timeBuf,
      _sensors.isSimulation() ? "true" : "false",
      _ctrl.getSoilDry(),
      _ctrl.getSoilWet(),
      _ctrl.getWaterMin()
    );
    _server.send(200, "application/json", json);
  }

  // Endpoint POST /api/pump
  void handlePostPump() {
    String body = _server.arg("plain");
    bool state = parseJsonBool(body, "state", false);
    bool ok = _ctrl.setManualPump(state, _sensors.getData());
    char resp[96];
    snprintf(resp, sizeof(resp), "{\"ok\":%s,\"pump\":%s}",
             ok ? "true" : "false",
             _ctrl.isPumpOn() ? "true" : "false");
    _server.send(ok ? 200 : 400, "application/json", resp);
  }

  // Endpoint POST /api/mode
  void handlePostMode() {
    String body = _server.arg("plain");
    bool isAuto = (body.indexOf("MANUAL") < 0);
    _ctrl.setMode(isAuto, _sensors.getData());
    char resp[96];
    snprintf(resp, sizeof(resp), "{\"ok\":true,\"mode\":\"%s\"}",
             _ctrl.isAutoMode() ? "AUTO" : "MANUAL");
    _server.send(200, "application/json", resp);
  }

  // Endpoint POST /api/threshold
  void handlePostThreshold() {
    String body = _server.arg("plain");
    int dry      = parseJsonInt(body, "soil_dry", _ctrl.getSoilDry());
    int wet      = parseJsonInt(body, "soil_wet", _ctrl.getSoilWet());
    int waterMin = parseJsonInt(body, "water_min", _ctrl.getWaterMin());
    bool ok = _ctrl.setThresholds(dry, wet, waterMin, _sensors.getData());
    _server.send(ok ? 200 : 400, "application/json",
                 ok ? "{\"ok\":true}" : "{\"ok\":false,\"error\":\"Ambang batas tidak valid\"}");
  }

  // Endpoint POST /api/sim (Mode Simulasi untuk uji tanpa hardware)
  void handlePostSim() {
    String body = _server.arg("plain");
    bool simOn   = parseJsonBool(body, "sim", !_sensors.isSimulation());
    int soilPct  = parseJsonInt(body, "soil_pct", -1);
    int waterPct = parseJsonInt(body, "water_level_pct", -1);
    _sensors.setSimulation(simOn, soilPct, waterPct);
    _ctrl.evaluate(_sensors.getData());
    _server.send(200, "application/json", "{\"ok\":true}");
  }

public:
  GardenWebServer(SensorManager& sensors, SystemController& ctrl)
    : _server(80), _sensors(sensors), _ctrl(ctrl), _fsMounted(false) {}

  // Inisialisasi LittleFS, koneksi WiFi STA + Fallback AP, dan route HTTP
  void begin() {
    _fsMounted = LittleFS.begin(true);
    if (_fsMounted) {
      Serial.println(F("[FS] LittleFS berhasil di-mount."));
    } else {
      Serial.println(F("[FS] LittleFS belum tersedia, menggunakan fallback PROGMEM."));
    }

    // Coba hubungkan ke WiFi STA terlebih dahulu
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_STA_SSID, WIFI_STA_PASS);
    Serial.print(F("[WiFi] Menghubungkan ke STA: "));
    Serial.print(WIFI_STA_SSID);

    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startAttempt) < WIFI_CONNECT_TIMEOUT_MS) {
      delay(250);
      Serial.print('.');
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
      Serial.print(F("[WiFi] Terhubung! Buka Dashboard di: http://"));
      Serial.println(WiFi.localIP());
    } else {
      // Fallback ke mode Access Point (AP) jika WiFi STA gagal
      Serial.println(F("[WiFi] Gagal konek STA. Mengaktifkan Fallback Access Point..."));
      WiFi.mode(WIFI_AP);
      WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASS);
      Serial.print(F("[WiFi] SSID AP: "));
      Serial.println(WIFI_AP_SSID);
      Serial.print(F("[WiFi] Buka Dashboard di: http://"));
      Serial.println(WiFi.softAPIP());
    }

    // Daftarkan rute file web & REST API
    _server.on("/", HTTP_GET, [this]() {
      serveStaticOrFallback("/index.html", "text/html", FALLBACK_INDEX_HTML);
    });
    _server.on("/index.html", HTTP_GET, [this]() {
      serveStaticOrFallback("/index.html", "text/html", FALLBACK_INDEX_HTML);
    });
    _server.on("/style.css", HTTP_GET, [this]() {
      serveStaticOrFallback("/style.css", "text/css", FALLBACK_STYLE_CSS);
    });
    _server.on("/app.js", HTTP_GET, [this]() {
      serveStaticOrFallback("/app.js", "application/javascript", FALLBACK_APP_JS);
    });

    _server.on("/api/data", HTTP_GET, [this]()      { handleGetData(); });
    _server.on("/api/pump", HTTP_POST, [this]()     { handlePostPump(); });
    _server.on("/api/mode", HTTP_POST, [this]()     { handlePostMode(); });
    _server.on("/api/threshold", HTTP_POST, [this](){ handlePostThreshold(); });
    _server.on("/api/sim", HTTP_POST, [this]()      { handlePostSim(); });

    _server.begin();
    Serial.println(F("[HTTP] Web server aktif pada port 80."));
  }

  void handleClient() {
    _server.handleClient();
  }
};

#endif // WEB_SERVER_H

