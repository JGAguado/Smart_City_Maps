#!/usr/bin/env python3
"""
Weather Data Provider for Smart City Maps.

Handles fetching weather data from OpenWeatherMap API and downloading weather icons.
"""

import requests
import io
import os
from typing import Optional, Dict
from PIL import Image

# Import configuration
from config.settings import PathConfig, FontConfig


class WeatherProvider:
    """
    Weather data provider using OpenWeatherMap API.
    
    Features:
    - Current weather data fetching
    - Weather icon downloading
    - Temperature in Celsius
    - Error handling and fallbacks
    """
    
    def __init__(self, api_key: Optional[str] = None, icons_dir: str = None):
        """
        Initialize weather provider.
        
        Args:
            api_key: OpenWeatherMap API key. If None, weather features will be disabled.
            icons_dir: Directory containing local weather icon PNG files (optional, uses config default)
        """
        self.api_key = api_key
        self.icons_dir = icons_dir or PathConfig.WEATHER_ICONS_FOLDER
        self.weather_url = "http://api.openweathermap.org/data/2.5/weather"
        
        # Verify icons directory exists
        if not os.path.exists(self.icons_dir):
            print(f"⚠️  Weather icons directory not found: {self.icons_dir}")
            print("   Weather icons will be disabled.")
        else:
            print(f"✅ Using local weather icons from: {self.icons_dir}")
        
        if not self.api_key:
            print("⚠️  OpenWeatherMap API key not found. Weather info will be disabled.")
            print("   Set OPENWEATHER_API_KEY environment variable to enable weather data.")
    
    def is_available(self) -> bool:
        """Check if weather provider is available (has API key)."""
        return self.api_key is not None
    
    def get_available_icons(self) -> list:
        """
        Get list of available weather icon codes.
        
        Returns:
            List of available icon codes (e.g., ['01d', '01n', '02d', ...])
        """
        if not os.path.exists(self.icons_dir):
            return []
        
        icons = []
        for filename in os.listdir(self.icons_dir):
            if filename.endswith('.png') and len(filename) == 7:  # e.g., '01d.png'
                icon_code = filename[:-4]  # Remove '.png' extension
                icons.append(icon_code)
        
        return sorted(icons)
    
    def get_weather_data(self, lat: float, lng: float) -> Optional[Dict]:
        """
        Get current weather data for coordinates.

        Args:
            lat: Latitude
            lng: Longitude

        Returns:
            Weather data dictionary or None if unavailable
        """
        if not self.api_key:
            return None

        try:
            params = {
                'lat': lat,
                'lon': lng,
                'appid': self.api_key,
                'units': 'metric'  # Celsius
            }

            response = requests.get(self.weather_url, params=params, timeout=10)
            response.raise_for_status()

            data = response.json()

            weather_info = {
                'temperature': round(data['main']['temp']),
                'description': data['weather'][0]['description'].title(),
                'icon': data['weather'][0]['icon'],
                'city_name': data['name'],
                'country_code': data['sys']['country']
            }

            print(f"🌤️  Weather: {weather_info['temperature']}°C, {weather_info['description']}")
            return weather_info

        except Exception as e:
            print(f"⚠️  Could not fetch weather data: {e}")
            return None

    def download_weather_icon(self, icon_code: str, size: int = None) -> Optional[Image.Image]:
        """
        Load local weather icon PNG file.

        Args:
            icon_code: OpenWeatherMap weather icon code (e.g., '01d', '10n')
            size: Size to resize the icon to (default: from config)

        Returns:
            PIL Image or None if icon not found
        """
        try:
            # Use default size from config if not specified
            if size is None:
                size = FontConfig.WEATHER_ICON_SIZE
            
            # Construct path to local icon file
            icon_path = os.path.join(self.icons_dir, f"{icon_code}.png")
            
            # Check if the icon file exists
            if not os.path.exists(icon_path):
                print(f"⚠️  Weather icon not found: {icon_path}")
                return None
            
            # Load the PNG image
            icon_image = Image.open(icon_path)
            
            # Ensure RGBA mode for proper transparency
            if icon_image.mode != 'RGBA':
                icon_image = icon_image.convert('RGBA')
            
            # Resize to requested size while maintaining aspect ratio
            original_size = icon_image.size
            if original_size[0] != size or original_size[1] != size:
                icon_image = icon_image.resize((size, size), Image.Resampling.LANCZOS)
                print(f"🌤️  Loaded and resized weather icon: {icon_code} ({original_size[0]}x{original_size[1]} → {size}x{size})")
            else:
                print(f"🌤️  Loaded weather icon: {icon_code} ({size}x{size})")
            
            return icon_image

        except Exception as e:
            print(f"⚠️  Could not load weather icon {icon_code}: {e}")
            return None
