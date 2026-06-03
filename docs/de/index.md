# BikeNav-HUD Dokumentation

Willkommen bei der offiziellen Dokumentation für das **BikeNav-HUD**.

## Übersicht
BikeNav-HUD ist ein minimalistisches, Bluetooth-basiertes Head-Up-Display (HUD) für Fahrräder, basierend auf dem ESP32-C3 und einem 1.28" Rund-Display (GC9A01). Es liefert präzise Richtungsanweisungen direkt im Sichtfeld des Fahrers.

## Kernfunktionen
* **Echtzeit-Navigation**: Anzeige von Abbiegehinweisen (Icons), Distanzen und Straßennamen.
* **Smart Preview**: Vorausschauendes Icon für den darauffolgenden Manöver-Schritt ("Danach links").
* **Dynamische Distanzberechnung**: Automatisches Herunterzählen der Meter basierend auf der aktuellen Geschwindigkeit (`SPD-CALC`), auch wenn die App gerade keine Daten sendet.
* **Gestensteuerung**: Helligkeitsregelung und Display-Rotation (0-270°) per Touch-Geste am Display.
* **Parser-Robustheit**: Das System ist gegen "0 m"-Fehlermeldungen von OsmAnd gehärtet und verarbeitet Meta-Daten zuverlässig.

## Installation
1. **Firmware**: Kompiliere das Projekt mit [PlatformIO](https://platformio.org/).
2. **App**: Installiere die Android Bridge App, um Navigationsdaten von Apps wie OsmAnd oder Organic Maps an das HUD zu senden.
3. **Koppeln**: Suche in der App nach "BikeNav_C3".

## Hardware
* **Controller**: ESP32-C3 (RISC-V).
* **Display**: 1.28 Zoll Round-LCD, 240x240 Pixel, IPS.
* **Touch**: Kapazitiver CST816S Controller.

## Fehlerbehebung
* **Parser-Fehler**: Sollten Navigationsdaten nicht korrekt angezeigt werden, prüfe die Serial-Ausgabe (`>>> RX-RAW`). Das System ist für das OsmAnd-Protokoll optimiert.
* **Verbindung**: Stelle sicher, dass Bluetooth am Smartphone aktiv ist und die Bridge App die Berechtigungen für Benachrichtigungszugriff hat.
# BikeNav-HUD Dokumentation

Willkommen bei der offiziellen Dokumentation für das **BikeNav-HUD**.

## Übersicht
BikeNav-HUD ist ein minimalistisches, Bluetooth-basiertes Head-Up-Display (HUD) für Fahrräder, basierend auf dem ESP32-C3 und einem 1.28" Rund-Display (GC9A01). Es liefert präzise Richtungsanweisungen direkt im Sichtfeld des Fahrers.

## Kernfunktionen
* **Echtzeit-Navigation**: Anzeige von Abbiegehinweisen (Icons), Distanzen und Straßennamen.
* **Smart Preview**: Vorausschauendes Icon für den darauffolgenden Manöver-Schritt ("Danach links").
* **Dynamische Distanzberechnung**: Automatisches Herunterzählen der Meter basierend auf der aktuellen Geschwindigkeit (`SPD-CALC`), auch wenn die App gerade keine Daten sendet.
* **Gestensteuerung**: Helligkeitsregelung und Display-Rotation (0-270°) per Touch-Geste am Display.
* **Parser-Robustheit**: Das System ist gegen "0 m"-Fehlermeldungen von OsmAnd gehärtet und verarbeitet Meta-Daten zuverlässig.

## Installation
1. **Firmware**: Kompiliere das Projekt mit [PlatformIO](https://platformio.org/).
2. **App**: Installiere die Android Bridge App, um Navigationsdaten von Apps wie OsmAnd oder Organic Maps an das HUD zu senden.
3. **Koppeln**: Suche in der App nach "BikeNav_C3".

## Hardware
* **Controller**: ESP32-C3 (RISC-V).
* **Display**: 1.28 Zoll Round-LCD, 240x240 Pixel, IPS.
* **Touch**: Kapazitiver CST816S Controller.

## Fehlerbehebung
* **Parser-Fehler**: Das System ist für das OsmAnd-Protokoll optimiert. Sollten Daten nicht korrekt angezeigt werden, prüfe die Serial-Ausgabe (`>>> RX-RAW`).
* **Verbindung**: Stelle sicher, dass Bluetooth am Smartphone aktiv ist und die Bridge App die Berechtigungen für Benachrichtigungszugriff hat.
