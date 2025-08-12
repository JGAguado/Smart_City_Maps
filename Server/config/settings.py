#!/usr/bin/env python3
"""
Configuration settings for Smart City Maps
Centralized configuration for all modules
"""

import os
from typing import Dict, List, Tuple

# API Configuration
class APIConfig:
    """API keys and endpoints configuration"""
    
    # API Keys (from environment variables)
    MAPBOX_ACCESS_TOKEN = os.getenv('MAPBOX_ACCESS_TOKEN')  
    OPENWEATHER_API_KEY = os.getenv('OPENWEATHER_API_KEY')
    GEONAMES_USERNAME = os.getenv('GEONAMES_USERNAME')  # Free account at geonames.org


    # Mapbox API (superior styling and traffic)
    MAPBOX_BASE_URL = "https://api.mapbox.com"
    
    # Geonames API (timezone data)
    GEONAMES_TIMEZONE_URL = "http://api.geonames.org/timezoneJSON"
    
    # Weather API
    OPENWEATHER_CURRENT_URL = "http://api.openweathermap.org/data/2.5/weather"

# Display Configuration
class DisplayConfig:
    """E-paper display and image settings"""
    
    # E-paper display dimensions
    FINAL_WIDTH = 480
    FINAL_HEIGHT = 800
    
    # Map generation settings
    SQUARE_SIZE = 800  # Square size for download to avoid deformation
    DEFAULT_ZOOM = 12
    
    # Supported resolutions for e-paper
    SUPPORTED_RESOLUTIONS = [(800, 480), (640, 400), (600, 448)]
    
    # E-paper 7-color palette (RGB values)
    EPAPER_PALETTE = [
        (0, 0, 0),           # 0 - Black
        (255, 255, 255),     # 1 - White  
        (67, 138, 28),       # 2 - Green
        (100, 64, 255),      # 3 - Blue
        (191, 0, 0),         # 4 - Red
        (255, 243, 56),      # 5 - Yellow
        (232, 126, 0),       # 6 - Orange
        (194, 164, 244)      # 7 - Light Purple (afterimage)
    ]
    
    # Overlay styling
    TEXT_COLOR = (0, 0, 0)                    # Pure black text
    OVERLAY_BACKGROUND = (255, 255, 255)      # White background for overlays



# File Paths Configuration
class PathConfig:
    """File and directory paths"""
    
    # Base directories
    MAPS_FOLDER = "Maps"
    FONTS_FOLDER = "fonts"
    WEATHER_ICONS_FOLDER = "utils/icons"
    CACHE_FILE = "locations_cache.json"
    
    # Font files
    PREFERRED_FONT = "Quicksand-VariableFont_wght.ttf"
    
    # Output file patterns
    MAP_FILE_PATTERN = "{city}_{country}.png"
    TRAFFIC_MAP_PATTERN = "{city}_{country}_traffic.png"
    BINARY_FILE_PATTERN = "{city}_{country}.bin"


# Font Configuration
class FontConfig:
    """Font sizes and styling for overlays"""
    
    # Font sizes for 480x800px display
    TITLE_SIZE = 24      # City/Country name
    COORD_SIZE = 16      # Coordinates
    TEMP_SIZE = 28       # Temperature
    DATE_SIZE = 18       # Date
    TIME_SIZE = 24       # Time
    WEATHER_SIZE = 18    # Weather information
    INFO_SIZE = 16       # General info text
    
    # Weather icon size
    WEATHER_ICON_SIZE = 80  # Weather icon size in pixels
    
    # Colors
    TEXT_COLOR = (0, 0, 0)        # Pure black
    COORD_COLOR = (60, 60, 60)    # Dark gray
    
    # Overlay settings
    OVERLAY_WIDTH = 320
    OVERLAY_HEIGHT = 160
    OVERLAY_MARGIN = 30
    CORNER_RADIUS = 15


# Timezone Configuration
class TimezoneConfig:
    """Timezone mapping for common cities/countries"""
    
    TIMEZONE_MAPPING = {
        # European cities
        'vienna': 'Europe/Vienna',
        'austria': 'Europe/Vienna',
        'berlin': 'Europe/Berlin', 
        'germany': 'Europe/Berlin',
        'paris': 'Europe/Paris',
        'france': 'Europe/Paris',
        'london': 'Europe/London',
        'uk': 'Europe/London',
        'united kingdom': 'Europe/London',
        'madrid': 'Europe/Madrid',
        'spain': 'Europe/Madrid',
        'rome': 'Europe/Rome',
        'italy': 'Europe/Rome',
        
        # American cities
        'new york': 'America/New_York',
        'usa': 'America/New_York',
        'united states': 'America/New_York',
        'los angeles': 'America/Los_Angeles',
        'chicago': 'America/Chicago',
        'miami': 'America/New_York',
        
        # Asian cities
        'tokyo': 'Asia/Tokyo',
        'japan': 'Asia/Tokyo',
        'beijing': 'Asia/Shanghai',
        'china': 'Asia/Shanghai',
        'seoul': 'Asia/Seoul',
        'south korea': 'Asia/Seoul',
        'mumbai': 'Asia/Kolkata',
        'india': 'Asia/Kolkata',
        
        # Other cities
        'sydney': 'Australia/Sydney',
        'australia': 'Australia/Sydney',
        'moscow': 'Europe/Moscow',
        'russia': 'Europe/Moscow',
        'cairo': 'Africa/Cairo',
        'egypt': 'Africa/Cairo',
        'dubai': 'Asia/Dubai',
        'uae': 'Asia/Dubai',
    }


# Map Styling Configuration
class MapStyleConfig:
    """Google Maps and Bing Maps styling configuration"""
    
    # Base map styles (labels removed for clean e-paper display)
    BASE_STYLES = [
        'feature:all|element:labels|visibility:off',
        'feature:administrative|element:labels|visibility:off',
        'feature:poi|element:labels|visibility:off',
        'feature:road|element:labels|visibility:off',
        'feature:transit|element:labels|visibility:off',
    ]
    
    # Color schemes for different map elements
    LANDSCAPE_COLORS = {
        'background': '0xf2f2f2',
        'natural': '0xe8e8e8',
        'parks': '0x66bb6a',
        'landcover': '0x8bc34a',
        'terrain': '0xa5d6a7'
    }
    
    WATER_COLORS = {
        'primary': '0x1976d2',
        'secondary': '0x42a5f5'
    }
    
    BUILDING_COLORS = {
        'man_made': '0x808080',
        'business': '0x808080',
        'government': '0x808080',
        'medical': '0x808080',
        'school': '0x999999',
        'worship': '0x888888',
        'establishment': '0x777777',
        'poi': '0x666666'
    }


# Validation and Error Handling
class ValidationConfig:
    """Validation rules and error handling"""
    
    # API timeouts
    GEOCODING_TIMEOUT = 10
    WEATHER_TIMEOUT = 10
    MAP_DOWNLOAD_TIMEOUT = 30
    MAP_TIMEOUT = 30  # For Azure Maps compatibility
    
    # Retry settings
    MAX_RETRIES = 3
    RETRY_DELAY = 1  # seconds
    
    # Cache settings
    CACHE_EXPIRY_HOURS = 24 * 7  # 1 week


def validate_configuration():
    """Validate that required configuration is available"""
    
    errors = []
    warnings = []
    
    # Check required API keys
    if not APIConfig.MAPBOX_ACCESS_TOKEN:
        errors.append("Mapbox API key not found. Set MAPBOX_ACCESS_TOKEN environment variable")
    
    if not APIConfig.OPENWEATHER_API_KEY:
        warnings.append("OpenWeather API key not found, weather data will be disabled")
    
    if not APIConfig.GEONAMES_USERNAME:
        warnings.append("Geonames username not found, will use coordinate-based timezone estimation")
        warnings.append("Get free username at: https://www.geonames.org/login")
    
    # Create required directories
    import os
    for folder in [PathConfig.MAPS_FOLDER, PathConfig.FONTS_FOLDER, PathConfig.WEATHER_ICONS_FOLDER]:
        if not os.path.exists(folder):
            os.makedirs(folder)
            print(f"📁 Created {folder} folder")
    
    # Return validation results
    return {
        'errors': errors,
        'warnings': warnings,
        'valid': len(errors) == 0
    }


if __name__ == "__main__":
    # Test configuration
    print("🔧 Smart Dashboard Configuration")
    print("=" * 40)
    
    validation = validate_configuration()
    
    if validation['valid']:
        print("✅ Configuration is valid")
        
        if validation['warnings']:
            for warning in validation['warnings']:
                print(f"⚠️  {warning}")
        
        print(f"\n📊 Configuration Summary:")
        print(f"  Display: {DisplayConfig.FINAL_WIDTH}x{DisplayConfig.FINAL_HEIGHT}")
        print(f"  Maps folder: {PathConfig.MAPS_FOLDER}")
        print(f"  Cache file: {PathConfig.CACHE_FILE}")
        print(f"  Weather icons folder: {PathConfig.WEATHER_ICONS_FOLDER}")
        print(f"  Font config sizes: Title={FontConfig.TITLE_SIZE}, Temp={FontConfig.TEMP_SIZE}")
        
    else:
        print("❌ Configuration errors:")
        for error in validation['errors']:
            print(f"  - {error}")
