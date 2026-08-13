// FILE: src/ble_handler.cpp
// STATUS: MASTER ANCHOR BikeNav_C3 - MODULAR BLE HANDLER
// DATE: 2026-03-29
// CHANGE: OTA-Service, Hardware-Version Abfrage (get_hw) und orangen Flash-Modus implementiert.

#include <Update.h>
#include "ble_handler.h"
#include "logic.h"
#include "graphics.h"
#include "config.h"
#include "fw_version.h"

// Zugriff auf globale Variablen der main.cpp
extern String displayText, streetNumber, nextStreet, pointNumber;
extern float currentDist, startDist, nextDist, angleExtraDist, lastKmh, currentHeading;
extern unsigned long arrivalTime, lastSpdTime, lastPktTime, lastCalcTime, lastPreviewUpdateTime, powerTimer;
extern bool isConnected, isReverse, actionActive, previewPendingClear;
extern bool isFlashing;
extern int navIcon, turnMod, nextNavIcon, nextTurnMod, spdWaitCount;

BLECharacteristic *pUARTChar = nullptr;
BLEServer *pServer = nullptr;

void wakeup();
void logLogicStatus();

void sendAck(BLECharacteristic *pChar) {
    pChar->setValue("ACK");
    pChar->notify();
    Serial.println(">>> [BLE-TX]: ACK");
}

class MyCharCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pChar) {
        lastPktTime = millis(); // Watchdog zurücksetzen
        wakeup(); 
        String rawValue = String(pChar->getValue().c_str());
        bool shouldAck = false;
        
        // DEBUG: Prüfen ob Daten ankommen
        Serial.print(">>> [BLE-RX]: "); Serial.println(rawValue);
        
        // Golden Rule 1: Rohdaten immer ausgeben
        Serial.print(">>> RX-RAW: "); Serial.println(rawValue);
        
        // Reboot-Logik (Fallback: Falls App den Befehl an UART statt OTA sendet)
        String checkReboot = rawValue;
        checkReboot.toLowerCase();
        checkReboot.trim();
        if (checkReboot.indexOf("abgeschlossen") >= 0 || checkReboot == "reboot") {
            Serial.println(">>> [SYSTEM]: OTA Abschluss via UART erkannt, starte neu...");
            isFlashing = false;
            delay(500);
            ESP.restart();
        }

        String incoming = fixUtf8(rawValue); 
        String lowRaw = incoming; 
        lowRaw.toLowerCase(); 
        lowRaw.trim(); // WICHTIG: Leerzeichen/Zeilenumbrüche entfernen
        
        // NEU: Expliziter Disconnect-Befehl
        if (lowRaw == "disconnect") {
            Serial.println(">>> [BLE]: Expliziter Disconnect-Befehl empfangen.");
            restartAdvertising();
            return; 
        }

        // NEU: Expliziter Keep-Alive Handler
        if (lowRaw == "stt:alive") {
            lastPktTime = millis(); // Watchdog zurücksetzen
            powerTimer = millis();  // Display-Timeout zurücksetzen
            Serial.println(">>> [BLE]: Keep-Alive empfangen");
            return; 
        }
        
        // NEU: Auf Anfrage der App antworten
        if (lowRaw == "get_hw") {
            String msg = "HW:" + String(HARDWARE_VERSION);
            pChar->setValue(msg.c_str()); // Antwort auf der gleichen Characteristic
            pChar->notify();              // Notification senden
            Serial.print(">>> [BLE-TX]: "); Serial.println(msg);
            return;
        }
        
        // NEU: Auf Anfrage der App antworten (FW)
        if (lowRaw == "get_fw") {
            String msg = "FW:" + String(FW_VERSION);
            pChar->setValue(msg.c_str());
            pChar->notify();
            Serial.print(">>> [BLE-TX]: "); Serial.println(msg);
            return;
        }
        
        if (lowRaw.startsWith("status:")) return; 
        
        // NAV-Präfix bereinigen
        if (lowRaw.startsWith("nav:")) {
            incoming = incoming.substring(4);
            lowRaw = incoming;
            lowRaw.toLowerCase();
        }

        // --- SPEED PARSER ---
        if (lowRaw.startsWith("spd:")) {
            lastKmh = incoming.substring(4).toFloat(); 
            unsigned long now = millis();
            if (spdWaitCount > 0 && currentDist <= 0) { actionActive = true; arrivalTime = now; }
            lastSpdTime = now; 
            // Golden Rule 2: Logic Feedback
            Serial.printf(">>> [SPD-CALC]: %.1f km/h\n", lastKmh);
            return;
        }

        // --- PKT / TOKEN PARSER ---
        if (lowRaw.startsWith("pkt:") || lowRaw.startsWith("pktr:")) {
            isReverse = lowRaw.startsWith("pktr:");
            int colonPos = incoming.indexOf(':');
            if (colonPos == -1) return; 

            String data = incoming.substring(colonPos + 1);
            data.trim();

            String tokens[5]; 
            int tCount = 0, lastPos = 0, nextSemi = 0;
            while ((nextSemi = data.indexOf(';', lastPos)) != -1 && tCount < 4) {
                tokens[tCount++] = data.substring(lastPos, nextSemi);
                lastPos = nextSemi + 1;
            }
            tokens[tCount++] = data.substring(lastPos);

            if (tCount >= 1) currentHeading = tokens[0].toFloat();
            if (tCount >= 2) {
                String dStr = tokens[1]; dStr.trim();
                String cleanD = "";
                for(int i = 0; i < (int)dStr.length(); i++) {
                    if(isDigit(dStr[i]) || dStr[i] == '.' || dStr[i] == ',') {
                        char c = dStr[i]; if(c==',') c='.'; cleanD += c;
                    }
                }
                angleExtraDist = cleanD.toFloat();
                String unitCheck = dStr; unitCheck.toLowerCase();
                if(unitCheck.indexOf("km") >= 0) angleExtraDist *= 1000.0f;
            }
            if (tCount >= 3) { pointNumber = tokens[2]; pointNumber.trim(); }
            
            lastPktTime = millis();
            shouldAck = true;
            return;
        }

        // --- OSMAND STRICT SPLIT ---
        String navPart = incoming;
        if (lowRaw.indexOf("|osmand") >= 0) {
            int pipePos = incoming.indexOf('|');
            if (pipePos != -1) {
                navPart = incoming.substring(0, pipePos);
                String metaPart = incoming.substring(pipePos + 1);
                
                navPart.trim();
                metaPart.trim();

                // 1. Meta-Part an OsmAnd-Parser senden (erzeugt META-JSON)
                parseOsmandMeta(metaPart); 

                // 2. Nav-Part für die Logik vorbereiten
                navPart.replace("dann ", "in ");
                navPart.replace("Dann ", "in ");
                
                incoming = navPart; 
                lowRaw = incoming;
                lowRaw.toLowerCase();
                
                // Fix: Ignoriere "0 m" Updates, um Überschreiben zu verhindern
                if (lowRaw.startsWith("0 m")) return;
            }
        }

        // --- NAVIGATION ---
        if (lowRaw.indexOf("abgebogen") >= 0 || lowRaw.indexOf("turned") >= 0) {
            if (nextNavIcon > 0) { 
                navIcon = nextNavIcon; turnMod = nextTurnMod; currentDist = nextDist; startDist = nextDist; 
                displayText = nextStreet; streetNumber = ""; nextNavIcon = 0; nextStreet = ""; nextDist = 0; 
            } else { navIcon = 3; turnMod = 0; currentDist = 0; displayText = ""; streetNumber = ""; }
            actionActive = false; arrivalTime = 0; lastCalcTime = millis(); 
            logLogicStatus(); 
            printNavJson(); // Output ACT/NEXT JSON
            shouldAck = true;
            return; 
        }

        // --- SPLIT LOGIC (DANACH / DANN) ---
        int splitPos = -1; 
        const char* splitWords[] = {"danach", "dann", "then", "after", "anschliessend"};
        for (int i = 0; i < 5; i++) { splitPos = lowRaw.indexOf(splitWords[i]); if (splitPos != -1) break; }
        
        if (splitPos != -1) { 
            String part1 = incoming.substring(0, splitPos), part2 = incoming.substring(splitPos); 
            part1.trim(); part2.trim(); 
            if (part1.length() > 3) extractDataFromPart(part1, navIcon, turnMod, currentDist, displayText, true); 
            if (part2.length() > 3) extractDataFromPart(part2, nextNavIcon, nextTurnMod, nextDist, nextStreet, false); 
            previewPendingClear = false;
        } else { 
            String oldText = displayText;
            extractDataFromPart(incoming, navIcon, turnMod, currentDist, displayText, true); 
            if (displayText == "KEEP_OLD") displayText = oldText;
            previewPendingClear = true; lastPreviewUpdateTime = millis();
        }

        if (lowRaw.indexOf("jetzt") >= 0 || lowRaw.indexOf("now") >= 0 || lowRaw.indexOf("arriv") >= 0) { 
            if (currentDist < 100) { currentDist = 0; arrivalTime = millis(); } 
        }
        
        Serial.print(">>> [DISPLAY-TEXT]: "); Serial.println(displayText); 
        logLogicStatus();
        printNavJson(); // Finaler Output der Navigationsdaten als JSON
        shouldAck = true;
        
        if (shouldAck) {
            sendAck(pChar);
        }
    }
};

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pS) { 
        isConnected = true; 
        lastPktTime = millis(); // <--- WICHTIG: Timer beim Verbinden zurücksetzen!
        wakeup(); 
        Serial.println(">>> [BLE]: Verbunden!"); 
    }

    void onMtuChange(BLEServer* pS, uint16_t mtu) {
        Serial.printf(">>> [BLE]: MTU ausgehandelt auf: %d Bytes\n", mtu);
    }

    void onDisconnect(BLEServer* pS) { 
        isConnected = false; 
        // Wir starten das Advertising hier neu, falls die App sauber trennt.
        // Wenn der Watchdog (restartAdvertising) aufgerufen wurde, 
        // ist das Advertising bereits gestoppt/gestartet worden.
        BLEDevice::startAdvertising(); 
        Serial.println(">>> [BLE]: Verbindung verloren - Suche aktiv..."); 
    }
};

class MyOTACallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pChar) {
        lastPktTime = millis(); 
        powerTimer = millis();
        
        std::string rawValue = pChar->getValue();
        
        // 1. Befehle (kurz)
        if (rawValue.length() < 50) {
            String value = String(rawValue.c_str());
            value.trim(); value.toLowerCase();
            
            if (value.startsWith("start:")) {
                // Vorheriges Update abbrechen, falls noch eines offen ist
                Update.abort(); 
                
                // Update initialisieren
                if (Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH)) {
                    isFlashing = true;
                    pChar->setValue("READY");
                    pChar->notify();
                    Serial.println(">>> [OTA]: Update begonnen.");
                } else {
                    isFlashing = false;
                    Serial.println(">>> [OTA]: FEHLER: Update.begin() fehlgeschlagen!");
                    Update.printError(Serial);
                }
            }
            else if (value.indexOf("abgeschlossen") >= 0 || value == "reboot") {
                if (isFlashing) {
                    isFlashing = false;
                    Serial.println(">>> [OTA]: Finalisiere Update...");
                    if (Update.end(true)) { 
                        Serial.println(">>> [OTA]: Erfolg! Neustart...");
                        delay(500);
                        ESP.restart();
                    } else {
                        Serial.println(">>> [OTA]: FEHLER beim Finalisieren!");
                        Update.printError(Serial);
                    }
                }
            }
        } 
        // 2. Daten-Chunks (lang)
        else {
            // Nur schreiben, wenn wir wirklich im Flash-Modus sind
            if (isFlashing) {
                // Sicherstellen, dass wir gültige Daten haben
                if (rawValue.length() > 0) {
                    // Wir schreiben die Daten direkt. 
                    // WICHTIG: Wir nutzen rawValue.data() direkt.
                    size_t written = Update.write((uint8_t*)rawValue.data(), rawValue.length());
                    
                    if (written != rawValue.length()) {
                        Serial.printf(">>> [OTA]: FEHLER: Nur %d/%d Bytes geschrieben!\n", written, rawValue.length());
                        Update.printError(Serial);
                        isFlashing = false;
                        Update.abort();
                    } else {
                        Serial.printf(">>> [OTA-DATA]: %d Bytes geschrieben\n", rawValue.length());
                    }
                }
            } else {
                Serial.println(">>> [OTA]: Daten empfangen, aber kein Update aktiv!");
            }
        }
    }
};

void setupBLE(const char* deviceName) {
    BLEDevice::init(deviceName);
    BLEDevice::setMTU(512);
    
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    
    // 1. UART Service (Unverändert)
    BLEService *pSer = pServer->createService("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
    pUARTChar = pSer->createCharacteristic(
        "6E400002-B5A3-F393-E0A9-E50E24DCCA9E", 
        BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR | BLECharacteristic::PROPERTY_NOTIFY
    );
    pUARTChar->addDescriptor(new BLE2902());
    pUARTChar->setCallbacks(new MyCharCallbacks());
    pSer->start();

    // 2. OTA Service (Mit Notify-Eigenschaft für Rückkanal)
    BLEService *pOTAService = pServer->createService("1D14D6EE-FD63-4FA1-BFA4-8F47B42119F0");
    BLECharacteristic *pOTAC = pOTAService->createCharacteristic(
        "1D14D6EF-FD63-4FA1-BFA4-8F47B42119F0",
        BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR | BLECharacteristic::PROPERTY_NOTIFY
    );
    pOTAC->addDescriptor(new BLE2902()); // Erforderlich für Notify
    pOTAC->setCallbacks(new MyOTACallbacks());
    pOTAService->start();

    // 3. Advertising (Explizite Definition beider Pakete)
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    
    // Primäres Advertising: OTA Service (Flags + OTA UUID)
    // Dies zwingt den OTA-Service in das primäre Paket
    BLEAdvertisementData advData;
    advData.setFlags(0x06); 
    advData.setCompleteServices(BLEUUID("1D14D6EE-FD63-4FA1-BFA4-8F47B42119F0"));
    pAdvertising->setAdvertisementData(advData);

    // Scan Response: UART Service + Name
    // Dies wird erst geladen, wenn die App die Details anfragt
    BLEAdvertisementData scanResponseData;
    scanResponseData.setName(deviceName);
    scanResponseData.setCompleteServices(BLEUUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E"));
    pAdvertising->setScanResponseData(scanResponseData);

    pAdvertising->setScanResponse(true);
    
    // Advertising Intervall auf 2s setzen (3200 * 0.625ms = 2000ms)
    pAdvertising->setMinInterval(3200);
    pAdvertising->setMaxInterval(3200);
    
    pAdvertising->start();
    Serial.println(">>> [BLE]: Advertising mit expliziter Trennung gestartet.");
}

void restartAdvertising() {
    Serial.println(">>> [BLE]: Force-Reset Advertising...");
    
    // 1. Advertising stoppen
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->stop();

    // 2. Alle Verbindungen erzwingen zu trennen
    if (pServer) {
        // Korrektur: Argument 'true' für getPeerDevices hinzugefügt
        std::map<uint16_t, conn_status_t> peers = pServer->getPeerDevices(true);
        
        // Korrektur: Klassische Iterator-Schleife statt C++17 Structured Bindings
        for (auto const& pair : peers) {
            pServer->disconnect(pair.first);
        }
    }

    // 3. Advertising neu starten
    pAdvertising->setMinInterval(3200);
    pAdvertising->setMaxInterval(3200);
    pAdvertising->start();
    Serial.println(">>> [BLE]: Advertising neu gestartet.");
}

String getBLEAddress() { return BLEDevice::getAddress().toString().c_str(); }
