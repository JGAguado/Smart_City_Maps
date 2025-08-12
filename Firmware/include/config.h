#ifndef CONFIG_H
#define CONFIG_H

// E-Paper Display GPIO Configuration
// 7.3" 7-color e-paper display connections
#define EPD_BUSY_PIN    5   // GPIO5 - Busy signal from display
#define EPD_RST_PIN     6   // GPIO6 - Reset pin
#define EPD_DC_PIN      7   // GPIO7 - Data/Command selection
#define EPD_CS_PIN      8   // GPIO8 - Chip Select (SPI)
#define EPD_DIN_PIN     35  // GPIO35 - SPI MOSI (Data In)
#define EPD_SCK_PIN     36  // GPIO36 - SPI Clock

// Display specifications
#define DISPLAY_WIDTH   800
#define DISPLAY_HEIGHT  480

// Network Configuration
#define AP_SSID         "SmartDashboard-Setup"
#define AP_PASSWORD     "configure123"
#define CONFIG_TIMEOUT  300000  // 5 minutes timeout for configuration

// Web server configuration
#define WEB_SERVER_PORT 80
#define DNS_PORT        53

// GitHub Configuration
#define GITHUB_HOST     "raw.githubusercontent.com"
#define GITHUB_PORT     443
#define MAX_IMAGE_SIZE  200000  // 200KB max image size

// Update intervals - optimized for deep sleep operation
#define UPDATE_INTERVAL_MS      10000    // 10 seconds check interval (only used in config mode)
#define WIFI_RETRY_DELAY_MS     30000    // 30 seconds between WiFi retries
#define CONFIG_CHECK_INTERVAL   5000     // Check for configuration every 5 seconds

// Deep sleep configuration
#define ACTIVE_HOURS_SLEEP_MIN   30      // 30 minutes during active hours (7:00-22:00)
#define INACTIVE_HOURS_SLEEP_H   9       // 9 hours during inactive hours (22:00-7:00)

// EEPROM Configuration addresses
#define EEPROM_SIZE             512
#define EEPROM_WIFI_SSID_ADDR   0
#define EEPROM_WIFI_PASS_ADDR   64
#define EEPROM_GITHUB_REPO_ADDR 128
#define EEPROM_GITHUB_PATH_ADDR 192
#define EEPROM_CONFIG_FLAG_ADDR 256

// Configuration validation
#define CONFIG_MAGIC_NUMBER     0xABCD
#define MAX_SSID_LENGTH         63
#define MAX_PASSWORD_LENGTH     63
#define MAX_REPO_LENGTH         63
#define MAX_PATH_LENGTH         63

#endif // CONFIG_H


#ifndef DEFAULT_CONFIG_H
#define DEFAULT_CONFIG_H

// =====================================================
// DEFAULT CONFIGURATION FOR SMART DASHBOARD
// =====================================================
// Edit these values before flashing to pre-configure your device
// If any value is left empty (""), the device will start in configuration mode

// WiFi Configuration
// Replace with your home WiFi network credentials
#define DEFAULT_WIFI_SSID       "MyHomeWiFi"       // Example: "MyHomeWiFi"
#define DEFAULT_WIFI_PASSWORD   "MyPassword123"   // Example: "MyPassword123"

// GitHub Repository Configuration
// This should point to your Smart City Maps repository
#define DEFAULT_GITHUB_REPO     "JGAguado/Smart_City_Maps"

// Image Path Configuration
// Choose which city/image to display by default
// Available options from your repository:
// - "Server/Maps/Berlin_Germany.png"
// - "Server/Maps/London_UK.png" 
// - "Server/Maps/Madrid_Spain.png"
// - "Server/Maps/Vienna_Austria.png"
#define DEFAULT_GITHUB_PATH     "Server/Maps/Vienna_Austria.png"

// =====================================================
// CONFIGURATION OPTIONS
// =====================================================
// Set this to true to always use default config (skip web portal)
// Set this to false to allow web portal configuration when defaults are empty
#define FORCE_DEFAULT_CONFIG    false

// Set this to true to show default config details on startup
#define SHOW_DEFAULT_CONFIG     true

// =====================================================
// VALIDATION MACROS (DO NOT EDIT)
// =====================================================
#define HAS_DEFAULT_WIFI        (strlen(DEFAULT_WIFI_SSID) > 0 && strcmp(DEFAULT_WIFI_SSID, "YOUR_WIFI_NAME") != 0)
#define HAS_DEFAULT_GITHUB      (strlen(DEFAULT_GITHUB_REPO) > 0 && strlen(DEFAULT_GITHUB_PATH) > 0)
#define HAS_COMPLETE_DEFAULT    (HAS_DEFAULT_WIFI && HAS_DEFAULT_GITHUB)

#endif // DEFAULT_CONFIG_H
