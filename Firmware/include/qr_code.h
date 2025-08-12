#ifndef QR_CODE_H
#define QR_CODE_H

#include <Arduino.h>

class QRCode {
public:
    QRCode();
    ~QRCode();
    
    // Generate QR code for WiFi network connection
    // Format: WIFI:T:WPA;S:SSID;P:PASSWORD;H:false;;
    static void generateWiFiQR(const char* ssid, const char* password, uint8_t* qrData, int size);
    
    // Generate QR code for URL
    static void generateUrlQR(const char* url, uint8_t* qrData, int size);
    
    // Simple QR code generation for basic text
    static void generateSimpleQR(const char* text, uint8_t* qrData, int size);
    
    // Convert QR data to e-paper display format
    static void convertToEPaperFormat(const uint8_t* qrData, int qrSize, 
                                     uint8_t* epaperData, int centerX, int centerY, int scale);

private:
    // Simple pattern generation for basic QR codes
    static void createFinderPattern(uint8_t* data, int size, int x, int y);
    static void createTimingPattern(uint8_t* data, int size);
    static void addQuietZone(uint8_t* data, int size);
};

#endif // QR_CODE_H
