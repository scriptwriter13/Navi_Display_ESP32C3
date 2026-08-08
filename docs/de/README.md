# BikeNav_C3 Dokumentation

Willkommen bei der Dokumentation für das BikeNav_C3 Projekt. Dieses System bietet eine modulare Navigation für Fahrräder basierend auf dem ESP32-C3.

## Neue Features (Stand: 2026-03-29)
- **OTA-Update-Service**: Firmware-Updates können nun bequem via Bluetooth Low Energy (BLE) durchgeführt werden.
- **Hardware-Versionierung**: Die Hardware-Version kann via `get_hw` über die UART-Schnittstelle abgefragt werden.
- **Orange Flash Mode**: Während eines OTA-Updates blinkt das Display-Lämpchen orange, um den Flash-Vorgang visuell zu signalisieren.
- **Build-Automatisierung**: Git-Hash-Integration via Python-Skript und Build-Flags für Versionierung.

## Dokumentations-Übersicht
- [BLE OTA Protokoll Spezifikation](../BLE_OTA.md)
- [Serial Uploader Anleitung](serial-uploader.md)
- [Architektur-Übersicht](../README_ARCHITECTURE.md)
