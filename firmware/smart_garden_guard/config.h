#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// KONFIGURASI JARINGAN WIFI (STA + FALLBACK AP)
// ============================================================================
// Ganti dengan nama hotspot HP atau router WiFi lokal Anda
#define WIFI_STA_SSID           "Hotspot_IoT"
#define WIFI_STA_PASS           "12345678"
#define WIFI_CONNECT_TIMEOUT_MS 10000UL

// Mode Access Point (AP) otomatis aktif jika gagal konek ke WiFi STA
#define WIFI_AP_SSID            "SmartGardenGuard-AP"
#define WIFI_AP_PASS            "12345678"

// ============================================================================
// KONFIGURASI PIN HARDWARE (SESUAI BOARD DI FOLDER IoT/Code_test)
// ============================================================================
// Referensi:
// - 5.2-Soil_And_Relay.ino        : Soil = GPIO34, Relay = GPIO19 (Active-HIGH)
// - 7.3-Weather_Station_With_Relay: DHT11 = GPIO32
// - 9.1-Ultrasonic_And_Buzzer.ino : TRIG = GPIO5, ECHO = GPIO18, Buzzer = GPIO13
// - 1.12-Led_Level_With_Ultrasonic: LED Indikator = GPIO27, GPIO26, GPIO2, GPIO4
#define PIN_SOIL                34  // ADC1_CH6 (input-only, aman saat WiFi aktif)
#define PIN_DHT                 32  // Digital 1-Wire untuk DHT11
#define DHT_TYPE                DHT11
#define PIN_TRIG                5   // Trigger Ultrasonic HC-SR04
#define PIN_ECHO                18  // Echo Ultrasonic HC-SR04
#define PIN_RELAY               19  // Modul Relay Pompa
#define PIN_BUZZER              13  // Buzzer Pasif (menggunakan fungsi tone)

// 4 LED onboard sebagai indikator fisik level air tandon (urutan sesuai 1.12)
#define PIN_LED_1               27
#define PIN_LED_2               26
#define PIN_LED_3               2
#define PIN_LED_4               4

// Karakteristik pemicu aktuator sesuai Code_test/5.2 & 9.1
#define RELAY_ACTIVE_HIGH       true   // HIGH = Pompa ON, LOW = Pompa OFF
#define BUZZER_PASSIVE          true   // true = tone(1000Hz), false = digitalWrite
#define BUZZER_FREQ_HZ          1000   // Frekuensi nada alarm buzzer (Hz)

// ============================================================================
// KALIBRASI SENSOR (MUDAH DIUBAH SESUAI KONDISI FISIK)
// ============================================================================
// Kalibrasi ADC Soil Moisture (12-bit: 0 - 4095, sesuai 5.2-Soil_And_Relay.ino)
// SOIL_RAW_MIN = nilai ADC saat basah maksimal (100%)
// SOIL_RAW_MAX = nilai ADC saat kering di udara (0%)
#define SOIL_RAW_MIN            0
#define SOIL_RAW_MAX            4095

// Kalibrasi jarak Ultrasonic HC-SR04 untuk tinggi air tandon (dalam cm)
// Sesuai 1.12-Led_Level_With_Ultrasonic.ino (jarak <= 7cm penuh, > 30cm kosong)
#define TANK_FULL_CM            5.0f   // Jarak sensor ke air saat tandon 100% penuh
#define TANK_EMPTY_CM           30.0f  // Jarak sensor ke dasar saat tandon 0% kosong
#define ULTRASONIC_TIMEOUT_US   25000UL // Timeout pulseIn 25ms (~4m) agar non-blocking

// ============================================================================
// AMBANG BATAS DEFAULT (DISIMPAN KE PREFERENCES / NVS)
// ============================================================================
#define DEFAULT_SOIL_DRY_PCT    35     // Pompa ON jika soil_pct < 35%
#define DEFAULT_SOIL_WET_PCT    65     // Pompa OFF jika soil_pct >= 65% (hysteresis)
#define DEFAULT_WATER_MIN_PCT   15     // Pompa paksa OFF & buzzer ON jika air < 15%

// ============================================================================
// INTERVAL WAKTU NON-BLOCKING (MILLIS) & MODE SIMULASI
// ============================================================================
#define SENSOR_INTERVAL_MS      2000UL // Interval pembacaan semua sensor (2 detik)
#define BUZZER_BEEP_MS          500UL  // Interval bunyi putus-putus buzzer alarm

// Ubah ke true jika ingin langsung menyala dalam mode simulasi tanpa sensor fisik
// (Mode simulasi juga dapat diubah kapan saja lewat tombol di Dashboard Web)
#define SIMULATION_MODE_DEFAULT false

#endif // CONFIG_H

