# BikeNav_C3 - Smart Bike HUD

BikeNav_C3 is a minimalist, Bluetooth-based Head-Up-Display (HUD) for bicycles, utilizing an ESP32-C3 and a 1.28" round IPS display (GC9A01).

## 📲 The Android Bridge App
The core strength of this project is its integration with the **Android Bridge App**, which acts as a smart link between your smartphone and the hardware:
* **TTS Interface**: The Bridge App intercepts the system's Text-to-Speech (voice output) data and converts it into visual navigation commands for the HUD.
* **Tested Compatibility**: The HUD works independently of the specific map provider. It has been successfully tested with:
    * **Organic Maps** (and its forks)
    * **OsmAnd**
    * Any navigation system providing standardized Android voice guidance or notifications.

## ✨ Key Features
* **Real-Time Navigation**: Display of turn-by-turn icons, distances, and street names via BLE.
* **Smart Preview**: Upcoming maneuver icon ("Next turn after this one").
* **Dynamic Distance Calculation**: Automatic countdown of meters to the next waypoint based on current speed (`SPD-CALC`).
* **Intelligent UI**: 
    * Automatic start screen with MAC address for easy pairing.
    * Color-coded feedback (Orange alert for distances < 30m).
* **Gesture Control**: Adjust brightness and display rotation (0-270°) via touch gestures.

## 🛠 Hardware
* **Controller**: ESP32-C3 (RISC-V).
* **Display**: 1.28 inch Round-LCD, 240x240 Pixels, IPS.
* **Touch**: Capacitive CST816S controller.

## 🚀 Quick Start
1. **Flash Firmware**: Upload to ESP32-C3 using PlatformIO.
2. **Pairing**: Search for "BikeNav_C3" in the Android Bridge App (check MAC on the start screen).
3. **Navigate**: The HUD automatically switches to navigation mode as soon as the app starts sending data.

---
*Detailed technical information regarding architecture and pinouts can be found in the `docs/` folder.*