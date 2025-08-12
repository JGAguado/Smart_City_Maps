# Smart Dashboard Firmware

The embedded firmware of the project is tailored for the [Smart Dashboard](https://smart-dashboard.readthedocs.io) driving the 7.3" 7-color E-paper and displays location-based maps with weather information. The device automatically fetches and displays map images from GitHub repositories with optimized power management through deep sleep functionality.

## 🔧 Hardware Requirements

### Main Components
- **[Smart Dashboard](https://smart-dashboard.readthedocs.io)** (internally powered by an ESP32-S2-mini)
- **[7.3" E-Paper Display](https://www.seeedstudio.com/7-3-Seven-Color-ePaper-Display-with-800x480-Pixels-p-5787.html)** (800x480, 7-color ACeP technology)
- **MAX17048 LiPo Fuel Gauge** (optional, for battery monitoring)
- **LiPo Battery** (optional, for portable operation)

### Pin Configuration
```cpp
// E-Paper Display GPIO Configuration (SPI)
#define EPD_BUSY_PIN    5   // GPIO5 - Busy signal from display
#define EPD_RST_PIN     6   // GPIO6 - Reset pin
#define EPD_DC_PIN      7   // GPIO7 - Data/Command selection
#define EPD_CS_PIN      8   // GPIO8 - Chip Select (SPI)
#define EPD_DIN_PIN     35  // GPIO35 - SPI MOSI (Data In)
#define EPD_SCK_PIN     36  // GPIO36 - SPI Clock

// Battery Monitor (MAX17048 LiPo Fuel Gauge via I2C)
// Sensor power: GPIO13
// SDA: GPIO33, SCL: GPIO34
```

## ⚡ Features

### Core Functionality
- **Automatic Map Updates**: Fetches images from configured GitHub repositories
- **Deep Sleep Power Management**:
    - **Active Hours**: 7:00-22:00 (30-minute sleep cycles)
    - **Inactive Hours**: 22:00-7:00 (9-hour sleep cycles)
- **Battery Monitoring**: Real-time battery percentage and voltage
- **E-Paper Optimization**: 7-color display (Black, White, Green, Blue, Red, Yellow, Orange)
- **Time Synchronization**: NTP-based scheduling for optimal wake times

## 🚀 Quick Start

### Prerequisites
- **PlatformIO** (recommended) or Arduino IDE

### Installation

1. **Clone and Setup**:
   ```bash
   cd Firmware/
   pio install  # Install dependencies via PlatformIO
   ```

2. **Hardware Connections**:
   - Connect the e-paper display according to pin configuration to the Smart Dashboard 
   - Optional: Connect LiPo battery for portable operation

3. **Build and Upload**:
   ```bash
   pio run -t upload  # Build and upload firmware
   pio device monitor # Monitor serial output
   ```

## 📁 Project Structure

```
Firmware/
├── platformio.ini              # PlatformIO configuration
├── include/                    # Header files
│   ├── config.h               #   - Hardware and system configuration
│   ├── display_handler.h      #   - E-paper display management
│   ├── github_fetcher.h       #   - Image downloading from GitHub
│   ├── battery_monitor.h      #   - Battery status monitoring
│   ├── config_manager.h       #   - Configuration storage (EEPROM)
│   ├── web_server.h          #   - Configuration web interface
│   ├── qr_code.h             #   - QR code generation
│   └── utils.h               #   - Utility functions
├── src/                       # Source files
│   ├── main.cpp              #   - Main program loop and setup
│   ├── display_handler.cpp   #   - E-paper display implementation
│   ├── github_fetcher.cpp    #   - GitHub API and image fetching
│   ├── battery_monitor.cpp   #   - MAX17048 fuel gauge integration
│   ├── config_manager.cpp    #   - EEPROM configuration management
│   ├── web_server.cpp        #   - WiFi setup web interface
│   ├── qr_code.cpp          #   - WiFi QR code generation
│   ├── utils.cpp            #   - Helper and utility functions
│   └── epd7in3f.cpp         #   - Low-level e-paper driver
└── font/                     # Font files for display
```

## ⚙️ Configuration

### System Settings (`config.h`)
```cpp
// Network Configuration

#define DEFAULT_WIFI_SSID       "MyHomeWiFi"      
#define DEFAULT_WIFI_PASSWORD   "MyPassword123"   
// GitHub Configuration
#define GITHUB_HOST     "raw.githubusercontent.com"
#define MAX_IMAGE_SIZE  200000  // 200KB max image size
// This should point to your Smart City Maps repository
#define DEFAULT_GITHUB_REPO     "JGAguado/Smart_City_Maps"
#define DEFAULT_GITHUB_PATH     "Server/Maps/Vienna_Austria.png"

// Deep Sleep Configuration
#define ACTIVE_HOURS_SLEEP_MIN   30  // 30 minutes during day
#define INACTIVE_HOURS_SLEEP_H   9   // 9 hours during night


```

### Runtime Configuration
The device stores configuration in EEPROM:
- WiFi credentials (SSID and password)
- GitHub repository and image path
- Configuration validation flags

## 🔋 Power Management

### Deep Sleep Operation
The firmware implements intelligent power management:

1. **Wake Up**: Device wakes from deep sleep
2. **WiFi Connect**: Attempts to connect to configured network
3. **Time Sync**: Synchronizes with NTP servers (UTC+2 in my case)
4. **Image Fetch**: Downloads latest image from GitHub
5. **Display Update**: Shows new image 
6. **Sleep Calculation**: Determines next wake time based on current time
7. **Deep Sleep**: Enters low-power mode until next update

### Sleep Duration Logic
```cpp
bool isActiveHours() {
    // Active hours: 7:00 AM - 10:00 PM
    // Inactive hours: 10:00 PM - 7:00 AM
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return true;
    
    int hour = timeinfo.tm_hour;
    return (hour >= 7 && hour < 22);
}
```

### Battery Monitoring
- **Real-time Monitoring**: Continuous battery percentage and voltage
- **Display Overlay (optional)**: Battery icon and percentage on map display 
- **Low Power Alerts (optional)**: Status reporting via serial console

## 📡 GitHub Integration

### Image Fetching Process
1. **HTTPS Connection**: Secure connection to `raw.githubusercontent.com`
2. **Repository Access**: Downloads from public repositories
3. **Binary Format**: Optimized for direct e-paper display
4. **Size Validation**: Ensures image fits within memory constraints
5. **Display Update**: Direct rendering to e-paper display

### Supported Image Formats
- **Binary Files**: Pre-converted e-paper format (`.bin` files)
- **Size Limit**: 200KB maximum (configurable)
- **Resolution**: 800x480 pixels, 7-color format
- **Repository**: Must be publicly accessible

## 📋 API Requirements

### GitHub Repository Structure
Your repository should contain:
```
repository/
├── Server/Maps/
│   ├── YourCity_YourCountry.bin    # E-paper binary file
│   ├── YourCity_YourCountry.png    # Original image (optional)
│   └── YourCity_YourCountry_epd.png # Preview (optional)
└── README.md
```

The firmware specifically looks for `.bin` files generated by the Smart City Maps component.

