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
extern bool isFlashing; // Status-Flag für orangen Blink-Modus während OTA-Flash

// Render-Funktionen
void renderStartScreen(Arduino_Canvas *canvas, bool connected, String mac);
void renderNavScreen(Arduino_Canvas *canvas, int icon, int mod, float dist, float startD, 
                     float extraD, String text, String streetNum, bool conn, bool reroute, 
                     unsigned long arrival, float heading, bool reverse, unsigned long lastPkt,
                     int nextIcon, int nextMod);

#endif
