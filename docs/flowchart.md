# Flowchart Alur Kerja Sistem — Smart Garden Guard

Flowchart berikut menjelaskan cara kerja *loop* non-blocking (`millis()`), proteksi level air tandon, mekanisme *hysteresis* pada mode `AUTO`, serta kendali `MANUAL` melalui antarmuka web.

```mermaid
flowchart TD
    START(["Mulai (Boot ESP32)"]) --> INIT["Inisialisasi Pin Sensor & Aktuator:<br/>Soil(34), DHT11(32), HC-SR04(5/18),<br/>Relay(19), Buzzer(13), LED(27,26,2,4)"]
    INIT --> LOAD_NVS["Muat Ambang Batas & Mode dari Preferences (NVS)<br/>& Mount Partisi LittleFS"]
    LOAD_NVS --> WIFI_TRY{"Coba Konek WiFi STA<br/>(Maks 10 Detik)"}
    WIFI_TRY -- "Berhasil" --> START_SRV["Jalankan HTTP WebServer (Port 80)<br/>pada IP Lokal (STA)"]
    WIFI_TRY -- "Gagal / Timeout" --> START_AP["Aktifkan Fallback Access Point<br/>(SSID: SmartGardenGuard-AP, IP: 192.168.4.1)"]
    START_AP --> START_SRV

    START_SRV --> LOOP["Loop Utama Non-Blocking:<br/>1. webServer.handleClient()<br/>2. updateBuzzerNonBlocking(millis)"]
    LOOP --> CHECK_TIME{"Selisih millis() >= 2000 ms?"}
    CHECK_TIME -- "Belum" --> LOOP
    CHECK_TIME -- "Ya" --> READ_SENS["Baca Sensor:<br/>- Soil Moisture (%)<br/>- DHT11 (Suhu °C & Lembap %)<br/>- HC-SR04 (Level Air %)<br/>- Update 4 LED Indikator Air"]

    READ_SENS --> CHECK_WATER{"Level Air Tandon <<br/>Ambang Minimum (15%)?"}
    CHECK_WATER -- "Ya (Tandon Habis)" --> STATE_EMPTY["Proteksi Pompa:<br/>- Paksa Pompa OFF (GPIO19 LOW)<br/>- Aktifkan Buzzer Beep (GPIO13)<br/>- Status = AIR TANDON HABIS"]
    STATE_EMPTY --> LOOP

    CHECK_WATER -- "Tidak (Air Cukup)" --> BUZ_OFF["Matikan Alarm Buzzer (GPIO13)"]
    BUZ_OFF --> CHECK_MODE{"Mode Kerja == AUTO?"}

    CHECK_MODE -- "Ya (Mode AUTO)" --> CHECK_DRY{"Kelembapan Tanah <<br/>Ambang Kering (35%)?"}
    CHECK_DRY -- "Ya" --> AUTO_ON["Nyalakan Pompa (GPIO19 HIGH)<br/>Status = MENYIRAM"]
    CHECK_DRY -- "Tidak" --> CHECK_WET{"Kelembapan Tanah >=<br/>Ambang Basah (65%)?"}
    CHECK_WET -- "Ya" --> AUTO_OFF["Matikan Pompa (GPIO19 LOW)<br/>Status = NORMAL"]
    CHECK_WET -- "Tidak (Di Antara Ambang)" --> AUTO_HOLD["Pertahankan Status Pompa Sebelumnya<br/>(Hysteresis Mencegah Relay Flapping)"]
    AUTO_ON & AUTO_OFF & AUTO_HOLD --> LOOP

    CHECK_MODE -- "Tidak (Mode MANUAL)" --> CHECK_MAN_PUMP{"Tombol Pompa ON<br/>dari Dashboard?"}
    CHECK_MAN_PUMP -- "Ya" --> MAN_ON["Nyalakan Pompa (GPIO19 HIGH)<br/>Status = MENYIRAM"]
    CHECK_MAN_PUMP -- "Tidak" --> CHECK_MAN_DRY{"Kelembapan Tanah <<br/>Ambang Kering?"}
    CHECK_MAN_DRY -- "Ya" --> MAN_DRY["Pompa OFF (GPIO19 LOW)<br/>Status = TANAH KERING"]
    CHECK_MAN_DRY -- "Tidak" --> MAN_NORM["Pompa OFF (GPIO19 LOW)<br/>Status = NORMAL"]
    MAN_ON & MAN_DRY & MAN_NORM --> LOOP
```

