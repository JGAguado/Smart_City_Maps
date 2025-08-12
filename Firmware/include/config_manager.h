#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include "config.h"

struct DashboardConfig {
    char wifiSSID[MAX_SSID_LENGTH + 1];
    char wifiPassword[MAX_PASSWORD_LENGTH + 1];
    char githubRepo[MAX_REPO_LENGTH + 1];        // e.g., "JGAguado/Smart_City_Maps"
    char githubImagePath[MAX_PATH_LENGTH + 1];   // e.g., "dashboard_480x800.png"
    bool isConfigured;
    uint16_t magicNumber;
};

class ConfigManager {
private:
    DashboardConfig config;
    bool configLoaded;
    
public:
    ConfigManager();
    bool init();
    bool loadConfig();
    bool saveConfig();
    void clearConfig();
    bool loadDefaultConfig();  // New method to load default configuration
    
    // Getters
    const char* getWiFiSSID() const { return config.wifiSSID; }
    const char* getWiFiPassword() const { return config.wifiPassword; }
    const char* getGitHubRepo() const { return config.githubRepo; }
    const char* getGitHubImagePath() const { return config.githubImagePath; }
    bool isConfigured() const { return config.isConfigured && configLoaded; }
    
    // Setters
    bool setWiFiCredentials(const char* ssid, const char* password);
    bool setGitHubInfo(const char* repo, const char* imagePath);
    void setConfigured(bool configured);
    
    // Validation
    bool validateConfig() const;
    String getConfigJson() const;
    bool setConfigFromJson(const String& jsonStr);
    
    // Debugging
    void printConfig() const;
};

#endif // CONFIG_MANAGER_H
