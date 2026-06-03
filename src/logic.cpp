// FILE: src/logic.cpp
// STATUS: MASTER-ANKER - FIX: UTF-8 Bullet Parsing & NAV-JSON Output
// DATE: 2026-03-29
// CHANGE: Integration der printNavJson Funktion zur seriellen Ausgabe des Systemzustands.
// CHANGE: Rerouting-Erkennung für "Route neu berechnet" hinzugefügt.
// CHANGE: Bullet-Point Stripping für OSMAND-Daten hinzugefügt.
// CHANGE: "ausfahrt" zu Trennern hinzugefügt.
// CHANGE: PFX-Algorithmus zur Distanz-Trend-Analyse hinzugefügt.
// CHANGE: Fix für Kreisverkehr-Ausfahrtsnummer (mod) in parseIconType (Unterstützung bis 9. Ausfahrt).
// CHANGE: Kreisverkehr-Ausgabe in extractDataFromPart korrigiert.
// CHANGE: Parser-Robustheit für "X Ausfahrt" (ohne Punkt) erhöht.
// CHANGE: Erzwinge "X. Ausfahrt" Anzeige bei Kreisverkehr.

#include "logic.h"

// Steuerung für Umlaut-Behandlung (False = Native Umlaute für FreeSans12pt8b)
const bool USE_UMLAUT_TRANSCRIPTION = false; 

// PFX-Algorithmus Puffer
float distHistory[4] = {0, 0, 0, 0};

extern String displayText, streetNumber, nextStreet;
extern String pointNumber; 
extern String metaDist, metaTime, metaEta, metaSpd;
extern float angleExtraDist, startDist, currentDist, nextDist;
extern unsigned long lastCalcTime, arrivalTime;
extern float distSinceAction;
extern bool actionActive, isRerouting;
extern int spdWaitCount;
extern int navIcon, turnMod, nextNavIcon, nextTurnMod;

const int shortFormsCount = 4;
ReplacePair shortForms[] = {
    {"Richtung", "Rtg."},
    {"Strasse", "Str."},
    {"Street", "St."},
    {"Weg", "W."}
};

// PFX-Algorithmus: Analysiert Distanz-Trend
void updateRouteProgress(float currentDist) {
    // Shift Buffer
    distHistory[0] = distHistory[1];
    distHistory[1] = distHistory[2];
    distHistory[2] = distHistory[3];
    distHistory[3] = currentDist;

    // Trend-Analyse: Summe der letzten 2 vs. Summe der vorherigen 2
    float lastTwo = distHistory[2] + distHistory[3];
    float prevTwo = distHistory[0] + distHistory[1];

    if (prevTwo > 0) {
        if (lastTwo > prevTwo) {
            // Wir entfernen uns vom Ziel
            Serial.println(">>> [PFX]: Distanz nimmt zu - Punkt passiert oder falsch?");
        } else {
            // Wir nähern uns
            Serial.println(">>> [PFX]: Distanz nimmt ab - Kurs korrekt.");
        }
    }
}

// Hilfsfunktion zur JSON-Ausgabe des Navigationsstatus
void printNavJson() {
    String json = "{";
    // Aktuelle Anweisung (ACT)
    json += "\"act\":{";
    json += "\"icon\":" + String(navIcon) + ",";
    json += "\"mod\":" + String(turnMod) + ",";
    json += "\"dist\":" + String(currentDist, 0) + ",";
    json += "\"street\":\"" + displayText + "\",";
    json += "\"nr\":\"" + streetNumber + "\"";
    json += "},";
    // Nächste Anweisung (NEXT)
    json += "\"next\":{";
    json += "\"icon\":" + String(nextNavIcon) + ",";
    json += "\"mod\":" + String(nextTurnMod) + ",";
    json += "\"dist\":" + String(nextDist, 0) + ",";
    json += "\"street\":\"" + nextStreet + "\"";
    json += "}";
    json += "}";
    Serial.print(">>> [NAV-JSON]: "); Serial.println(json);
}

String applyShortForms(String text) {
    for (int i = 0; i < shortFormsCount; i++) {
        String searchLower = shortForms[i].search; 
        searchLower.toLowerCase();
        String currentTextLower = text;
        currentTextLower.toLowerCase();
        int pos = currentTextLower.indexOf(searchLower);
        while (pos != -1) {
            String replacement = shortForms[i].replace;
            if (islower(text[pos])) {
                replacement.toLowerCase();
            }
            text = text.substring(0, pos) + replacement + text.substring(pos + shortForms[i].search.length());
            currentTextLower = text;
            currentTextLower.toLowerCase();
            pos = currentTextLower.indexOf(searchLower, pos + replacement.length());
        }
    }
    return text;
}

String fixUtf8(String str) {
    String result = "";
    for (int i = 0; i < (int)str.length(); i++) {
        uint8_t c = (uint8_t)str[i];
        if (c == 0xC3) { 
            i++;
            if (i < (int)str.length()) {
                uint8_t next = (uint8_t)str[i];
                if (USE_UMLAUT_TRANSCRIPTION) {
                    switch (next) {
                        case 0xA4: result += "ae"; break; 
                        case 0xB6: result += "oe"; break; 
                        case 0xBC: result += "ue"; break; 
                        case 0x84: result += "Ae"; break; 
                        case 0x96: result += "Oe"; break; 
                        case 0x9C: result += "Ue"; break; 
                        case 0x9F: result += "ss"; break; 
                        default: break;
                    }
                } else {
                    result += (char)0xC3;
                    result += (char)next;
                }
            }
        } else { result += (char)c; }
    }
    return result;
}

String stripUtf8Headers(String str) {
    String out = "";
    for(int i=0; i<(int)str.length(); i++) {
        if((uint8_t)str[i] != 0xC3) out += str[i];
    }
    return out;
}

// Entfernt Bullet-Points (0xE2 0x80 0xA2) aus Strings
String stripBullets(String str) {
    String out = "";
    for (int i = 0; i < (int)str.length(); i++) {
        if (i + 2 < (int)str.length() && (uint8_t)str[i] == 0xE2 && (uint8_t)str[i+1] == 0x80 && (uint8_t)str[i+2] == 0xA2) {
            i += 2;
        } else {
            out += str[i];
        }
    }
    return out;
}

float wordToNum(String low) {
    String clean = stripUtf8Headers(low);
    clean.toLowerCase();
    String res = ""; 
    bool foundDigit = false;
    for(int i=0; i<(int)clean.length(); i++) {
        if(isDigit(clean[i]) || clean[i] == '.' || clean[i] == ',') {
            char c = clean[i]; if (c == ',') c = '.';
            res += c; foundDigit = true;
        } else if(foundDigit && clean[i] != ' ') break; 
    }
    if(res.length() > 0) return res.toFloat();
    float val = 0;
    if (clean.indexOf("tausend") >= 0 || clean.indexOf("thousand") >= 0) val += 1000;
    if (clean.indexOf("neunhundert") >= 0) val += 900;
    else if (clean.indexOf("achthundert") >= 0) val += 800;
    else if (clean.indexOf("siebenhundert") >= 0) val += 700;
    else if (clean.indexOf("sechshundert") >= 0) val += 600;
    else if (clean.indexOf("fuenfhundert") >= 0 || clean.indexOf("fünfhundert") >= 0) val += 500;
    else if (clean.indexOf("vierhundert") >= 0) val += 400;
    else if (clean.indexOf("dreihundert") >= 0) val += 300;
    else if (clean.indexOf("zweihundert") >= 0) val += 200;
    else if (clean.indexOf("hundert") >= 0 || clean.indexOf("hundred") >= 0) val += 100;
    if (clean.indexOf("neunzig") >= 0 || clean.indexOf("ninety") >= 0) val += 90;
    else if (clean.indexOf("achtzig") >= 0 || clean.indexOf("eighty") >= 0) val += 80;
    else if (clean.indexOf("siebzig") >= 0 || clean.indexOf("seventy") >= 0) val += 70;
    else if (clean.indexOf("sechzig") >= 0 || clean.indexOf("sixty") >= 0) val += 60;
    else if (clean.indexOf("fuenfzig") >= 0 || clean.indexOf("fifty") >= 0) val += 50;
    else if (clean.indexOf("vierzig") >= 0 || clean.indexOf("forty") >= 0) val += 40;
    else if (clean.indexOf("dreissig") >= 0 || clean.indexOf("thirty") >= 0) val += 30;
    else if (clean.indexOf("zwanzig") >= 0 || clean.indexOf("twenty") >= 0) val += 20;
    if (clean.indexOf("einen") >= 0 || clean.indexOf("einem") >= 0 || clean.indexOf(" ein ") >= 0 || clean.startsWith("ein ") || clean == "ein" || clean == "one") val += 1;
    else if (clean.indexOf("zwei") >= 0 || clean.indexOf("two") >= 0) val += 2;
    else if (clean.indexOf("drei") >= 0 || clean.indexOf("three") >= 0) val += 3;
    return val;
}

String cleanNavText(String text, float dist) {
    text = stripBullets(text); // Bullet-Points entfernen
    String low = text; low.toLowerCase();
    if (low.startsWith("route:")) return "Route: " + text.substring(6);
    if (low.indexOf("neuberechnung") >= 0 || low.indexOf("recalculating") >= 0 || low.indexOf("route neu berechnet") >= 0) { 
        isRerouting = true; return "KEEP_OLD"; 
    }
    isRerouting = false;
    if (low.indexOf("ankomm") >= 0 || low.indexOf("ziel") >= 0 || low.indexOf("arriv") >= 0) {
        return (dist > 10.0f) ? "Ziel voraus" : "Ziel erreicht";
    }
    if (low.indexOf("wenden") >= 0 || low.indexOf("u-turn") >= 0) return "BITTE WENDEN";
    int roadIdx = -1;
    const char* separators[] = {"auf die ", "auf der ", "in die ", "in der ", "richtung ", "onto ", "towards ", "bis zum ", "bis zur ", "auf ", "in ", "ausfahrt "};
    for (int i = 0; i < 12; i++) {
        int found = low.lastIndexOf(separators[i]); 
        if (found >= 0) { 
            int candidate = found + strlen(separators[i]);
            if (candidate > roadIdx) roadIdx = candidate; 
        } 
    }
    if (roadIdx == -1) return "";
    String road = text.substring(roadIdx); road.trim();
    int pipePos = road.indexOf('|');
    if (pipePos != -1) road = road.substring(0, pipePos);
    road.trim();
    if (road.length() < 2 || road == "." || road == ",") return "";
    const char* articles[] = {"der ", "die ", "das ", "dem ", "den ", "the "};
    bool foundArticle;
    do {
        foundArticle = false;
        String roadLow = road; roadLow.toLowerCase();
        for (int a = 0; a < 6; a++) {
            if (roadLow.startsWith(articles[a])) {
                road = road.substring(strlen(articles[a]));
                road.trim(); foundArticle = true;
            }
        }
    } while (foundArticle);
    road = fixUtf8(road); 
    road = applyShortForms(road);
    if (road.length() > 24) road = road.substring(0, 22) + "..";
    return road;
}

void parseIconType(String low, int &icon, int &mod) {
    if (low.indexOf("ankomm") >= 0 || low.indexOf("angekomm") >= 0 || low.indexOf("destin") >= 0 || low.indexOf("arriv") >= 0 || low.indexOf("ziel") >= 0 || low.indexOf("target") >= 0) icon = 7; 
    else if (low.indexOf("kreis") >= 0 || low.indexOf("roundabout") >= 0 || low.indexOf("ausfahrt") >= 0) {
        icon = 4;
        // Erweiterte Erkennung der Ausfahrtsnummer bis 9
        const char* words[] = {
            "erste","first","1.",
            "zweite","second","2.",
            "dritte","third","3.",
            "vierte","fourth","4.",
            "fuenfte","fifth","5.",
            "sechste","sixth","6.",
            "siebte","seventh","7.",
            "achte","eighth","8.",
            "neunte","ninth","9."
        };
        for (int i=0; i<27; i++) { 
            if (low.indexOf(words[i]) >= 0) { 
                mod = (i/3)+1; 
                break; 
            } 
        }
        // Fallback für "X Ausfahrt" ohne Punkt
        if (mod == 0) {
            for (int i = 1; i <= 9; i++) {
                if (low.indexOf(String(i) + " ausfahrt") >= 0) {
                    mod = i;
                    break;
                }
            }
        }
        // Fallback, falls keine Nummer gefunden wurde, aber Kreisverkehr erkannt
        if (mod == 0) mod = 1; 
    } 
    else if (low.indexOf("links") >= 0 || low.indexOf("left") >= 0) icon = 1; 
    else if (low.indexOf("rechts") >= 0 || low.indexOf("right") >= 0) icon = 2;
    else if (low.indexOf("gerade") >= 0 || low.indexOf("straight") >= 0 || low.indexOf("folgen") >= 0 || low.indexOf("follow") >= 0) icon = 3; 
    else if (low.indexOf("wenden") >= 0 || low.indexOf("u-turn") >= 0) icon = 5;
    if ((icon == 1 || icon == 2)) { 
        if (low.indexOf("scharf") >= 0 || low.indexOf("sharp") >= 0) mod = 2; 
        else if (low.indexOf("leicht") >= 0 || low.indexOf("slight") >= 0 || low.indexOf("halten") >= 0 || low.indexOf("keep") >= 0) mod = 1; 
    }
}

void extractDataFromPart(String part, int &targetIcon, int &targetMod, float &targetDist, String &targetStreet, bool isMain) {
    String lowCheck = part; lowCheck.toLowerCase();
    int dannPos = lowCheck.indexOf("dann ");
    if (dannPos >= 0) {
        String prefix = part.substring(0, dannPos);
        String suffix = part.substring(dannPos + 5); 
        part = prefix + "in " + suffix;
        lowCheck = part; lowCheck.toLowerCase();
    }
    if (lowCheck.indexOf("weichen") >= 0 || lowCheck.indexOf("abweichen") >= 0 || lowCheck.indexOf("off route") >= 0) {
        if (isMain) { targetStreet = "Routenabweichung"; streetNumber = ""; }
        return; 
    }
    if (isMain) {
        int firstSemi = part.indexOf(';'); angleExtraDist = 0;
        if (firstSemi != -1) {
            String extraPart = part.substring(firstSemi + 1); extraPart.trim();
            int secondSemi = extraPart.indexOf(';');
            String distPart = (secondSemi != -1) ? extraPart.substring(0, secondSemi) : extraPart;
            distPart.trim();
            String resE = "";
            for (int i = 0; i < (int)distPart.length(); i++) {
                if (isDigit(distPart[i]) || distPart[i] == '.' || distPart[i] == ',') {
                    char c = distPart[i]; if (c == ',') c = '.'; resE += c;
                }
            }
            if (resE.length() > 0) {
                angleExtraDist = resE.toFloat();
                String distPartLow = distPart; distPartLow.toLowerCase();
                if (distPartLow.indexOf("km") >= 0) angleExtraDist *= 1000.0f;
            }
            if (secondSemi != -1) {
                pointNumber = extraPart.substring(secondSemi + 1); pointNumber.trim();
            } else { pointNumber = ""; }
            part = part.substring(0, firstSemi);
        } else { pointNumber = ""; }
    }
    String low = part; low.toLowerCase();
    float d = -1.0f; bool distFound = false;
    const char* units[] = {" kilometer", " kilometern", " km", " metern", " meter", " m", " metre"};
    for (int i = 0; i < 7; i++) {
        int unitIdx = low.indexOf(units[i]);
        if (unitIdx != -1) {
            int startScan = max(0, unitIdx - 30);
            String lookBefore = low.substring(startScan, unitIdx);
            d = wordToNum(lookBefore);
            if (d >= 0) {
                if (low.indexOf("km") >= 0 || low.indexOf("kilometer") >= 0) d *= 1000.0f;
                distFound = true; break;
            }
        }
    }
    if (distFound) {
        targetDist = d;
        if (isMain) { startDist = d; currentDist = d; arrivalTime = 0; distSinceAction = 0; actionActive = true; lastCalcTime = millis(); }
    } else if (isMain) {
        int ti, tm; parseIconType(low, ti, tm);
        if (ti > 0) { targetDist = 0; spdWaitCount = 1; lastCalcTime = millis(); }
    }
    int tempIcon = 0, tempMod = 0; parseIconType(low, tempIcon, tempMod);
    if (tempIcon > 0) { targetIcon = tempIcon; targetMod = tempMod; }
    if (isMain) {
        if (targetIcon == 3 && distFound) {
            bool hasSeparator = false;
            const char* seps[] = {" auf ", " in ", " onto ", " towards ", " bis zum ", " bis zur "};
            for(int s=0; s<6; s++) { if(low.indexOf(seps[s]) >= 0) hasSeparator = true; }
            if (!hasSeparator) { targetStreet = ""; streetNumber = ""; return; }
        }
        
        // Fix: Kreisverkehr-Ausgabe erzwingen
        if (targetIcon == 4) {
            targetStreet = String(targetMod) + ". Ausfahrt";
            streetNumber = "";
        } else {
            String st = cleanNavText(part, targetDist);
            if (st != "KEEP_OLD") {
                int lastIdx = max(st.lastIndexOf(','), st.lastIndexOf(';'));
                if (lastIdx != -1 && !st.startsWith("Route:")) { 
                    streetNumber = st.substring(0, lastIdx); streetNumber.trim(); 
                    targetStreet = st.substring(lastIdx + 1); targetStreet.trim(); 
                } else { targetStreet = st; streetNumber = ""; }
            }
        }
    }
}

// Neuer, wasserdichter 3-Phasen Parser
void parseBleData(String raw) {
    raw.trim(); 

    // 1. Suffix "|OSMAND" entfernen
    int osmandPos = raw.indexOf("|OSMAND");
    if (osmandPos != -1) {
        raw = raw.substring(0, osmandPos);
    }
    raw.trim();

    // 2. SPEZIFISCHE FEHLER-ERKENNUNG:
    // Wenn das Muster "0 m • :" vorkommt, ist es der Fehlerfall.
    // Wir delegieren das KOMPLETT an parseOsmandMeta.
    if (raw.indexOf("0 m • :") != -1) {
        parseOsmandMeta(raw);
        return; 
    }

    // 3. Standard-Parsing (für normale Nachrichten)
    int pipePos = raw.indexOf('|');
    String navPart = (pipePos != -1) ? raw.substring(0, pipePos) : raw;
    String metaPart = (pipePos != -1) ? raw.substring(pipePos + 1) : "";
    
    // Nav-Extraktion
    int bulletPos = -1;
    for(int i=0; i < (int)navPart.length() - 2; i++) {
        if((uint8_t)navPart[i] == 0xE2 && (uint8_t)navPart[i+1] == 0x80 && (uint8_t)navPart[i+2] == 0xA2) {
            bulletPos = i;
            break;
        }
    }
    
    if (bulletPos != -1) {
        String distStr = navPart.substring(0, bulletPos);
        String instrStr = navPart.substring(bulletPos + 3);
        extractDataFromPart(distStr + " " + instrStr, navIcon, turnMod, currentDist, displayText, true);
    } else {
        extractDataFromPart(navPart, navIcon, turnMod, currentDist, displayText, true);
    }

    // Meta-Extraktion
    if (metaPart.length() > 0) {
        parseOsmandMeta(metaPart);
    }
}

String cleanMetaPart(String s) {
    s.replace(":", "");
    s.trim();
    return s;
}

void parseOsmandMeta(String meta) {
    String parts[5] = {"", "", "", "", ""};
    int count = 0;
    int lastPos = 0;

    for (int i = 0; i < (int)meta.length() - 2; i++) {
        if ((uint8_t)meta[i] == 0xE2 && (uint8_t)meta[i+1] == 0x80 && (uint8_t)meta[i+2] == 0xA2) {
            if (count < 5) {
                parts[count] = meta.substring(lastPos, i);
                parts[count].trim();
            }
            count++;
            lastPos = i + 3; 
            i += 2; 
        }
    }
    if (count < 5) {
        parts[count] = meta.substring(lastPos);
        parts[count].trim();
    }

    // Logik: Wenn das erste Segment "0 m" oder "0m" ist, überspringen
    int offset = 0;
    String p0 = parts[0];
    p0.toLowerCase();
    if (p0 == "0 m" || p0 == "0m") {
        offset = 1;
    }

    // Zuweisung mit Offset und Bereinigung (cleanMetaPart entfernt den Doppelpunkt!)
    metaDist = cleanMetaPart(parts[0 + offset]);
    metaTime = cleanMetaPart(parts[1 + offset]);
    metaEta  = cleanMetaPart(parts[2 + offset]);
    metaSpd  = cleanMetaPart(parts[3 + offset]);

    String json = "{";
    json += "\"dist\":\"" + metaDist + "\",";
    json += "\"time\":\"" + metaTime + "\",";
    json += "\"eta\":\"" + metaEta + "\",";
    json += "\"spd\":\"" + metaSpd + "\"";
    json += "}";
    
    Serial.print(">>> [META-JSON]: "); Serial.println(json);
}
