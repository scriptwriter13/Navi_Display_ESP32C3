#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <Arduino_GFX_Library.h>

// Globale Variablen (extern Deklaration für Zugriff aus graphics.cpp)
extern String displayText;
extern String streetNumber;
extern String pointNumber; // Der Punkt-Index (z.B. "1/12")
extern String macAddr;
extern float currentDist;
extern float lastKmh;
extern bool isConnected;
extern int navIcon;
extern int turnMod;

// Render-Funktionen
void renderStartScreen(Arduino_Canvas *canvas, bool connected, String mac);
void renderNavScreen(Arduino_Canvas *canvas, int icon, int mod, float dist, float startD, 
                     float extraD, String text, String streetNum, bool conn, bool reroute, 
                     unsigned long arrival, float heading, bool reverse, unsigned long lastPkt,
                     int nextIcon, int nextMod);

#endif