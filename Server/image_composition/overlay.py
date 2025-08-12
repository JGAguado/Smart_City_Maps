#!/usr/bin/env python3
"""
Overlay Composer for Smart City Maps.

Handles creating information overlays on map images with weather, time, and location data.
"""

from PIL import Image, ImageDraw
from typing import Optional, Dict, Tuple
from .fonts import FontManager
from config.settings import FontConfig


class OverlayComposer:
    """
    Overlay composer for adding information overlays to map images.
    
    Features:
    - Rounded rectangle overlays with solid backgrounds
    - Weather icon integration
    - Multi-line text layout
    - Responsive positioning and sizing
    - Cross-platform font support
    """
    
    def __init__(self, final_width: int = 480, final_height: int = 800):
        """
        Initialize overlay composer.
        
        Args:
            final_width: Final image width in pixels
            final_height: Final image height in pixels
        """
        self.final_width = final_width
        self.final_height = final_height
        self.font_manager = FontManager()
        
        # Use font sizes from configuration
        self.TITLE_SIZE = FontConfig.TITLE_SIZE
        self.COORD_SIZE = FontConfig.COORD_SIZE
        self.TEMP_SIZE = FontConfig.TEMP_SIZE
        self.DATE_SIZE = FontConfig.DATE_SIZE
        self.TIME_SIZE = FontConfig.TIME_SIZE
        self.WEATHER_ICON_SIZE = FontConfig.WEATHER_ICON_SIZE
        
        print(f"🔤 Using font configuration for {final_width}x{final_height}px - Title: {self.TITLE_SIZE}px, Temp: {self.TEMP_SIZE}px, Time: {self.TIME_SIZE}px")
    
    def add_info_overlay(self, image: Image.Image, city: str, country: str, 
                        lat: float, lng: float, date_str: str, time_str: str,
                        weather_data: Optional[Dict] = None, weather_icon: Optional[Image.Image] = None) -> Image.Image:
        """
        Add information overlay with solid white background and rounded corners ON TOP of the map.

        Args:
            image: Base map image
            city: City name
            country: Country name
            lat: Latitude
            lng: Longitude
            date_str: Formatted date string
            time_str: Formatted time string
            weather_data: Weather information dictionary
            weather_icon: Weather icon image (PIL Image)

        Returns:
            Image with overlay ON TOP
        """
        # Make sure we're working with RGB mode to avoid alpha issues
        if image.mode != 'RGB':
            image = image.convert('RGB')

        # Work directly on the image
        draw = ImageDraw.Draw(image)

        # Overlay dimensions and position - optimized for 480x800px
        overlay_width = 320      # Slightly wider for better text layout
        overlay_height = 160     # Slightly taller for comfortable spacing
        x = (self.final_width - overlay_width) // 2  # Centered horizontally
        y = self.final_height - overlay_height - 30  # 30px margin from bottom

        # Draw rounded rectangle directly ON the map with PURE WHITE background
        corner_radius = 15

        # Draw the solid white rounded rectangle - NO ALPHA, just pure RGB
        draw.rounded_rectangle(
            [x, y, x + overlay_width, y + overlay_height],
            radius=corner_radius,
            fill=(255, 255, 255),  # Pure white RGB
            outline=(180, 180, 180),  # Gray border for definition
            width=2
        )

        # Load fonts with consistent sizing
        try:
            title_font = self.font_manager.get_font(self.TITLE_SIZE)
            coord_font = self.font_manager.get_font(self.COORD_SIZE)
            temp_font = self.font_manager.get_font(self.TEMP_SIZE)
            date_font = self.font_manager.get_font(self.DATE_SIZE)
            time_font = self.font_manager.get_font(self.TIME_SIZE)
            
        except Exception as e:
            print(f"⚠️ Font loading error: {e}")
            # Final fallback to default
            from PIL import ImageFont
            title_font = ImageFont.load_default()
            coord_font = ImageFont.load_default()
            temp_font = ImageFont.load_default()
            date_font = ImageFont.load_default()
            time_font = ImageFont.load_default()

        # Text colors - pure black for maximum contrast
        text_color = (0, 0, 0)  # Pure black
        coord_color = (60, 60, 60)  # Darker gray for coordinates

        # Calculate text positions relative to the overlay box - increased margins for better spacing
        text_y_base = y + 25  # Top margin - more space from top

        # City, Country (top line)
        city_text = f"{city.title()}, {country.title()}"
        text_bbox = draw.textbbox((0, 0), city_text, font=title_font)
        text_width = text_bbox[2] - text_bbox[0]

        # Center the text within the overlay
        text_x = x + (overlay_width - text_width) // 2
        draw.text((text_x, text_y_base), city_text, fill=text_color, font=title_font)

        # Coordinates (second line) - Improved format with N/S and E/W indicators
        lat_dir = "N" if lat >= 0 else "S"
        lng_dir = "E" if lng >= 0 else "W"
        coord_text = f"{abs(lat):.4f}°{lat_dir}   {abs(lng):.4f}°{lng_dir}"

        text_bbox = draw.textbbox((0, 0), coord_text, font=coord_font)
        text_width = text_bbox[2] - text_bbox[0]
        text_x = x + (overlay_width - text_width) // 2
        draw.text((text_x, text_y_base + 35), coord_text, fill=coord_color, font=coord_font)  # Increased spacing

        # Weather and datetime information (bottom section) - properly aligned groups
        if weather_data:
            temp_text = f"{weather_data['temperature']}°C"

            if weather_icon:
                # Use icon size from configuration
                icon_size = self.WEATHER_ICON_SIZE

                # Resize first, then handle transparency properly
                weather_icon = weather_icon.resize((icon_size, icon_size), Image.Resampling.LANCZOS)
                
                # Keep transparency by preserving RGBA mode for proper blending
                if weather_icon.mode != 'RGBA':
                    weather_icon = weather_icon.convert('RGBA')

                # Calculate dimensions for datetime group (treated as single block)
                date_bbox = draw.textbbox((0, 0), date_str, font=date_font)
                time_bbox = draw.textbbox((0, 0), time_str, font=time_font)

                # DateTime group dimensions - width is max of date/time, height includes both
                datetime_width = max(date_bbox[2] - date_bbox[0], time_bbox[2] - time_bbox[0])
                datetime_height = (date_bbox[3] - date_bbox[1]) + (time_bbox[3] - time_bbox[1]) + 5  # 5px spacing

                # Calculate dimensions for temperature text
                temp_bbox = draw.textbbox((0, 0), temp_text, font=temp_font)
                temp_width = temp_bbox[2] - temp_bbox[0]
                temp_height = temp_bbox[3] - temp_bbox[1]

                # Calculate total width for horizontal layout: datetime_group + icon + temp
                group_spacing = 15
                total_width = datetime_width + group_spacing + icon_size + group_spacing + temp_width
                start_x = x + (overlay_width - total_width) // 2

                # Position groups horizontally
                datetime_group_x = start_x
                icon_x = datetime_group_x + datetime_width + group_spacing
                temp_x = icon_x + icon_size + group_spacing

                # Vertical positioning - center all elements on same baseline
                element_y = text_y_base + 45

                # Calculate vertical centering for each element
                max_height = max(datetime_height, icon_size, temp_height)

                # Draw datetime group (left) - vertically centered
                datetime_y_offset = (max_height - int(datetime_height*0.75)) // 2
                date_x = datetime_group_x + (datetime_width - (date_bbox[2] - date_bbox[0])) // 2
                time_x = datetime_group_x + (datetime_width - (time_bbox[2] - time_bbox[0])) // 2

                draw.text((date_x, element_y + datetime_y_offset), date_str, fill=text_color, font=date_font)
                draw.text((time_x, element_y + datetime_y_offset + 20), time_str, fill=text_color, font=time_font)

                # Draw weather icon (center) - vertically centered with proper transparency
                icon_y_offset = (max_height - int(icon_size*0.75)) // 2
                # Use the alpha channel as mask for proper transparency blending
                image.paste(weather_icon, (icon_x, element_y + icon_y_offset), weather_icon)

                # Draw temperature (right) - align with weather icon center for better visual balance
                # Instead of centering vertically, align with the icon's vertical center
                temp_y_offset = icon_y_offset + (icon_size - temp_height*2) // 2
                draw.text((temp_x, element_y + temp_y_offset), temp_text, fill=text_color, font=temp_font)

            else:
                # No icon, show datetime and temperature groups only
                # Calculate positioning for two groups
                date_bbox = draw.textbbox((0, 0), date_str, font=date_font)
                time_bbox = draw.textbbox((0, 0), time_str, font=time_font)
                temp_bbox = draw.textbbox((0, 0), temp_text, font=temp_font)

                datetime_width = max(date_bbox[2] - date_bbox[0], time_bbox[2] - time_bbox[0])
                temp_width = temp_bbox[2] - temp_bbox[0]

                group_spacing = 30
                total_width = datetime_width + group_spacing + temp_width
                start_x = x + (overlay_width - total_width) // 2
                element_y = text_y_base + 65

                # Draw datetime group
                date_x = start_x + (datetime_width - (date_bbox[2] - date_bbox[0])) // 2
                time_x = start_x + (datetime_width - (time_bbox[2] - time_bbox[0])) // 2

                draw.text((date_x, element_y), date_str, fill=text_color, font=date_font)
                draw.text((time_x, element_y + 20), time_str, fill=text_color, font=time_font)

                # Draw temperature group
                temp_x = start_x + datetime_width + group_spacing
                draw.text((temp_x, element_y + 10), temp_text, fill=text_color, font=temp_font)
        else:
            # No weather data, show datetime only
            # Calculate positioning for datetime group only
            date_bbox = draw.textbbox((0, 0), date_str, font=date_font)
            time_bbox = draw.textbbox((0, 0), time_str, font=time_font)

            datetime_width = max(date_bbox[2] - date_bbox[0], time_bbox[2] - time_bbox[0])
            start_x = x + (overlay_width - datetime_width) // 2
            element_y = text_y_base + 65

            # Draw datetime group centered
            date_x = start_x + (datetime_width - (date_bbox[2] - date_bbox[0])) // 2
            time_x = start_x + (datetime_width - (time_bbox[2] - time_bbox[0])) // 2

            draw.text((date_x, element_y), date_str, fill=text_color, font=date_font)
            draw.text((time_x, element_y + 20), time_str, fill=text_color, font=time_font)

        return image
    
    def crop_to_final_size(self, image: Image.Image) -> Image.Image:
        """
        Crop square image to final size from center.

        Args:
            image: Square input image

        Returns:
            Cropped image at final dimensions
        """
        width, height = image.size

        # Calculate coordinates for centered crop
        left = (width - self.final_width) // 2
        top = (height - self.final_height) // 2
        right = left + self.final_width
        bottom = top + self.final_height

        # Crop image
        cropped = image.crop((left, top, right, bottom))
        return cropped
