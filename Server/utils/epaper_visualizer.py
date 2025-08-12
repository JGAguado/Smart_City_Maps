#!/usr/bin/env python3
"""
E-Paper Binary to PNG Visualizer for Smart City Maps.

This utility converts e-paper binary files back to PNG images for visualization,
allowing you to see how images look after Floyd-Steinberg dithering and 7-color quantization.
"""

import numpy as np
from PIL import Image
import os


class EpaperVisualizer:
    """Converts e-paper binary files back to PNG images for visualization"""
    
    # 7-color e-paper palette (RGB values) - must match converter
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
    
    def detect_resolution(self, file_size):
        """
        Detect image resolution based on binary file size.
        
        Args:
            file_size: Size of binary file in bytes
            
        Returns:
            tuple: (width, height) or None if not recognized
        """
        for width, height in self.SUPPORTED_RESOLUTIONS:
            expected_size = (width * height) // 2  # 4 bits per pixel, 2 pixels per byte
            if file_size == expected_size:
                return width, height
        
        return None
    
    def convert_bin_to_png(self, bin_path, png_path=None, width=None, height=None):
        """
        Convert e-paper binary file to PNG for visualization.
        
        Args:
            bin_path: Path to input binary file
            png_path: Path for output PNG file (optional)
            width: Image width (optional, will try to auto-detect)
            height: Image height (optional, will try to auto-detect)
            
        Returns:
            str: Path to generated PNG file, or None if failed
        """
        try:
            print(f"Converting binary file: {bin_path}")
            
            # Read binary file
            with open(bin_path, 'rb') as f:
                binary_data = f.read()
            
            file_size = len(binary_data)
            print(f"Binary file size: {file_size} bytes")
            
            # Detect resolution if not provided
            if width is None or height is None:
                detected = self.detect_resolution(file_size)
                if detected:
                    width, height = detected
                    print(f"Auto-detected resolution: {width}x{height}")
                else:
                    print(f"❌ Could not detect resolution from file size {file_size}")
                    print(f"Supported resolutions: {self.SUPPORTED_RESOLUTIONS}")
                    return None
            
            # Verify file size matches expected size
            expected_size = (width * height) // 2
            if file_size != expected_size:
                print(f"⚠️  Warning: File size {file_size} doesn't match expected {expected_size} for {width}x{height}")
            
            # Generate output path if not provided
            if png_path is None:
                base_name = os.path.splitext(bin_path)[0]
                png_path = f"{base_name}_visualized.png"
            
            # Create image array
            img_array = np.zeros((height, width, 3), dtype=np.uint8)
            
            print("Unpacking binary data...")
            
            # Unpack binary data
            byte_index = 0
            for y in range(height):
                for x in range(0, width, 2):  # Process 2 pixels at a time
                    if byte_index >= len(binary_data):
                        print(f"⚠️  Unexpected end of data at byte {byte_index}")
                        break
                    
                    packed_byte = binary_data[byte_index]
                    
                    # Extract two 4-bit color indices
                    color1_idx = (packed_byte >> 4) & 0x0F  # Upper 4 bits
                    color2_idx = packed_byte & 0x0F         # Lower 4 bits
                    
                    # Ensure color indices are valid
                    if color1_idx >= len(self.PALETTE):
                        print(f"⚠️  Invalid color index {color1_idx} at byte {byte_index}")
                        color1_idx = 0
                    if color2_idx >= len(self.PALETTE):
                        print(f"⚠️  Invalid color index {color2_idx} at byte {byte_index}")
                        color2_idx = 0
                    
                    # Set pixel colors
                    img_array[y, x] = self.PALETTE[color1_idx]
                    if x + 1 < width:
                        img_array[y, x + 1] = self.PALETTE[color2_idx]
                    
                    byte_index += 1
            
            # Create PIL image and save
            img = Image.fromarray(img_array, 'RGB')
            img = img.rotate(90, expand=True)

            img.save(png_path, 'PNG', optimize=True)
            
            print(f"✅ Visualization saved to: {png_path}")
            print(f"📊 Image dimensions: {width}x{height}")
            print(f"🎨 Colors used: {len(set(np.unique(img_array.reshape(-1, 3), axis=0)))} unique colors")
            
            return png_path
            
        except Exception as e:
            print(f"❌ Error converting binary file: {e}")
            return None
    
    def analyze_binary_file(self, bin_path):
        """
        Analyze e-paper binary file and provide statistics.
        
        Args:
            bin_path: Path to binary file
            
        Returns:
            dict: Analysis results
        """
        try:
            with open(bin_path, 'rb') as f:
                binary_data = f.read()
            
            file_size = len(binary_data)
            
            # Detect resolution
            resolution = self.detect_resolution(file_size)
            
            # Count color usage
            color_counts = [0] * len(self.PALETTE)
            
            for byte_val in binary_data:
                color1_idx = (byte_val >> 4) & 0x0F
                color2_idx = byte_val & 0x0F
                
                if color1_idx < len(self.PALETTE):
                    color_counts[color1_idx] += 1
                if color2_idx < len(self.PALETTE):
                    color_counts[color2_idx] += 1
            
            total_pixels = sum(color_counts)
            
            analysis = {
                'file_size': file_size,
                'resolution': resolution,
                'total_pixels': total_pixels,
                'color_usage': {}
            }
            
            # Calculate color usage percentages
            for i, count in enumerate(color_counts):
                if count > 0:
                    percentage = (count / total_pixels) * 100
                    color_name = ['Black', 'White', 'Green', 'Blue', 'Red', 'Yellow', 'Orange', 'Purple'][i]
                    analysis['color_usage'][color_name] = {
                        'count': count,
                        'percentage': percentage,
                        'rgb': self.PALETTE[i]
                    }
            
            return analysis
            
        except Exception as e:
            print(f"❌ Error analyzing binary file: {e}")
            return None


def visualize_epaper_binary(bin_path, png_path=None):
    """
    Convenience function to visualize e-paper binary file.
    
    Args:
        bin_path: Path to binary file
        png_path: Output PNG path (optional)
        
    Returns:
        str: Path to generated PNG file, or None if failed
    """
    visualizer = EpaperVisualizer()
    return visualizer.convert_bin_to_png(bin_path, png_path)


def analyze_epaper_binary(bin_path):
    """
    Convenience function to analyze e-paper binary file.
    
    Args:
        bin_path: Path to binary file
        
    Returns:
        dict: Analysis results
    """
    visualizer = EpaperVisualizer()
    return visualizer.analyze_binary_file(bin_path)


# Test when run directly
if __name__ == "__main__":
    import sys
    
    if len(sys.argv) > 1:
        bin_file = sys.argv[1]
        
        # Analyze the binary file
        print("📊 Analyzing binary file...")
        analysis = analyze_epaper_binary(bin_file)
        
        if analysis:
            print(f"\n📈 Analysis Results:")
            print(f"  File size: {analysis['file_size']} bytes")
            if analysis['resolution']:
                print(f"  Resolution: {analysis['resolution'][0]}x{analysis['resolution'][1]}")
            else:
                print(f"  Resolution: Unknown (file size doesn't match standard resolutions)")
            print(f"  Total pixels: {analysis['total_pixels']}")
            
            print(f"\n🎨 Color Usage:")
            for color_name, info in analysis['color_usage'].items():
                print(f"  {color_name:8} {info['rgb']}: {info['count']:6} pixels ({info['percentage']:5.1f}%)")
        
        # Convert to PNG
        print(f"\n🖼️  Converting to PNG...")
        png_path = visualize_epaper_binary(bin_file)
        
        if png_path:
            print(f"\n✅ Conversion complete!")
            print(f"   Original binary: {bin_file}")
            print(f"   Visualization:   {png_path}")
        else:
            print("❌ Conversion failed")
    else:
        # Test with existing binary files
        print("Looking for binary files to visualize...")
        
        # Look for binary files in Maps folder
        maps_folder = "../Maps"
        if os.path.exists(maps_folder):
            for filename in os.listdir(maps_folder):
                if filename.endswith('.bin'):
                    bin_path = os.path.join(maps_folder, filename)
                    print(f"\nProcessing: {bin_path}")
                    
                    # Analyze
                    analysis = analyze_epaper_binary(bin_path)
                    if analysis and analysis['resolution']:
                        print(f"Resolution: {analysis['resolution'][0]}x{analysis['resolution'][1]}")
                        
                        # Visualize
                        png_path = visualize_epaper_binary(bin_path)
                        if png_path:
                            print(f"✅ Visualized: {png_path}")
                        else:
                            print("❌ Visualization failed")
                    break
        else:
            print("No Maps folder found. Please run with a binary file path as argument.")
            print("Usage: python epaper_visualizer.py <binary_file.bin>")
