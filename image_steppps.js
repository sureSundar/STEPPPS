// Enhanced STEPPPS Pixel for Real Image Processing
class ImageSTEPPPSPixel {
  constructor(id, x, y, imageData) {
    this.id = id;
    this.imageData = imageData;

    this.data = {
      space: {
        x, y,
        z: 0, // Will be calculated from image
        neighbors: [],
        imageSource: true
      },
      temporal: {
        timestamp: new Date().toISOString(),
        stateHistory: [],
        processingTime: 0
      },
      event: {
        lastEvent: null,
        eventData: {}
      },
      psychology: {
        emotion: "neutral",
        attentionScore: 0.5,
        colorMood: "balanced"
      },
      pixel: {
        originalColor: { r: imageData.r, g: imageData.g, b: imageData.b, a: imageData.a },
        color: `rgb(${imageData.r}, ${imageData.g}, ${imageData.b})`,
        brightness: (imageData.r + imageData.g + imageData.b) / (3 * 255),
        contrast: 0,
        saturation: 0
      },
      depth: {
        z: 0,
        layer: 0,
        depthFromBrightness: 0,
        depthFromColor: 0,
        depthFromGradient: 0,
        occlusion: 0,
        lightness: 1,
        perspective: 1
      },
      analysis: {
        colorTemp: "neutral", // warm/cool/neutral
        dominantChannel: "balanced", // r/g/b/balanced
        intensity: 0,
        edgeStrength: 0,
        localContrast: 0
      },
      prompt: {
        text: "",
        llmConfig: { model: "claude", temperature: 0.7 },
        lastResponse: ""
      },
      script: {
        callback: null,
        params: {}
      }
    };

    this.isSelected = false;
    this.calculateDepth();
    this.analyzeColor();
    this.inferPsychology();
  }

  // Calculate depth based on different image properties
  calculateDepth() {
    const { r, g, b, a } = this.imageData;

    // Method 1: Brightness-based depth
    this.data.depth.depthFromBrightness = 1 - this.data.pixel.brightness;

    // Method 2: Color intensity depth
    const colorIntensity = Math.sqrt(r*r + g*g + b*b) / Math.sqrt(3 * 255 * 255);
    this.data.depth.depthFromColor = 1 - colorIntensity;

    // Method 3: Blue channel depth (distance haze effect)
    const blueDepth = (b / 255) * 0.7 + 0.3; // Blue = farther
    this.data.depth.depthFromGradient = blueDepth;

    // Set primary depth based on current mode
    this.updateDepthMode(imageProcessor.depthMode);
  }

  updateDepthMode(mode) {
    switch (mode) {
      case 'brightness':
        this.data.depth.z = this.data.depth.depthFromBrightness;
        break;
      case 'color':
        this.data.depth.z = this.data.depth.depthFromColor;
        break;
      case 'gradient':
        this.data.depth.z = this.data.depth.depthFromGradient;
        break;
      case 'edges':
        this.data.depth.z = this.data.analysis.edgeStrength;
        break;
    }

    this.data.depth.z *= imageProcessor.depthScale;
    this.data.depth.layer = Math.floor(this.data.depth.z * 10);
    this.data.depth.lightness = Math.max(0.1, 1 - (this.data.depth.z * 0.5));
    this.data.depth.perspective = Math.max(0.5, 1 - (this.data.depth.z * 0.2));
  }

  // Analyze color properties
  analyzeColor() {
    const { r, g, b } = this.imageData;

    // Color temperature
    if (r > g && r > b) {
      this.data.analysis.colorTemp = "warm";
      this.data.analysis.dominantChannel = "r";
    } else if (b > r && b > g) {
      this.data.analysis.colorTemp = "cool";
      this.data.analysis.dominantChannel = "b";
    } else if (g > r && g > b) {
      this.data.analysis.dominantChannel = "g";
    } else {
      this.data.analysis.dominantChannel = "balanced";
    }

    // Color intensity
    this.data.analysis.intensity = Math.max(r, g, b) / 255;

    // Saturation
    const max = Math.max(r, g, b);
    const min = Math.min(r, g, b);
    this.data.pixel.saturation = max === 0 ? 0 : (max - min) / max;

    // Contrast (will be calculated relative to neighbors)
    this.data.pixel.contrast = this.data.pixel.brightness;
  }

  // Infer psychological properties from color and depth
  inferPsychology() {
    const { r, g, b } = this.imageData;
    const brightness = this.data.pixel.brightness;
    const depth = this.data.depth.z;

    // Emotion inference from color psychology
    if (r > 180 && g < 100 && b < 100) {
      this.data.psychology.emotion = "energetic"; // Red
    } else if (r < 100 && g > 180 && b < 100) {
      this.data.psychology.emotion = "contemplative"; // Green
    } else if (r < 100 && g < 100 && b > 180) {
      this.data.psychology.emotion = "curious"; // Blue
    } else if (r > 150 && g > 150 && b < 100) {
      this.data.psychology.emotion = "excited"; // Yellow
    } else if (brightness > 0.8) {
      this.data.psychology.emotion = "focused"; // Bright
    } else if (brightness < 0.3) {
      this.data.psychology.emotion = "contemplative"; // Dark
    } else {
      this.data.psychology.emotion = "neutral";
    }

    // Attention score based on contrast and brightness
    this.data.psychology.attentionScore = Math.min(1, brightness + this.data.pixel.saturation + (1 - depth));

    // Color mood
    if (this.data.analysis.colorTemp === "warm") {
      this.data.psychology.colorMood = "energetic";
    } else if (this.data.analysis.colorTemp === "cool") {
      this.data.psychology.colorMood = "calming";
    } else {
      this.data.psychology.colorMood = "balanced";
    }
  }

  // Calculate edge strength (for edge-based depth)
  calculateEdgeStrength(neighbors) {
    let edgeStrength = 0;
    const myBrightness = this.data.pixel.brightness;

    neighbors.forEach(neighbor => {
      if (neighbor) {
        const diff = Math.abs(myBrightness - neighbor.data.pixel.brightness);
        edgeStrength += diff;
      }
    });

    this.data.analysis.edgeStrength = Math.min(1, edgeStrength / 4); // Average of 4 neighbors
    this.data.analysis.localContrast = edgeStrength;
  }

  // Render pixel with depth effects
  render(ctx, pixelSize, depthMode) {
    const x = this.data.space.x * pixelSize;
    const y = this.data.space.y * pixelSize;

    // Apply depth effects to color
    let finalColor = this.data.pixel.color;
    let finalAlpha = this.data.pixel.originalColor.a / 255;

    // Depth-based modifications
    finalAlpha *= this.data.depth.lightness;

    // Atmospheric perspective for far objects
    if (this.data.depth.z > 0.5) {
      const atmosBlend = (this.data.depth.z - 0.5) * 2;
      finalColor = this.blendWithAtmosphere(finalColor, atmosBlend);
    }

    // Size scaling based on perspective
    const scaledSize = Math.max(0.5, pixelSize * this.data.depth.perspective);

    ctx.globalAlpha = finalAlpha;
    ctx.fillStyle = finalColor;

    // Render main pixel
    ctx.fillRect(x, y, scaledSize, scaledSize);

    // Selection highlight
    if (this.isSelected) {
      ctx.strokeStyle = "#FFD700";
      ctx.lineWidth = Math.max(1, pixelSize * 0.1);
      ctx.globalAlpha = 1;
      ctx.strokeRect(x - 1, y - 1, scaledSize + 2, scaledSize + 2);
    }

    // Depth visualization (optional overlay)
    if (imageProcessor.showDepthOverlay) {
      ctx.globalAlpha = 0.3;
      ctx.fillStyle = `rgba(255, 255, 255, ${this.data.depth.z})`;
      ctx.fillRect(x, y, scaledSize, scaledSize);
    }

    ctx.globalAlpha = 1;
  }

  // Blend color with atmospheric haze
  blendWithAtmosphere(color, blend) {
    const atmosColor = '#87CEEB'; // Light sky blue

    // Parse RGB
    const rgbMatch = color.match(/rgb\((\d+),\s*(\d+),\s*(\d+)\)/);
    if (!rgbMatch) return color;

    const r = parseInt(rgbMatch[1]);
    const g = parseInt(rgbMatch[2]);
    const b = parseInt(rgbMatch[3]);

    // Blend with atmosphere
    const ar = 135, ag = 206, ab = 235; // Sky blue RGB
    const fr = Math.floor(r * (1 - blend) + ar * blend);
    const fg = Math.floor(g * (1 - blend) + ag * blend);
    const fb = Math.floor(b * (1 - blend) + ab * blend);

    return `rgb(${fr}, ${fg}, ${fb})`;
  }

  // Handle click events
  handleClick() {
    this.data.psychology.emotion = "focused";
    this.data.psychology.attentionScore = 0.9;
    this.data.event.lastEvent = "click";
    this.data.temporal.stateHistory.push({
      time: new Date().toISOString(),
      state: "user_clicked",
      depth: this.data.depth.z,
      emotion: this.data.psychology.emotion
    });

    // Generate contextual prompt
    this.generateContextualPrompt();
  }

  // Generate LLM prompt based on image context
  generateContextualPrompt() {
    const color = this.data.analysis.colorTemp;
    const emotion = this.data.psychology.emotion;
    const depth = this.data.depth.z;
    const brightness = this.data.pixel.brightness;

    this.data.prompt.text = `Analyze this image pixel: ${color} color temperature, ${emotion} emotional inference, depth ${depth.toFixed(3)}, brightness ${brightness.toFixed(2)}. What does this pixel represent in the image context?`;
  }
}

// Image Processing Engine
class ImageSTEPPPSProcessor {
  constructor() {
    this.originalCanvas = document.getElementById('originalCanvas');
    this.stepppsCanvas = document.getElementById('stepppsCanvas');
    this.originalCtx = this.originalCanvas.getContext('2d');
    this.stepppsCtx = this.stepppsCanvas.getContext('2d');

    this.pixels = [];
    this.currentImage = null;
    this.resolution = 128;
    this.depthMode = 'color';
    this.depthScale = 1.0;
    this.showDepthOverlay = false;
    this.selectedPixel = null;

    this.stats = {
      pixelCount: 0,
      processTime: 0,
      memoryUsage: 0,
      depthLayers: 0
    };
  }

  // Load and process image
  async processImage(imageFile) {
    const startTime = performance.now();
    this.updateStatus('loading', 'Processing image...');

    try {
      // Load image
      const img = await this.loadImage(imageFile);
      this.currentImage = img;

      // Draw original image
      this.drawOriginalImage(img);

      // Convert to STEPPPS pixels
      await this.convertToSTEPPPS(img);

      // Render STEPPPS version
      this.renderSTEPPPS();

      // Update stats
      this.stats.processTime = performance.now() - startTime;
      this.updateStats();
      this.updateStatus('success', `✅ Processed ${this.stats.pixelCount} pixels in ${this.stats.processTime.toFixed(1)}ms`);

      console.log(`Image processed: ${this.stats.pixelCount} STEPPPS pixels created`);

    } catch (error) {
      console.error('Image processing failed:', error);
      this.updateStatus('error', `❌ Processing failed: ${error.message}`);
    }
  }

  // Load image from file or URL
  loadImage(source) {
    return new Promise((resolve, reject) => {
      const img = new Image();

      img.onload = () => resolve(img);
      img.onerror = reject;

      if (source instanceof File) {
        const reader = new FileReader();
        reader.onload = (e) => img.src = e.target.result;
        reader.onerror = reject;
        reader.readAsDataURL(source);
      } else {
        img.src = source;
      }
    });
  }

  // Draw original image to canvas
  drawOriginalImage(img) {
    // Scale to fit canvas while maintaining aspect ratio
    const canvasAspect = this.originalCanvas.width / this.originalCanvas.height;
    const imgAspect = img.width / img.height;

    let drawWidth, drawHeight, offsetX = 0, offsetY = 0;

    if (imgAspect > canvasAspect) {
      drawWidth = this.originalCanvas.width;
      drawHeight = drawWidth / imgAspect;
      offsetY = (this.originalCanvas.height - drawHeight) / 2;
    } else {
      drawHeight = this.originalCanvas.height;
      drawWidth = drawHeight * imgAspect;
      offsetX = (this.originalCanvas.width - drawWidth) / 2;
    }

    this.originalCtx.fillStyle = '#f0f0f0';
    this.originalCtx.fillRect(0, 0, this.originalCanvas.width, this.originalCanvas.height);
    this.originalCtx.drawImage(img, offsetX, offsetY, drawWidth, drawHeight);
  }

  // Convert image to STEPPPS pixel array
  async convertToSTEPPPS(img) {
    this.pixels = [];

    // Create temporary canvas for pixel sampling
    const tempCanvas = document.createElement('canvas');
    const tempCtx = tempCanvas.getContext('2d');
    tempCanvas.width = this.resolution;
    tempCanvas.height = this.resolution;

    // Draw scaled image for sampling
    tempCtx.drawImage(img, 0, 0, this.resolution, this.resolution);
    const imageData = tempCtx.getImageData(0, 0, this.resolution, this.resolution);

    // Create STEPPPS pixels
    for (let x = 0; x < this.resolution; x++) {
      for (let y = 0; y < this.resolution; y++) {
        const index = (y * this.resolution + x) * 4;
        const pixelData = {
          r: imageData.data[index],
          g: imageData.data[index + 1],
          b: imageData.data[index + 2],
          a: imageData.data[index + 3]
        };

        const pixel = new ImageSTEPPPSPixel(`img_${x}_${y}`, x, y, pixelData);
        this.pixels.push(pixel);
      }
    }

    // Calculate neighbor relationships and edge strengths
    this.calculateNeighborEffects();

    // Update stats
    this.stats.pixelCount = this.pixels.length;
    this.stats.depthLayers = Math.max(...this.pixels.map(p => p.data.depth.layer)) + 1;
    this.stats.memoryUsage = Math.round(this.pixels.length * 2); // Rough estimate in KB
  }

  // Calculate neighbor effects for edge detection and context
  calculateNeighborEffects() {
    for (let x = 0; x < this.resolution; x++) {
      for (let y = 0; y < this.resolution; y++) {
        const pixel = this.pixels.find(p => p.data.space.x === x && p.data.space.y === y);
        if (!pixel) continue;

        // Get 4-directional neighbors
        const neighbors = [
          this.pixels.find(p => p.data.space.x === x-1 && p.data.space.y === y), // Left
          this.pixels.find(p => p.data.space.x === x+1 && p.data.space.y === y), // Right
          this.pixels.find(p => p.data.space.x === x && p.data.space.y === y-1), // Up
          this.pixels.find(p => p.data.space.x === x && p.data.space.y === y+1)  // Down
        ];

        pixel.calculateEdgeStrength(neighbors);
        pixel.data.space.neighbors = neighbors.filter(n => n).map(n => n.id);
      }
    }

    // Update depth for edge-based mode
    if (this.depthMode === 'edges') {
      this.pixels.forEach(pixel => pixel.updateDepthMode('edges'));
    }
  }

  // Render STEPPPS pixels to canvas
  renderSTEPPPS() {
    const pixelSize = this.stepppsCanvas.width / this.resolution;

    // Clear canvas
    this.stepppsCtx.fillStyle = '#ffffff';
    this.stepppsCtx.fillRect(0, 0, this.stepppsCanvas.width, this.stepppsCanvas.height);

    // Sort by depth for proper rendering order
    const sortedPixels = [...this.pixels].sort((a, b) => b.data.depth.z - a.data.depth.z);

    // Render each pixel
    sortedPixels.forEach(pixel => {
      pixel.render(this.stepppsCtx, pixelSize, this.depthMode);
    });
  }

  // Handle pixel selection
  selectPixel(x, y) {
    const pixelX = Math.floor((x / this.stepppsCanvas.width) * this.resolution);
    const pixelY = Math.floor((y / this.stepppsCanvas.height) * this.resolution);

    const pixel = this.pixels.find(p =>
      p.data.space.x === pixelX && p.data.space.y === pixelY
    );

    if (pixel) {
      // Clear previous selection
      if (this.selectedPixel) {
        this.selectedPixel.isSelected = false;
      }

      // Select new pixel
      pixel.isSelected = true;
      pixel.handleClick();
      this.selectedPixel = pixel;

      // Update UI
      this.displayPixelInfo(pixel);
      this.renderSTEPPPS(); // Re-render to show selection
    }
  }

  // Display pixel information
  displayPixelInfo(pixel) {
    const info = document.getElementById('pixelInfo');
    const data = document.getElementById('pixelData');
    const depthIndicator = document.getElementById('depthIndicator');

    // Update pixel info display
    info.innerHTML = `
      <h4>Pixel: ${pixel.id}</h4>
      <p><strong>Position:</strong> (${pixel.data.space.x}, ${pixel.data.space.y})</p>
      <p><strong>Original Color:</strong> rgb(${pixel.data.pixel.originalColor.r}, ${pixel.data.pixel.originalColor.g}, ${pixel.data.pixel.originalColor.b})</p>
      <p><strong>Depth:</strong> ${pixel.data.depth.z.toFixed(3)} (Layer ${pixel.data.depth.layer})</p>
      <p><strong>Brightness:</strong> ${pixel.data.pixel.brightness.toFixed(2)}</p>
      <p><strong>Emotion:</strong> ${pixel.data.psychology.emotion}</p>
      <p><strong>Color Temp:</strong> ${pixel.data.analysis.colorTemp}</p>
      <p><strong>Attention:</strong> ${pixel.data.psychology.attentionScore.toFixed(2)}</p>
      <p><strong>Neighbors:</strong> ${pixel.data.space.neighbors.length}</p>
    `;

    // Update JSON data
    data.textContent = JSON.stringify(pixel.data, null, 2);

    // Update depth indicator
    if (depthIndicator) {
      const depthPercent = pixel.data.depth.z * 100;
      depthIndicator.style.left = `${Math.min(100, Math.max(0, depthPercent))}%`;
    }
  }

  // Update processing mode
  updateDepthMode(mode) {
    this.depthMode = mode;
    this.pixels.forEach(pixel => pixel.updateDepthMode(mode));
    if (mode === 'edges') {
      this.calculateNeighborEffects();
    }
    this.renderSTEPPPS();
  }

  updateDepthScale(scale) {
    this.depthScale = scale;
    this.pixels.forEach(pixel => {
      pixel.updateDepthMode(this.depthMode);
    });
    this.renderSTEPPPS();
  }

  updateResolution(resolution) {
    this.resolution = parseInt(resolution);
    if (this.currentImage) {
      this.processImage(this.currentImage);
    }
  }

  // Load sample image
  async loadSampleImage() {
    // Create a sample procedural image
    const canvas = document.createElement('canvas');
    const ctx = canvas.getContext('2d');
    canvas.width = 300;
    canvas.height = 300;

    // Draw sample scene
    const gradient = ctx.createRadialGradient(150, 150, 0, 150, 150, 150);
    gradient.addColorStop(0, '#ffeb3b');
    gradient.addColorStop(0.3, '#ff9800');
    gradient.addColorStop(0.6, '#f44336');
    gradient.addColorStop(1, '#3f51b5');

    ctx.fillStyle = gradient;
    ctx.fillRect(0, 0, 300, 300);

    // Add some shapes for depth variation
    ctx.fillStyle = '#4caf50';
    ctx.fillRect(50, 50, 100, 100);

    ctx.fillStyle = '#2196f3';
    ctx.beginPath();
    ctx.arc(200, 200, 50, 0, Math.PI * 2);
    ctx.fill();

    canvas.toBlob(blob => {
      this.processImage(blob);
    });
  }

  // Update status display
  updateStatus(type, message) {
    const status = document.getElementById('status');
    status.className = `status ${type}`;
    status.textContent = message;
  }

  // Update statistics display
  updateStats() {
    document.getElementById('pixelCount').textContent = this.stats.pixelCount.toLocaleString();
    document.getElementById('depthLayers').textContent = this.stats.depthLayers;
    document.getElementById('processTime').textContent = `${this.stats.processTime.toFixed(1)}ms`;
    document.getElementById('memoryUsage').textContent = `${this.stats.memoryUsage} KB`;
  }

  // Export canvas as PNG
  exportCanvas() {
    const link = document.createElement('a');
    link.download = `steppps_image_${this.resolution}x${this.resolution}_${Date.now()}.png`;
    link.href = this.stepppsCanvas.toDataURL();
    link.click();
  }
}

// Global processor instance
let imageProcessor;

// UI Event Handlers
function processImage() {
  if (imageProcessor.currentImage) {
    imageProcessor.processImage(imageProcessor.currentImage);
  }
}

function exportCanvas() {
  imageProcessor.exportCanvas();
}

function loadSampleImage() {
  imageProcessor.loadSampleImage();
}

// Initialize when DOM loads
document.addEventListener('DOMContentLoaded', function() {
  imageProcessor = new ImageSTEPPPSProcessor();

  // File input handler
  document.getElementById('imageInput').addEventListener('change', (e) => {
    const file = e.target.files[0];
    if (file) {
      imageProcessor.processImage(file);
    }
  });

  // Drag and drop
  const dropZone = document.getElementById('dropZone');

  dropZone.addEventListener('dragover', (e) => {
    e.preventDefault();
    dropZone.classList.add('dragover');
  });

  dropZone.addEventListener('dragleave', () => {
    dropZone.classList.remove('dragover');
  });

  dropZone.addEventListener('drop', (e) => {
    e.preventDefault();
    dropZone.classList.remove('dragover');

    const file = e.dataTransfer.files[0];
    if (file && file.type.startsWith('image/')) {
      imageProcessor.processImage(file);
    }
  });

  // Control handlers
  document.getElementById('resolution').addEventListener('change', (e) => {
    imageProcessor.updateResolution(e.target.value);
  });

  document.getElementById('depthMode').addEventListener('change', (e) => {
    imageProcessor.updateDepthMode(e.target.value);
  });

  document.getElementById('depthScale').addEventListener('input', (e) => {
    document.getElementById('depthValue').textContent = e.target.value;
    imageProcessor.updateDepthScale(parseFloat(e.target.value));
  });

  // Canvas click handler
  document.getElementById('stepppsCanvas').addEventListener('click', (e) => {
    const rect = e.target.getBoundingClientRect();
    const x = e.clientX - rect.left;
    const y = e.clientY - rect.top;
    imageProcessor.selectPixel(x, y);
  });

  console.log('🎨 Image STEPPPS Processor initialized');
});