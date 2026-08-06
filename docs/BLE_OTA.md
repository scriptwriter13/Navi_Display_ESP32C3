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
1. **Hardware-Version abfragen:** Sende `get_hw` an die UART-Characteristic (`6E400002...`). Das Device antwortet mit `HW:ESP32-2424S012-V1.0`.
2. **OTA Initialisieren:** Sende an die OTA-Characteristic: `"START:" + OTA_SECRET_KEY`
   *(Der `OTA_SECRET_KEY` ist in `include/config.h` definiert).*                                                                             
                                                                                                                                          
**Antwort des Devices:** Das Device antwortet mit `"READY"`, sobald es bereit ist.                                                                                                                                          
                                                                                                                                          
### Schritt B: Datenübertragung                                                                                                           
Nach erfolgreicher Autorisierung können die Binärdaten der Firmware übertragen werden.                                     
*   **Format:** Binäre Datenpakete (Chunks).                                                                                              
*   **Schreib-Modus:** Verwende `WRITE_NO_RESPONSE`.                             
                                                                                                                                          
### Schritt C: Abschluss                                                                                                                  
Nachdem alle Daten übertragen wurden, sende den String:                                                                                   
`"OTA Übertragung abgeschlossen"`                                                                                                                                   
Der ESP32 wird daraufhin das Update validieren und automatisch neu starten.                                                               
                                                                                                                                          
## 3. Visuelles Feedback (Orange Flash Mode)
Während der OTA-Übertragung wechselt das Display-Lämpchen in den **Orange Flash Mode** (oranges Blinken). Dies signalisiert dem Nutzer, dass das Gerät aktiv geflasht wird und nicht unterbrochen werden darf.

## 4. Implementierungshinweise für Android                                                                                                
                                                                                                                                          
1.  **MTU Negotiation:** Fordere nach dem Verbindungsaufbau eine höhere MTU an (z.B. `requestMtu(517)`).
2.  **Flow Control:** Baue eine Verzögerung von ca. **20ms bis 50ms** zwischen den Paketen ein.
3.  **Subscription:** Stelle sicher, dass die App auf die OTA-Characteristic subscribed hat (`setCharacteristicNotification(..., true)`), um das `"READY"`-Signal zu empfangen.
                                                                                                                                          


