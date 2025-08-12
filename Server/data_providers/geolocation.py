#!/usr/bin/env python3
"""
Geolocation Provider for Smart City Maps.

Combines location geocoding and timezone functionality into a unified provider.
Handles caching of coordinates and timezone data.
"""

import json
import os
import requests
from typing import Dict, Tuple, Optional
from datetime import datetime
import pytz
from map_providers.mapbox import get_mapbox_provider


class GeolocationProvider:
    """
    Unified geolocation provider for coordinates and timezone data.
    
    Workflow:
    1. Check cache for city, country
    2. If found: return cached lat, lng, utc_offset
    3. If not found:
       3.1 Get lat, lng from Mapbox
       3.2 Get utc_offset from Geonames API
       3.3 Cache the results
    """
    
    def __init__(self, cache_file: str = "locations_cache.json", geonames_username: str = None):
        """
        Initialize geolocation provider.
        
        Args:
            cache_file: Path to JSON file for caching location data
            geonames_username: Geonames.org username for API access (optional)
        """
        self.cache_file = cache_file
        self.geonames_username = geonames_username or os.getenv('GEONAMES_USERNAME')
        self.locations_cache = self._load_cache()
        
        # Initialize Mapbox provider
        self.mapbox = get_mapbox_provider()
        if not self.mapbox.is_available():
            raise ValueError("Mapbox API key is required. Please set MAPBOX_ACCESS_TOKEN environment variable.")
        
        # Geonames API endpoint
        self.geonames_timezone_url = "http://api.geonames.org/timezoneJSON"
        
        if not self.geonames_username:
            print("⚠️  Geonames username not found. Timezone will use coordinate estimation fallback.")
            print("   Set GEONAMES_USERNAME environment variable for accurate timezone data.")
            print("   Get free username at: https://www.geonames.org/login")
    
    def _load_cache(self) -> Dict:
        """Load locations cache from JSON file."""
        try:
            if os.path.exists(self.cache_file):
                with open(self.cache_file, 'r', encoding='utf-8') as f:
                    cache = json.load(f)
                    print(f"📂 Loaded {len(cache)} cached locations from {self.cache_file}")
                    return cache
            else:
                print(f"📂 Creating new cache file: {self.cache_file}")
                return {}
        except Exception as e:
            print(f"⚠️  Error loading cache file: {e}. Starting with empty cache.")
            return {}

    def _save_cache(self):
        """Save locations cache to JSON file."""
        try:
            with open(self.cache_file, 'w', encoding='utf-8') as f:
                json.dump(self.locations_cache, f, indent=2, ensure_ascii=False)
            print(f"💾 Cache saved with {len(self.locations_cache)} locations")
        except Exception as e:
            print(f"⚠️  Error saving cache: {e}")

    def _get_cache_key(self, city: str, country: str) -> str:
        """Generate cache key for city/country combination."""
        return f"{city.lower().strip()}, {country.lower().strip()}"

    def _get_coordinates_from_mapbox(self, city: str, country: str) -> Tuple[float, float]:
        """Get coordinates using Mapbox geocoding."""
        try:
            lat, lng = self.mapbox.get_coordinates(city, country)
            print(f"📍 Mapbox geocoding: {city}, {country} -> {lat:.4f}, {lng:.4f}")
            return lat, lng
        except Exception as e:
            raise Exception(f"Mapbox geocoding failed: {e}")

    def _get_timezone_from_geonames(self, lat: float, lng: float) -> Tuple[str, str]:
        """
        Get timezone data from Geonames API.
        
        Returns:
            Tuple of (timezone_name, utc_offset_string)
        """
        if not self.geonames_username:
            return self._estimate_timezone_from_coordinates(lat, lng)
        
        try:
            params = {
                'lat': lat,
                'lng': lng,
                'username': self.geonames_username,
                'formatted': 'true'
            }
            
            print(f"🌍 Getting timezone from Geonames for {lat:.4f}, {lng:.4f}...")
            response = requests.get(self.geonames_timezone_url, params=params, timeout=10)
            response.raise_for_status()
            
            data = response.json()
            
            # Check for API errors
            if 'status' in data:
                error_msg = data['status'].get('message', 'Unknown error')
                print(f"⚠️  Geonames API error: {error_msg}")
                return self._estimate_timezone_from_coordinates(lat, lng)
            
            timezone_id = data.get('timezoneId')
            raw_offset = data.get('rawOffset', 0)  # UTC offset in hours
            
            if not timezone_id:
                print("⚠️  No timezone data from Geonames, using estimation")
                return self._estimate_timezone_from_coordinates(lat, lng)
            
            # Format UTC offset string
            if raw_offset == 0:
                offset_str = "±0"
            elif raw_offset > 0:
                offset_str = f"+{int(raw_offset)}"
            else:
                offset_str = str(int(raw_offset))
            
            print(f"🕐 Geonames timezone: {timezone_id} (UTC{offset_str})")
            return timezone_id, offset_str
            
        except Exception as e:
            print(f"⚠️  Geonames API error: {e}, using coordinate estimation")
            return self._estimate_timezone_from_coordinates(lat, lng)

    def _estimate_timezone_from_coordinates(self, lat: float, lng: float) -> Tuple[str, str]:
        """
        Fallback: Estimate timezone from coordinates using longitude-based calculation.
        """
        # Basic estimation: UTC offset = longitude / 15
        utc_offset_hours = round(lng / 15)
        utc_offset_hours = max(-12, min(12, utc_offset_hours))  # Clamp to valid range
        
        if utc_offset_hours == 0:
            timezone_name = "UTC"
            offset_str = "±0"
        else:
            # Create a generic timezone name
            timezone_name = f"Etc/GMT{-utc_offset_hours:+d}"
            offset_str = f"{utc_offset_hours:+d}" if utc_offset_hours != 0 else "±0"
        
        print(f"⚠️  Using estimated timezone: {timezone_name} (UTC{offset_str})")
        return timezone_name, offset_str

    def get_location_data(self, city: str, country: str) -> Dict:
        """
        Get complete location data (coordinates + timezone) for a city and country.
        
        Workflow:
        1. Check cache first
        2. If not cached: get coordinates from Mapbox, timezone from Geonames
        3. Cache and return results
        
        Args:
            city: City name
            country: Country name
            
        Returns:
            Dictionary with location data:
            {
                'lat': float,
                'lng': float, 
                'timezone': str,
                'utc_offset': str,
                'city': str,
                'country': str,
                'formatted_address': str
            }
        """
        cache_key = self._get_cache_key(city, country)
        
        # Step 1: Check cache
        if cache_key in self.locations_cache:
            cached_data = self.locations_cache[cache_key]
            
            # Verify we have all required data
            if all(key in cached_data for key in ['lat', 'lng', 'timezone', 'utc_offset']):
                print(f"📍 Using cached data for {city}, {country}")
                return cached_data
            else:
                print(f"📍 Partial cache data for {city}, {country} - refreshing...")
        
        # Step 2: Get fresh data
        print(f"🔍 Getting location data for {city}, {country}...")
        
        try:
            # Step 3.1: Get coordinates from Mapbox
            lat, lng = self._get_coordinates_from_mapbox(city, country)
            
            # Step 3.2: Get timezone from Geonames
            timezone_name, utc_offset = self._get_timezone_from_geonames(lat, lng)
            
            # Step 3.3: Create complete location data
            location_data = {
                'lat': lat,
                'lng': lng,
                'timezone': timezone_name,
                'utc_offset': utc_offset,
                'city': city.title(),
                'country': country.title(),
                'formatted_address': f"{city.title()}, {country.title()}"
            }
            
            # Cache the results
            self.locations_cache[cache_key] = location_data
            self._save_cache()
            
            print(f"✅ Location data cached for {city}, {country}")
            return location_data
            
        except Exception as e:
            raise Exception(f"Failed to get location data for {city}, {country}: {e}")

    def get_local_datetime(self, timezone_name: str) -> Tuple[str, str]:
        """
        Get formatted local date and time for a timezone.
        
        Args:
            timezone_name: Timezone name (e.g., 'Europe/Vienna')
            
        Returns:
            Tuple of (date_string, time_string)
        """
        try:
            # Use pytz for timezone handling
            timezone = pytz.timezone(timezone_name)
            local_time = datetime.now(timezone)

            # Format: "03 August" instead of "03/01/2025"
            date_str = local_time.strftime("%d %B")
            time_str = local_time.strftime("%H:%M")

            return date_str, time_str

        except Exception as e:
            print(f"⚠️  Could not get local time for {timezone_name}: {e}")
            # Fallback to UTC
            utc_time = datetime.utcnow()
            date_str = utc_time.strftime("%d %B")
            time_str = utc_time.strftime("%H:%M")
            return date_str, time_str

    def list_cached_locations(self) -> Dict:
        """
        Get all cached locations.

        Returns:
            Dictionary with cached locations
        """
        return self.locations_cache.copy()

    def clear_cache(self):
        """Clear all cached locations."""
        self.locations_cache.clear()
        self._save_cache()
        print("🗑️  Cache cleared")

    def is_available(self) -> bool:
        """Check if geolocation provider is available (has Mapbox access)."""
        return self.mapbox.is_available()
