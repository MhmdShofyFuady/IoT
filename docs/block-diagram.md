# Diagram Blok Sistem — Smart Garden Guard

Diagram blok di bawah ini menggambarkan arsitektur perangkat keras (sensor dan aktuator pada board `IoT/Code_test`), unit pemrosesan utama ESP32, penyimpanan non-volatil (NVS & LittleFS), serta komunikasi HTTP REST API ke dashboard web di jaringan lokal.

```mermaid
flowchart LR
    subgraph Input["Unit Input (Sensor)"]
        SOIL["Soil Moisture Sensor<br/>(Analog ADC1 - GPIO34)"]
        DHT["Sensor DHT11<br/>(Suhu & Lembap - GPIO32)"]
        ULT["Ultrasonic HC-SR04<br/>(TRIG GPIO5, ECHO GPIO18)"]
    end

    subgraph MCU["Unit Pemroses & Penyimpanan (ESP32)"]
        ADC["Pembacaan Berkala<br/>(Non-blocking millis 2s)"]
        CTRL["State Machine &<br/>Hysteresis Controller"]
        NVS["Preferences (NVS)<br/>(Simpan Ambang Batas)"]
        LFS["Partisi LittleFS / PROGMEM<br/>(HTML, CSS, JS)"]
        HTTP["HTTP WebServer (Port 80)<br/>(WiFi STA / Fallback AP)"]
    end

    subgraph Output["Unit Output (Aktuator & Indikator)"]
        RELAY["Modul Relay + Pompa Air<br/>(Active-HIGH - GPIO19)"]
        BUZ["Passive Buzzer Alarm<br/>(PWM Tone - GPIO13)"]
        LEDS["4 LED Indikator Level Air<br/>(GPIO27, 26, 2, 4)"]
    end

    subgraph User["Antarmuka Pengguna (Jaringan Lokal)"]
        DASH["Dashboard Web Responsif<br/>(Polling JSON /api/data 2s)"]
    end

    SOIL --> ADC
    DHT --> ADC
    ULT --> ADC
    ADC --> CTRL
    NVS <--> CTRL
    CTRL --> RELAY
    CTRL --> BUZ
    CTRL --> LEDS
    CTRL <--> HTTP
    LFS --> HTTP
    HTTP <-->|"REST API JSON via WiFi Lokal"| DASH
```

