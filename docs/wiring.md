# Tabel Koneksi Pin (Wiring) — Smart Garden Guard

Konfigurasi pin pada project ini telah disesuaikan sepenuhnya dengan jalur hardware **Khurslabs ESP32 IoT Trainer Kit** yang terdapat pada direktori [`Code_test/`](file:///Users/muhammadshofyfuady/Documents/semester%205/IoT/Code_test).

---

## 1. Tabel Koneksi Pin Utama

| Komponen / Modul | Pin Komponen | Pin ESP32 (`Code_test`) | Mode / Karakteristik | Referensi Kode di `Code_test/` |
| :--- | :--- | :---: | :--- | :--- |
| **Soil Moisture Sensor** | `AOUT` (Analog) | **`GPIO34`** | `INPUT` (`ADC1_CH6`, 12-bit `0–4095`) | [`5.2-Soil_And_Relay.ino`](file:///Users/muhammadshofyfuady/Documents/semester%205/IoT/Code_test/5%20-%20Soil/5.2-Soil_And_Relay/5.2-Soil_And_Relay.ino) |
| **Sensor Suhu & Lembap DHT11** | `DATA` / `OUT` | **`GPIO32`** | Digital 1-Wire | [`7.3-Weather_Station_With_Relay.ino`](file:///Users/muhammadshofyfuady/Documents/semester%205/IoT/Code_test/7%20-%20DHT11/7.3-Weather_Station_With_Relay/7.3-Weather_Station_With_Relay.ino) |
| **Ultrasonic HC-SR04** | `TRIG` | **`GPIO5`** | `OUTPUT` (Pulsa trigger 10 µs) | [`9.1-Ultrasonic_And_Buzzer.ino`](file:///Users/muhammadshofyfuady/Documents/semester%205/IoT/Code_test/9%20-%20Ultrasonic/9.1-Ultrasonic_And_Buzzer/9.1-Ultrasonic_And_Buzzer.ino) |
| **Ultrasonic HC-SR04** | `ECHO` | **`GPIO18`** | `INPUT` (Durasi pulsa pantulan) | [`9.1-Ultrasonic_And_Buzzer.ino`](file:///Users/muhammadshofyfuady/Documents/semester%205/IoT/Code_test/9%20-%20Ultrasonic/9.1-Ultrasonic_And_Buzzer/9.1-Ultrasonic_And_Buzzer.ino) |
| **Modul Relay (Pompa Air)** | `IN` | **`GPIO19`** | `OUTPUT` (**Active-HIGH**: `HIGH` = ON) | [`5.2-Soil_And_Relay.ino`](file:///Users/muhammadshofyfuady/Documents/semester%205/IoT/Code_test/5%20-%20Soil/5.2-Soil_And_Relay/5.2-Soil_And_Relay.ino) |
| **Buzzer Alarm** | `+` / `IN` | **`GPIO13`** | `OUTPUT` (**Passive Buzzer**, `tone(13, 1000)`) | [`9.1-Ultrasonic_And_Buzzer.ino`](file:///Users/muhammadshofyfuady/Documents/semester%205/IoT/Code_test/9%20-%20Ultrasonic/9.1-Ultrasonic_And_Buzzer/9.1-Ultrasonic_And_Buzzer.ino) |
| **LED Indikator Air 1 (25%)** | Anoda | **`GPIO27`** (`LED4`) | `OUTPUT` (`HIGH` = Nyala) | [`1.12-Led_Level_With_Ultrasonic.ino`](file:///Users/muhammadshofyfuady/Documents/semester%205/IoT/Code_test/1%20-%20Lamp/1.12-Led_Level_With_Ultrasonic/1.12-Led_Level_With_Ultrasonic.ino) |
| **LED Indikator Air 2 (50%)** | Anoda | **`GPIO26`** (`LED3`) | `OUTPUT` (`HIGH` = Nyala) | [`1.12-Led_Level_With_Ultrasonic.ino`](file:///Users/muhammadshofyfuady/Documents/semester%205/IoT/Code_test/1%20-%20Lamp/1.12-Led_Level_With_Ultrasonic/1.12-Led_Level_With_Ultrasonic.ino) |
| **LED Indikator Air 3 (75%)** | Anoda | **`GPIO2`** (`LED2`) | `OUTPUT` (`HIGH` = Nyala) | [`1.12-Led_Level_With_Ultrasonic.ino`](file:///Users/muhammadshofyfuady/Documents/semester%205/IoT/Code_test/1%20-%20Lamp/1.12-Led_Level_With_Ultrasonic/1.12-Led_Level_With_Ultrasonic.ino) |
| **LED Indikator Air 4 (100%)** | Anoda | **`GPIO4`** (`LED1`) | `OUTPUT` (`HIGH` = Nyala) | [`1.12-Led_Level_With_Ultrasonic.ino`](file:///Users/muhammadshofyfuady/Documents/semester%205/IoT/Code_test/1%20-%20Lamp/1.12-Led_Level_With_Ultrasonic/1.12-Led_Level_With_Ultrasonic.ino) |

---

## 2. Alasan Perubahan Pin dari Rancangan Awal

Pada board praktikum di folder `IoT/Code_test`, pin **`GPIO4`**, **`GPIO26`**, dan **`GPIO27`** sudah terhubung secara fisik ke deretan **LED Indikator (`LED1`, `LED3`, `LED4`)**. Agar tidak terjadi bentrok jalur (*pin conflict*) pada PCB trainer kit:

1. **DHT11 (`GPIO4` → `GPIO32`)**:
   - Dipindahkan ke `GPIO32` sesuai jalur sensor DHT11 pada [`7.3-Weather_Station_With_Relay.ino`](file:///Users/muhammadshofyfuady/Documents/semester%205/IoT/Code_test/7%20-%20DHT11/7.3-Weather_Station_With_Relay/7.3-Weather_Station_With_Relay.ino).
2. **Relay Pompa (`GPIO26` → `GPIO19`)**:
   - Dipindahkan ke `GPIO19` (tipe pemicu **Active-HIGH**) sesuai jalur modul relay pada [`5.2-Soil_And_Relay.ino`](file:///Users/muhammadshofyfuady/Documents/semester%205/IoT/Code_test/5%20-%20Soil/5.2-Soil_And_Relay/5.2-Soil_And_Relay.ino).
3. **Buzzer (`GPIO27` → `GPIO13`)**:
   - Dipindahkan ke `GPIO13` (tipe **Passive Buzzer** menggunakan sinyal PWM `tone()`) sesuai jalur buzzer pada [`9.1-Ultrasonic_And_Buzzer.ino`](file:///Users/muhammadshofyfuady/Documents/semester%205/IoT/Code_test/9%20-%20Ultrasonic/9.1-Ultrasonic_And_Buzzer/9.1-Ultrasonic_And_Buzzer.ino).
4. **Pemanfaatan `GPIO4`, `GPIO2`, `GPIO26`, `GPIO27` (`LED1`–`LED4`)**:
   - Keempat LED onboard tetap dimanfaatkan sebagai indikator bar level air tandon secara langsung di board, mengadopsi logika pada [`1.12-Led_Level_With_Ultrasonic.ino`](file:///Users/muhammadshofyfuady/Documents/semester%205/IoT/Code_test/1%20-%20Lamp/1.12-Led_Level_With_Ultrasonic/1.12-Led_Level_With_Ultrasonic.ino).
5. **Pemilihan `GPIO34` pada Soil Moisture (`ADC1`)**:
   - `GPIO34` berada pada blok **ADC1 (`ADC1_CH6`)**. Pada ESP32, blok `ADC2` digunakan oleh driver internal WiFi sehingga pembacaan `analogRead()` pada pin `ADC2` akan gagal ketika WiFi aktif. Dengan menggunakan `GPIO34` (`ADC1`), pembacaan analog kelembapan tanah tetap akurat meskipun web server WiFi terus berjalan.

---

## 3. Catatan Pembagi Tegangan (Voltage Divider) HC-SR04

Jika merakit di luar trainer kit (menggunakan breadboard dengan suplai `VCC = 5V` pada HC-SR04), pin `ECHO` mengeluarkan sinyal logika `5V` sedangkan pin `GPIO18` ESP32 hanya toleran terhadap tegangan `3.3V`. Gunakan pembagi tegangan resistor:

```text
HC-SR04 ECHO (5V) ───[ R1 = 1 kΩ ]───┬───> ESP32 GPIO18 (~3.3V)
                                     │
                                [ R2 = 2 kΩ ]
                                     │
                                    GND
```
*(Pada Khurslabs Trainer Kit di folder `IoT/Code_test`, jalur pin `TRIG=5` dan `ECHO=18` sudah disesuaikan pada board sehingga siap langsung digunakan).*

