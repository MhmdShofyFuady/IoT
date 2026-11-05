# Smart Garden Guard — ESP32 Automatic Plant Watering System

Sistem penyiraman tanaman otomatis berbasis **ESP32** dengan dashboard web *real-time* di jaringan lokal (tanpa ketergantungan internet). Seluruh konfigurasi pin telah disesuaikan dengan **Khurslabs ESP32 IoT Trainer Kit** pada direktori [`Code_test/`](file:///Users/muhammadshofyfuady/Documents/semester%205/IoT/Code_test).

---

## 1. Struktur Direktori

```text
IoT/
├── README.md                          # Panduan upload, akses dashboard, & langkah demo
├── platformio.ini                     # Konfigurasi PlatformIO
├── firmware/
│   └── smart_garden_guard/
│       ├── smart_garden_guard.ino     # Sketch utama (setup & loop non-blocking millis)
│       ├── config.h                   # Kredensial WiFi STA/AP, pin Code_test, & kalibrasi
│       ├── sensors.h                  # Pembacaan Soil (34), DHT11 (32), HC-SR04 (5/18) & Simulasi
│       ├── controller.h               # Logika hysteresis, proteksi air, Buzzer (13), LED, & NVS
│       ├── web_server.h               # HTTP WebServer, REST API JSON, & LittleFS handler
│       └── web_fallback.h             # Cadangan file web di PROGMEM jika LittleFS belum di-flash
├── data/                              # File antarmuka web untuk partisi LittleFS
│   ├── index.html
│   ├── style.css
│   └── app.js
└── docs/
    ├── wiring.md                      # Tabel koneksi pin & alasan penyesuaian pin Code_test
    ├── block-diagram.md               # Diagram blok sistem (Mermaid)
    ├── flowchart.md                   # Flowchart alur kerja non-blocking & hysteresis (Mermaid)
    ├── test-plan.md                   # Tabel skenario pengujian sesuai rubrik dosen
    └── draft-laporan.md               # Draft laporan lengkap siap salin ke Word/PDF
```

---

## 2. Ringkasan Pin Hardware (`IoT/Code_test`)

| Komponen | Pin ESP32 | Karakteristik | Referensi `Code_test` |
| :--- | :---: | :--- | :--- |
| **Soil Moisture (Analog)** | `GPIO34` | `ADC1_CH6` (`0–4095` → `100%–0%`) | `5.2-Soil_And_Relay.ino` |
| **DHT11 (Suhu & Lembap)** | `GPIO32` | Digital 1-Wire (`°C` & `%`) | `7.3-Weather_Station_With_Relay.ino` |
| **Ultrasonic HC-SR04** | `TRIG: 5`, `ECHO: 18` | Tinggi air tandon (`5 cm` penuh – `30 cm` kosong) | `9.1` & `1.12` |
| **Relay Pompa Air** | `GPIO19` | **Active-HIGH** (`HIGH` = ON, `LOW` = OFF) | `5.2-Soil_And_Relay.ino` |
| **Buzzer Alarm** | `GPIO13` | **Passive Buzzer** (`tone(13, 1000)` / `noTone(13)`) | `9.1-Ultrasonic_And_Buzzer.ino` |
| **LED Indikator Air 1–4** | `GPIO27, 26, 2, 4` | Menyala bertingkat (0–4 LED) sesuai level air | `1.12-Led_Level_With_Ultrasonic.ino` |

---

## 3. Cara Upload Firmware & Filesystem

### Opsi A: Menggunakan Arduino IDE 2.x (Terinstal di Mac Anda)
1. **Buka Sketch**: Buka file [`firmware/smart_garden_guard/smart_garden_guard.ino`](file:///Users/muhammadshofyfuady/Documents/semester%205/IoT/firmware/smart_garden_guard/smart_garden_guard.ino) di **Arduino IDE**.
2. **Instal Library DHT (Satu Kali)**:
   - Buka **Library Manager** di sidebar kiri Arduino IDE.
   - Cari dan instal **`DHT sensor library`** (oleh *Adafruit*) beserta **`Adafruit Unified Sensor`**.
   - *(Semua library lain seperti `WiFi`, `WebServer`, `LittleFS`, dan `Preferences` sudah bawaan core ESP32).*
3. **Atur WiFi di `config.h`**:
   - Buka tab [`config.h`](file:///Users/muhammadshofyfuady/Documents/semester%205/IoT/firmware/smart_garden_guard/config.h) lalu sesuaikan `WIFI_STA_SSID` dan `WIFI_STA_PASS` dengan nama hotspot HP atau WiFi router Anda.
4. **Pilih Board & Upload**:
   - Pilih **Tools → Board → esp32 → ESP32 Dev Module** (atau *DOIT ESP32 DEVKIT V1*) dan pilih **Port** USB ESP32 Anda.
   - Klik tombol **Upload (→)**.
   - **Catatan Anti-Gagal**: Firmware sudah dilengkapi [`web_fallback.h`](file:///Users/muhammadshofyfuady/Documents/semester%205/IoT/firmware/smart_garden_guard/web_fallback.h) di memori program (`PROGMEM`). Artinya, begitu Anda klik **Upload**, dashboard web **langsung berfungsi penuh** bahkan sebelum Anda melakukan upload partisi `LittleFS`!
   - *(Opsional)* Jika ingin meng-upload folder `/data` ke partisi LittleFS lewat Arduino IDE 2.x, tekan `Cmd + Shift + P` → pilih **Upload LittleFS to Pico/ESP8266/ESP32** (jika plugin `arduino-littlefs-upload` terpasang).

### Opsi B: Menggunakan PlatformIO CLI / VSCode Extension
Jika menggunakan PlatformIO:
```bash
# 1. Upload kode firmware ke ESP32
pio run --target upload

# 2. Upload file web (/data) ke partisi LittleFS ESP32
pio run --target uploadfs

# 3. Buka Serial Monitor (115200 baud)
pio device monitor
```

---

## 4. Cara Mengakses Dashboard Web

1. Buka **Serial Monitor** pada baud rate **`115200`** lalu tekan tombol `RST` (Reset) di board ESP32.
2. **Jika Terhubung ke Hotspot/Router (Mode STA)**:
   - Serial Monitor akan menampilkan alamat IP lokal, misalnya:
     `[WiFi] Terhubung! Buka Dashboard di: http://192.168.43.128`
   - Pastikan HP atau laptop Anda terhubung ke hotspot/WiFi yang sama, lalu ketik alamat IP tersebut di browser (Chrome/Safari).
3. **Jika Tanpa Hotspot/Router (Fallback Mode AP)**:
   - Setelah 10 detik gagal menemukan WiFi STA, ESP32 otomatis memancarkan WiFi:
     - **SSID**: `SmartGardenGuard-AP`
     - **Password**: `12345678`
   - Hubungkan WiFi HP/laptop ke `SmartGardenGuard-AP`, lalu buka **`http://192.168.4.1`** di browser.

---

## 5. Cara Menguji Tiap Poin Demo di Depan Dosen

Berikut panduan langkah demi langkah untuk mendemonstrasikan 5 poin wajib penilaian:

### Poin 1: ESP32 Berhasil Terhubung ke Jaringan
- Tunjukkan output pada **Serial Monitor (`115200` baud)** saat ESP32 menyala yang menampilkan status koneksi WiFi dan alamat IP lokal.
- Buka alamat IP tersebut dari browser HP dan laptop secara bersamaan; tunjukkan badge hijau **`Terhubung`** serta waktu **`Terakhir update: HH:MM:SS`** dan **`Uptime`** yang terus berjalan setiap 2 detik.

### Poin 2: Sensor Membaca Perubahan Kondisi Objek
- **Sensor Soil Moisture (`GPIO34`)**: Angkat probe sensor ke udara kering (nilai turun mendekati `0–20%`), lalu tempelkan ke tisu/tanah basah (nilai naik ke `70–100%`).
- **Sensor Ultrasonic HC-SR04 (`GPIO5` & `GPIO18`)**: Dekatkan telapak tangan atau permukaan air ke depan sensor dari jarak `30 cm` menuju `5 cm`. Tunjukkan nilai **Level air tandon (`%`)** di web yang naik dari `0%` ke `100%` **sekaligus** ke-4 LED onboard (`GPIO27, 26, 2, 4`) yang menyala bertingkat (1 sampai 4 LED).
- **Sensor DHT11 (`GPIO32`)**: Dekatkan jari atau hembuskan napas hangat ke sensor DHT11 dan tunjukkan kenaikan angka **Suhu udara (`°C`)** dan **Kelembapan udara (`%`)**.
- *(Jika diuji tanpa hardware fisik)*: Klik tombol **Mode sensor fisik (Klik untuk simulasi)** di bagian bawah dashboard untuk mengaktifkan **Mode Simulasi**, lalu gunakan 3 tombol skenario cepat.

### Poin 3: Data Sensor Dikirim dan Ditampilkan Otomatis pada Web
- Tunjukkan bahwa tanpa menekan tombol *Refresh* browser, angka keempat kartu sensor, waktu pembaruan terakhir, serta kurva pada **Grafik riwayat sensor** bergeser dan ter-update otomatis setiap **2 detik** melalui endpoint `GET /api/data`.

### Poin 4: Status pada Antarmuka Sesuai Kondisi Perangkat
Peragakan keempat status sistem:
1. **`NORMAL`** (Badge Hijau): Saat tanah cukup lembap (`>= 35%`) dan air tandon cukup (`>= 15%`).
2. **`MENYIRAM`** (Badge Biru): Saat pompa menyala (baik otomatis karena tanah `< 35%` maupun dinyalakan manual).
3. **`TANAH KERING`** (Badge Oranye): Ubah ke mode **Manual**, matikan pompa, lalu keringkan sensor tanah (`< 35%`).
4. **`AIR TANDON HABIS`** (Badge Merah + Banner Peringatan): Jauhkan objek dari sensor HC-SR04 (`> 26 cm` / air `< 15%`) atau klik tombol **Simulasi tandon habis**.

### Poin 5: Aktuator Bekerja Sesuai Rancangan (Hysteresis, Manual, & Proteksi)
1. **Demo Otomatis & Hysteresis (`AUTO`)**:
   - Pastikan mode **Otomatis** terpilih (default ambang kering `35%`, ambang basah `65%`).
   - Keringkan sensor tanah di bawah `35%` → **Relay (`GPIO19`) menyala** (`Status: MENYIRAM`).
   - Basahi sedikit hingga sekitar `50%` → tunjukkan ke dosen bahwa **Relay tetap menyala** (tidak *flapping* cetak-cetek) karena belum melewati ambang basah `65%`.
   - Basahi hingga melewati `65%` → **Relay otomatis mati** (`Status: NORMAL`).
2. **Demo Kendali Manual (`MANUAL`)**:
   - Klik tombol **Manual** di dashboard, lalu klik **Nyalakan pompa** dan **Matikan pompa**. Tunjukkan relay langsung merespons klik dari web.
3. **Demo Proteksi Pompa & Alarm Buzzer (`AIR TANDON HABIS`)**:
   - Saat pompa sedang menyala, buat level air tandon di bawah `15%` (jauhkan HC-SR04 > `26 cm` atau klik **Simulasi tandon habis**).
   - Tunjukkan bahwa **Relay pompa langsung dipaksa OFF**, tombol pompa di web terkunci (*disabled*), dan **Buzzer (`GPIO13`) berbunyi *beep* berkala**.
4. **Demo Penyimpanan Ambang Batas ke NVS (`Preferences`)**:
   - Ubah nilai **Ambang kering (%)** menjadi `40` dan **Ambang basah (%)** menjadi `70`, lalu klik **Simpan ke memori**.
   - Tekan tombol **Reset (`RST`)** pada ESP32; setelah menyala kembali, tunjukkan bahwa angka `40%` dan `70%` tetap tersimpan permanen.

