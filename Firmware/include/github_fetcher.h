#ifndef GITHUB_FETCHER_H
#define GITHUB_FETCHER_H

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "config_manager.h"

class GitHubImageFetcher {
private:
    ConfigManager* configManager;
    WiFiClientSecure client;
    HTTPClient http;
    uint8_t* imageBuffer;
    size_t bufferSize;
    bool bufferAllocated;
    
    String buildImageURL();
    bool downloadImage(const String& url, uint8_t*& buffer, size_t& size);
    void freeBuffer();
    
public:
    GitHubImageFetcher(ConfigManager* configMgr);
    ~GitHubImageFetcher();
    
    bool fetchLatestImage();
    uint8_t* getImageBuffer() const { return imageBuffer; }
    size_t getImageSize() const { return bufferSize; }
    bool hasImage() const { return bufferAllocated && imageBuffer != nullptr; }
    
    // Testing and debugging
    bool testConnection();
    String getLastError() const;
};

#endif // GITHUB_FETCHER_H
