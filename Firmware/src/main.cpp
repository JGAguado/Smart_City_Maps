#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <Update.h>
#include <time.h>
#include "serial_config.h"  // Must be included first
#include "config.h"
#include "config_manager.h"
#include "display_handler.h"
#include "web_server.h"
#include "github_fetcher.h"
#include "utils.h"
#include "battery_monitor.h"

// Global objects
ConfigManager configManager;
DisplayHandler display;
WebConfigServer webServer(&configManager);
GitHubImageFetcher imageFetcher(&configManager);
BatteryMonitor batteryMonitor;

// State variables
bool isConfigMode = false;
unsigned long lastUpdateTime = 0;
unsigned long lastWiFiCheck = 0;
bool firstRun = true;

// Function declarations
void setup();
void loop();
bool connectToWiFi();
void enterConfigMode();
void exitConfigMode();
void updateDashboard();
void checkWiFiConnection();
void printSystemInfo();
void enterDeepSleep();
bool isActiveHours();
void setupTimeSync();

void setup() {
    initSerial();  // Initialize USB CDC serial first
    delay(2000);   // Extra time for USB enumeration
    
    // Send test message immediately
    Serial.println("");
    Serial.println("=== ESP32-S2 STARTING ===");
    Serial.flush();
    delay(100);
    
    Serial.println("\n" + repeat("=", 50));
    Serial.println("ESP32-S2 Smart Dashboard Starting...");
    Serial.println("Version: 1.0.0");
    Serial.println("Display: 7.3\" 7-color E-Paper (800x480)");
    Serial.println(repeat("=", 50));
    Serial.flush();
    
    // Initialize EEPROM and configuration
    Serial.println("Initializing configuration manager...");
    if (!configManager.init()) {
        Serial.println("Failed to initialize configuration manager");
    }
    
    // Initialize display
    Serial.println("Initializing display...");
    if (!display.initialize()) {
        Serial.println("WARNING: Display initialization failed!");
        Serial.println("Continuing without display...");
    }
    
    // Initialize battery monitor
    Serial.println("Initializing battery monitor...");
    if (!batteryMonitor.initialize()) {
        Serial.println("WARNING: Battery monitor initialization failed!");
        Serial.println("Continuing without battery monitoring...");
    } else {
        Serial.println("Battery monitor initialized successfully!");
    }
    
    // Check if device is configured
    if (!configManager.isConfigured()) {
        Serial.println("No saved configuration found");
        
        // Try to load default configuration
        Serial.println("Attempting to load default configuration...");
        if (configManager.loadDefaultConfig()) {
            Serial.println("Default configuration loaded successfully!");
            // Continue to WiFi connection attempt below
        } else {
            Serial.println("No default configuration available - entering configuration mode");
            display.showStatus("Configuration Mode");  // This will show the QR code
            enterConfigMode();
            printSystemInfo();
            Serial.println("Setup complete - in configuration mode!");
            return; // Exit setup early when in config mode
        }
    } else {
        Serial.println("Saved configuration found");
        configManager.printConfig();
    }
    
    // At this point we have configuration (either saved or default)
    Serial.println("Configuration available - attempting to connect to WiFi");
    
    if (connectToWiFi()) {
        Serial.println("Connected to WiFi - starting normal operation");
        
        // Setup time synchronization for deep sleep scheduling
        setupTimeSync();
        
        // Don't display anything - wait for image fetch
        lastUpdateTime = 0; // Force immediate update
    } else {
        Serial.println("Failed to connect to WiFi - entering configuration mode");
        // Only show display for configuration mode
        display.showStatus("Configuration Mode");
        enterConfigMode();
    }
    
    // printSystemInfo();
    Serial.println("Setup complete!");
}

void loop() {
    if (isConfigMode) {
        // Handle configuration mode
        webServer.handleClient();
        delay(100);
    } else {
        // Normal operation mode - check for immediate update on startup
        checkWiFiConnection();
        
        // Check if it's time to update the dashboard
        unsigned long currentTime = millis();
        if (firstRun || (currentTime - lastUpdateTime >= UPDATE_INTERVAL_MS)) {
            updateDashboard();
            // updateDashboard() will call enterDeepSleep(), so we shouldn't reach this point
            // If we do reach here, something went wrong - wait and try again
            lastUpdateTime = currentTime;
            firstRun = false;
        }
        
        // Update battery monitor
        batteryMonitor.update();
        
        // Short delay to prevent excessive CPU usage
        delay(1000);  // Increased delay since we're not sleeping
    }
}

bool connectToWiFi() {
    if (!configManager.isConfigured()) {
        Serial.println("Cannot connect to WiFi: no configuration");
        return false;
    }
    
    const char* ssid = configManager.getWiFiSSID();
    const char* password = configManager.getWiFiPassword();
    
    Serial.printf("Connecting to WiFi: %s\n", ssid);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    const int maxAttempts = 30; // 30 seconds timeout
    
    while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
        delay(1000);
        attempts++;
        Serial.printf("WiFi connection attempt %d/%d\n", attempts, maxAttempts);
        
        // Don't show progress on display - keep it blank
        if (attempts % 5 == 0) {
            // Just log progress, don't display anything
            Serial.printf("WiFi connection progress: %d/%d attempts\n", attempts, maxAttempts);
        }
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi connected successfully!");
        Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("RSSI: %d dBm\n", WiFi.RSSI());
        return true;
    } else {
        Serial.println("WiFi connection failed");
        return false;
    }
}

void enterConfigMode() {
    Serial.println("Entering configuration mode...");
    isConfigMode = true;
    
    if (!webServer.startConfigAP()) {
        Serial.println("Failed to start configuration server");
        display.showStatus("Config Server Failed");
        return;
    }
    
    // Show QR code for easy WiFi connection
    display.showStatus("Configuration Mode");
    
    Serial.println("Configuration mode active - waiting for user input");
}

void exitConfigMode() {
    Serial.println("Exiting configuration mode...");
    isConfigMode = false;
    webServer.stopServer();
}

void updateDashboard() {
    Serial.println("\n" + repeat("-", 40));
    Serial.println("Starting dashboard update...");
    
    // Don't display anything during fetch - keep display blank
    
    // Test GitHub connection first
    if (!imageFetcher.testConnection()) {
        Serial.println("GitHub connection test failed");
        // Don't display error - just log it
        return;
    }
    
    // Fetch the latest image
    if (imageFetcher.fetchLatestImage()) {
        Serial.println("Image fetched successfully");
        
        uint8_t* imageData = imageFetcher.getImageBuffer();
        size_t imageSize = imageFetcher.getImageSize();
        
        Serial.printf("Displaying image (%d bytes)\n", imageSize);
        // Display the fetched image with battery overlay
        // display.displayImageWithBatteryOverlay(imageData, imageSize, &batteryMonitor);
        display.displayImage(imageData, imageSize);

        Serial.println("Dashboard update completed successfully");
        
        // After successful display update, enter deep sleep
        delay(1000);  // Allow display to complete
        enterDeepSleep();
    } else {
        Serial.println("Failed to fetch image from GitHub");
        // Don't display error - just log it and keep display blank
    }
    
    Serial.println(repeat("-", 40));
}

void checkWiFiConnection() {
    unsigned long currentTime = millis();
    
    // Check WiFi every 30 seconds
    if (currentTime - lastWiFiCheck >= WIFI_RETRY_DELAY_MS) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi disconnected - attempting to reconnect");
            
            if (!connectToWiFi()) {
                Serial.println("WiFi reconnection failed - entering configuration mode");
                display.showStatus("WiFi Lost - Config Mode");
                delay(3000);
                enterConfigMode();
            }
        }
        lastWiFiCheck = currentTime;
    }
}

void printSystemInfo() {
    Serial.println("\n" + repeat("=", 50));
    Serial.println("SYSTEM INFORMATION");
    Serial.println(repeat("=", 50));
    Serial.flush();
    
    Serial.printf("Chip Model: %s\n", ESP.getChipModel());
    Serial.flush();
    
    Serial.printf("Chip Revision: %d\n", ESP.getChipRevision());
    Serial.flush();
    
    Serial.printf("CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
    Serial.flush();
    
    Serial.printf("Flash Size: %d KB\n", ESP.getFlashChipSize() / 1024);
    Serial.flush();
    
    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.flush();
    
    // Skip PSRAM calls as they might cause issues on some ESP32-S2 boards
    Serial.println("PSRAM: Checking...");
    Serial.flush();
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("WiFi SSID: %s\n", WiFi.SSID().c_str());
        Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("MAC Address: %s\n", WiFi.macAddress().c_str());
        Serial.printf("RSSI: %d dBm\n", WiFi.RSSI());
        Serial.flush();
    }
    
    Serial.printf("Configuration Status: %s\n", configManager.isConfigured() ? "Configured" : "Not Configured");
    Serial.printf("Display Status: Initialized\n");
    Serial.printf("Operating Mode: %s\n", isConfigMode ? "Configuration" : "Normal");
    Serial.flush();
    
    // Battery information
    if (batteryMonitor.isConnected()) {
        Serial.printf("Battery Status: %.1f%% (%.2fV)\n", 
                     batteryMonitor.getBatteryPercentage(), 
                     batteryMonitor.getBatteryVoltage());
    } else {
        Serial.println("Battery Status: Not Connected");
    }
    Serial.flush();
    
    Serial.println(repeat("=", 50));
    Serial.flush();
}

void setupTimeSync() {
    Serial.println("Setting up time synchronization...");
    
    // Configure NTP with UTC+2 timezone (7200 seconds offset, no daylight saving)
    // For automatic daylight saving time, you could use: 
    // configTime(2 * 3600, 3600, "pool.ntp.org", "time.nist.gov");
    configTime(2 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    
    // Wait for time to be set
    struct tm timeinfo;
    int attempts = 0;
    while (!getLocalTime(&timeinfo) && attempts < 10) {
        Serial.print(".");
        delay(1000);
        attempts++;
    }
    
    if (attempts < 10) {
        Serial.println("\nTime synchronized successfully (UTC+2)");
        Serial.printf("Local time: %04d-%02d-%02d %02d:%02d:%02d\n",
                     timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                     timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    } else {
        Serial.println("\nFailed to synchronize time - using default schedule");
    }
}

bool isActiveHours() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to get current time - assuming active hours");
        return true; // Default to active hours if time is unavailable
    }
    
    int hour = timeinfo.tm_hour;
    Serial.printf("Current hour: %d\n", hour);
    
    // Active hours: 7:00 - 22:00 (7 AM to 10 PM)
    return (hour >= 7 && hour < 22);
}

void enterDeepSleep() {
    Serial.println("\n" + repeat("=", 50));
    Serial.println("PREPARING FOR DEEP SLEEP");
    
    // Check current time and determine sleep duration
    bool activeHours = isActiveHours();
    unsigned long sleepTimeMs;
    
    if (activeHours) {
        // Active hours (7:00-22:00): sleep for 30 minutes
        sleepTimeMs = 30UL * 60UL * 1000UL * 1000UL; // 30 minutes in microseconds
        Serial.println("Active hours detected - sleeping for 30 minutes");
    } else {
        // Inactive hours (22:00-7:00): sleep for 9 hours
        sleepTimeMs = 9UL * 60UL * 60UL * 1000UL * 1000UL; // 9 hours in microseconds
        Serial.println("Inactive hours detected - sleeping for 9 hours");
    }
    
    // Put display to sleep
    display.sleep();
    Serial.println("Display put to sleep");
    
    // Disconnect WiFi to save power
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.println("WiFi disconnected");
    
    // Print sleep information
    Serial.printf("Sleep duration: %lu seconds\n", sleepTimeMs / 1000000);
    
    // Battery status before sleep
    if (batteryMonitor.isConnected()) {
        Serial.printf("Battery before sleep: %.1f%% (%.2fV)\n", 
                     batteryMonitor.getBatteryPercentage(), 
                     batteryMonitor.getBatteryVoltage());
    }
    
    Serial.println("Entering deep sleep...");
    Serial.println(repeat("=", 50));
    Serial.flush();
    
    // Configure ESP32-S2 for deep sleep with timer wakeup
    esp_sleep_enable_timer_wakeup(sleepTimeMs);
    
    // Enter deep sleep
    esp_deep_sleep_start();
}