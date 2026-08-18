/*                                                                                                                                        
 * Copyright (C) 2026 by scriptwriter13                                                                                       
 *                                                                                                                                        
 * Dieses Programm ist freie Software: Sie können es unter den Bedingungen der                                                            
 * GNU General Public License, wie von der Free Software Foundation veröffentlicht,                                                       
 * entweder Version 3 der Lizenz oder (nach Ihrer Option) jeder späteren                                                                  
 * Version, weiterverbreiten und/oder modifizieren.                                                                                       
 *                                                                                                                                        
 * Dieses Programm wird in der Hoffnung, dass es nützlich sein wird, aber                                                                 
 * OHNE JEDE GEWÄHRLEISTUNG, sogar ohne die implizite Gewährleistung der                                                                  
 * MARKTGÄNGIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK. Siehe die                                                                     
 * GNU General Public License für weitere Details.                                                                                        
 *                                                                                                                                        
 * Sie sollten eine Kopie der GNU General Public License zusammen mit diesem                                                              
 * Programm erhalten haben. Wenn nicht, siehe <https://www.gnu.org/licenses/>.                                                            
 */
// FILE: src/main.cpp
// STATUS: FINAL STABLE - POWER-TIMER FIX + GESTURE DEBOUNCE + LIGHT SLEEP + WAKEUP FIX + OTA + ORANGE FLASH + DYNAMIC TOUCH
// DATE: 2026-03-29
// NOTE: Ich arbeite mit der Version vom 29. März weiter.
// HINWEIS ZU BISECT: Ich habe keinen Zugriff auf dein lokales Git-Log. 
// Bitte führe `git log --oneline` aus und wähle einen Commit, bei dem OTA noch funktionierte, als "good".

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <CST816S.h>  
#include <Wire.h>
#include <Preferences.h>
#include <esp_sleep.h>
#include <driver/uart.h>

#include "config.h"
#include "layout_config.h" 
#include "logic.h" 
#include "graphics.h"
#include "ble_handler.h"

// --- GLOBALE VARIABLEN ---
unsigned long fastAdvStartTime = 0;
bool isFastAdvertising = false;
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
bool isFlashing = false;
bool touchEnabled = false; // Flag für Touch-Erkennung
bool firmwareSelected = false; // NEU: Status der Auswahl
bool debugMode = true; // Manuelle Steuerung: true = Debug (kein Sleep), false = Prod (Sleep erlaubt)
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
    
    // Manuelle Debug-Steuerung
    if (debugMode) {
        Serial.println(">>> DEBUG-MODUS AKTIV: Light-Sleep GESPERRT <<<");
    } else {
        Serial.println(">>> PRODUKTIONS-MODUS: Light-Sleep ERLAUBT <<<");
    }

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
    
    // Touch-Erkennung via I2C
    Wire.begin(TOUCH_SDA, TOUCH_SCL); 
    Wire.beginTransmission(0x15);
    if (Wire.endTransmission() == 0) {
        Serial.println(">>> [SYSTEM]: CST816S Touch-Chip erkannt.");
        touchEnabled = true;
        touch.begin(Wire, TOUCH_INT);
        
        // Wakeup Konfiguration nur wenn Touch vorhanden
        esp_sleep_enable_gpio_wakeup();
        gpio_wakeup_enable((gpio_num_t)TOUCH_INT, GPIO_INTR_LOW_LEVEL);
    } else {
        Serial.println(">>> [SYSTEM]: Kein Touch-Chip gefunden.");
    }

    // UART Wakeup konfigurieren
    uart_set_wakeup_threshold(UART_NUM_0, 1);
    esp_sleep_enable_uart_wakeup(UART_NUM_0);

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

    // Fast-to-Slow Advertising Switch (nach 60s)
    if (isFastAdvertising && (now - fastAdvStartTime > 60000)) {
        Serial.println(">>> [BLE]: Fast Advertising Timeout - Wechsle zu Slow Mode.");
        setAdvertisingInterval(1600); // 1000ms
        isFastAdvertising = false;
    }

    // 0. BLE Watchdog: Wenn 15s keine Daten, erzwinge Reset (während OTA 60s)
    unsigned long timeout = isFlashing ? 60000 : 15000;
    if (isConnected && (now - lastPktTime > timeout)) {
        Serial.println(">>> [BLE]: Timeout! Verbindung tot. Force-Reset.");
        restartAdvertising();
        isConnected = false; 
    }

    // 0.1 OTA-Sicherheits-Timeout: Wenn 10s keine OTA-Daten, Flash-Modus abbrechen
    if (isFlashing && (now - lastPktTime > 10000)) {
        Serial.println(">>> [OTA]: Timeout! Flash-Modus abgebrochen (keine Daten).");
        isFlashing = false;
    }

    // 1. Touch-Handler mit Debouncing (nur wenn Chip vorhanden)
    if (touchEnabled && touch.available()) {
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

            // NEU: Wenn keine Geste erkannt wurde, ist es ein Tap -> Auswahl umschalten
            if (g == "NONE") {
                firmwareSelected = !firmwareSelected;
                Serial.printf(">>> [UI]: Firmware Auswahl: %s\n", firmwareSelected ? "AN" : "AUS");
                lastGestureTime = now;
            }
            else if (g.indexOf("LEFT") >= 0) { 
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
    
    // Wenn Display aus, CPU in Light Sleep versetzen (nur wenn nicht im Debug-Modus)
    if (!displayOn && !debugMode) { 
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
        renderStartScreen(canvas, isConnected, macAddr, firmwareSelected);
    } else {
        renderNavScreen(canvas, navIcon, turnMod, currentDist, startDist, angleExtraDist, 
                        displayText, streetNumber, isConnected, isRerouting, arrivalTime, 
                        currentHeading, isReverse, lastPktTime, nextNavIcon, nextTurnMod);
    }
    canvas->flush();
    
    yield();
    delay(35);
}
