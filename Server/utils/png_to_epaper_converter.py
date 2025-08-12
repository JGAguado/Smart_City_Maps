#!/usr/bin/env python3
"""
PNG to 7-Color E-Paper Converter for ESP32-S2 Smart Dashboard
Uses Floyd-Steinberg dithering for optimal image quality on 7-color e-paper displays.
"""

import numpy as np
from PIL import Image
import os


class EpaperColorConverter:
    """Converts PNG images to 7-color e-paper C array format with Floyd-Steinberg dithering"""
    
    # 7-color e-paper palette (RGB values)
    PALETTE = [
        (0, 0, 0),           # 0 - Black
        (255, 255, 255),     # 1 - White  
        (67, 138, 28),       # 2 - Green
        (100, 64, 255),      # 3 - Blue
        (191, 0, 0),         # 4 - Red
        (255, 243, 56),      # 5 - Yellow
        (232, 126, 0),       # 6 - Orange
        (194, 164, 244)      # 7 - Light Purple (afterimage)
    ]
    
    # Supported resolutions
    SUPPORTED_RESOLUTIONS = [(800, 480), (640, 400), (600, 448)]
    
    def __init__(self):
        self.palette_array = np.array(self.PALETTE)
    
    def find_closest_color(self, rgb_color):
        """Find the closest color in the 7-color palette using Euclidean distance."""
        rgb_array = np.array(rgb_color)
        distances = np.sum((self.palette_array - rgb_array) ** 2, axis=1)
        return np.argmin(distances)
    
    def quantize_to_palette(self, img_array):
        """Simple quantization to palette colors without dithering."""
        height, width, channels = img_array.shape
        quantized = np.zeros_like(img_array)
        
        print("Applying simple quantization (no dithering)...")
        
        for y in range(height):
            for x in range(width):
                original_pixel = img_array[y, x]
                closest_color_idx = self.find_closest_color(original_pixel)
                quantized[y, x] = self.PALETTE[closest_color_idx]
        
        return quantized.astype(np.uint8)
    
    def apply_floyd_steinberg_dithering(self, img_array):
        """
        Apply Floyd-Steinberg dithering to improve color representation.
        This distributes quantization error to neighboring pixels.
        """
        height, width, channels = img_array.shape
        dithered = img_array.astype(np.float32)
        
        print("Applying Floyd-Steinberg dithering...")
        
        for y in range(height):
            for x in range(width):
                # Get current pixel
                old_pixel = dithered[y, x].copy()
                
                # Find closest palette color
                closest_color_idx = self.find_closest_color(old_pixel)
                new_pixel = np.array(self.PALETTE[closest_color_idx], dtype=np.float32)
                
                # Set the new color
                dithered[y, x] = new_pixel
                
                # Calculate quantization error
                error = old_pixel - new_pixel
                
                # Distribute error to neighboring pixels
                # Floyd-Steinberg distribution pattern:
                #   X   7/16
                # 3/16 5/16 1/16
                
                if x + 1 < width:
                    dithered[y, x + 1] += error * (7/16)
                
                if y + 1 < height:
                    if x - 1 >= 0:
                        dithered[y + 1, x - 1] += error * (3/16)
                    dithered[y + 1, x] += error * (5/16)
                    if x + 1 < width:
                        dithered[y + 1, x + 1] += error * (1/16)
        
        # Clamp values to valid range
        dithered = np.clip(dithered, 0, 255)
        return dithered.astype(np.uint8)
    
    def validate_image_size(self, width, height):
        """Validate image dimensions and check if rotation is needed."""
        # Check if image is in portrait and can be rotated to fit landscape
        for supported_width, supported_height in self.SUPPORTED_RESOLUTIONS:
            if width == supported_height and height == supported_width:
                print(f"Image is {width}x{height} (portrait), rotating to {supported_width}x{supported_height} (landscape)")
                return supported_width, supported_height, True  # needs rotation
            elif width == supported_width:
                if height > supported_height:
                    print(f"Warning: Height {height} exceeds maximum {supported_height} for width {width}")
                    return supported_width, supported_height, False
                return width, height, False  # no rotation needed
        
        print(f"Error: Unsupported dimensions {width}x{height}")
        print(f"Supported resolutions: {self.SUPPORTED_RESOLUTIONS}")
        return None, None, False
    
    def convert_png_to_epaper(self, input_path, output_path=None, generate_c_file=False):
        """Convert PNG image to 7-color e-paper binary format with Floyd-Steinberg dithering.
        
        Args:
            input_path: Path to input PNG file
            output_path: Path for C array file (only used if generate_c_file=True)
            generate_c_file: Whether to generate C array file (for debugging/development)
        """
        try:
            print(f"Converting: {input_path}")
            
            # Load and validate image
            with Image.open(input_path) as img:
                # Convert to RGB if needed
                if img.mode != 'RGB':
                    print(f"Converting from {img.mode} to RGB")
                    img = img.convert('RGB')
                
                width, height = img.size
                print(f"Input image: {width}x{height}")
                
                # Validate dimensions and check rotation
                target_width, target_height, should_rotate = self.validate_image_size(width, height)
                if target_width is None:
                    return None
                
                # Apply rotation if needed
                if should_rotate:
                    print("Rotating image 90° clockwise...")
                    img = img.rotate(-90, expand=True)
                    width, height = img.size
                    print(f"After rotation: {width}x{height}")
                
                # Crop if necessary
                if (width, height) != (target_width, target_height):
                    img = img.crop((0, 0, target_width, target_height))
                    print(f"Cropped to: {target_width}x{target_height}")
                
                # Convert image to numpy array
                img_array = np.array(img)
                
                # Apply Floyd-Steinberg dithering
                dithered_array = self.apply_floyd_steinberg_dithering(img_array)
                
                print("Converting dithered image to e-paper format...")
                
                # Create output buffer
                buffer_size = (target_width * target_height) // 2
                output_buffer = bytearray(buffer_size)
                
                byte_index = 0
                for y in range(target_height):
                    for x in range(0, target_width, 2):  # Process 2 pixels at a time
                        # Get first pixel color
                        rgb1 = tuple(dithered_array[y, x])
                        color1 = self.find_closest_color(rgb1)
                        
                        # Get second pixel color (handle odd width)
                        if x + 1 < target_width:
                            rgb2 = tuple(dithered_array[y, x + 1])
                            color2 = self.find_closest_color(rgb2)
                        else:
                            color2 = color1  # Duplicate last pixel if odd width
                        
                        # Pack 2 pixels into 1 byte (4 bits each)
                        packed_byte = (color1 << 4) | color2
                        output_buffer[byte_index] = packed_byte
                        byte_index += 1
                
                print(f"Conversion complete: {len(output_buffer)} bytes generated")
                
                # Always save binary file (this is the main output for ESP32)
                bin_path = None
                if output_path:
                    # Generate binary file path
                    bin_path = output_path.replace('.c', '.bin')
                else:
                    # Generate binary file path from input
                    base_name = os.path.splitext(input_path)[0]
                    bin_path = f"{base_name}.bin"
                
                with open(bin_path, 'wb') as f:
                    f.write(output_buffer)
                print(f"✅ Binary file saved to: {bin_path}")
                
                # Optionally generate C array code (for development/debugging)
                if generate_c_file and output_path:
                    c_code = f"// 7 Color Image Data {target_width}*{target_height}\n"
                    c_code += f"const unsigned char Image7color[{len(output_buffer)}] = {{\n"
                    
                    for i, byte_val in enumerate(output_buffer):
                        if i % 16 == 0 and i > 0:
                            c_code += "\n"
                        c_code += f"0x{byte_val:02X},"
                    
                    c_code += "\n};\n"
                    
                    # Save C array file
                    with open(output_path, 'w') as f:
                        f.write(c_code)
                    print(f"✅ C array saved to: {output_path}")
                    
                    return c_code
                
                # Return binary file path instead of C code when not generating C file
                return bin_path
                
        except Exception as e:
            print(f"❌ Error converting image: {e}")
            return None


def convert_png_to_c_file(png_path, c_path=None, generate_c_file=True):
    """
    Convenience function to convert PNG to binary/C array file.
    
    Args:
        png_path: Path to PNG file
        c_path: Output C file path (if None, uses PNG name with .c extension)
        generate_c_file: Whether to generate C array file (defaults to True for compatibility)
    
    Returns:
        str: Path to generated binary file, or None if failed
    """
    if c_path is None and generate_c_file:
        c_path = os.path.splitext(png_path)[0] + '.c'
    
    converter = EpaperColorConverter()
    result = converter.convert_png_to_epaper(png_path, c_path, generate_c_file)
    return result


def convert_png_to_bin_only(png_path):
    """
    Streamlined function to convert PNG to binary file only (no C file generated).
    This is the optimal workflow for production firmware.
    
    Args:
        png_path: Path to PNG file
    
    Returns:
        str: Path to generated binary file, or None if failed
    """
    converter = EpaperColorConverter()
    result = converter.convert_png_to_epaper(png_path, generate_c_file=False)
    return result


# Test when run directly
if __name__ == "__main__":
    import sys
    
    if len(sys.argv) > 1:
        png_file = sys.argv[1]
        print(f"Converting {png_file} to binary only...")
        bin_path = convert_png_to_bin_only(png_file)
        if bin_path:
            # Verify binary file
            import os
            size = os.path.getsize(bin_path)
            print(f"✅ Success! Binary file: {bin_path} ({size} bytes)")
            
            # Quick verification: 800x480 should be exactly 192000 bytes
            expected_size = 800 * 480 // 2  # 4-bit packed
            if size == expected_size:
                print("✅ Binary size verification passed!")
            else:
                print(f"⚠️  Unexpected size: got {size}, expected {expected_size}")
        else:
            print("❌ Conversion failed")
    else:
        # Test with Vienna image
        print("Testing binary-only conversion with Vienna_Austria.png...")
        bin_path = convert_png_to_bin_only('Maps/Vienna_Austria.png')
        if bin_path:
            import os
            size = os.path.getsize(bin_path)
            print(f"✅ Success! Binary file: {bin_path} ({size} bytes)")
            print("✅ No C file generated - streamlined for firmware efficiency!")
