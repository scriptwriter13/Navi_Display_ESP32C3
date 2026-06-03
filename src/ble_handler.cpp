// FILE: src/ble_handler.cpp
// STATUS: MASTER ANCHOR BikeNav_C3 - MODULAR BLE HANDLER
// DATE: 2026-03-29
// CHANGE: Aufruf von printNavJson() nach Nav-Updates für ACT/NEXT JSON-Output.
// CHANGE: NAV-Präfix Parsing hinzugefügt.
// CHANGE: Ignoriere "0 m" Updates im OSMAND-Block.

#include "ble_handler.h"
#include "logic.h"
#include "graphics.h"
#include "config.h"

// Zugriff auf globale Variablen der main.cpp
extern String displayText, streetNumber, nextStreet, pointNumber;
extern float currentDist, startDist, nextDist, angleExtraDist, lastKmh, currentHeading;
extern unsigned long arrivalTime, lastSpdTime, lastPktTime, lastCalcTime, lastPreviewUpdateTime;
extern bool isConnected, isReverse, actionActive, previewPendingClear;
extern int navIcon, turnMod, nextNavIcon, nextTurnMod, spdWaitCount;

void wakeup();
void logLogicStatus();

class MyCharCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pChar) {
        wakeup(); 
        String rawValue = String(pChar->getValue().c_str());
        
        // Golden Rule 1: Rohdaten immer ausgeben
        Serial.print(">>> RX-RAW: "); Serial.println(rawValue);
        
        String incoming = fixUtf8(rawValue); 
        String lowRaw = incoming; 
        lowRaw.toLowerCase(); 
        
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
    }
};

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pS) { isConnected = true; wakeup(); Serial.println(">>> [BLE]: Verbunden!"); }
    void onDisconnect(BLEServer* pS) { 
        isConnected = false; 
        delay(100); 
        BLEDevice::startAdvertising(); 
        Serial.println(">>> [BLE]: Verbindung verloren - Suche aktiv..."); 
    }
};

void setupBLE(const char* deviceName) {
    BLEDevice::init(deviceName);
    BLEServer *pS = BLEDevice::createServer();
    pS->setCallbacks(new MyServerCallbacks());
    
    BLEService *pSer = pS->createService("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
    BLECharacteristic *pC = pSer->createCharacteristic(
        "6E400002-B5A3-F393-E0A9-E50E24DCCA9E", 
        BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR
    );
    pC->setCallbacks(new MyCharCallbacks());
    pSer->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    
    BLEAdvertisementData advData;
    advData.setFlags(0x06); 
    advData.setCompleteServices(BLEUUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E"));
    pAdvertising->setAdvertisementData(advData);

    BLEAdvertisementData scanResponseData;
    scanResponseData.setName(deviceName);
    scanResponseData.setAppearance(0x0440); 
    pAdvertising->setScanResponseData(scanResponseData);

    pAdvertising->setScanResponse(true);
    pAdvertising->start();
    Serial.println(">>> [BLE]: Advertising mit UUID & Name gestartet.");
}

String getBLEAddress() { return BLEDevice::getAddress().toString().c_str(); }
