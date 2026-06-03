# BikeNav_C3 - Smart Bike HUD

BikeNav_C3 is a minimalist, Bluetooth-based Head-Up Display (HUD) for bicycles, based on the ESP32-C3 and a 1.28" round display (GC9A01).

### What is this device used for?
The BikeNav_C3 turns your smartphone into a discreet navigation system. Instead of constantly looking at your phone on the handlebars while riding, the HUD delivers precise turn-by-turn instructions directly into your field of view. It is ideal for:
* **Safe Cycling**: Focus remains on the road, not on the smartphone display.
* **Minimalist Cockpit**: No bulky phone on the handlebars, just an elegant, round display.
* **Weather Independence**: The display is easy to read even in direct sunlight.

<table>
  <tr>
    <td><img src="assets/screen_disconn_appwaiting.jpeg" width="200" alt="Start Screen"></td>
    <td><img src="assets/screen_turn_right.jpeg" width="200" alt="Navigation View"></td>
    <td><img src="assets/screen_turn_halve_right.jpeg" width="200" alt="Navigation View 2"></td>
  </tr>
</table>

## 📲 The Android Bridge App
The heart of the system is the pairing with the **Android Bridge App** [TTS2BluetoothSerial](../../../TTS2BluetoothSerial). This acts as an intelligent link between the smartphone and the HUD:
* **TTS Interface**: The Bridge App uses the Android system's text-to-speech (TTS) and converts it into visual commands for the display in real-time.
* **Tested Compatibility**: The system is independent of the map app used. It has been successfully tested with:
    * **Organic Maps** (and its forks)
    * **OsmAnd**
    * All navigation systems that use standardized Android voice instructions.

## ✨ Core Features
* **Real-time Navigation**: Display of turn instructions (icons), distances, and street names.
* **Smart Preview**: Predictive icon for the next maneuver step ("Then turn left").
* **Dynamic Distance Calculation**: Automatic countdown of meters based on the last known speed (`SPD-CALC`), even if the app is not currently sending data.
* **Intelligent UI**:
    * Start screen with MAC address for easy pairing.
    * Color feedback for critical distances (Orange alert at < 30m).
* **Gesture Control**: Brightness adjustment and display rotation (0-270°) via touch gesture on the display.

## 🛠 Hardware
* **Controller**: ESP32-C3 (RISC-V).
* **Display**: 1.28 inch Round-LCD, 240x240 pixels, IPS.
* **Touch**: Capacitive CST816S controller.
* **Compatibility**: The code is adaptable for the ESP32-2424S012 display (commonly found on AliExpress) or similar GC9A01-based displays.

## 🚀 Quick Start & Installation

### Installation via PlatformIO
1. **Preparation**: Install [PlatformIO](https://platformio.org/) (as a VS Code extension).
2. **Compile & Flash**:
   - Connect the ESP32-C3 via USB.
   - Open the project in VS Code.
   - Use the "Upload" button in the PlatformIO toolbar.
   - The `platformio.ini` automatically configures the necessary libraries (`GFX Library`, `NimBLE`, `CST816S`).
3. **Monitoring**: Open the Serial Monitor (Baud rate: 115200) to see the boot process and BLE status.

### Pairing
1. Start the Android Bridge App.
2. Search for "BikeNav_C3".
3. Once connected, the display will show the navigation screen.

## ⚙️ Technical Functionality

The system follows an event-driven data flow:

1. **BLE Communication**:
   - The device acts as a BLE server.
   - Incoming messages are received via the `NimBLE` stack.
   - **Important**: According to the *Verbosity Rule*, every received message is output to the Serial Monitor for diagnostic purposes.

2. **Data Processing**:
   - The received strings are parsed (see `src/logic.cpp`) to extract icons, distances, and street names.
   - The `SPD-CALC` logic calculates the distance based on the last known speed if data is missing.

3. **Rendering**:
   - The UI is rendered on the GC9A01 display using the `GFX Library`.
   - `renderStartScreen()`: Shows the connection status (Blue = Connected, Red = Disconnected).
   - `renderNavScreen()`: Dynamically updates the navigation instructions.
