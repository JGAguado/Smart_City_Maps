#include "qr_code.h"
#include <string.h>

QRCode::QRCode() {
}

QRCode::~QRCode() {
}

void QRCode::generateWiFiQR(const char* ssid, const char* password, uint8_t* qrData, int size) {
    // Create WiFi QR code content
    char wifiString[256];
    snprintf(wifiString, sizeof(wifiString), "WIFI:T:WPA;S:%s;P:%s;H:false;;", ssid, password);
    
    generateSimpleQR(wifiString, qrData, size);
}

void QRCode::generateUrlQR(const char* url, uint8_t* qrData, int size) {
    generateSimpleQR(url, qrData, size);
}

void QRCode::generateSimpleQR(const char* text, uint8_t* qrData, int size) {
    // Clear the QR code data
    memset(qrData, 0, size * size);
    
    // For simplicity, create a basic pattern that looks like a QR code
    // This is a simplified version - a full QR implementation would be much more complex
    
    // Add finder patterns (corners)
    createFinderPattern(qrData, size, 0, 0);           // Top-left
    createFinderPattern(qrData, size, size-7, 0);      // Top-right
    createFinderPattern(qrData, size, 0, size-7);      // Bottom-left
    
    // Add timing patterns
    createTimingPattern(qrData, size);
    
    // Add some data pattern based on text hash
    unsigned int hash = 0;
    for (int i = 0; text[i] != 0; i++) {
        hash = hash * 31 + text[i];
    }
    
    // Fill center area with pseudo-random pattern based on text
    for (int y = 9; y < size - 9; y++) {
        for (int x = 9; x < size - 9; x++) {
            // Skip timing patterns
            if (x == 6 || y == 6) continue;
            
            // Create pattern based on position and text hash
            unsigned int pattern = (x * 7 + y * 11 + hash) % 3;
            if (pattern == 0) {
                qrData[y * size + x] = 1;
            }
        }
    }
    
    // Add quiet zone
    addQuietZone(qrData, size);
}

void QRCode::createFinderPattern(uint8_t* data, int size, int x, int y) {
    // 7x7 finder pattern
    for (int dy = 0; dy < 7; dy++) {
        for (int dx = 0; dx < 7; dx++) {
            if (x + dx >= size || y + dy >= size) continue;
            
            // Outer border (7x7)
            if (dx == 0 || dx == 6 || dy == 0 || dy == 6) {
                data[(y + dy) * size + (x + dx)] = 1;
            }
            // Inner square (3x3 in center)
            else if (dx >= 2 && dx <= 4 && dy >= 2 && dy <= 4) {
                data[(y + dy) * size + (x + dx)] = 1;
            }
            // White area between outer and inner
            else {
                data[(y + dy) * size + (x + dx)] = 0;
            }
        }
    }
}

void QRCode::createTimingPattern(uint8_t* data, int size) {
    // Horizontal timing pattern (row 6)
    for (int x = 8; x < size - 8; x++) {
        if (6 < size) {
            data[6 * size + x] = (x % 2 == 0) ? 1 : 0;
        }
    }
    
    // Vertical timing pattern (column 6)
    for (int y = 8; y < size - 8; y++) {
        if (6 < size) {
            data[y * size + 6] = (y % 2 == 0) ? 1 : 0;
        }
    }
}

void QRCode::addQuietZone(uint8_t* data, int size) {
    // Add 4-module quiet zone around the QR code
    // For simplicity, we just ensure the border is clear
    for (int i = 0; i < size; i++) {
        data[0 * size + i] = 0;  // Top border
        data[(size-1) * size + i] = 0;  // Bottom border
        data[i * size + 0] = 0;  // Left border
        data[i * size + (size-1)] = 0;  // Right border
    }
}

void QRCode::convertToEPaperFormat(const uint8_t* qrData, int qrSize, 
                                  uint8_t* epaperData, int centerX, int centerY, int scale) {
    // Clear the e-paper data area first
    for (int y = centerY - (qrSize * scale) / 2; y < centerY + (qrSize * scale) / 2; y++) {
        for (int x = centerX - (qrSize * scale) / 2; x < centerX + (qrSize * scale) / 2; x++) {
            if (x >= 0 && x < 800 && y >= 0 && y < 480) {
                // Calculate position in e-paper buffer (2 pixels per byte)
                int bufferIndex = (y * 400) + (x / 2);  // 800 pixels / 2 = 400 bytes per row
                
                if (x % 2 == 0) {
                    // Left pixel (upper 4 bits)
                    epaperData[bufferIndex] = (epaperData[bufferIndex] & 0x0F) | (0x10);  // White
                } else {
                    // Right pixel (lower 4 bits)
                    epaperData[bufferIndex] = (epaperData[bufferIndex] & 0xF0) | 0x01;   // White
                }
            }
        }
    }
    
    // Draw the QR code
    for (int qrY = 0; qrY < qrSize; qrY++) {
        for (int qrX = 0; qrX < qrSize; qrX++) {
            if (qrData[qrY * qrSize + qrX] == 1) {  // Black module
                // Scale up the QR code
                for (int sy = 0; sy < scale; sy++) {
                    for (int sx = 0; sx < scale; sx++) {
                        int displayX = centerX - (qrSize * scale) / 2 + qrX * scale + sx;
                        int displayY = centerY - (qrSize * scale) / 2 + qrY * scale + sy;
                        
                        if (displayX >= 0 && displayX < 800 && displayY >= 0 && displayY < 480) {
                            // Calculate position in e-paper buffer
                            int bufferIndex = (displayY * 400) + (displayX / 2);
                            
                            if (displayX % 2 == 0) {
                                // Left pixel (upper 4 bits)
                                epaperData[bufferIndex] = (epaperData[bufferIndex] & 0x0F) | (0x00);  // Black
                            } else {
                                // Right pixel (lower 4 bits)
                                epaperData[bufferIndex] = (epaperData[bufferIndex] & 0xF0) | 0x00;   // Black
                            }
                        }
                    }
                }
            }
        }
    }
}
