# BikeNav_C3 - Smart Bike HUD

BikeNav_C3 ist ein minimalistisches, Bluetooth-basiertes Head-Up-Display (HUD) für Fahrräder, basierend auf dem ESP32-C3 und einem 1.28" Rund-Display (GC9A01). Es liefert präzise Richtungsanweisungen direkt im Sichtfeld des Fahrers.

## 📲 Die Android Bridge App
Das Herzstück des Systems ist die Kopplung mit der **Android Bridge App**. Diese fungiert als intelligentes Bindeglied zwischen dem Smartphone und dem HUD:
* **TTS-Schnittstelle**: Die Bridge App nutzt die Sprachausgabe (Text-to-Speech) des Android-Systems und wandelt diese in Echtzeit in visuelle Befehle für das Display um.
* **Getestete Kompatibilität**: Das System ist unabhängig von der genutzten Karten-App. Es wurde erfolgreich getestet mit:
    * **Organic Maps** (und deren Forks)
    * **OsmAnd**
    * Allen Navigationssystemen, die standardisierte Android-Sprachanweisungen nutzen.

## ✨ Kernfunktionen
* **Echtzeit-Navigation**: Anzeige von Abbiegehinweisen (Icons), Distanzen und Straßennamen.
* **Smart Preview**: Vorausschauendes Icon für den darauffolgenden Manöver-Schritt ("Danach links").
* **Dynamische Distanzberechnung**: Automatisches Herunterzählen der Meter basierend auf der aktuellen Geschwindigkeit (`SPD-CALC`), auch wenn die App gerade keine Daten sendet.
* **Intelligentes UI**: 
    * Startscreen mit MAC-Adresse zur einfachen Kopplung.
    * Farbliches Feedback bei kritischen Distanzen (Orange-Alarm bei < 30m).
* **Gestensteuerung**: Helligkeitsregelung und Display-Rotation (0-270°) per Touch-Geste am Display.

## 🛠 Hardware
* **Controller**: ESP32-C3 (RISC-V).
* **Display**: 1.28 Zoll Round-LCD, 240x240 Pixel, IPS.
* **Touch**: Kapazitiver CST816S Controller.

## 🚀 Schnellstart
1. **Firmware**: Mit PlatformIO auf den ESP32-C3 flashen.
2. **Koppeln**: In der Android Bridge App nach "BikeNav_C3" suchen.
3. **Start**: Sobald die Navigation läuft, übernimmt das HUD automatisch.

---
*Technische Details zur Architektur und Pin-Belegung finden sich im Ordner `docs/`.*