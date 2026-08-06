// FILE: include/config.h
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
// FILE: include/config.h
// STATUS: VOLLSTÄNDIG - Hardware-Anker für ESP32-2424S012

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- HARDWARE CONFIG [env:esp32-2424S012] ---

// --- Fallbacks, falls die build_flags in der platformio.ini mal vergessen gehen ---
#ifndef HARDWARE_VERSION
#define HARDWARE_VERSION "ESP32-2424S012-V1.0"
#define TFT_BL 3
#define SCK_PIN 6
#define MOSI_PIN 7
#define MISO_PIN -1      
#define DC_PIN 2       
#define CS_PIN 10
#define RST_PIN 1       
#define TOUCH_SDA 4
#define TOUCH_SCL 5
#define TOUCH_INT 0      
#define TOUCH_RST 1      
#endif

// --- FARBSCHEMA (Master-Anker) ---
#define NAV_GREEN  0xAFE5 
#define NAV_ORANGE 0xFD20 
#define NAV_GRAY   0x4208
#define NAV_SHADOW 0x4A49 

#endif

#define OTA_SECRET_KEY "BIKE_HUD_OTA_2026"
