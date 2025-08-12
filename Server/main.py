#!/usr/bin/env python3
"""
Smart City Maps - Main Entry Point.

This is the main entry point for the Smart City Maps.
The core functionality has been refactored into specialized modules:

- data_providers: Weather, timezone, and location data
- image_composition: Overlay creation and font management  
- utils: File conversion utilities
- map_providers: Map generation providers

The main MapGenerator class is now in map_generator.py
"""

from map_generator import MapGenerator, main

# Re-export the main class for backward compatibility
__all__ = ['MapGenerator', 'main']

if __name__ == "__main__":
    main()
