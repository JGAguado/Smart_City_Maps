#ifndef SERIAL_CONFIG_H
#define SERIAL_CONFIG_H

// ESP32-S2 Serial Configuration
// Use USB CDC Serial for direct USB connection
// No additional wiring required - uses built-in USB connector

#include <Arduino.h>

// Use default Serial (USB CDC)
// No redefinition needed - Serial is already USB CDC

// Initialize USB CDC serial with extra time for enumeration
inline void initSerial() {
    Serial.begin(115200);
    delay(2000); // Give USB more time to enumerate
    
    // Force USB CDC to be ready
    while (!Serial) {
        delay(10);
    }
    
    Serial.flush();
}

#endif // SERIAL_CONFIG_H
