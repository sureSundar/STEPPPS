# STEPPPS - Spatial Temporal Event Psychology Pixel Prompt Script

A revolutionary pixel-based rendering system that transforms images into interactive, psychologically-aware data structures with true depth perception.

## Overview

STEPPPS converts every pixel in an image into a comprehensive JSON data structure containing spatial, temporal, psychological, and event information. Each pixel becomes an intelligent entity with depth, emotion, attention tracking, and AI-powered interactions.

## Features

### 🎨 Live Image Processing
- **Real Image Conversion**: Upload any image and convert it to STEPPPS pixels
- **Multiple Depth Modes**: Brightness, color intensity, gradient, and edge detection
- **True Depth Rendering**: 3D perspective with atmospheric effects
- **High Resolution**: Support up to 512x512 pixels (262K+ individual STEPPPS structures)

### 🧠 AI Psychology Integration
- **Emotion Inference**: Colors mapped to psychological states (red=energetic, blue=curious)
- **Attention Scoring**: Dynamic attention values based on contrast and brightness
- **Color Psychology**: Warm/cool temperature analysis and mood inference
- **Contextual Prompts**: AI-generated prompts based on pixel context

### 📊 Complete Data Structure
Each pixel contains:
```json
{
  "space": { "x": 0, "y": 0, "z": 0.5, "neighbors": [] },
  "temporal": { "timestamp": "2024-01-01T...", "stateHistory": [] },
  "event": { "lastEvent": "click", "eventData": {} },
  "psychology": { "emotion": "curious", "attentionScore": 0.7 },
  "pixel": { "originalColor": {...}, "brightness": 0.8 },
  "depth": { "z": 0.5, "layer": 5, "lightness": 0.7 },
  "analysis": { "colorTemp": "warm", "intensity": 0.9 },
  "prompt": { "text": "AI prompt here...", "llmConfig": {} }
}
```

## Demo Files

### 1. Image STEPPPS (`image_steppps.html`)
**The main implementation** - Upload real images and see them converted to STEPPPS pixels with full depth rendering.

**Features:**
- Side-by-side original vs STEPPPS comparison
- 4 depth calculation modes
- Interactive pixel selection with JSON data display
- Drag & drop image upload
- Export functionality

### 2. Simple STEPPPS (`simple_steppps.html`)
A streamlined version showing the core concept with a 50x50 grid (2,500 pixels).

**Features:**
- Rainbow gradient demonstration
- Click-to-inspect pixel data
- Lightweight and fast rendering
- Complete STEPPPS JSON structure

### 3. HD Canvas (`steppps_hd_canvas.html`)
Advanced version with enhanced rendering features and multiple resolution options.

**Features:**
- Up to 512x512 resolution
- Multiple rendering modes (flat, depth, layered, volumetric)
- Anti-aliasing and blend modes
- Performance statistics

### 4. Original POC (`steppps_poc.html`)
The original proof-of-concept with Claude AI integration and image processing.

**Features:**
- LLM integration via FastAPI
- Image upload and conversion
- Interactive editing of pixel properties
- Real-time AI responses

## Usage

### Quick Start
1. Open `image_steppps.html` in your browser
2. Upload any image file or drag & drop
3. Watch as thousands of pixels are converted to STEPPPS structures
4. Click on any pixel to see its complete JSON data
5. Experiment with different depth modes and scales

### Advanced Usage
- **Resolution Control**: Choose from 64x64 to 512x512 pixels
- **Depth Modes**:
  - `brightness`: Darker pixels = farther away
  - `color`: Less saturated = more distant
  - `gradient`: Blue tones = atmospheric distance
  - `edges`: Sharp edges = foreground objects
- **Interactive Analysis**: Click pixels to see psychological analysis
- **Export**: Save processed images as PNG files

## Technical Implementation

### Core Classes
- `ImageSTEPPPSPixel`: Individual pixel with complete data structure
- `ImageSTEPPPSProcessor`: Image processing and conversion engine
- `HDSTEPPPSRenderer`: Advanced rendering with depth effects

### Depth Calculation Methods
1. **Brightness-based**: `depth = 1 - brightness`
2. **Color intensity**: `depth = 1 - colorIntensity`
3. **Atmospheric**: Blue channel indicates distance haze
4. **Edge detection**: Sharp contrasts indicate foreground

### Psychology Mapping
- **Red pixels** → "energetic" emotion
- **Blue pixels** → "curious" emotion
- **Green pixels** → "contemplative" emotion
- **Bright pixels** → High attention scores
- **High contrast** → Increased focus values

## Browser Compatibility
- Chrome/Chromium (Recommended)
- Firefox
- Safari
- Edge

## File Structure
```
├── image_steppps.html      # Main live image processor
├── image_steppps.js        # Core processing engine
├── simple_steppps.html     # Lightweight demo
├── steppps_hd_canvas.html  # HD rendering version
├── steppps_hd_renderer.js  # Advanced renderer
├── steppps_poc.html        # Original POC
├── steppps_poc.js          # POC implementation
└── README.md               # This file
```

## Future Development
- Real-time video processing
- Machine learning integration for emotion detection
- Collaborative multi-user pixel editing
- VR/AR visualization modes
- Cloud processing for massive images

## Contributing
This is an experimental project exploring the intersection of computer vision, psychology, and AI. Contributions and ideas welcome!

## License
MIT License - Feel free to experiment and build upon this work.

---

*STEPPPS: Where every pixel tells a story* 🎨✨