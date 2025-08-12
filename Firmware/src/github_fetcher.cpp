#include "github_fetcher.h"
#include "config.h"
#include "serial_config.h"  // Must be included before Arduino.h
#include <Arduino.h>

GitHubImageFetcher::GitHubImageFetcher(ConfigManager* configMgr) : 
    configManager(configMgr), imageBuffer(nullptr), bufferSize(0), bufferAllocated(false) {
    
    // Configure SSL client to skip certificate verification for GitHub
    client.setInsecure();
}

GitHubImageFetcher::~GitHubImageFetcher() {
    freeBuffer();
}

bool GitHubImageFetcher::fetchLatestImage() {
    if (!configManager || !configManager->isConfigured()) {
        Serial.println("Cannot fetch image: configuration not available");
        return false;
    }
    
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Cannot fetch image: WiFi not connected");
        return false;
    }
    
    String imageURL = buildImageURL();
    if (imageURL.length() == 0) {
        Serial.println("Cannot fetch image: invalid URL");
        return false;
    }
    
    Serial.printf("Fetching image from: %s\n", imageURL.c_str());
    
    // Free previous buffer if exists
    freeBuffer();
    
    return downloadImage(imageURL, imageBuffer, bufferSize);
}

String GitHubImageFetcher::buildImageURL() {
    if (!configManager || !configManager->isConfigured()) {
        return "";
    }
    
    // GitHub raw URL format: https://raw.githubusercontent.com/owner/repo/branch/path
    String url = "https://raw.githubusercontent.com/";
    url += configManager->getGitHubRepo();
    url += "/main/";  // Assuming main branch
    
    // Convert PNG path to binary e-paper format path
    String imagePath = configManager->getGitHubImagePath();
    if (imagePath.endsWith(".png")) {
        imagePath = imagePath.substring(0, imagePath.length() - 4) + ".bin";
    } else if (!imagePath.endsWith(".bin")) {
        imagePath += ".bin";
    }
    
    url += imagePath;
    
    return url;
}

bool GitHubImageFetcher::downloadImage(const String& url, uint8_t*& buffer, size_t& size) {
    HTTPClient http;
    http.begin(client, url);
    
    // Set timeout
    http.setTimeout(30000); // 30 seconds
    
    // Add headers for binary content
    http.addHeader("User-Agent", "ESP32-SmartDashboard/1.0");
    http.addHeader("Accept", "application/octet-stream");
    
    Serial.println("Starting HTTP GET request for binary e-paper data...");
    int httpCode = http.GET();
    
    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("HTTP GET failed with code: %d\n", httpCode);
        if (httpCode > 0) {
            String payload = http.getString();
            Serial.printf("Error response: %s\n", payload.c_str());
        }
        http.end();
        return false;
    }
    
    size = http.getSize();
    Serial.printf("Binary e-paper data size: %d bytes\n", size);
    
    if (size <= 0 || size > MAX_IMAGE_SIZE) {
        Serial.printf("Invalid binary data size: %d bytes (max: %d)\n", size, MAX_IMAGE_SIZE);
        http.end();
        return false;
    }
    
    // Allocate buffer for binary e-paper data
    buffer = (uint8_t*)malloc(size);
    if (!buffer) {
        Serial.printf("Failed to allocate %d bytes for e-paper buffer\n", size);
        http.end();
        return false;
    }
    
    // Read binary e-paper data
    WiFiClient* stream = http.getStreamPtr();
    size_t bytesRead = 0;
    size_t totalRead = 0;
    unsigned long timeout = millis();
    
    Serial.println("Downloading binary e-paper data...");
    
    while (totalRead < size && (millis() - timeout) < 30000) {
        if (stream->available()) {
            bytesRead = stream->readBytes(buffer + totalRead, size - totalRead);
            totalRead += bytesRead;
            timeout = millis(); // Reset timeout on successful read
            
            if (totalRead % 10000 == 0 || totalRead == size) {
                Serial.printf("Downloaded: %d/%d bytes (%.1f%%)\n", 
                             totalRead, size, (float)totalRead * 100.0 / size);
            }
        }
        delay(1);
    }
    
    http.end();
    
    if (totalRead != size) {
        Serial.printf("Download incomplete: %d/%d bytes\n", totalRead, size);
        free(buffer);
        buffer = nullptr;
        size = 0;
        return false;
    }
    
    bufferAllocated = true;
    bufferSize = size;
    Serial.println("Binary image downloaded successfully!");
    
    return true;
}

void GitHubImageFetcher::freeBuffer() {
    if (bufferAllocated && imageBuffer) {
        free(imageBuffer);
        imageBuffer = nullptr;
        bufferSize = 0;
        bufferAllocated = false;
    }
}

bool GitHubImageFetcher::testConnection() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected");
        return false;
    }
    
    HTTPClient http;
    http.begin(client, "https://api.github.com");
    http.setTimeout(10000);
    
    int httpCode = http.GET();
    http.end();
    
    if (httpCode == HTTP_CODE_OK) {
        Serial.println("GitHub connection test successful");
        return true;
    } else {
        Serial.printf("GitHub connection test failed with code: %d\n", httpCode);
        return false;
    }
}

String GitHubImageFetcher::getLastError() const {
    // For now, return a generic message
    // In a more complete implementation, this would track the last error
    return "Check serial output for detailed error information";
}
