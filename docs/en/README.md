# BikeNav_C3 Documentation

Welcome to the documentation for the BikeNav_C3 project. This system provides modular navigation for bicycles based on the ESP32-C3.

## New Features (Status: 2026-03-29)
- **OTA Update Service**: Firmware updates can now be performed conveniently via Bluetooth Low Energy (BLE).
- **Hardware Versioning**: The hardware version can be queried via `get_hw` using the UART interface.
- **Orange Flash Mode**: During an OTA update, the display LED flashes orange to visually signal the flashing process.
- **Build Automation**: Git-Hash integration via Python script and build flags for versioning.

## Documentation Overview
- [BLE OTA Protocol Specification](../BLE_OTA.md)
- [Serial Uploader Guide](serial-uploader.md)
- [Architecture Overview](../README_ARCHITECTURE.md)
