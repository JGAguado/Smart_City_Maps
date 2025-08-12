"""
Data providers package for Smart City Maps.

This package contains modules for fetching external data:
- weather: Weather data from OpenWeatherMap
- geolocation: Unified location and timezone handling (replaces location and timezone modules)
"""

from .weather import WeatherProvider
from .geolocation import GeolocationProvider

__all__ = ['WeatherProvider', 'GeolocationProvider']
