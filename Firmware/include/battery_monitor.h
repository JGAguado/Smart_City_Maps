#ifndef BATTERY_MONITOR_H
#define BATTERY_MONITOR_H

#include <Arduino.h>
#include <Wire.h>
#include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h>

class BatteryMonitor {
public:
    BatteryMonitor();
    ~BatteryMonitor();
    
    bool initialize();
    float getBatteryPercentage();
    float getBatteryVoltage();
    bool isConnected();
    void update();
    
    // Get battery icon index (0-10) based on percentage
    int getBatteryIconIndex();
    
private:
    bool initialized;
    float lastPercentage;
    float lastVoltage;
    unsigned long lastUpdateTime;
    
    static const unsigned long UPDATE_INTERVAL = 10000; // Update every 10 seconds for testing
    
    SFE_MAX1704X lipo{MAX1704X_MAX17048}; // Create a MAX17048 object
};

#endif // BATTERY_MONITOR_H
