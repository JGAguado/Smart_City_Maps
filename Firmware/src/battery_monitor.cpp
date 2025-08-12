#include "battery_monitor.h"
#include "serial_config.h"

BatteryMonitor::BatteryMonitor() : 
    initialized(false), 
    lastPercentage(0.0), 
    lastVoltage(0.0), 
    lastUpdateTime(0) {
}

BatteryMonitor::~BatteryMonitor() {
}

bool BatteryMonitor::initialize() {
    Serial.println("Initializing MAX17048 battery monitor...");
    Serial.flush();
    
    // Enable I2C sensors by setting GPIO4 HIGH
    Serial.println("Enabling I2C sensors via GPIO4...");
    pinMode(13, OUTPUT);
    digitalWrite(13, HIGH);
    delay(100); // Give sensors time to power up
    Serial.flush();
    
    // Initialize I2C with correct pins: SDA=GPIO33, SCL=GPIO34
    Serial.println("Starting I2C on SDA=GPIO33, SCL=GPIO34...");
    Serial.flush();
    Wire.begin(33, 34); // SDA=GPIO33, SCL=GPIO34
    delay(100);
    
    // Initialize the SparkFun MAX17048 library
    Serial.println("Initializing SparkFun MAX17048...");
    Serial.flush();
    
    // Set up MAX17048 LiPo fuel gauge
    if (lipo.begin() == false) {
        Serial.println("MAX17048 not detected. Please check wiring.");
        Serial.flush();
        
        // Let's scan I2C bus to see what's connected
        Serial.println("Scanning I2C bus...");
        bool foundDevice = false;
        for (uint8_t addr = 1; addr < 127; addr++) {
            Wire.beginTransmission(addr);
            if (Wire.endTransmission() == 0) {
                Serial.printf("I2C device found at address 0x%02X\n", addr);
                foundDevice = true;
            }
        }
        if (!foundDevice) {
            Serial.println("No I2C devices found on the bus");
        }
        Serial.flush();
        
        return false;
    }
    
    Serial.println("MAX17048 connected!");
    
    // Quick start restarts the MAX17048 in hopes of getting a more accurate
    // guess for the SOC.
    lipo.quickStart();
    
    // We can set an interrupt to alert when the battery SoC gets too low.
    // We can alert at anywhere from 1% to 32%:
    lipo.setThreshold(20); // Set alert threshold to 20%.
    
    Serial.println("MAX17048 battery monitor initialized successfully!");
    Serial.flush();
    
    initialized = true;
    
    // Get initial reading
    update();
    
    return true;
}

float BatteryMonitor::getBatteryPercentage() {
    if (!initialized) return 0.0;
    return lastPercentage;
}

float BatteryMonitor::getBatteryVoltage() {
    if (!initialized) return 0.0;
    return lastVoltage;
}

bool BatteryMonitor::isConnected() {
    return initialized;
}

void BatteryMonitor::update() {
    if (!initialized) return;
    
    unsigned long currentTime = millis();
    if (currentTime - lastUpdateTime < UPDATE_INTERVAL) {
        return; // Don't update too frequently
    }
    
    // Read battery data using SparkFun MAX17048 library
    lastPercentage = lipo.getSOC();
    lastVoltage = lipo.getVoltage();
    
    // Ensure percentage is within valid bounds (0-100%)
    if (lastPercentage > 100.0) lastPercentage = 100.0;
    if (lastPercentage < 0.0) lastPercentage = 0.0;
    
    lastUpdateTime = currentTime;
    
    Serial.printf("Battery: %.1f%%, %.2fV", lastPercentage, lastVoltage);
    
    // Check if battery is low
    if (lipo.getAlert()) {
        Serial.print(" - LOW BATTERY ALERT!");
        lipo.clearAlert();
    }
    
    Serial.println();
    Serial.flush();
}

int BatteryMonitor::getBatteryIconIndex() {
    if (!initialized) return 0;
    
    float percentage = getBatteryPercentage();
    
    // Map percentage to icon index (0-10)
    if (percentage >= 95) return 10;
    if (percentage >= 85) return 9;
    if (percentage >= 75) return 8;
    if (percentage >= 65) return 7;
    if (percentage >= 55) return 6;
    if (percentage >= 45) return 5;
    if (percentage >= 35) return 4;
    if (percentage >= 25) return 3;
    if (percentage >= 15) return 2;
    if (percentage >= 5) return 1;
    return 0;
}
