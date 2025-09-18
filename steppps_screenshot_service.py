#!/usr/bin/env python3
"""
STEPPPS Screenshot Service - Clean, modular screenshot capture with SOLID principles
Follows OOAD, KISS, DRY, SOLID design patterns
"""

from fastapi import FastAPI, HTTPException, BackgroundTasks
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import FileResponse
from pydantic import BaseModel, HttpUrl
from typing import Dict, List, Optional, Any, Tuple
import asyncio
import json
import base64
from datetime import datetime
import uuid
from pathlib import Path
import logging

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# === MODELS (Single Responsibility) ===

class ScreenshotRequest(BaseModel):
    url: HttpUrl
    width: int = 1024
    height: int = 768
    wait_time: int = 2000  # milliseconds

class STEPPPSPixel(BaseModel):
    coordinates: Tuple[int, int]
    color: str
    rgba: Tuple[int, int, int, int]
    luminance: float
    region: str
    ai_analyzed: bool = False
    ai_response: Optional[str] = None

class STEPPPSData(BaseModel):
    id: str
    source_url: str
    resolution: str
    created_at: datetime
    total_pixels: int
    analyzed_pixels: int
    pixels: Dict[str, STEPPPSPixel]

class ConversionRequest(BaseModel):
    screenshot_id: str
    analysis_mode: str = "sample"  # sample, grid, full
    api_key: Optional[str] = None

# === INTERFACES (Interface Segregation) ===

class ScreenshotCapture:
    """Interface for screenshot capture services"""
    async def capture(self, url: str, width: int, height: int) -> bytes:
        raise NotImplementedError

class PixelAnalyzer:
    """Interface for pixel analysis services"""
    async def analyze_pixel(self, pixel_data: dict, context: dict) -> str:
        raise NotImplementedError

class DataStorage:
    """Interface for data storage services"""
    async def save(self, data: Any, key: str) -> str:
        raise NotImplementedError

    async def load(self, key: str) -> Any:
        raise NotImplementedError

# === CONCRETE IMPLEMENTATIONS (Single Responsibility) ===

class DemoScreenshotCapture(ScreenshotCapture):
    """Demo implementation - creates sample screenshots"""

    async def capture(self, url: str, width: int, height: int) -> bytes:
        """Create demo screenshot data"""
        try:
            # For demo, create a simple pattern
            # In production, use playwright/selenium
            import io
            from PIL import Image, ImageDraw, ImageFont

            # Create image
            img = Image.new('RGB', (width, height), color='white')
            draw = ImageDraw.Draw(img)

            # Add gradient background
            for y in range(height):
                color_val = int(255 * (y / height))
                draw.line([(0, y), (width, y)], fill=(color_val, 100, 255 - color_val))

            # Add demo elements
            draw.rectangle([50, 50, 200, 100], fill='red')
            draw.rectangle([250, 150, 400, 200], fill='blue')
            draw.ellipse([500, 100, 600, 200], fill='green')

            # Add URL text
            try:
                font = ImageFont.load_default()
                draw.text((width//2 - 100, height//2), f"DEMO: {url}", fill='black', font=font)
            except:
                draw.text((width//2 - 100, height//2), f"DEMO: {url}", fill='black')

            # Convert to bytes
            buffer = io.BytesIO()
            img.save(buffer, format='PNG')
            return buffer.getvalue()

        except Exception as e:
            logger.error(f"Screenshot capture failed: {e}")
            raise HTTPException(status_code=500, detail=f"Capture failed: {e}")

class ClaudePixelAnalyzer(PixelAnalyzer):
    """Claude AI pixel analysis implementation"""

    def __init__(self, claude_service):
        self.claude_service = claude_service

    async def analyze_pixel(self, pixel_data: dict, context: dict) -> str:
        """Analyze pixel using Claude AI"""
        prompt = f"""STEPPPS Pixel Intelligence Analysis:

Position: ({pixel_data['x']}, {pixel_data['y']}) - {pixel_data['region']} region
Color: {pixel_data['color']}
Luminance: {pixel_data['luminance']:.1%}
Context: Screenshot from {context.get('source_url', 'unknown')}

As an intelligent pixel, what visual element or content do you likely represent in this webpage?
Respond as the pixel in 1-2 sentences."""

        try:
            return await self.claude_service.query(prompt, max_tokens=80)
        except Exception as e:
            logger.warning(f"AI analysis failed: {e}")
            return "AI analysis unavailable"

class InMemoryStorage(DataStorage):
    """Simple in-memory storage for demo"""

    def __init__(self):
        self.storage: Dict[str, Any] = {}

    async def save(self, data: Any, key: str) -> str:
        self.storage[key] = data
        logger.info(f"Saved data with key: {key}")
        return key

    async def load(self, key: str) -> Any:
        if key not in self.storage:
            raise HTTPException(status_code=404, detail="Data not found")
        return self.storage[key]

# === SERVICES (Business Logic) ===

class STEPPPSConverter:
    """Main STEPPPS conversion service - follows Single Responsibility"""

    def __init__(self, screenshot_service: ScreenshotCapture,
                 pixel_analyzer: PixelAnalyzer,
                 storage: DataStorage):
        self.screenshot_service = screenshot_service
        self.pixel_analyzer = pixel_analyzer
        self.storage = storage

    async def capture_screenshot(self, request: ScreenshotRequest) -> str:
        """Capture screenshot and store"""
        screenshot_id = str(uuid.uuid4())

        # Capture screenshot
        image_data = await self.screenshot_service.capture(
            str(request.url), request.width, request.height
        )

        # Store screenshot
        await self.storage.save(image_data, f"screenshot_{screenshot_id}")

        # Store metadata
        metadata = {
            "id": screenshot_id,
            "url": str(request.url),
            "width": request.width,
            "height": request.height,
            "created_at": datetime.now().isoformat(),
            "size": len(image_data)
        }
        await self.storage.save(metadata, f"metadata_{screenshot_id}")

        logger.info(f"Screenshot captured: {screenshot_id}")
        return screenshot_id

    async def convert_to_steppps(self, request: ConversionRequest) -> STEPPPSData:
        """Convert screenshot to STEPPPS format"""
        # Load screenshot and metadata
        screenshot_data = await self.storage.load(f"screenshot_{request.screenshot_id}")
        metadata = await self.storage.load(f"metadata_{request.screenshot_id}")

        # Convert image to pixel array
        pixel_array = await self._extract_pixels(screenshot_data)

        # Sample pixels based on analysis mode
        pixels_to_analyze = self._sample_pixels(pixel_array, request.analysis_mode)

        # Create STEPPPS structure
        steppps_data = STEPPPSData(
            id=str(uuid.uuid4()),
            source_url=metadata["url"],
            resolution=f"{metadata['width']}x{metadata['height']}",
            created_at=datetime.now(),
            total_pixels=len(pixel_array),
            analyzed_pixels=len(pixels_to_analyze),
            pixels={}
        )

        # Analyze pixels
        for pixel_data in pixels_to_analyze:
            steppps_pixel = await self._analyze_pixel(pixel_data, metadata, request.api_key)
            coord_key = f"{pixel_data['x']},{pixel_data['y']}"
            steppps_data.pixels[coord_key] = steppps_pixel

        # Store STEPPPS data
        await self.storage.save(steppps_data.dict(), f"steppps_{steppps_data.id}")

        logger.info(f"STEPPPS conversion complete: {steppps_data.id}")
        return steppps_data

    async def _extract_pixels(self, image_data: bytes) -> List[Dict]:
        """Extract pixel data from image"""
        try:
            from PIL import Image
            import io

            img = Image.open(io.BytesIO(image_data)).convert('RGBA')
            width, height = img.size

            pixels = []
            for y in range(height):
                for x in range(width):
                    r, g, b, a = img.getpixel((x, y))

                    pixels.append({
                        'x': x, 'y': y,
                        'r': r, 'g': g, 'b': b, 'a': a,
                        'color': f'rgba({r},{g},{b},{a/255:.2f})',
                        'luminance': (0.299 * r + 0.587 * g + 0.114 * b) / 255,
                        'region': self._determine_region(x, y, width, height)
                    })

            return pixels

        except Exception as e:
            logger.error(f"Pixel extraction failed: {e}")
            raise HTTPException(status_code=500, detail="Pixel extraction failed")

    def _sample_pixels(self, pixels: List[Dict], mode: str) -> List[Dict]:
        """Sample pixels based on analysis mode - DRY principle"""
        import random

        total_pixels = len(pixels)

        if mode == "sample":
            # Random sample of 50 pixels
            return random.sample(pixels, min(50, total_pixels))
        elif mode == "grid":
            # Every 20th pixel in a grid
            return [p for i, p in enumerate(pixels) if i % 20 == 0]
        elif mode == "full":
            # All pixels (expensive)
            return pixels
        else:
            raise HTTPException(status_code=400, detail="Invalid analysis mode")

    async def _analyze_pixel(self, pixel_data: Dict, metadata: Dict, api_key: Optional[str]) -> STEPPPSPixel:
        """Convert pixel data to STEPPPS format"""
        steppps_pixel = STEPPPSPixel(
            coordinates=(pixel_data['x'], pixel_data['y']),
            color=pixel_data['color'],
            rgba=(pixel_data['r'], pixel_data['g'], pixel_data['b'], pixel_data['a']),
            luminance=pixel_data['luminance'],
            region=pixel_data['region']
        )

        # Add AI analysis if API key provided
        if api_key and api_key.startswith('sk-ant-'):
            try:
                ai_response = await self.pixel_analyzer.analyze_pixel(
                    pixel_data, {"source_url": metadata["url"]}
                )
                steppps_pixel.ai_analyzed = True
                steppps_pixel.ai_response = ai_response
            except Exception as e:
                logger.warning(f"AI analysis failed for pixel: {e}")

        return steppps_pixel

    def _determine_region(self, x: int, y: int, width: int, height: int) -> str:
        """Determine pixel region - extracted for reusability (DRY)"""
        quad_w, quad_h = width // 3, height // 3

        if x < quad_w and y < quad_h: return 'top-left'
        if x > quad_w * 2 and y < quad_h: return 'top-right'
        if x < quad_w and y > quad_h * 2: return 'bottom-left'
        if x > quad_w * 2 and y > quad_h * 2: return 'bottom-right'
        if y < quad_h: return 'top-center'
        if y > quad_h * 2: return 'bottom-center'
        if x < quad_w: return 'left-center'
        if x > quad_w * 2: return 'right-center'
        return 'center'

# === DEPENDENCY INJECTION (Dependency Inversion Principle) ===

class ClaudeService:
    """Wrapper for Claude API calls"""

    def __init__(self, claude_proxy_url: str):
        self.claude_proxy_url = claude_proxy_url

    async def query(self, prompt: str, max_tokens: int = 100) -> str:
        """Query Claude via proxy - simplified for demo"""
        # In production, make actual HTTP request to Claude proxy
        return f"AI analysis: {prompt[:50]}... [Demo response]"

# === APPLICATION SETUP (Clean Architecture) ===

def create_app() -> FastAPI:
    """Factory pattern for app creation"""
    app = FastAPI(title="STEPPPS Screenshot Service", version="2.0.0")

    # Add CORS
    app.add_middleware(
        CORSMiddleware,
        allow_origins=["*"],
        allow_credentials=True,
        allow_methods=["*"],
        allow_headers=["*"],
    )

    # Dependency injection
    storage = InMemoryStorage()
    screenshot_service = DemoScreenshotCapture()
    claude_service = ClaudeService("/api/claude")
    pixel_analyzer = ClaudePixelAnalyzer(claude_service)

    converter = STEPPPSConverter(screenshot_service, pixel_analyzer, storage)

    # Store dependencies in app state
    app.state.converter = converter
    app.state.storage = storage

    return app

# === API ENDPOINTS (Interface Layer) ===

app = create_app()

@app.get("/")
async def root():
    """Serve the URL converter interface"""
    return FileResponse("steppps_url_converter.html")

@app.post("/api/screenshot")
async def capture_screenshot(request: ScreenshotRequest):
    """Capture screenshot of URL"""
    try:
        screenshot_id = await app.state.converter.capture_screenshot(request)
        return {
            "success": True,
            "screenshot_id": screenshot_id,
            "message": "Screenshot captured successfully"
        }
    except Exception as e:
        logger.error(f"Screenshot capture failed: {e}")
        raise HTTPException(status_code=500, detail=str(e))

@app.post("/api/convert")
async def convert_to_steppps(request: ConversionRequest):
    """Convert screenshot to STEPPPS format"""
    try:
        steppps_data = await app.state.converter.convert_to_steppps(request)
        return {
            "success": True,
            "steppps_data": steppps_data.dict(),
            "message": "Conversion completed successfully"
        }
    except Exception as e:
        logger.error(f"Conversion failed: {e}")
        raise HTTPException(status_code=500, detail=str(e))

@app.get("/api/steppps/{steppps_id}")
async def get_steppps_data(steppps_id: str):
    """Retrieve STEPPPS data"""
    try:
        data = await app.state.storage.load(f"steppps_{steppps_id}")
        return {"success": True, "data": data}
    except Exception as e:
        raise HTTPException(status_code=404, detail="STEPPPS data not found")

@app.get("/health")
async def health_check():
    """Health check endpoint"""
    return {
        "status": "healthy",
        "service": "STEPPPS Screenshot Service",
        "version": "2.0.0",
        "timestamp": datetime.now().isoformat()
    }

if __name__ == "__main__":
    import uvicorn
    print("🚀 Starting STEPPPS Screenshot Service...")
    print("🌐 URL Converter: http://localhost:8003/")
    print("📸 Screenshot API: http://localhost:8003/api/screenshot")
    print("🧠 Conversion API: http://localhost:8003/api/convert")
    print("🛑 Press Ctrl+C to stop")

    uvicorn.run(app, host="0.0.0.0", port=8003, log_level="info")