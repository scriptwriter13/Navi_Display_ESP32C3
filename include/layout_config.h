// FILE: include/layout_config.h
#ifndef LAYOUT_CONFIG_H
#define LAYOUT_CONFIG_H

#include <Arduino.h>

// --- FARBEN ---
#define COL_MAIN_WHITE    0xFFFF
#define COL_ACCENT_GREEN  0x07E0
#define COL_ALERT_ORANGE  0xFDA0
#define COL_DIM_GRAY      0x7BEF
#define COL_STREET_NUM    0xBDD7
#define COL_BG_BLACK      0x0000
#define COL_PKT_BLUE      0x001F
#define COL_SUB_DIST      0x5DFF // Hellblau für die Klammer
#define NAV_GRAY          0x4208 

// --- ELEMENT: HAUPT-DISTANZ (Nutzt FreeSans15pt8b) ---
const int DIST_MAIN_X     = 160; // Leicht angepasst für bessere Zentrierung rechts
const int DIST_MAIN_Y     = 80;  // Ankerpunkt für Y=80 Linie
const uint16_t DIST_MAIN_COLOR = COL_MAIN_WHITE;

// --- ELEMENT: KLAMMER-DISTANZ (Nutzt FreeSans12pt8b) ---
const int DIST_SUB_X      = 175;
const int DIST_SUB_Y      = 125; // Positioniert unter Haupt-Distanz
const uint16_t DIST_SUB_COLOR = COL_SUB_DIST;

// --- ELEMENT: HAUPT-ICON ---
const int ICON_MAIN_X     = 40;
const int ICON_MAIN_Y     = 105; 
const int ICON_MAIN_W     = 55;
const int ICON_MAIN_H     = 60;

// --- ELEMENT: VORSCHAU-ICON (Unten) ---
const int PREVIEW_X       = 103; 
const int PREVIEW_Y       = 220; 
const int PREVIEW_SIZE    = 35;

// --- ELEMENT: STRASSENNAME (Nutzt FreeSans12pt8b) ---
const int STREET_X        = 120;
const int STREET_Y        = 165;
const uint16_t STREET_COLOR = COL_MAIN_WHITE;

// --- ELEMENT: HAUSNUMMER / ZUSATZ ---
const int STR_NUM_X       = 120;
const int STR_NUM_Y       = 190;
const uint16_t STR_NUM_COLOR = COL_STREET_NUM;

// --- ELEMENT: STATUS LED (Oben) ---
const int LED_X           = 120;
const int LED_Y           = 20;
const int LED_R           = 6;

#endif