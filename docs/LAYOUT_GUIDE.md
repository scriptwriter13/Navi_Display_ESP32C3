# Layout-Gesetz (BikeNav_C3)

Dieses Dokument definiert das mathematisch ausgerichtete Schema für das 240x240 Rund-Display. 
Ziel ist eine perfekte vertikale Zentrierung aller Elemente bei gleichzeitiger Einhaltung der Schutzabstände zum grauen Fortschrittsring (Radius 117).

## 1. Die zentrale Y=80 Linie (Fokus-Ebene)
Das Haupt-Icon und die Haupt-Distanz bilden das visuelle Zentrum der oberen 2/3 des Displays. 
Ihr gemeinsames Zentrum liegt exakt auf der **Y=80** Horizontalen.

| Element | Anker-Punkt | Berechnung | Resultat |
| :--- | :--- | :--- | :--- |
| **Haupt-Icon** | Bottom-Y = 110 | 110px minus 60px Höhe | Mitte bei Y=80 |
| **Distanz** | Center-Y = 80 | `setCursor(X, 80 + (halbe Schrifthöhe))` | Mitte bei Y=80 |

## 2. Element-Platzierung (Pixel-genau)

| Sektor | Element | X-Pos | Y-Pos | Details |
| :--- | :--- | :--- | :--- | :--- |
| **Top** | Status-LED | 120 | 20 | Punkt (6px Radius) |
| **Obere 2/3** | **Haupt-Icon** | 35 | 110 | Größe: 60x60px |
| | **Distanz (m/km)**| 170 | 85 | Größe 4, zentriert auf Y=80 |
| | **Zusatz-Distanz** | 170 | 130 | Größe 3, in Klammern |
| **Mitte** | Straßenname | 120 | 165 | Größe 2, zentriert |
| | Hausnummer | 120 | 190 | Farbe: `COL_STREET_NUM` |
| **Unteres 1/3**| **Vorschau-Icon** | 103 | 225 | 35x35px, im Bogen des Rings |

## 3. Optische Regeln & Schutzbereiche
* **Fortschrittsring:** Der graue Ring liegt bei Radius 117 (Zentrum 120, 120).
* **Schutzabstand:** Alle Elemente halten einen Mindestabstand von 3–5 Pixeln zum Ring ein, um Überschneidungen bei der Rundung zu vermeiden.
* **Farb-Hierarchie:** * `COL_ACCENT_GREEN`: Aktive Navigation & Fortschritt.
    * `COL_ALERT_ORANGE`: Sofortige Aktion (Abbiegen jetzt) oder Ziel erreicht.
    * `COL_DIM_GRAY`: Inaktive Elemente & Vorschau-Icon (um Fokus nicht zu stören).
    * `COL_SUB_DIST`: Hellblau für die Klammer-Distanz zur besseren Differenzierung.

## 4. Koordinaten-System
Das Display nutzt ein Top-Left Koordinatensystem (0,0 oben links). Da es sich um ein Rund-Display handelt, ist die Mitte bei (120, 120). 
- **Zentrierung:** `X = 120 - (Textbreite / 2)`