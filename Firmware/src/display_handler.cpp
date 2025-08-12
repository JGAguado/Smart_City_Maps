#include "display_handler.h"
#include "battery_monitor.h"
#include "serial_config.h"  // Must be included before Arduino.h
#include <Arduino.h>

DisplayHandler::DisplayHandler() : initialized(false) {
}

DisplayHandler::~DisplayHandler() {
    if (initialized) {
        sleep();
    }
}

bool DisplayHandler::initialize() {
    Serial.println("Initializing e-paper display...");
    
    if (epd.init() != 0) {
        Serial.println("E-paper initialization failed");
        return false;
    }
    
    initialized = true;
    Serial.println("E-paper display initialized successfully");
    
    // Don't clear or display anything - keep display blank until image is fetched
    
    return true;
}

void DisplayHandler::clear() {
    if (!initialized) return;
    
    Serial.println("Clearing display...");
    epd.clear(EPD_7IN3F_WHITE);
}

void DisplayHandler::showStatus(const char* message) {
    if (!initialized) return;
    
    Serial.printf("Showing status: %s\n", message);
    
    // If this is configuration mode, show QR code
    if (strcmp(message, "Configuration Mode") == 0) {
        showConfigurationQR();
    } else {
        // For other statuses, show color blocks
        showColorTest();
    }
}

void DisplayHandler::showConfigurationQR() {
    if (!initialized) return;
    
    Serial.println("Displaying configuration QR code...");
    
    // Allocate buffer for e-paper data (800x480, 2 pixels per byte)
    size_t bufferSize = (DISPLAY_WIDTH * DISPLAY_HEIGHT) / 2;
    uint8_t* epaperBuffer = (uint8_t*)malloc(bufferSize);
    
    if (!epaperBuffer) {
        Serial.println("Failed to allocate buffer for QR display");
        showColorTest();
        return;
    }
    
    // Clear buffer with white background
    for (size_t i = 0; i < bufferSize; i++) {
        epaperBuffer[i] = 0x11;  // White pixels (2 pixels per byte)
    }
    
    // Generate QR code for WiFi connection
    const int qrSize = 41;  // 41x41 QR code
    uint8_t* qrData = (uint8_t*)malloc(qrSize * qrSize);
    
    if (qrData) {
        // Generate WiFi QR code
        QRCode::generateWiFiQR(AP_SSID, AP_PASSWORD, qrData, qrSize);
        
        // Convert and draw QR code on display (centered, scaled 8x)
        QRCode::convertToEPaperFormat(qrData, qrSize, epaperBuffer, 
                                     DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 - 50, 8);
        
        free(qrData);
    }
    
    // Add text instructions around the QR code
    drawText(epaperBuffer, "Smart Dashboard Setup", 200, 50, 2);
    drawText(epaperBuffer, "1. Scan QR code to connect to WiFi", 150, 380, 1);
    drawText(epaperBuffer, "2. Open browser to 192.168.4.1", 180, 410, 1);
    drawText(epaperBuffer, "3. Configure your settings", 220, 440, 1);
    
    // Display the buffer
    epd.display(epaperBuffer);
    
    free(epaperBuffer);
    Serial.println("Configuration QR code displayed");
}

void DisplayHandler::showSimpleMessage(const char* message) {
    if (!initialized) return;
    
    Serial.printf("Showing simple message: %s\n", message);
    
    // Allocate buffer for e-paper data (800x480, 2 pixels per byte)
    size_t bufferSize = (DISPLAY_WIDTH * DISPLAY_HEIGHT) / 2;
    uint8_t* epaperBuffer = (uint8_t*)malloc(bufferSize);
    
    if (!epaperBuffer) {
        Serial.println("Failed to allocate buffer for message display");
        return;
    }
    
    // Clear buffer with white background
    for (size_t i = 0; i < bufferSize; i++) {
        epaperBuffer[i] = 0x11;  // White pixels (2 pixels per byte)
    }
    
    // Calculate text position to center it
    int textLen = strlen(message);
    int textWidth = textLen * 6 * 2;  // 6 pixels per char * scale 2
    int x = (DISPLAY_WIDTH - textWidth) / 2;
    int y = DISPLAY_HEIGHT / 2 - 7;  // 7 pixels high text
    
    // Draw the message
    drawText(epaperBuffer, message, x, y, 2);
    
    // Display the buffer
    epd.display(epaperBuffer);
    
    free(epaperBuffer);
}

void DisplayHandler::showColorTest() {
    if (!initialized) return;
    
    Serial.println("Showing color test pattern...");
    epd.showColorBlocks();
}

void DisplayHandler::displayImage(const uint8_t* imageData, size_t dataSize) {
    if (!initialized) return;
    
    Serial.printf("Displaying image (%d bytes)...\n", dataSize);
    
    // Calculate expected size for 800x480 display
    // Each pixel uses 4 bits (2 pixels per byte)
    size_t expectedSize = (DISPLAY_WIDTH * DISPLAY_HEIGHT) / 2;
    
    if (dataSize < expectedSize) {
        Serial.printf("Warning: Image data too small (%d < %d)\n", dataSize, expectedSize);
        showStatus("Image Error: Size Mismatch");
        return;
    }
    
    // If the image data is already in the correct format, display it directly
    epd.display(imageData);
    
    Serial.println("Image displayed successfully");
}

void DisplayHandler::displayImageWithBatteryOverlay(const uint8_t* imageData, size_t dataSize, BatteryMonitor* batteryMonitor) {
    if (!initialized || !batteryMonitor) return;
    
    Serial.printf("Displaying image with battery overlay (%d bytes)...\n", dataSize);
    
    // Calculate expected size for 800x480 display
    size_t expectedSize = (DISPLAY_WIDTH * DISPLAY_HEIGHT) / 2;
    
    if (dataSize < expectedSize) {
        Serial.printf("Warning: Image data too small (%d < %d)\n", dataSize, expectedSize);
        showStatus("Image Error: Size Mismatch");
        return;
    }
    
    // Create a copy of the image data to modify
    uint8_t* modifiedImage = (uint8_t*)ps_malloc(expectedSize);
    if (!modifiedImage) {
        modifiedImage = (uint8_t*)malloc(expectedSize);
        if (!modifiedImage) {
            Serial.println("Failed to allocate memory for image modification");
            return;
        }
    }
    
    // Copy original image data
    memcpy(modifiedImage, imageData, expectedSize);
    
    // Get battery percentage and draw overlay
    int batteryPercentage = (int)batteryMonitor->getBatteryPercentage();
    drawBatteryOverlay(modifiedImage, batteryPercentage);
    
    // Display the modified image
    epd.display(modifiedImage);
    
    // Free the temporary buffer
    free(modifiedImage);
    
    Serial.println("Image with battery overlay displayed successfully");
}

void DisplayHandler::sleep() {
    if (!initialized) return;
    
    Serial.println("Putting display to sleep...");
    epd.sleep();
}

uint8_t DisplayHandler::getClosestColor(uint8_t r, uint8_t g, uint8_t b) {
    // Simple color mapping to 7-color e-paper display
    // This is a basic implementation - can be refined
    
    if (r < 50 && g < 50 && b < 50) return EPD_7IN3F_BLACK;
    if (r > 200 && g > 200 && b > 200) return EPD_7IN3F_WHITE;
    if (g > r && g > b) return EPD_7IN3F_GREEN;
    if (b > r && b > g) return EPD_7IN3F_BLUE;
    if (r > g && r > b) return EPD_7IN3F_RED;
    if (r > 150 && g > 150 && b < 100) return EPD_7IN3F_YELLOW;
    if (r > 150 && g > 100 && b < 100) return EPD_7IN3F_ORANGE;
    
    return EPD_7IN3F_WHITE; // Default
}

void DisplayHandler::convertImageData(const uint8_t* rgbData, size_t dataSize, uint8_t* epdData) {
    // Convert RGB image to e-paper format
    // This is a placeholder - real implementation would need proper image processing
    
    size_t pixelCount = DISPLAY_WIDTH * DISPLAY_HEIGHT;
    size_t epdByteCount = pixelCount / 2;
    
    for (size_t i = 0; i < epdByteCount && i * 6 < dataSize; i++) {
        // Process 2 pixels at a time (each uses 4 bits)
        uint8_t r1 = (i * 6 < dataSize) ? rgbData[i * 6] : 255;
        uint8_t g1 = (i * 6 + 1 < dataSize) ? rgbData[i * 6 + 1] : 255;
        uint8_t b1 = (i * 6 + 2 < dataSize) ? rgbData[i * 6 + 2] : 255;
        uint8_t r2 = (i * 6 + 3 < dataSize) ? rgbData[i * 6 + 3] : 255;
        uint8_t g2 = (i * 6 + 4 < dataSize) ? rgbData[i * 6 + 4] : 255;
        uint8_t b2 = (i * 6 + 5 < dataSize) ? rgbData[i * 6 + 5] : 255;
        
        uint8_t color1 = getClosestColor(r1, g1, b1);
        uint8_t color2 = getClosestColor(r2, g2, b2);
        
        epdData[i] = (color1 << 4) | color2;
    }
}

void DisplayHandler::drawText(uint8_t* buffer, const char* text, int x, int y, int scale) {
    // Simple 5x7 font bitmap for basic characters
    const uint8_t font5x7[][5] = {
        {0x00, 0x00, 0x00, 0x00, 0x00}, // Space
        {0x00, 0x00, 0x5F, 0x00, 0x00}, // !
        {0x00, 0x07, 0x00, 0x07, 0x00}, // "
        {0x14, 0x7F, 0x14, 0x7F, 0x14}, // #
        {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // $
        {0x23, 0x13, 0x08, 0x64, 0x62}, // %
        {0x36, 0x49, 0x55, 0x22, 0x50}, // &
        {0x00, 0x05, 0x03, 0x00, 0x00}, // '
        {0x00, 0x1C, 0x22, 0x41, 0x00}, // (
        {0x00, 0x41, 0x22, 0x1C, 0x00}, // )
        {0x08, 0x2A, 0x1C, 0x2A, 0x08}, // *
        {0x08, 0x08, 0x3E, 0x08, 0x08}, // +
        {0x00, 0x50, 0x30, 0x00, 0x00}, // ,
        {0x08, 0x08, 0x08, 0x08, 0x08}, // -
        {0x00, 0x60, 0x60, 0x00, 0x00}, // .
        {0x20, 0x10, 0x08, 0x04, 0x02}, // /
        {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
        {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
        {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
        {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
        {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
        {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
        {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
        {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
        {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
        {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
        {0x00, 0x36, 0x36, 0x00, 0x00}, // :
        {0x00, 0x56, 0x36, 0x00, 0x00}, // ;
        {0x00, 0x08, 0x14, 0x22, 0x41}, // <
        {0x14, 0x14, 0x14, 0x14, 0x14}, // =
        {0x41, 0x22, 0x14, 0x08, 0x00}, // >
        {0x02, 0x01, 0x51, 0x09, 0x06}, // ?
        {0x32, 0x49, 0x79, 0x41, 0x3E}, // @
        {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A
        {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
        {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
        {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
        {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
        {0x7F, 0x09, 0x09, 0x01, 0x01}, // F
        {0x3E, 0x41, 0x41, 0x51, 0x32}, // G
        {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
        {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
        {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
        {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
        {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
        {0x7F, 0x02, 0x04, 0x02, 0x7F}, // M
        {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
        {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
        {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
        {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
        {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
        {0x46, 0x49, 0x49, 0x49, 0x31}, // S
        {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
        {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
        {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
        {0x7F, 0x20, 0x18, 0x20, 0x7F}, // W
        {0x63, 0x14, 0x08, 0x14, 0x63}, // X
        {0x03, 0x04, 0x78, 0x04, 0x03}, // Y
        {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
    };
    
    int textLength = strlen(text);
    
    for (int i = 0; i < textLength; i++) {
        char c = text[i];
        int charIndex;
        
        // Convert character to font index
        if (c >= ' ' && c <= 'Z') {
            charIndex = c - ' ';
        } else {
            charIndex = 0; // Default to space for unknown characters
        }
        
        // Draw character
        for (int row = 0; row < 7; row++) {
            uint8_t fontRow = (charIndex < 91) ? font5x7[charIndex][row < 5 ? row : 4] : 0;
            
            for (int col = 0; col < 5; col++) {
                if (fontRow & (1 << (4 - col))) {  // Fixed bit order - MSB first
                    // Draw pixel(s) for this bit
                    for (int sy = 0; sy < scale; sy++) {
                        for (int sx = 0; sx < scale; sx++) {
                            int pixelX = x + i * 6 * scale + col * scale + sx;
                            int pixelY = y + row * scale + sy;
                            
                            if (pixelX >= 0 && pixelX < DISPLAY_WIDTH && 
                                pixelY >= 0 && pixelY < DISPLAY_HEIGHT) {
                                
                                setPixel(buffer, pixelX, pixelY, EPD_7IN3F_BLACK);
                            }
                        }
                    }
                }
            }
        }
    }
}

void DisplayHandler::setPixel(uint8_t* buffer, int x, int y, uint8_t color) {
    if (x < 0 || x >= DISPLAY_WIDTH || y < 0 || y >= DISPLAY_HEIGHT) return;
    
    int bufferIndex = (y * (DISPLAY_WIDTH / 2)) + (x / 2);
    
    if (x % 2 == 0) {
        // Left pixel (upper 4 bits)
        buffer[bufferIndex] = (buffer[bufferIndex] & 0x0F) | (color << 4);
    } else {
        // Right pixel (lower 4 bits)
        buffer[bufferIndex] = (buffer[bufferIndex] & 0xF0) | color;
    }
}

void DisplayHandler::drawBatteryOverlay(uint8_t* buffer, int percentage) {
    // Battery overlay specifications (accounting for 90° rotation to the other side):
    // - 75x25px white-filled rounded rectangle with black border
    // - 10px from what appears as "top" edge when rotated, horizontally centered
    // - Contains battery percentage text and battery icon
    // 
    // Since display is rotated 90° the other way, what appears as "top" is actually the right side
    // Display coordinates: 800x480, but rotated means:
    // - Visual "width" = 480 pixels (DISPLAY_HEIGHT)  
    // - Visual "height" = 800 pixels (DISPLAY_WIDTH)
    // - Visual "top" = right side of actual display buffer
    
    // Cap percentage to 100%
    if (percentage > 100) percentage = 100;
    if (percentage < 0) percentage = 0;
    
    int overlayWidth = 90;   // Increased size for bigger elements
    int overlayHeight = 30;  // Increased height
    
    // Position for rotated display - 10px from visual top (right side), centered horizontally
    int overlayX = DISPLAY_WIDTH - overlayHeight - 10;  // 10px from right edge (visual top)
    int overlayY = (DISPLAY_HEIGHT - overlayWidth) / 2;  // Centered in visual width (height dimension)
    
    // Since we're rotating, swap width/height for the actual rectangle
    drawRoundedRect(buffer, overlayX, overlayY, overlayHeight, overlayWidth, 8, EPD_7IN3F_WHITE, EPD_7IN3F_BLACK);
    
    // Draw battery percentage text (positioned for rotation, centered in the rectangle)
    char percentText[5];
    snprintf(percentText, sizeof(percentText), "%d%%", percentage);
    
    // For rotated display, we need to render the text rotated 90 degrees
    // Calculate text positioning to center it in the rectangle
    int textLen = strlen(percentText);
    int textWidth = textLen * 6 * 2;  // 6 pixels per char * scale 2
    int textHeight = 7 * 2;           // 7 pixels height * scale 2
    
    int textX = overlayX + (overlayHeight - textWidth) / 2;  // Center in rectangle width (which is overlayHeight)
    int textY = overlayY + 8;  // Small margin from top
    drawText(buffer, percentText, textX, textY, 2);  // Scale 2 for bigger text
    
    // Draw battery icon (positioned for rotation, bigger size)
    int iconX = overlayX + (overlayHeight - 10) / 2;  // Center the icon horizontally
    int iconY = overlayY + overlayWidth - 25;   // Near bottom of overlay
    drawBatteryIcon(buffer, iconX, iconY, percentage);
}

void DisplayHandler::drawRoundedRect(uint8_t* buffer, int x, int y, int width, int height, int radius, uint8_t fillColor, uint8_t borderColor) {
    // Draw rounded rectangle with proper corners
    
    // Clamp radius to not exceed half the smaller dimension
    if (radius > width / 2) radius = width / 2;
    if (radius > height / 2) radius = height / 2;
    
    // Fill the main body (rectangular part without corners)
    for (int dy = radius; dy < height - radius; dy++) {
        for (int dx = 0; dx < width; dx++) {
            setPixel(buffer, x + dx, y + dy, fillColor);
        }
    }
    
    // Fill the top and bottom rectangles (between corners)
    for (int dy = 0; dy < radius; dy++) {
        for (int dx = radius; dx < width - radius; dx++) {
            setPixel(buffer, x + dx, y + dy, fillColor);              // Top
            setPixel(buffer, x + dx, y + height - 1 - dy, fillColor); // Bottom
        }
    }
    
    // Draw rounded corners using simple circle approximation
    for (int dy = 0; dy < radius; dy++) {
        for (int dx = 0; dx < radius; dx++) {
            // Distance from corner center
            int distSq = (dx - radius + 1) * (dx - radius + 1) + (dy - radius + 1) * (dy - radius + 1);
            int radiusSq = radius * radius;
            
            if (distSq <= radiusSq) {
                // Top-left corner
                setPixel(buffer, x + radius - 1 - dx, y + radius - 1 - dy, fillColor);
                // Top-right corner
                setPixel(buffer, x + width - radius + dx, y + radius - 1 - dy, fillColor);
                // Bottom-left corner
                setPixel(buffer, x + radius - 1 - dx, y + height - radius + dy, fillColor);
                // Bottom-right corner
                setPixel(buffer, x + width - radius + dx, y + height - radius + dy, fillColor);
            }
        }
    }
    
    // Draw border edges (straight parts)
    // Top and bottom edges
    for (int dx = radius; dx < width - radius; dx++) {
        setPixel(buffer, x + dx, y, borderColor);                    // Top edge
        setPixel(buffer, x + dx, y + height - 1, borderColor);       // Bottom edge
    }
    
    // Left and right edges  
    for (int dy = radius; dy < height - radius; dy++) {
        setPixel(buffer, x, y + dy, borderColor);                    // Left edge
        setPixel(buffer, x + width - 1, y + dy, borderColor);        // Right edge
    }
    
    // Draw rounded border corners
    for (int dy = 0; dy < radius; dy++) {
        for (int dx = 0; dx < radius; dx++) {
            int distSq = (dx - radius + 1) * (dx - radius + 1) + (dy - radius + 1) * (dy - radius + 1);
            int radiusSq = radius * radius;
            int innerRadiusSq = (radius - 1) * (radius - 1);
            
            if (distSq <= radiusSq && distSq > innerRadiusSq) {
                // Top-left corner border
                setPixel(buffer, x + radius - 1 - dx, y + radius - 1 - dy, borderColor);
                // Top-right corner border
                setPixel(buffer, x + width - radius + dx, y + radius - 1 - dy, borderColor);
                // Bottom-left corner border
                setPixel(buffer, x + radius - 1 - dx, y + height - radius + dy, borderColor);
                // Bottom-right corner border
                setPixel(buffer, x + width - radius + dx, y + height - radius + dy, borderColor);
            }
        }
    }
}

void DisplayHandler::drawBatteryIcon(uint8_t* buffer, int x, int y, int percentage) {
    // Battery icon oriented for 90° rotated display
    // Bigger vertical battery icon: 10x18 pixels (scaled up from 6x12)
    // Battery body: 10x15 pixels + terminal: 4x3 pixels at top
    
    // Ensure percentage is within bounds
    if (percentage > 100) percentage = 100;
    if (percentage < 0) percentage = 0;
    
    uint8_t batteryColor = EPD_7IN3F_BLACK;
    uint8_t fillColor = EPD_7IN3F_GREEN;
    
    if (percentage < 20) {
        fillColor = EPD_7IN3F_RED;
    } else if (percentage < 50) {
        fillColor = EPD_7IN3F_ORANGE;
    }
    
    // Draw battery terminal (positive end at top) - bigger terminal
    for (int dx = 3; dx < 7; dx++) {
        for (int dy = 0; dy < 3; dy++) {
            setPixel(buffer, x + dx, y + dy, batteryColor);
        }
    }
    
    // Draw battery body outline (10x15, starting from y+3)
    for (int dx = 0; dx < 10; dx++) {
        setPixel(buffer, x + dx, y + 3, batteryColor);         // Top edge of body
        setPixel(buffer, x + dx, y + 17, batteryColor);        // Bottom edge of body
    }
    for (int dy = 3; dy < 18; dy++) {
        setPixel(buffer, x, y + dy, batteryColor);             // Left edge
        setPixel(buffer, x + 9, y + dy, batteryColor);         // Right edge
    }
    
    // Draw battery fill level (from bottom up) - bigger fill area
    int fillHeight = ((percentage * 13) / 100);  // 13 pixels max fill height (15 - 2 for borders)
    for (int dy = 0; dy < fillHeight && dy < 13; dy++) {
        for (int dx = 1; dx < 9; dx++) {
            setPixel(buffer, x + dx, y + 16 - dy, fillColor);  // Fill from bottom up
        }
    }
}
