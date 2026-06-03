// FILE: src/main.cpp
// STATUS: FINAL STABLE - POWER-TIMER FIX + GESTURE DEBOUNCE + LIGHT SLEEP + WAKEUP FIX
// DATE: 2026-03-10
// CHANGE: Entfernung von esp_sleep_enable_bt_wakeup() für C3/S3 Kompatibilität
// CHANGE: Casual Greeting angepasst

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <CST816S.h>  
#include <Wire.h>
#include <Preferences.h>
#include <esp_sleep.h>

#include "config.h"
#include "layout_config.h" 
#include "logic.h" 
#include "graphics.h"
#include "ble_handler.h"

// --- GLOBALE VARIABLEN ---
String displayText = "warte auf App", macAddr = "", streetNumber = "";
String pointNumber = ""; 
String nextStreet = ""; 
String metaDist = "", metaTime = "", metaEta = "", metaSpd = "";
float currentDist = 0, startDist = 0, nextDist = 0, angleExtraDist = 0; 
float distSinceAction = 0; 
float currentHeading = 0; 
float lastKmh = 0; 
unsigned long lastSpdTime = 0, arrivalTime = 0, powerTimer = 0;
unsigned long lastPktTime = 0, lastCalcTime = 0, lastPreviewUpdateTime = 0; 
unsigned long lastGestureTime = 0; // Debounce Timer für Gesten
bool previewPendingClear = false, isConnected = false, displayOn = true, isRerouting = false;
bool actionActive = false, isReverse = false, justWokeUp = false; 
int navIcon = 0, turnMod = 0, nextNavIcon = 0, nextTurnMod = 0; 
int brightness = 200, displayRot = 0, spdWaitCount = 0; 
Preferences prefs;

// Hardware Objekte
Arduino_DataBus *bus = new Arduino_ESP32SPI(DC_PIN, CS_PIN, SCK_PIN, MOSI_PIN, MISO_PIN);
Arduino_GFX *gfx = new Arduino_GC9A01(bus, RST_PIN, 0, true);
Arduino_Canvas *canvas = new Arduino_Canvas(240, 240, gfx);
CST816S touch(TOUCH_SDA, TOUCH_SCL, TOUCH_RST, TOUCH_INT);

// --- SYSTEM FUNKTIONEN ---
void logLogicStatus() {
    Serial.printf(">>> [REPORT]: Icon:%d | Dist:%.0fm | Pt:%s | Text:[%s]\n", 
                  navIcon, currentDist, pointNumber.c_str(), displayText.c_str());
}

void wakeup() { 
    if (!displayOn) { 
        displayOn = true; 
        justWokeUp = true; 
        
        gfx->displayOn(); 
        pinMode(TFT_BL, OUTPUT);
        analogWrite(TFT_BL, brightness); 
        
        Serial.println(">>> [SYSTEM]: Wakeup (Display ON)");
    }
    powerTimer = millis(); // Timer immer zurücksetzen, wenn berührt
}

void setup() {
    Serial.begin(115200);
    prefs.begin("bikenav", false); 
    displayRot = prefs.getInt("rot", 0);
    brightness = prefs.getInt("bright", 200);

    // Hardware Init
    pinMode(RST_PIN, OUTPUT); digitalWrite(RST_PIN, LOW); delay(200); digitalWrite(RST_PIN, HIGH);
    gfx->begin(); gfx->setRotation(displayRot); canvas->begin();
    
    // Initiales Backlight Setup
    pinMode(TFT_BL, OUTPUT); 
    digitalWrite(TFT_BL, HIGH); 
    analogWrite(TFT_BL, brightness);
    
    Wire.begin(TOUCH_SDA, TOUCH_SCL); touch.begin();

    // Wakeup Konfiguration für Light Sleep (Kompatibel mit C3/S3)
    esp_sleep_enable_gpio_wakeup();
    gpio_wakeup_enable((gpio_num_t)TOUCH_INT, GPIO_INTR_LOW_LEVEL);
    
    // BLE Setup
    setupBLE("BikeNav_C3");
    macAddr = getBLEAddress();

    Serial.println("\n--- Hey, BikeNav_C3 ist bereit! ---");
    Serial.print(">>> DEVICE MAC: "); Serial.println(macAddr);

    powerTimer = millis();
    lastCalcTime = millis();
}

void loop() {
    unsigned long now = millis();

    // 1. Touch-Handler mit Debouncing
    if (touch.available()) {
        // Geste auslesen
        String g = touch.gesture(); 
        
        // Wenn Display aus, nur aufwecken
        if (!displayOn) {
            wakeup();
        } 
        // Wenn Display an, nur auf Gesten reagieren (nicht auf einfache Taps)
        else if (!justWokeUp && (now - lastGestureTime > 500)) {
            // Timer zurücksetzen bei Interaktion
            powerTimer = now; 

            if (g.indexOf("LEFT") >= 0) { 
                displayRot = (displayRot + 1) % 4; 
                gfx->setRotation(displayRot); 
                prefs.putInt("rot", displayRot); 
                lastGestureTime = now;
            }
            else if (g.indexOf("RIGHT") >= 0) { 
                displayRot = (displayRot > 0) ? displayRot - 1 : 3; 
                gfx->setRotation(displayRot); 
                prefs.putInt("rot", displayRot); 
                lastGestureTime = now;
            }
            else if (g.indexOf("UP") >= 0) { 
                brightness = min(255, brightness + 30); 
                analogWrite(TFT_BL, brightness); 
                prefs.putInt("bright", brightness); 
                lastGestureTime = now;
            }
            else if (g.indexOf("DOWN") >= 0) { 
                brightness = max(10, brightness - 30); 
                analogWrite(TFT_BL, brightness); 
                prefs.putInt("bright", brightness); 
                lastGestureTime = now;
            }
        }
    }

    // 2. Power-Save Logik (Stabilisiert)
    if (displayOn) {
        // Sicherung: Falls gerade aufgewacht, Timer synchronisieren
        if (justWokeUp) {
            powerTimer = now;
            justWokeUp = false;
        }

        // Timeout Check (90 Sekunden)
        if (now - powerTimer > 90000) { 
            displayOn = false; 
            analogWrite(TFT_BL, 0); 
            digitalWrite(TFT_BL, LOW); 
            gfx->displayOff(); 
            Serial.println(">>> [SYSTEM]: Standby nach Timeout");
        }
    }
    
    // Wenn Display aus, CPU in Light Sleep versetzen
    if (!displayOn) { 
        esp_light_sleep_start();
        return; 
    }

    // 3. Preview-Auto-Clear
    if (previewPendingClear && (now - lastPreviewUpdateTime > 5000)) {
        nextNavIcon = 0; nextTurnMod = 0; nextDist = 0; nextStreet = ""; previewPendingClear = false;
    }

    // 4. SPD Deduction (Golden Rule 2)
    if (isConnected && lastKmh > 0.5f && (now - lastCalcTime >= 2000)) {
        float dDelta = (lastKmh / 3.6f) * ((float)(now - lastCalcTime) / 1000.0f);
        if (currentDist > 0) {
            currentDist = max(0.0f, currentDist - dDelta);
            if (currentDist <= 0 && arrivalTime == 0) arrivalTime = now;
        }
        lastCalcTime = now;
    }

    // 5. Rendering
    canvas->fillScreen(0);
    if (displayText == "warte auf App" && !isRerouting) {
        renderStartScreen(canvas, isConnected, macAddr);
    } else {
        renderNavScreen(canvas, navIcon, turnMod, currentDist, startDist, angleExtraDist, 
                        displayText, streetNumber, isConnected, isRerouting, arrivalTime, 
                        currentHeading, isReverse, lastPktTime, nextNavIcon, nextTurnMod);
    }
    canvas->flush();
    
    yield();
    delay(35);
}
