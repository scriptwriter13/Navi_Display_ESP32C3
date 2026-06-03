# Software-Architektur - BikeNav_C3

Dieses Dokument beschreibt das modulare Zusammenspiel der Komponenten im BikeNav_C3 Projekt. Das System folgt dem Prinzip der **Separation of Concerns** (Trennung der Zuständigkeiten).

## 1. Modul-Übersicht

| Datei | Verantwortung | Kern-Aufgabe |
| :--- | :--- | :--- |
| **main.cpp** | **Orchestrator** | BLE-Events, Hardware-Init, State-Machine, Haupt-Loop. |
| **logic.cpp/h** | **Daten-Parser** | Umwandlung von RAW-Strings in Navigations-Daten (Distanz, Icons). |
| **graphics.cpp/h**| **Render-Engine** | Vektor-Berechnung der Pfeile und UI-Kompositions-Logik. |
| **config.h** | **Hardware-HAL** | Definition der GPIO-Pins für den ESP32-C3. |
| **layout_config.h**| **Design-Konfig** | Absolute Pixel-Koordinaten und Farb-Definitionen. |

## 2. Datenfluss (Data Flow)

1. **Empfang**: Ein verschlüsselter oder RAW-String kommt via BLE rein (`MyCharCallbacks`).
2. **Verarbeitung**: Die `main.cpp` nutzt Funktionen aus der `logic.h`, um Entfernungen zu berechnen (`spd:`) oder Icons zuzuweisen (`pkt:`).
3. **Zustand**: Die globalen Variablen in der `main.cpp` (z.B. `navIcon`, `currentDist`) werden aktualisiert.
4. **Ausgabe**: In jedem Frame der `loop()` wird `renderNavScreen()` aufgerufen. Hier werden die aktuellen Werte an die `graphics.cpp` übergeben.
5. **Zeichnen**: Die `graphics.cpp` liest die festen Koordinaten aus der `layout_config.h` und bringt die Pixel auf den Canvas.

## 3. Die "Golden Rules" (Hardcoded)

Um die Stabilität des Projekts zu gewährleisten, müssen folgende Regeln bei jeder Änderung beachtet werden:

* **Verbosity Rule**: Jede eingehende BLE-Nachricht muss mit `>>> RX-RAW:` im Serial Monitor geloggt werden.
* **Logic-Feedback**: Mathematische Korrekturen (wie die Distanz-Deduction) müssen als `>>> [SPD-CALC]:` ausgegeben werden.
* **No-Strip Policy**: Debug-Logs bleiben im Code, außer es wird explizit ein "Production Build" angefordert.
* **MAC-Visibility**: Die MAC-Adresse wird ausschließlich auf dem Start-Screen angezeigt, um den Nav-Screen sauber zu halten.

## 4. Abhängigkeiten
* `Arduino_GFX`: Für das GC9A01 Rund-Display.
* `NimBLE-Arduino`: Für die Bluetooth-Kommunikation.
* `CST816S`: Für die Touch-Gesten-Steuerung.