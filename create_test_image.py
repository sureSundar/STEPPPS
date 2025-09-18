#!/usr/bin/env python3
"""
Create a simple test image for STEPPPS conversion
"""

from PIL import Image, ImageDraw
import os

def create_test_image():
    """Create a colorful test image"""
    # Create a 200x150 image
    width, height = 200, 150
    img = Image.new('RGB', (width, height), 'white')
    draw = ImageDraw.Draw(img)
    
    # Add colorful shapes
    # Red rectangle
    draw.rectangle([20, 20, 80, 60], fill='red')
    
    # Blue circle
    draw.ellipse([100, 30, 160, 90], fill='blue')
    
    # Green triangle (using polygon)
    draw.polygon([(50, 100), (100, 80), (150, 120)], fill='green')
    
    # Yellow rectangle
    draw.rectangle([10, 120, 190, 140], fill='yellow')
    
    # Add some gradient-like effect
    for i in range(20):
        color = (255 - i * 10, i * 10, 128)
        draw.line([(170 + i, 20), (170 + i, 70)], fill=color)
    
    # Save the test image
    output_path = '/media/sf_vboxshare/lab/STEPPPS/test_image.jpg'
    img.save(output_path, 'JPEG', quality=95)
    print(f"Test image created: {output_path}")
    return output_path

if __name__ == "__main__":
    create_test_image()
