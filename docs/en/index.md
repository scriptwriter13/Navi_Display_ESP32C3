# BikeNav-HUD Documentation

Welcome to the official documentation for the **BikeNav-HUD**.

## Overview
BikeNav-HUD is a minimalist, Bluetooth-based Head-Up Display (HUD) for bicycles, based on the ESP32-C3 and a 1.28" round display (GC9A01). It provides precise turn-by-turn directions directly in the rider's field of view.

## Key Features
* **Real-time Navigation**: Display of turn icons, distances, and street names.
* **Smart Preview**: Predictive icon for the next maneuver ("Then turn left").
* **Dynamic Distance Calculation**: Automatic countdown of meters based on current speed (`SPD-CALC`), even when the app is not sending data.
* **Gesture Control**: Brightness adjustment and display rotation (0-270°) via touch gestures.
* **Parser Robustness**: The system is hardened against "0 m" error messages from OsmAnd and processes meta-data reliably.

## Installation
1. **Firmware**: Compile the project using [PlatformIO](https://platformio.org/).
2. **App**: Install the Android Bridge App to send navigation data from apps like OsmAnd or Organic Maps to the HUD.
3. **Pairing**: Search for "BikeNav_C3" in the app.

## Hardware
* **Controller**: ESP32-C3 (RISC-V).
* **Display**: 1.28-inch Round-LCD, 240x240 pixels, IPS.
* **Touch**: Capacitive CST816S controller.

## Troubleshooting
* **Parser Errors**: If navigation data is not displayed correctly, check the serial output (`>>> RX-RAW`). The system is optimized for the OsmAnd protocol.
* **Connection**: Ensure Bluetooth is active on your smartphone and the Bridge App has notification access permissions.
# BikeNav-HUD Documentation

Welcome to the official documentation for the **BikeNav-HUD**.

## Overview
BikeNav-HUD is a minimalist, Bluetooth-based Head-Up Display (HUD) for bicycles, based on the ESP32-C3 and a 1.28" round display (GC9A01). It provides precise turn-by-turn directions directly in the rider's field of view.

## Key Features
* **Real-time Navigation**: Display of turn icons, distances, and street names.
* **Smart Preview**: Predictive icon for the next maneuver ("Then turn left").
* **Dynamic Distance Calculation**: Automatic countdown of meters based on current speed (`SPD-CALC`), even when the app is not sending data.
* **Gesture Control**: Brightness adjustment and display rotation (0-270°) via touch gestures.
* **Parser Robustness**: The system is hardened against "0 m" error messages from OsmAnd and processes meta-data reliably.

## Installation
1. **Firmware**: Compile the project using [PlatformIO](https://platformio.org/).
2. **App**: Install the Android Bridge App to send navigation data from apps like OsmAnd or Organic Maps to the HUD.
3. **Pairing**: Search for "BikeNav_C3" in the app.

## Hardware
* **Controller**: ESP32-C3 (RISC-V).
* **Display**: 1.28-inch Round-LCD, 240x240 pixels, IPS.
* **Touch**: Capacitive CST816S controller.

## Troubleshooting
* **Parser Errors**: The system is optimized for the OsmAnd protocol. If navigation data is not displayed correctly, check the serial output (`>>> RX-RAW`).
* **Connection**: Ensure Bluetooth is active on your smartphone and the Bridge App has notification access permissions.
