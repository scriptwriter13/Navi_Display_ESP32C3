// FILE: include/config.h
// STATUS: VOLLSTÄNDIG - Hardware-Anker für ESP32-2424S012

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- HARDWARE CONFIG [env:esp32-2424S012] ---
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

// --- FARBSCHEMA (Master-Anker) ---
#define NAV_GREEN  0xAFE5 
#define NAV_ORANGE 0xFD20 
#define NAV_GRAY   0x4208
#define NAV_SHADOW 0x4A49 

#endif