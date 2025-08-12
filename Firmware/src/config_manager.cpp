#include "config_manager.h"
#include "config.h"
#include "serial_config.h"  // Must be included before Arduino.h
#include <Arduino.h>

ConfigManager::ConfigManager() : configLoaded(false) {
    memset(&config, 0, sizeof(config));
}

bool ConfigManager::init() {
    if (!EEPROM.begin(EEPROM_SIZE)) {
        Serial.println("Failed to initialize EEPROM");
        return false;
    }
    
    Serial.println("EEPROM initialized successfully");
    return loadConfig();
}

bool ConfigManager::loadConfig() {
    Serial.println("Loading configuration from EEPROM...");
    
    // Read magic number first to check if config exists
    uint16_t magicNumber;
    EEPROM.get(EEPROM_CONFIG_FLAG_ADDR, magicNumber);
    
    if (magicNumber != CONFIG_MAGIC_NUMBER) {
        Serial.println("No valid configuration found in EEPROM");
        config.isConfigured = false;
        config.magicNumber = 0;
        configLoaded = true;
        return false;
    }
    
    // Read WiFi credentials
    for (int i = 0; i < MAX_SSID_LENGTH; i++) {
        config.wifiSSID[i] = EEPROM.read(EEPROM_WIFI_SSID_ADDR + i);
    }
    config.wifiSSID[MAX_SSID_LENGTH] = '\0';
    
    for (int i = 0; i < MAX_PASSWORD_LENGTH; i++) {
        config.wifiPassword[i] = EEPROM.read(EEPROM_WIFI_PASS_ADDR + i);
    }
    config.wifiPassword[MAX_PASSWORD_LENGTH] = '\0';
    
    // Read GitHub info
    for (int i = 0; i < MAX_REPO_LENGTH; i++) {
        config.githubRepo[i] = EEPROM.read(EEPROM_GITHUB_REPO_ADDR + i);
    }
    config.githubRepo[MAX_REPO_LENGTH] = '\0';
    
    for (int i = 0; i < MAX_PATH_LENGTH; i++) {
        config.githubImagePath[i] = EEPROM.read(EEPROM_GITHUB_PATH_ADDR + i);
    }
    config.githubImagePath[MAX_PATH_LENGTH] = '\0';
    
    config.magicNumber = magicNumber;
    config.isConfigured = validateConfig();
    configLoaded = true;
    
    if (config.isConfigured) {
        Serial.println("Configuration loaded successfully!");
        printConfig();
        return true;
    } else {
        Serial.println("Loaded configuration is invalid");
        return false;
    }
}

bool ConfigManager::saveConfig() {
    Serial.println("Saving configuration to EEPROM...");
    
    if (!validateConfig()) {
        Serial.println("Cannot save invalid configuration");
        return false;
    }
    
    // Write WiFi credentials
    for (int i = 0; i < MAX_SSID_LENGTH; i++) {
        EEPROM.write(EEPROM_WIFI_SSID_ADDR + i, 
                    i < strlen(config.wifiSSID) ? config.wifiSSID[i] : 0);
    }
    
    for (int i = 0; i < MAX_PASSWORD_LENGTH; i++) {
        EEPROM.write(EEPROM_WIFI_PASS_ADDR + i, 
                    i < strlen(config.wifiPassword) ? config.wifiPassword[i] : 0);
    }
    
    // Write GitHub info
    for (int i = 0; i < MAX_REPO_LENGTH; i++) {
        EEPROM.write(EEPROM_GITHUB_REPO_ADDR + i, 
                    i < strlen(config.githubRepo) ? config.githubRepo[i] : 0);
    }
    
    for (int i = 0; i < MAX_PATH_LENGTH; i++) {
        EEPROM.write(EEPROM_GITHUB_PATH_ADDR + i, 
                    i < strlen(config.githubImagePath) ? config.githubImagePath[i] : 0);
    }
    
    // Write magic number last to indicate valid config
    config.magicNumber = CONFIG_MAGIC_NUMBER;
    EEPROM.put(EEPROM_CONFIG_FLAG_ADDR, config.magicNumber);
    
    // Commit changes to EEPROM
    if (EEPROM.commit()) {
        config.isConfigured = true;
        Serial.println("Configuration saved successfully!");
        return true;
    } else {
        Serial.println("Failed to commit configuration to EEPROM");
        return false;
    }
}

void ConfigManager::clearConfig() {
    Serial.println("Clearing configuration...");
    
    // Clear all EEPROM data
    for (int i = 0; i < EEPROM_SIZE; i++) {
        EEPROM.write(i, 0);
    }
    
    EEPROM.commit();
    
    // Reset config structure
    memset(&config, 0, sizeof(config));
    config.isConfigured = false;
    configLoaded = true;
    
    Serial.println("Configuration cleared");
}

bool ConfigManager::loadDefaultConfig() {
    Serial.println("Loading default configuration...");
    
    // Check if we have default WiFi settings
    if (!HAS_DEFAULT_WIFI) {
        Serial.println("No default WiFi configuration available");
        if (SHOW_DEFAULT_CONFIG) {
            Serial.println("To use default config, edit default_config.h and set DEFAULT_WIFI_SSID and DEFAULT_WIFI_PASSWORD");
        }
        return false;
    }
    
    // Check if we have default GitHub settings
    if (!HAS_DEFAULT_GITHUB) {
        Serial.println("No default GitHub configuration available");
        if (SHOW_DEFAULT_CONFIG) {
            Serial.println("To use default config, edit default_config.h and set DEFAULT_GITHUB_REPO and DEFAULT_GITHUB_PATH");
        }
        return false;
    }
    
    // Load default WiFi credentials
    if (!setWiFiCredentials(DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASSWORD)) {
        Serial.println("Failed to set default WiFi credentials");
        return false;
    }
    
    // Load default GitHub information
    if (!setGitHubInfo(DEFAULT_GITHUB_REPO, DEFAULT_GITHUB_PATH)) {
        Serial.println("Failed to set default GitHub information");
        return false;
    }
    
    // Validate the configuration
    if (!validateConfig()) {
        Serial.println("Default configuration is invalid");
        return false;
    }
    
    config.isConfigured = true;
    configLoaded = true;
    
    if (SHOW_DEFAULT_CONFIG) {
        Serial.println("Default configuration loaded successfully!");
        printConfig();
    }
    
    // Optionally save the default config to EEPROM so it persists
    if (FORCE_DEFAULT_CONFIG) {
        Serial.println("Force default config enabled - saving to EEPROM");
        return saveConfig();
    }
    
    return true;
}

bool ConfigManager::setWiFiCredentials(const char* ssid, const char* password) {
    if (!ssid || !password || strlen(ssid) == 0 || strlen(ssid) > MAX_SSID_LENGTH || 
        strlen(password) > MAX_PASSWORD_LENGTH) {
        Serial.println("Invalid WiFi credentials");
        return false;
    }
    
    strncpy(config.wifiSSID, ssid, MAX_SSID_LENGTH);
    config.wifiSSID[MAX_SSID_LENGTH] = '\0';
    
    strncpy(config.wifiPassword, password, MAX_PASSWORD_LENGTH);
    config.wifiPassword[MAX_PASSWORD_LENGTH] = '\0';
    
    Serial.printf("WiFi credentials set: SSID=%s\n", config.wifiSSID);
    return true;
}

bool ConfigManager::setGitHubInfo(const char* repo, const char* imagePath) {
    if (!repo || !imagePath || strlen(repo) == 0 || strlen(imagePath) == 0 ||
        strlen(repo) > MAX_REPO_LENGTH || strlen(imagePath) > MAX_PATH_LENGTH) {
        Serial.println("Invalid GitHub information");
        return false;
    }
    
    strncpy(config.githubRepo, repo, MAX_REPO_LENGTH);
    config.githubRepo[MAX_REPO_LENGTH] = '\0';
    
    strncpy(config.githubImagePath, imagePath, MAX_PATH_LENGTH);
    config.githubImagePath[MAX_PATH_LENGTH] = '\0';
    
    Serial.printf("GitHub info set: Repo=%s, Path=%s\n", config.githubRepo, config.githubImagePath);
    return true;
}

void ConfigManager::setConfigured(bool configured) {
    config.isConfigured = configured;
}

bool ConfigManager::validateConfig() const {
    if (strlen(config.wifiSSID) == 0) {
        Serial.println("Validation failed: WiFi SSID is empty");
        return false;
    }
    
    if (strlen(config.githubRepo) == 0) {
        Serial.println("Validation failed: GitHub repo is empty");
        return false;
    }
    
    if (strlen(config.githubImagePath) == 0) {
        Serial.println("Validation failed: GitHub image path is empty");
        return false;
    }
    
    // Check for valid repo format (owner/repo)
    if (!strchr(config.githubRepo, '/')) {
        Serial.println("Validation failed: GitHub repo must be in format 'owner/repo'");
        return false;
    }
    
    return true;
}

String ConfigManager::getConfigJson() const {
    JsonDocument doc;
    
    doc["wifiSSID"] = config.wifiSSID;
    doc["wifiPassword"] = config.wifiPassword;
    doc["githubRepo"] = config.githubRepo;
    doc["githubImagePath"] = config.githubImagePath;
    doc["isConfigured"] = config.isConfigured;
    
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

bool ConfigManager::setConfigFromJson(const String& jsonStr) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonStr);
    
    if (error) {
        Serial.printf("JSON parsing failed: %s\n", error.c_str());
        return false;
    }
    
    if (!doc["wifiSSID"].is<const char*>() || !doc["githubRepo"].is<const char*>() || 
        !doc["githubImagePath"].is<const char*>()) {
        Serial.println("JSON missing required fields");
        return false;
    }
    
    const char* ssid = doc["wifiSSID"];
    const char* password = doc["wifiPassword"] | "";
    const char* repo = doc["githubRepo"];
    const char* imagePath = doc["githubImagePath"];
    
    if (!setWiFiCredentials(ssid, password) || !setGitHubInfo(repo, imagePath)) {
        return false;
    }
    
    return validateConfig();
}

void ConfigManager::printConfig() const {
    Serial.println("=== Current Configuration ===");
    Serial.printf("WiFi SSID: %s\n", config.wifiSSID);
    Serial.printf("WiFi Password: %s\n", strlen(config.wifiPassword) > 0 ? "***set***" : "***empty***");
    Serial.printf("GitHub Repo: %s\n", config.githubRepo);
    Serial.printf("GitHub Image Path: %s\n", config.githubImagePath);
    Serial.printf("Is Configured: %s\n", config.isConfigured ? "Yes" : "No");
    Serial.printf("Magic Number: 0x%04X\n", config.magicNumber);
    Serial.println("=============================");
}
