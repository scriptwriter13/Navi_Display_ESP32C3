# Hardware Mapping - BikeNav_C3

Dieses Dokument beschreibt die Pin-Belegung des ESP32-C3 für das 1.28" Round Display.

## Display (SPI)
| Pin (ESP32-C3) | Funktion | Variable in config.h |
| :--- | :--- | :--- |
| GPIO 10 | Backlight (PWM) | `TFT_BL` |
| GPIO 2 | Data/Command | `DC_PIN` |
| GPIO 1 | Chip Select | `CS_PIN` |
| GPIO 41 | SPI Clock | `SCK_PIN` |
| GPIO 40 | SPI MOSI (SDA) | `MOSI_PIN` |
| GPIO 42 | Reset | `RST_PIN` |

## Touch Controller (I2C)
| Pin (ESP32-C3) | Funktion | Variable in config.h |
| :--- | :--- | :--- |
| GPIO 4 | SDA | `TOUCH_SDA` |
| GPIO 5 | SCL | `TOUCH_SCL` |
| GPIO 0 | Interrupt | `TOUCH_INT` |
| GPIO 1 | Reset (Shared) | `TOUCH_RST` |