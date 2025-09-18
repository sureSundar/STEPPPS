#!/usr/bin/env python3
"""
Simple Image to STEPPPS JSON Converter
Converts JPEG/PNG images to STEPPPS format and vice versa
"""

import json
import base64
from PIL import Image
import io
from datetime import datetime
from typing import Dict, List, Tuple, Optional
import argparse
import sys

class STEPPPSConverter:
    """Convert images to STEPPPS format and back"""
    
    def __init__(self):
        self.version = "1.0.0"
    
    def image_to_steppps(self, image_path: str, analysis_mode: str = "sample", max_pixels: int = 100) -> Dict:
        """Convert image to STEPPPS JSON format"""
        try:
            # Load image
            with Image.open(image_path) as img:
                # Convert to RGBA for consistency
                img = img.convert('RGBA')
                width, height = img.size
                
                print(f"Processing image: {width}x{height} pixels")
                
                # Extract pixel data
                pixels = self._extract_pixels(img, analysis_mode, max_pixels)
                
                # Create STEPPPS structure
                steppps_data = {
                    "metadata": {
                        "version": self.version,
                        "source_type": "static_image",
                        "source_path": image_path,
                        "resolution": f"{width}x{height}",
                        "analysis_mode": analysis_mode,
                        "created_at": datetime.now().isoformat(),
                        "total_pixels": width * height,
                        "analyzed_pixels": len(pixels),
                        "format": "STEPPPS-JSON-v1.0"
                    },
                    "canvas": {
                        "width": width,
                        "height": height,
                        "color_space": "RGBA"
                    },
                    "pixels": pixels
                }
                
                return steppps_data
                
        except Exception as e:
            raise Exception(f"Failed to convert image to STEPPPS: {e}")
    
    def _extract_pixels(self, img: Image.Image, mode: str, max_pixels: int) -> Dict:
        """Extract pixel data based on analysis mode"""
        width, height = img.size
        pixels = {}
        
        if mode == "full":
            # TRUE STEPPPS: Every single pixel for perfect reconstruction
            sampled_positions = [(x, y) for x in range(width) for y in range(height)]
            
        elif mode == "grid":
            # Grid sampling
            step_x = max(1, width // int(max_pixels ** 0.5))
            step_y = max(1, height // int(max_pixels ** 0.5))
            sampled_positions = [(x, y) for x in range(0, width, step_x) for y in range(0, height, step_y)]
            
        elif mode == "edges":
            # Edge detection sampling (simple)
            sampled_positions = []
            for y in range(height):
                for x in range(width):
                    if x == 0 or x == width-1 or y == 0 or y == height-1:
                        sampled_positions.append((x, y))
                    elif len(sampled_positions) < max_pixels:
                        # Add some interior points
                        if (x + y) % 10 == 0:
                            sampled_positions.append((x, y))
            sampled_positions = sampled_positions[:max_pixels]
            
        else:  # sample - demo only
            import random
            positions = [(x, y) for x in range(width) for y in range(height)]
            sampled_positions = random.sample(positions, min(max_pixels, len(positions)))
        
        # Extract pixel data
        for x, y in sampled_positions:
            r, g, b, a = img.getpixel((x, y))
            
            pixel_data = {
                "space": {
                    "coordinates": [x, y],
                    "region": self._determine_region(x, y, width, height)
                },
                "pixel": {
                    "rgba": [r, g, b, a],
                    "hex": f"#{r:02x}{g:02x}{b:02x}",
                    "luminance": (0.299 * r + 0.587 * g + 0.114 * b) / 255,
                    "saturation": self._calculate_saturation(r, g, b)
                },
                "time": {
                    "extracted": datetime.now().isoformat()
                },
                "intelligence": {
                    "ai_analyzed": False,
                    "description": None,
                    "context": None
                }
            }
            
            pixels[f"{x},{y}"] = pixel_data
        
        return pixels
    
    def _determine_region(self, x: int, y: int, width: int, height: int) -> str:
        """Determine which region of the image the pixel belongs to"""
        third_w, third_h = width // 3, height // 3
        
        if x < third_w and y < third_h:
            return "top-left"
        elif x >= 2 * third_w and y < third_h:
            return "top-right"
        elif x < third_w and y >= 2 * third_h:
            return "bottom-left"
        elif x >= 2 * third_w and y >= 2 * third_h:
            return "bottom-right"
        elif y < third_h:
            return "top-center"
        elif y >= 2 * third_h:
            return "bottom-center"
        elif x < third_w:
            return "left-center"
        elif x >= 2 * third_w:
            return "right-center"
        else:
            return "center"
    
    def _calculate_saturation(self, r: int, g: int, b: int) -> float:
        """Calculate color saturation"""
        max_val = max(r, g, b) / 255
        min_val = min(r, g, b) / 255
        diff = max_val - min_val
        
        if diff == 0:
            return 0.0
        
        lightness = (max_val + min_val) / 2
        if lightness > 0.5:
            return diff / (2 - max_val - min_val)
        else:
            return diff / (max_val + min_val)
    
    def steppps_to_image(self, steppps_data: Dict, output_path: str) -> None:
        """Convert STEPPPS JSON back to image with improved reconstruction"""
        try:
            # Extract canvas info
            canvas = steppps_data.get("canvas", {})
            metadata = steppps_data.get("metadata", {})
            width = canvas.get("width", 800)
            height = canvas.get("height", 600)
            analysis_mode = metadata.get("analysis_mode", "sample")
            total_pixels = width * height
            analyzed_pixels = len(steppps_data.get("pixels", {}))
            
            print(f"Reconstructing {width}x{height} image from {analyzed_pixels} pixels ({analysis_mode} mode)")
            
            # Create blank image with neutral background
            img = Image.new('RGBA', (width, height), (240, 240, 240, 255))
            
            pixels_data = steppps_data.get("pixels", {})
            
            if analysis_mode == "full":
                # Perfect pixel reconstruction - no background needed
                img = Image.new('RGBA', (width, height), (0, 0, 0, 0))  # Transparent
                img = self._reconstruct_dense(img, pixels_data, width, height)
            elif analysis_mode == "sample" and analyzed_pixels < total_pixels * 0.1:
                # Sparse reconstruction with interpolation
                img = self._reconstruct_sparse(img, pixels_data, width, height)
            else:
                # Dense reconstruction
                img = self._reconstruct_dense(img, pixels_data, width, height)
            
            # Save image
            if output_path.lower().endswith('.png'):
                img.save(output_path, 'PNG')
            else:
                # Convert to RGB for JPEG
                rgb_img = Image.new('RGB', img.size, (255, 255, 255))
                rgb_img.paste(img, mask=img.split()[-1])  # Use alpha as mask
                rgb_img.save(output_path, 'JPEG', quality=95)
            
            print(f"Image saved to: {output_path}")
            
        except Exception as e:
            raise Exception(f"Failed to convert STEPPPS to image: {e}")
    
    def _reconstruct_sparse(self, img, pixels_data, width, height):
        """Reconstruct image from sparse pixel data using interpolation"""
        from PIL import ImageDraw
        
        draw = ImageDraw.Draw(img)
        brush_size = max(2, min(width, height) // 100)
        
        for coord, pixel_data in pixels_data.items():
            x, y = map(int, coord.split(','))
            rgba = tuple(pixel_data["pixel"]["rgba"])
            
            if 0 <= x < width and 0 <= y < height:
                # Draw larger circles for sparse data
                bbox = [
                    max(0, x - brush_size),
                    max(0, y - brush_size),
                    min(width - 1, x + brush_size),
                    min(height - 1, y + brush_size)
                ]
                draw.ellipse(bbox, fill=rgba)
        
        return img
    
    def _reconstruct_dense(self, img, pixels_data, width, height):
        """Reconstruct image from dense pixel data"""
        for coord, pixel_data in pixels_data.items():
            x, y = map(int, coord.split(','))
            rgba = tuple(pixel_data["pixel"]["rgba"])
            
            if 0 <= x < width and 0 <= y < height:
                img.putpixel((x, y), rgba)
        
        return img
    
    def save_steppps_json(self, steppps_data: Dict, output_path: str) -> None:
        """Save STEPPPS data to JSON file"""
        try:
            with open(output_path, 'w') as f:
                json.dump(steppps_data, f, indent=2)
            print(f"STEPPPS JSON saved to: {output_path}")
        except Exception as e:
            raise Exception(f"Failed to save STEPPPS JSON: {e}")
    
    def load_steppps_json(self, json_path: str) -> Dict:
        """Load STEPPPS data from JSON file"""
        try:
            with open(json_path, 'r') as f:
                return json.load(f)
        except Exception as e:
            raise Exception(f"Failed to load STEPPPS JSON: {e}")


def main():
    parser = argparse.ArgumentParser(description="Convert images to STEPPPS format and vice versa")
    parser.add_argument("command", choices=["to-steppps", "to-image"], help="Conversion direction")
    parser.add_argument("input", help="Input file path")
    parser.add_argument("output", help="Output file path")
    parser.add_argument("--mode", choices=["full", "grid", "edges", "sample"], default="full", 
                       help="Analysis mode for pixel extraction (full=True STEPPPS)")
    parser.add_argument("--max-pixels", type=int, default=1000000, 
                       help="Maximum number of pixels to analyze (ignored in full mode)")
    
    args = parser.parse_args()
    
    converter = STEPPPSConverter()
    
    try:
        if args.command == "to-steppps":
            print(f"Converting image to STEPPPS: {args.input}")
            steppps_data = converter.image_to_steppps(args.input, args.mode, args.max_pixels)
            
            if args.output.endswith('.json'):
                converter.save_steppps_json(steppps_data, args.output)
            else:
                converter.save_steppps_json(steppps_data, args.output + '.json')
            
            # Print summary
            metadata = steppps_data["metadata"]
            print(f"\nConversion Summary:")
            print(f"  Resolution: {metadata['resolution']}")
            print(f"  Total pixels: {metadata['total_pixels']:,}")
            print(f"  Analyzed pixels: {metadata['analyzed_pixels']:,}")
            print(f"  Analysis mode: {metadata['analysis_mode']}")
            
        elif args.command == "to-image":
            print(f"Converting STEPPPS to image: {args.input}")
            steppps_data = converter.load_steppps_json(args.input)
            converter.steppps_to_image(steppps_data, args.output)
            
            # Print summary
            metadata = steppps_data.get("metadata", {})
            print(f"\nConversion Summary:")
            print(f"  Original resolution: {metadata.get('resolution', 'unknown')}")
            print(f"  Pixels rendered: {len(steppps_data.get('pixels', {})):,}")
    
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
