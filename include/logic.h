// FILE: include/logic.h
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
// FILE: include/logic.h
// STATUS: LINKER-SYNCHRONIZED - Definitive Schnittstelle
// DATE: 2026-03-29
// CHANGE: PFX-Algorithmus Prototyp hinzugefügt.
// CHANGE: Fehlende Prototypen für OsmAnd und JSON-Output hinzugefügt.
// CHANGE: Integration von isFlashing Status-Flag.

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
