#!/usr/bin/env python3
"""
E-paper File Converter for Smart City Maps.

Wrapper around the existing PNG to e-paper converter functionality.
"""

import os
from .png_to_epaper_converter import convert_png_to_c_file


class EpaperConverter:
    """
    E-paper format converter.
    
    Features:
    - PNG to C array conversion
    - Binary file generation
    - Floyd-Steinberg dithering
    - Multi-resolution support
    """
    
    @staticmethod
    def convert_png_to_epaper(png_path: str, output_dir: str = None) -> tuple[str, str]:
        """
        Convert PNG file to e-paper format (C array and binary).
        
        Args:
            png_path: Path to input PNG file
            output_dir: Output directory (optional, defaults to same as PNG)
            
        Returns:
            Tuple of (c_file_path, bin_file_path) or (None, None) if conversion fails
        """
        try:
            # Determine output paths
            if output_dir:
                base_name = os.path.splitext(os.path.basename(png_path))[0]
                c_path = os.path.join(output_dir, f"{base_name}.c")
            else:
                c_path = os.path.splitext(png_path)[0] + '.c'
            
            bin_path = os.path.splitext(c_path)[0] + '.bin'
            
            print(f"🔄 Converting to e-paper format with Floyd-Steinberg dithering...")
            
            # Use the existing converter
            if convert_png_to_c_file(png_path, c_path):
                print(f"✅ E-paper C array saved to: {c_path}")
                print(f"✅ E-paper binary saved to: {bin_path} (firmware will fetch this)")
                return c_path, bin_path
            else:
                print(f"⚠️  Failed to generate e-paper format")
                return None, None
                
        except Exception as e:
            print(f"⚠️  Error converting to e-paper format: {e}")
            return None, None
