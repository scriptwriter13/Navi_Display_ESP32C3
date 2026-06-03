// FILE: src/graphics.cpp
// STATUS: FULL ABSOLUTE CONTROL (MASTER ANKER)
// DATE: 2026-03-10
// CHANGE: Robust String concatenation for bracket values to prevent "364" merging.
// CHANGE: Ausfahrtsnummer im Kreisverkehr-Icon (type 4) hinzugefügt.
// CHANGE: Kreisverkehr-Radius um weitere 5px vergrößert.
// CHANGE: Pfeile um 5px nach links und 5px nach unten verschoben.
// CHANGE: U-Turn Icon: Körper um 10px nach unten verschoben.
// CHANGE: Startscreen: MAC-Position um 10px nach oben verschoben.
// CHANGE: Startscreen: Versions-Info hinzugefügt (automatisiert via Build-Flags).

#include "graphics.h"
#include "layout_config.h"
#include <Arduino.h>
#include "FreeSans12pt.h" 
#include "FreeSans15pt.h" 

// --- Fallback für Build-Flags, falls diese nicht über platformio.ini übergeben werden ---
#ifndef BUILD_VERSION
#define BUILD_VERSION "v1.0.0"
#endif
#ifndef BUILD_COMMIT
#define BUILD_COMMIT "dev"
#endif

// Extern deklariert, um auf die Punkt-Nummer aus der Logik/Main zuzugreifen
extern String pointNumber;
extern String metaDist, metaTime;

// --- Hilfsfunktion: UTF-8 zu GFX-Font Mapping ---
String utf8ToGfx(String s) {
    String out = "";
    for (int i = 0; i < (int)s.length(); i++) {
        uint8_t c = (uint8_t)s[i];
        if (c < 128) {
            out += (char)c;
        } else if (c == 0xc3) {
            i++;
            if (i < (int)s.length()) {
                uint8_t next = (uint8_t)s[i];
                out += (char)(next + 64); 
            }
        }
    }
    return out;
}

// --- Pfeil-Geometrie (Original Vektor-Logik) ---
void drawArrowGeometry(Arduino_Canvas *canvas, int cx, int cy, float angleDeg, int thick, int headLen, int totalSize, uint16_t color) {
    float rad = (90.0f - angleDeg) * 0.01745329f;
    float orth = rad + 1.570796f; 
    int stemH = totalSize / 2;
    
    canvas->fillRect(cx - (thick / 2), cy, thick, stemH, color);
    canvas->fillCircle(cx, cy, (thick / 2), color); 
    
    int armLen = (int)(totalSize * 0.65f); 
    float endX = (float)cx + (cos(rad) * armLen);
    float endY = (float)cy - (sin(rad) * armLen);
    
    float hThick = (float)thick / 2.0f;
    float dx = cos(orth) * hThick;
    float dy = sin(orth) * hThick;
    
    int p1x = round((float)cx - dx); int p1y = round((float)cy + dy);
    int p2x = round((float)cx + dx); int p2y = round((float)cy - dy);
    int p3x = round(endX + dx);      int p3y = round(endY - dy);
    int p4x = round(endX - dx);      int p4y = round(endY + dy);
    
    canvas->fillTriangle(p1x, p1y, p2x, p2y, p3x, p3y, color);
    canvas->fillTriangle(p1x, p1y, p3x, p3y, p4x, p4y, color);
    
    float tipX = (float)cx + (cos(rad) * (armLen + headLen));
    float tipY = (float)cy - (sin(rad) * (armLen + headLen));
    float sDx = cos(orth) * (thick * 1.3f); 
    float sDy = sin(orth) * (thick * 1.3f);
    
    int s1x = round(endX + sDx); int s1y = round(endY - sDy);
    int s2x = round(endX - sDx); int s2y = round(endY + sDy);
    canvas->fillTriangle(round(tipX), round(tipY), s1x, s1y, s2x, s2y, color);
}

void drawSingleNavIcon(Arduino_Canvas *canvas, int type, int mod, uint16_t color, int blX, int blY, int w, int h, int thicknessExtra) {
    float scale = (float)w / 60.0f; 
    int cx = blX + (w / 2), cy = blY - (h / 2); 
    int thick = max(6, (int)(11 * scale)) + thicknessExtra; 
    int headLen = (int)(20 * scale);

    if (type == 1 || type == 2 || type == 3 || type == 6) {
        float angle = (type == 3) ? 0 : (type == 6) ? 45.0f : ((mod == 1) ? 45.0f : (mod == 2) ? 105.0f : 90.0f);
        if (type == 1) angle *= -1.0f;
        
        // Verschiebung: Pfeile um 5px nach links und 5px nach unten
        int xShift = (type == 1) ? (w/6) : ((type == 2 || type == 6) ? -(w/6) - 20 : 0);
        drawArrowGeometry(canvas, cx + xShift - 5, cy + (h/4) + 5, angle, thick, headLen, h, color);
    } 
    else if (type == 4) { 
        int r_size = (w/2) - 2 + 10; // Radius um weitere 5px vergrößert
        for(int i=0; i<thick; i++) canvas->drawArc(cx, cy, r_size-i, r_size-i, 40, 320, color);
        canvas->fillTriangle(cx+(r_size*0.7), cy-(r_size*0.5), cx+(r_size*1.3), cy-(r_size*0.5), cx+(r_size*1.0), cy-(r_size*1.0), color);
        
        // Ausfahrtsnummer zeichnen (nur im Haupt-Icon, nicht im Schatten)
        if (mod > 0 && color != COL_BG_BLACK) {
            canvas->setFont(&FreeSans12pt8b);
            canvas->setTextSize(1);
            canvas->setTextColor(color);
            String modStr = String(mod);
            int16_t tx, ty; uint16_t tw, th;
            canvas->getTextBounds(modStr.c_str(), 0, 0, &tx, &ty, &tw, &th);
            canvas->setCursor(cx - (tw / 2), cy + (th / 2));
            canvas->print(modStr);
        }
    } 
    else if (type == 5) { 
        int r_base = cy - (h/4) + 10; // Körper um 10px nach unten verschoben
        for(int i=0; i<thick; i++) canvas->drawArc(cx, r_base, (w/2)-i, (w/2)-i, 180, 360, color);
        canvas->fillRect(blX, r_base, thick, h/4, color); 
        canvas->fillRect(blX + w - thick, r_base, thick, h/4, color);
        canvas->fillTriangle(blX - (thick/2), blY - (h/8), blX + (thick*1.5), blY - (h/8), blX + (thick/2), blY, color);
    }
    else if (type == 7) { 
        int ballRadius = ((thick * 4) / 2); 
        int stemHeight = h - (ballRadius / 2);
        canvas->fillRect(cx - (thick / 2), blY - stemHeight, thick, stemHeight, color);
        canvas->fillCircle(cx, blY - h + ballRadius, ballRadius, color);
    } 
}

void drawNavIcon(Arduino_Canvas *canvas, int type, int mod, uint16_t color, int blX, int blY, int w, int h) {
    if (type <= 0) return;
    drawSingleNavIcon(canvas, type, mod, COL_BG_BLACK, blX + 3, blY + 3, w, h, 0);
    drawSingleNavIcon(canvas, type, mod, color, blX, blY, w, h, 0);
}

// --- Startscreen ---
void renderStartScreen(Arduino_Canvas *canvas, bool isConnected, String macAddr) {
    canvas->fillScreen(COL_BG_BLACK);
    canvas->fillCircle(LED_X, LED_Y + 5, 8, isConnected ? COL_PKT_BLUE : 0xF800);
    
    canvas->setFont(&FreeSans12pt8b);
    canvas->setTextSize(1); 
    canvas->setTextColor(COL_MAIN_WHITE); 
    canvas->setCursor(45, 80); 
    canvas->print("ESP-Navi-HUD");

    int16_t x1, y1; uint16_t w1, h1;
    canvas->getTextBounds(macAddr.c_str(), 0, 0, &x1, &y1, &w1, &h1);
    canvas->setTextColor(COL_ACCENT_GREEN); 
    canvas->setCursor(120 - (w1 / 2), 130); // 10px nach oben verschoben (von 140 auf 130)
    canvas->print(macAddr);

    // Versions-Info hinzufügen
    canvas->setTextColor(COL_DIM_GRAY);
    String verInfo = String(BUILD_VERSION) + " (" + String(BUILD_COMMIT) + ")";
    int16_t vx, vy; uint16_t vw, vh;
    canvas->getTextBounds(verInfo.c_str(), 0, 0, &vx, &vy, &vw, &vh);
    canvas->setCursor(120 - (vw / 2), 160);
    canvas->print(verInfo);

    canvas->setTextColor(COL_MAIN_WHITE); 
    canvas->setCursor(40, 200); 
    canvas->print("warte auf App");
}

// --- Nav-Screen ---
void renderNavScreen(Arduino_Canvas *canvas, int navIcon, int turnMod, float currentDist, 
                     float startDist, float angleExtraDist, String displayText, 
                     String streetNumber, bool isConnected, bool isRerouting, 
                     unsigned long arrivalTime, float currentHeading, bool isReverse, 
                     unsigned long lastPktTime, int nextNavIcon, int nextTurnMod) {
    unsigned long now = millis();
    canvas->fillScreen(COL_BG_BLACK);
    
    if (!isConnected) canvas->fillCircle(LED_X, LED_Y, LED_R, 0xF800);
    canvas->drawCircle(120, 120, 117, COL_DIM_GRAY);

    // Heading-Punkt Logik
    if (lastPktTime > 0 && (now - lastPktTime < 45000)) {
        float absHeading = abs(currentHeading); 
        uint16_t pktColor = (absHeading < 5.0f) ? COL_ACCENT_GREEN : (absHeading < 25.0f ? COL_ALERT_ORANGE : 0xF800);
        float radH = (currentHeading - 90.0f) * 0.01745f;
        int pktX = 120 + (int)(cos(radH) * 109.0f);
        int pktY = 120 + (int)(sin(radH) * 109.0f);
        
        if (absHeading <= 15.0f || ((now / 250) % 2 == 0)) {
            canvas->fillCircle(pktX, pktY, 12, COL_BG_BLACK);
            canvas->fillCircle(pktX, pktY, 10, isReverse ? COL_PKT_BLUE : COL_MAIN_WHITE);
            canvas->fillCircle(pktX, pktY, 6, pktColor);
        }
    }

    if (isRerouting) {
        canvas->setFont(&FreeSans15pt8b);
        canvas->setTextSize(1); canvas->setTextColor(COL_MAIN_WHITE); 
        canvas->setCursor(45, 125); canvas->print("RE-ROUTE");
    } else {
        bool isAlert = (arrivalTime > 0 && (now - arrivalTime < 3000));
        uint16_t iconColor = (isAlert || (navIcon == 7 && (int)currentDist <= 0)) ? COL_ALERT_ORANGE : COL_ACCENT_GREEN;
        
        // Progress Ring
        if (isAlert) {
            if ((now / 400) % 2 == 0) { for(int i=0; i<360; i+=3) { float r=(i-90)*0.01745f; canvas->fillCircle(120+cos(r)*116, 120+sin(r)*116, 4, COL_ALERT_ORANGE); } }
        } else if (startDist > 0) {
            int angle = (int)(min(1.0f, (startDist-currentDist)/startDist)*360.0f);
            for(int i=0; i<angle; i+=3) { float r=(i-90)*0.01745f; canvas->fillCircle(120+cos(r)*116, 120+sin(r)*116, 4, COL_ACCENT_GREEN); }
        }

        if (!displayText.startsWith("Route:")) {
            drawNavIcon(canvas, navIcon, turnMod, iconColor, ICON_MAIN_X, ICON_MAIN_Y, ICON_MAIN_W, ICON_MAIN_H);
            
            // --- DISTANZEN ---
            if (!(navIcon == 3 && (int)currentDist <= 0)) {
                // Haupt-Distanz
                canvas->setFont(&FreeSans12pt8b);
                canvas->setTextSize(2); 
                canvas->setTextColor(DIST_MAIN_COLOR);
                String dStr = (isAlert || (int)currentDist <= 0) ? "0m" : ((currentDist >= 1000.0f) ? String(currentDist/1000.0f, 1)+"km" : String((int)currentDist)+"m");
                int16_t tx, ty; uint16_t tw, th;
                canvas->getTextBounds(dStr.c_str(), 0, 0, &tx, &ty, &tw, &th);
                canvas->setCursor(DIST_MAIN_X - (tw / 2), DIST_MAIN_Y + (th / 2)); 
                canvas->print(dStr);

                // --- KLAMMER-DISTANZ & PUNKT-NUMMER (FIXED STRING BUILDING) ---
                if (angleExtraDist > 0 || (pointNumber.length() > 0 && pointNumber != "0")) {
                    canvas->setFont(&FreeSans12pt8b);
                    canvas->setTextSize(1); 
                    canvas->setTextColor(DIST_SUB_COLOR);
                    
                    String bracketContent = "";
                    if (pointNumber.length() > 0 && pointNumber != "0") {
                        bracketContent += pointNumber;
                    }
                    if (angleExtraDist > 0) {
                        if (bracketContent.length() > 0) bracketContent += " "; 
                        bracketContent += String((int)angleExtraDist) + "m";
                    }

                    if (bracketContent.length() > 0) {
                        String finalOutput = "(" + bracketContent + ")"; 
                        int16_t etx, ety; uint16_t etw, eth;
                        canvas->getTextBounds(finalOutput.c_str(), 0, 0, &etx, &ety, &etw, &eth);
                        canvas->setCursor(DIST_SUB_X - (etw / 2), DIST_SUB_Y); 
                        canvas->print(finalOutput);
                    }
                }
            }
        }
        
        // VORSCHAU-ICON
        if (nextNavIcon > 0) {
            drawNavIcon(canvas, nextNavIcon, nextTurnMod, COL_DIM_GRAY, PREVIEW_X, PREVIEW_Y, PREVIEW_SIZE, PREVIEW_SIZE);
        }

        // STRASSE
        canvas->setFont(&FreeSans12pt8b);
        if (displayText.length() > 0) {
            String gfxText = utf8ToGfx(displayText);
            canvas->setTextSize(1); canvas->setTextColor(STREET_COLOR);
            int16_t tx1, ty1; uint16_t tw1, th1; 
            canvas->getTextBounds(gfxText.c_str(), 0, 0, &tx1, &ty1, &tw1, &th1);
            canvas->setCursor(max(15, 120-(tw1/2)), STREET_Y); 
            canvas->print(gfxText);
        }

        // NEU: Logik für streetNumber / Meta-Daten
        if (streetNumber.length() == 0) {
            streetNumber = metaDist;
            if (metaTime.length() > 0) {
                streetNumber += " • " + metaTime;
            }
        }

        if (streetNumber.length() > 0) {
            String gfxStreetNum = utf8ToGfx(streetNumber);
            canvas->setTextSize(1); 
            canvas->setTextColor(STREET_COLOR); 
            int16_t tx2, ty2; uint16_t tw2, th2; 
            canvas->getTextBounds(gfxStreetNum.c_str(), 0, 0, &tx2, &ty2, &tw2, &th2);
            canvas->setCursor(max(15, 120-(tw2/2)), STREET_Y + 25); // 25px unter der Hauptzeile
            canvas->print(gfxStreetNum);
        }
    }
}
