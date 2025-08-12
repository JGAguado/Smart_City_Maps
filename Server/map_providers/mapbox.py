#!/usr/bin/env python3
"""
Mapbox Provider for Smart Dashboard
Handles map downloading, geocoding, and traffic integration for e-paper displays
"""

import os
import requests
from typing import Optional, Tuple
from PIL import Image
import io


class MapboxProvider:
    """
    Complete Mapbox provider for Smart Dashboard.
    
    Features:
    - Map downloading with custom styles
    - Geocoding (address to coordinates)
    - Traffic integration
    - E-paper optimized (clean, label-free maps)
    - Configurable pixel sizes and zoom levels
    - Optional PNG debugging output
    """
    
    def __init__(self, access_token: str):
        """
        Initialize Mapbox provider.
        
        Args:
            access_token: Mapbox access token
        """
        self.access_token = access_token
        self.base_url = "https://api.mapbox.com"
        
    def is_available(self) -> bool:
        """Check if Mapbox API is available with current token"""
        if not self.access_token:
            return False
        
        try:
            # Test with a simple geocoding request
            response = requests.get(
                f"{self.base_url}/geocoding/v5/mapbox.places/test.json",
                params={'access_token': self.access_token, 'limit': 1},
                timeout=5
            )
            return response.status_code == 200
        except:
            return False
    
    def get_coordinates(self, city: str, country: str) -> Tuple[float, float]:
        """
        Get coordinates for a city using Mapbox geocoding.
        
        Args:
            city: City name
            country: Country name
            
        Returns:
            Tuple of (latitude, longitude)
            
        Raises:
            Exception: If geocoding fails
        """
        query = f"{city}, {country}"
        url = f"{self.base_url}/geocoding/v5/mapbox.places/{query}.json"
        
        params = {
            'access_token': self.access_token,
            'limit': 1,
            'types': 'place'
        }
        
        response = requests.get(url, params=params)
        
        if response.status_code != 200:
            raise Exception(f"Geocoding failed: {response.status_code} - {response.text}")
        
        data = response.json()
        
        if not data.get('features'):
            raise Exception(f"No results found for {query}")
        
        coordinates = data['features'][0]['geometry']['coordinates']
        # Mapbox returns [longitude, latitude], we need [latitude, longitude]
        return coordinates[1], coordinates[0]
    
    def download_map(self, 
                    lat: float, 
                    lng: float, 
                    zoom: int = 13,
                    width: int = 480, 
                    height: int = 800,
                    style: str = "custom",
                    save_debug_png: bool = False,
                    debug_filename: str = "debug_map.png") -> Optional[Image.Image]:
        """
        Download a map from Mapbox with specified parameters.
        
        Args:
            lat: Latitude
            lng: Longitude 
            zoom: Zoom level (1-20)
            width: Map width in pixels
            height: Map height in pixels
            style: Style to use ("custom", "light", "dark", "satellite")
            save_debug_png: Whether to save a debug PNG file
            debug_filename: Filename for debug PNG (if save_debug_png=True)
            
        Returns:
            PIL Image object or None if failed
        """
        try:
            # Get the style ID
            style_id = self._get_style_id(style)
            
            # Build the Static Images API URL
            style_path = style_id.replace('mapbox://styles/', '')
            endpoint = f"{self.base_url}/styles/v1/{style_path}/static/{lng},{lat},{zoom}/{width}x{height}"
            
            params = {
                'access_token': self.access_token,
                'attribution': 'false',
                'logo': 'false',
                'retina': '@2x'
            }
            
            # Make the request
            response = requests.get(endpoint, params=params)
            
            if response.status_code != 200:
                print(f"❌ Map download failed: {response.status_code} - {response.text}")
                return None
            
            # Convert to PIL Image
            image = Image.open(io.BytesIO(response.content))
            
            # Handle transparency (convert RGBA to RGB if needed)
            if image.mode == 'RGBA':
                # Create white background for e-paper displays
                background = Image.new('RGB', image.size, (255, 255, 255))
                background.paste(image, mask=image.split()[3] if len(image.split()) == 4 else None)
                image = background
            
            
            # Save debug PNG if requested
            if save_debug_png:
                # Handle relative paths properly
                if os.path.dirname(debug_filename):
                    # If debug_filename contains a folder, create full path
                    debug_path = debug_filename
                    os.makedirs(os.path.dirname(debug_path), exist_ok=True)
                else:
                    # Otherwise save to Maps folder as before
                    os.makedirs("Maps", exist_ok=True)
                    debug_path = os.path.join("Maps", debug_filename)
                
                image.save(debug_path, 'PNG', optimize=True)
                print(f"🐛 Debug PNG saved: {debug_path}")
            
            return image
            
        except Exception as e:
            print(f"❌ Error downloading map: {e}")
            return None
    
    def generate_map_image(self, 
                          lat: float, 
                          lng: float, 
                          zoom: int = 13, 
                          style: str = "default") -> Optional[Image.Image]:
        """
        Generate map image compatible with main_modular.py expectations.
        
        Args:
            lat: Latitude
            lng: Longitude
            zoom: Zoom level
            include_traffic: Whether to include traffic (future feature)
            style: Style type ("clean" for e-paper optimized)
            
        Returns:
            PIL Image object or None if failed
        """
        
        # Use standard e-paper dimensions
        return self.download_map(
            lat=lat,
            lng=lng, 
            zoom=zoom,
            width=480,
            height=800,
            style=style, 
            save_debug_png=False
        )
    
    def _get_style_id(self, style: str) -> str:
        """
        Get the Mapbox style ID based on style name.
        
        Args:
            style: Style name ("default", "blueprint", "custom", "light", "dark", "satellite")
            
        Returns:
            Mapbox style URL
        """
        styles = {
            "default": "mapbox://styles/jgaguado/cmdxbgtdp001501r2a5zl20e3",  # Your default clean style
            "blueprint": "mapbox://styles/jgaguado/cmdxb7jsl00g701pj0xs91e5a",  # Your blueprint style
            "custom": "mapbox://styles/jgaguado/cmdxbgtdp001501r2a5zl20e3",   # Same as default for backward compatibility
            "light": "mapbox://styles/mapbox/light-v11",
            "dark": "mapbox://styles/mapbox/dark-v11", 
            "satellite": "mapbox://styles/mapbox/satellite-v9",
            "streets": "mapbox://styles/mapbox/streets-v12"
        }
        
        return styles.get(style, styles["default"])
    
    def _get_clean_style_id(self, include_traffic: bool = False) -> str:
        """
        Get style ID for clean maps (used by main_modular.py).
        
        Args:
            include_traffic: Whether to include traffic overlay
            
        Returns:
            Style ID string
        """
        return "mapbox://styles/jgaguado/cmdxbgtdp001501r2a5zl20e3"


def get_mapbox_provider() -> MapboxProvider:
    """
    Get configured Mapbox provider instance.
    
    Returns:
        MapboxProvider instance with token from environment or config
    """
    # Try to get token from environment variable
    token = os.getenv('MAPBOX_ACCESS_TOKEN')
    
    return MapboxProvider(token)


# Example usage and testing
if __name__ == "__main__":
    print("🧪 Testing Mapbox Provider - All Styles")
    
    mapbox = get_mapbox_provider()
    
    if not mapbox.is_available():
        print("❌ Mapbox not available - check token")
        exit(1)
    
    print("✅ Mapbox provider ready")
    
    # Create styles folder
    styles_folder = os.path.join("map_providers", "styles")
    os.makedirs(styles_folder, exist_ok=True)
    print(f"📁 Created styles folder: {styles_folder}")
    
    # Test geocoding
    try:
        lat, lng = mapbox.get_coordinates("Vienna", "Austria")
        print(f"📍 Vienna coordinates: {lat}, {lng}")
        
        # Get all available styles
        all_styles = ["default", "blueprint", "light", "dark", "satellite", "streets"]
        
        print(f"\n🎨 Testing ALL {len(all_styles)} styles and saving to styles folder...")
        
        for style_name in all_styles:
            print(f"\n  🔧 Testing {style_name.upper()} style...")
            
            try:
                map_image = mapbox.download_map(
                    lat=lat,
                    lng=lng,
                    zoom=11,
                    width=480,
                    height=800,
                    style=style_name,
                    save_debug_png=True,
                    debug_filename=os.path.join("map_providers", "styles", f"vienna_{style_name}_style.png")
                )
                
                if map_image:
                    print(f"     ✅ {style_name} style: {map_image.size} - SAVED")
                else:
                    print(f"     ❌ {style_name} style: FAILED")
                    
            except Exception as e:
                print(f"     ❌ {style_name} style: ERROR - {e}")
        
        print(f"\n📋 Style Summary:")
        print(f"  • default: mapbox://styles/jgaguado/cmdxbgtdp001501r2a5zl20e3")
        print(f"  • blueprint: mapbox://styles/jgaguado/cmdxb7jsl00g701pj0xs91e5a")
        print(f"  • light: mapbox://styles/mapbox/light-v11")
        print(f"  • dark: mapbox://styles/mapbox/dark-v11")
        print(f"  • satellite: mapbox://styles/mapbox/satellite-v9")
        print(f"  • streets: mapbox://styles/mapbox/streets-v12")
        
        print(f"\n💾 All style previews saved to: {styles_folder}")
            
    except Exception as e:
        print(f"❌ Test failed: {e}")

