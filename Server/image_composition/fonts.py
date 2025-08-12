#!/usr/bin/env python3
"""
Font Manager for Smart City Maps.

Handles cross-platform font loading and management with fallbacks.
"""

import os
from PIL import ImageFont
from typing import Optional


class FontManager:
    """
    Font manager for cross-platform font loading.
    
    Features:
    - Quicksand variable font support
    - Cross-platform fallbacks
    - Bold weight preferences
    - Size-based font caching
    """
    
    def __init__(self, fonts_dir: str = "fonts"):
        """
        Initialize font manager.
        
        Args:
            fonts_dir: Directory containing font files
        """
        self.fonts_dir = fonts_dir
        self.font_path = self._setup_fonts()
        self._font_cache = {}
    
    def _setup_fonts(self) -> Optional[str]:
        """
        Setup Quicksand font for cross-platform compatibility.
        Returns path to the font file.
        """
        os.makedirs(self.fonts_dir, exist_ok=True)
        
        # Check for the manually added Quicksand variable font
        font_file = os.path.join(self.fonts_dir, "Quicksand-VariableFont_wght.ttf")
        if os.path.exists(font_file):
            print(f"✅ Using Quicksand font (Bold): {font_file}")
            return font_file
        
        # If font is not found, return None and use system fonts
        print("⚠️ Quicksand font not found in fonts folder")
        print("🔄 Will use system fonts as fallback")
        return None
    
    def get_font(self, size: int) -> ImageFont.ImageFont:
        """
        Get font with specified size and bold weight, with fallback to system fonts.
        
        Args:
            size: Font size in pixels
            
        Returns:
            PIL ImageFont object
        """
        # Check cache first
        cache_key = f"{size}_{self.font_path}"
        if cache_key in self._font_cache:
            return self._font_cache[cache_key]
        
        font = None
        
        try:
            if self.font_path and os.path.exists(self.font_path):
                # For variable fonts, specify a bolder weight (700 = Bold)
                # Quicksand variable font supports weights from 300 to 700
                try:
                    # Load font with bold weight (700)
                    font = ImageFont.truetype(self.font_path, size)
                    # For variable fonts, we can set font variations
                    if hasattr(font, 'set_variation_by_axes'):
                        font.set_variation_by_axes([700])  # Bold weight
                    elif hasattr(font, 'set_variation_by_name'):
                        font.set_variation_by_name(weight=700)
                except:
                    # Fallback to regular loading if variation setting fails
                    font = ImageFont.truetype(self.font_path, size)
        except Exception as e:
            print(f"⚠️ Could not load Quicksand font: {e}")
        
        # If primary font failed, try fallback fonts
        if font is None:
            font = self._get_fallback_font(size)
        
        # Cache the font
        self._font_cache[cache_key] = font
        return font
    
    def _get_fallback_font(self, size: int) -> ImageFont.ImageFont:
        """
        Get fallback font for different operating systems.
        
        Args:
            size: Font size in pixels
            
        Returns:
            PIL ImageFont object
        """
        # Fallback fonts for different operating systems - prioritize bold versions
        fallback_fonts = [
            # Windows bold fonts
            "C:/Windows/Fonts/arialbd.ttf",  # Arial Bold
            "C:/Windows/Fonts/arial.ttf",
            "arialbd.ttf", 
            "arial.ttf",
            "Arial.ttf",
            # Linux bold fonts
            "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
            "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf", 
            "/usr/share/fonts/TTF/DejaVuSans-Bold.ttf",
            "/usr/share/fonts/TTF/DejaVuSans.ttf",
            "DejaVuSans-Bold.ttf",
            "DejaVuSans.ttf",
            "LiberationSans-Bold.ttf",
            "LiberationSans-Regular.ttf",
            "FreeSans.ttf",
            # Mac bold fonts
            "/System/Library/Fonts/Arial Bold.ttf",
            "/System/Library/Fonts/Arial.ttf",
            "/System/Library/Fonts/Helvetica.ttc",
        ]
        
        for font_name in fallback_fonts:
            try:
                return ImageFont.truetype(font_name, size)
            except:
                continue
        
        # Final fallback to default font
        print(f"⚠️ Using default font for size {size}")
        return ImageFont.load_default()
