// FILE: include/logic.h
// STATUS: LINKER-SYNCHRONIZED - Definitive Schnittstelle
// DATE: 2026-03-29
// CHANGE: PFX-Algorithmus Prototyp hinzugefügt.
// CHANGE: Fehlende Prototypen für OsmAnd und JSON-Output hinzugefügt.

#ifndef LOGIC_H
#define LOGIC_H

#include <Arduino.h>

// Strukturen
struct ReplacePair { String search; String replace; };

// Funktionen
String fixUtf8(String str);
String applyShortForms(String text);
float wordToNum(String low);
String cleanNavText(String text, float dist);
void parseIconType(String low, int &icon, int &mod);
void extractDataFromPart(String part, int &targetIcon, int &targetMod, float &targetDist, String &targetStreet, bool isMain);
void updateRouteProgress(float currentDist); // PFX-Algorithmus
void parseBleData(String raw);
void parseOsmandMeta(String meta);
String cleanMetaPart(String s);
void printNavJson();

// EXTERN-BLOCK: Das "Inhaltsverzeichnis" für alle Module
extern String streetNumber;
extern String pointNumber;      // Wichtig für den PKT-Parser!
extern float angleExtraDist, startDist, currentDist;
extern float currentHeading;    // Hinzugefügt
extern float lastKmh;           // Hinzugefügt
extern unsigned long lastCalcTime, arrivalTime, lastPktTime; 
extern float distSinceAction;
extern bool actionActive, isRerouting, isReverse;
extern int spdWaitCount;

#endif
