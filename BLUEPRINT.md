# Blueprint: Bike-Navi Display (Anker: display)
**Datum:** 25. Januar 2026
**Hardware:** ESP32-C3 (2424S012 - Round Display)
**Status:** Master-Build Stabil (17% Flash)

## 1. System-Architektur
Das Projekt ist in drei logische Einheiten getrennt:
* **`hal.h`**: Hardware Abstraction Layer (Pins, BLE-Power, Timeouts).
* **`config.h`**: App-Branding (UUIDs, Farben, Schwellenwerte).
* **`main.cpp`**: Logik-Engine (BLE-Parser, Icon-Renderer, UI-State).

## 2. Hardware-Konfiguration (hal.h)
| Komponente | Pin / Wert | Info |
| :--- | :--- | :--- |
| **TFT Driver** | GC9A01 | 240x240 Round |
| **TFT Backlight** | GPIO 3 | PWM-fähig |
| **Touch SDA/SCL**| GPIO 4 / 5 | CST816S Treiber |
| **BLE TX Power** | ESP_PWR_LVL_N3 | Optimiert auf 3m |
| **Sleep Timeout** | 30000 ms | 30s Inaktivität |

## 3. BLE Interface (config.h)
* **Device Name:** "Navi-Pro-V10"
* **Service-UUID:** `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`
* **Filter-Modus:** Service-UUID steht im Advertising-Paket (Android-optimiert).
* **Intervalle:** 20ms - 40ms (extrem schneller Reconnect).

## 4. Software-Features
* **Start-Screen:** Zeigt "warte auf Android-App" und die Geräte-MAC zur Diagnose.
* **Parser:**
    * `NAV:<Anweisung>`: Extrahiert Text und Distanz (m/km).
    * `SPD:<Zahl>`: Berechnet Echtzeit-Distanzabzug zwischen Navi-Updates.
* **Gestik:**
    * *Double-Click:* Rotiert Display (0-3).
    * *Long-Press:* Wechselt Tag/Nacht-Modus.
* **Persistence:** Alle Einstellungen (Modus/Rotation) werden im NVS gespeichert.

## 5. PlatformIO Settings
```ini
board_build.partitions = huge_app.csv
build_flags = 
    -D MODEL_ESP32_2424S012=1
    -D USER_SETUP_LOADED=1