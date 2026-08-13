/*                                                                                                                                        
 * Copyright (C) 2026 by scriptwriter13                                                                                                   
 *                                                                                                                                        
 * Dieses Programm ist freie Software: Sie können es unter den Bedingungen der                                                            
 * GNU General Public License, wie von der Free Software Foundation veröffentlicht,                                                       
 * entweder Version 3 der Lizenz oder (nach Ihrer Option) jeder späteren                                                                  
 * Version, weiterverbreiten und/oder modifizieren.                                                                                       
 */                                                                                                                                       
                                                                                                                                          
# BLE OTA Protokoll Spezifikation                                                                                                         
                                                                                                                                          
Dieses Dokument beschreibt die Implementierung des Over-the-Air (OTA) Updates via Bluetooth Low Energy (BLE) für das BikeNav-Projekt.     
                                                                                                                                          
## 1. Verbindungsdaten                                                                                                                    
Die App muss sich mit dem ESP32 verbinden und den folgenden Service sowie die Characteristic ansteuern:                                   
                                                                                                                                          
*   **Service UUID:** `1D14D6EE-FD63-4FA1-BFA4-8F47B42119F0`                                                                              
*   **Characteristic UUID:** `1D14D6EF-FD63-4FA1-BFA4-8F47B42119F0`                                                                       
                                                                                                                                          
## 2. Protokoll-Ablauf                                                                                                                    
Das Protokoll ist zustandsbasiert und erfordert eine Autorisierung.                                                                       
                                                                                                                                          
### Schritt A: Autorisierung & Hardware-Check
1. **Hardware-Version abfragen:** Sende `get_hw` an die UART-Characteristic (`6E400002...`). Das Device antwortet mit `HW:ESP32-2424S012-V1.0` (entsprechend der in `include/git_version.h` definierten Version).
2. **Firmware-Version abfragen:** Sende `get_fw` an die UART-Characteristic (`6E400002...`). Das Device antwortet mit `FW:1.1.1` (entsprechend der in `include/fw_version.h` definierten Version).
3. **OTA Initialisieren:** Sende an die OTA-Characteristic: `"START:" + OTA_SECRET_KEY`
   *(Der `OTA_SECRET_KEY` ist in `include/config.h` definiert).*
                                                                                                                                          
**Antwort des Devices:** Das Device antwortet mit `"READY"`, sobald es bereit ist.                                                                                                                                          
                                                                                                                                          
### Schritt B: Datenübertragung                                                                                                           
Nach erfolgreicher Autorisierung können die Binärdaten der Firmware übertragen werden.                                     
*   **Format:** Binäre Datenpakete (Chunks).                                                                                              
*   **Schreib-Modus:** Verwende `WRITE_NO_RESPONSE`.                             
                                                                                                                                          
### Schritt C: Abschluss
Nachdem alle Daten übertragen wurden, sende den String:
"OTA Übertragung abgeschlossen"
Der ESP32 wird daraufhin das Update validieren und automatisch neu starten.

**WICHTIG:** Sende diesen Befehl erst, wenn **alle** Datenpakete vollständig übertragen wurden. Ein zu frühes Senden führt zu einem Abbruch der Validierung (Checksum-Fehler).
                                                                                                                                          
## 3. Visuelles Feedback (Orange Flash Mode)
Während der OTA-Übertragung wechselt das Display-Lämpchen in den **Orange Flash Mode** (oranges Blinken). Dies signalisiert dem Nutzer, dass das Gerät aktiv geflasht wird und nicht unterbrochen werden darf.

## 4. Keep-Alive Mechanismus
Um zu verhindern, dass das Gerät während einer aktiven Navigation in den Standby-Modus wechselt, sollte die App in regelmäßigen Abständen ein Keep-Alive-Signal senden.

*   **Befehl:** `STT:ALIVE`
*   **Intervall:** Alle 10 Sekunden.
*   **Wirkung:** Das Gerät setzt den internen `powerTimer` zurück und bleibt aktiv. Bei Ausbleiben des Signals für mehr als 15 Sekunden wird die Verbindung als unterbrochen gewertet und das Advertising neu gestartet.

## 5. Verbindungssteuerung
Um eine saubere Trennung der Verbindung zu gewährleisten (z.B. beim Beenden der App), sollte die App den Befehl `DISCONNECT` senden.

*   **Befehl:** `DISCONNECT`
*   **Wirkung:** Das Gerät trennt die aktive Verbindung sofort und startet das Advertising neu, um für eine neue Verbindung bereit zu sein.

## 6. Implementierungshinweise für Android                                                                                                
                                                                                                                                          
1.  **MTU Negotiation:** Fordere nach dem Verbindungsaufbau eine höhere MTU an (z.B. `requestMtu(517)`).
2.  **Flow Control:** Baue eine Verzögerung von ca. **20ms bis 50ms** zwischen den Paketen ein.
3.  **Subscription:** Stelle sicher, dass die App auf die OTA-Characteristic subscribed hat (`setCharacteristicNotification(..., true)`), um das `"READY"`-Signal zu empfangen.
                                                                                                                                          


