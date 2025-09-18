// Enhanced HD STEPPPS Pixel with True Depth Rendering
class HDSTEPPPSPixel {
  constructor(id, x, y, z = 0) {
    this.id = id;
    this.data = {
      space: {
        x, y, z,
        neighbors: [],
        depth: z,
        worldX: x,
        worldY: y,
        worldZ: z
      },
      temporal: {
        timestamp: new Date().toISOString(),
        stateHistory: [],
        animationPhase: 0,
        lastUpdate: Date.now()
      },
      event: { lastEvent: null, eventData: {} },
      psychology: {
        emotion: "neutral",
        attentionScore: 0.5,
        energy: 0.5,
        curiosity: 0.5
      },
      pixel: {
        color: "#FFFFFF",
        brightness: 1,
        alpha: 1,
        size: 1,
        blur: 0,
        glow: 0
      },
      depth: {
        z: z,
        layer: Math.floor(z * 10),
        occlusion: 0,
        shadow: 0,
        lightness: 1 - (z * 0.3), // Further = darker
        perspective: 1 - (z * 0.1) // Further = smaller
      },
      rendering: {
        screenX: x,
        screenY: y,
        scale: 1,
        rotation: 0,
        opacity: 1,
        blendMode: 'source-over'
      },
      prompt: { text: "", llmConfig: { model: "claude", temperature: 0.7 }, lastResponse: "" },
      script: { callback: null, params: {} }
    };

    this.isHovered = false;
    this.isSelected = false;
    this.animationTime = 0;
  }

  // Update pixel with enhanced depth calculations
  update(deltaTime) {
    this.animationTime += deltaTime * 0.001;

    // Simple screen positioning first
    this.data.rendering.screenX = this.data.space.worldX;
    this.data.rendering.screenY = this.data.space.worldY;
    this.data.rendering.scale = 1.0 - (this.data.space.z * 0.1); // Slight perspective

    // Update temporal properties
    this.data.temporal.lastUpdate = Date.now();

    // Animation effects
    if (this.data.temporal.animationPhase > 0) {
      const pulse = Math.sin(this.animationTime * 6) * 0.5 + 0.5;
      this.data.pixel.glow = pulse * 0.8;
      this.data.temporal.animationPhase -= deltaTime * 0.001;
    }

    // Depth-based lighting
    this.data.depth.lightness = Math.max(0.1, 1 - (this.data.space.z * 0.4));

    // Occlusion and shadows (simplified)
    this.calculateOcclusion();
  }

  // Calculate occlusion from other pixels
  calculateOcclusion() {
    if (!hdRenderer.pixels) return;

    let occlusion = 0;
    const range = 5; // Check nearby pixels

    for (const other of hdRenderer.pixels) {
      if (other === this) continue;

      const dx = other.data.space.x - this.data.space.x;
      const dy = other.data.space.y - this.data.space.y;
      const dz = other.data.space.z - this.data.space.z;
      const dist = Math.sqrt(dx*dx + dy*dy);

      if (dist < range && dz > 0.1) {
        occlusion += (1 - dist / range) * dz * 0.3;
      }
    }

    this.data.depth.occlusion = Math.min(0.8, occlusion);
  }

  // Enhanced rendering with true depth
  render(ctx, depthMode = 'depth') {
    const x = this.data.rendering.screenX;
    const y = this.data.rendering.screenY;
    const pixelSize = Math.max(1, hdRenderer.pixelSize);

    if (x < 0 || x >= hdRenderer.canvas.width || y < 0 || y >= hdRenderer.canvas.height) return;

    ctx.save();

    // Set blend mode
    ctx.globalCompositeOperation = this.data.rendering.blendMode;

    // Calculate final color based on depth and lighting
    let finalColor = this.data.pixel.color;
    let finalAlpha = this.data.pixel.alpha * this.data.rendering.opacity;

    switch (depthMode) {
      case 'depth':
        // Apply depth-based darkening and size scaling
        finalAlpha *= this.data.depth.lightness;
        finalAlpha *= (1 - this.data.depth.occlusion);
        break;

      case 'layered':
        // Layer-based rendering with z-buffer effect
        const layerAlpha = 1 - (this.data.depth.layer * 0.1);
        finalAlpha *= Math.max(0.1, layerAlpha);
        break;

      case 'volumetric':
        // Volumetric rendering with atmospheric perspective
        const atmosFade = Math.exp(-this.data.space.z * 2);
        finalAlpha *= atmosFade;
        finalColor = this.blendColorWithAtmosphere(finalColor, this.data.space.z);
        break;
    }

    // Apply glow effect
    if (this.data.pixel.glow > 0) {
      const glowSize = pixelSize * (1 + this.data.pixel.glow * 2);
      ctx.shadowColor = finalColor;
      ctx.shadowBlur = this.data.pixel.glow * 20;
      ctx.fillStyle = finalColor;
      ctx.globalAlpha = finalAlpha * this.data.pixel.glow * 0.3;
      ctx.fillRect(x - glowSize/2, y - glowSize/2, glowSize, glowSize);
      ctx.shadowBlur = 0;
    }

    // Main pixel rendering
    ctx.globalAlpha = finalAlpha;
    ctx.fillStyle = finalColor;

    // Simple pixel rendering
    ctx.fillRect(Math.floor(x), Math.floor(y), Math.ceil(pixelSize), Math.ceil(pixelSize));

    // Selection and hover effects
    this.renderEffects(ctx, x, y, pixelSize);

    ctx.restore();
  }

  // Anti-aliased pixel rendering
  renderAntiAliased(ctx, x, y, size) {
    const gradient = ctx.createRadialGradient(x, y, 0, x, y, size/2);
    gradient.addColorStop(0, this.data.pixel.color);
    gradient.addColorStop(0.7, this.data.pixel.color);
    gradient.addColorStop(1, this.data.pixel.color + '00');

    ctx.fillStyle = gradient;
    ctx.beginPath();
    ctx.arc(x, y, size/2, 0, Math.PI * 2);
    ctx.fill();
  }

  // Render selection and hover effects
  renderEffects(ctx, x, y, size) {
    if (this.isHovered) {
      ctx.strokeStyle = "#FFFFFF";
      ctx.lineWidth = Math.max(1, size * 0.1);
      ctx.globalAlpha = 0.8;
      ctx.strokeRect(x - size/2 - 1, y - size/2 - 1, size + 2, size + 2);
    }

    if (this.isSelected) {
      ctx.strokeStyle = "#FFD700";
      ctx.lineWidth = Math.max(2, size * 0.15);
      ctx.globalAlpha = 1.0;
      ctx.strokeRect(x - size/2 - 2, y - size/2 - 2, size + 4, size + 4);
    }
  }

  // Blend color with atmospheric perspective
  blendColorWithAtmosphere(color, depth) {
    const atmosColor = '#87CEEB'; // Sky blue
    const blend = Math.min(0.7, depth);

    // Simple color blending (would be more sophisticated in real implementation)
    const hex = color.replace('#', '');
    const r = parseInt(hex.substr(0, 2), 16);
    const g = parseInt(hex.substr(2, 2), 16);
    const b = parseInt(hex.substr(4, 2), 16);

    const atmosHex = atmosColor.replace('#', '');
    const ar = parseInt(atmosHex.substr(0, 2), 16);
    const ag = parseInt(atmosHex.substr(2, 2), 16);
    const ab = parseInt(atmosHex.substr(4, 2), 16);

    const fr = Math.floor(r * (1 - blend) + ar * blend);
    const fg = Math.floor(g * (1 - blend) + ag * blend);
    const fb = Math.floor(b * (1 - blend) + ab * blend);

    return `#${fr.toString(16).padStart(2, '0')}${fg.toString(16).padStart(2, '0')}${fb.toString(16).padStart(2, '0')}`;
  }

  // Handle events with enhanced effects
  handleEvent(eventType, eventData) {
    this.data.event.lastEvent = eventType;
    this.data.event.eventData = eventData;
    this.data.temporal.stateHistory.push({
      time: new Date().toISOString(),
      state: `Event: ${eventType}`
    });

    switch(eventType) {
      case "click":
        this.data.psychology.emotion = "focused";
        this.data.psychology.attentionScore = 0.9;
        this.isSelected = true;
        this.startAnimation();
        break;
      case "hover":
        this.data.psychology.emotion = "curious";
        this.data.psychology.attentionScore = 0.7;
        this.isHovered = true;
        this.data.pixel.glow = 0.3;
        break;
      case "mouseleave":
        this.isHovered = false;
        this.data.pixel.glow = 0;
        if (!this.isSelected) {
          this.data.psychology.emotion = "neutral";
          this.data.psychology.attentionScore = 0.5;
        }
        break;
    }
  }

  // Start animation sequence
  startAnimation() {
    this.data.temporal.animationPhase = 2000; // 2 seconds
  }
}

// HD STEPPPS Renderer with True Depth
class HDSTEPPPSRenderer {
  constructor(canvasId) {
    this.canvas = document.getElementById(canvasId);
    this.ctx = this.canvas.getContext('2d');
    this.pixels = [];
    this.resolution = 128;
    this.pixelSize = 1;
    this.depthMode = 'depth';
    this.antiAliasing = true;
    this.isAnimating = true;
    this.lastTime = 0;
    this.fps = 0;
    this.frameCount = 0;
    this.depthScale = 1;

    this.stats = {
      pixelCount: 0,
      renderTime: 0,
      memoryUsage: 0,
      depthLayers: 0
    };

    this.initializeRenderer();
    this.setupEventListeners();
    this.startRenderLoop();
  }

  initializeRenderer() {
    console.log('Initializing renderer...');
    console.log(`Canvas size: ${this.canvas.width}x${this.canvas.height}`);
    console.log(`Resolution: ${this.resolution}x${this.resolution}`);

    this.updatePixelSize();
    console.log(`Pixel size: ${this.pixelSize}`);

    this.generateTestPattern();
    console.log(`HD Renderer initialized: ${this.resolution}x${this.resolution} pixels`);

    // Update stats display immediately
    const statsDiv = document.getElementById('statsDisplay');
    if (statsDiv) {
      statsDiv.textContent = `HD Canvas Initialized! Resolution: ${this.resolution}x${this.resolution}`;
      statsDiv.style.color = 'green';
    }
  }

  updatePixelSize() {
    this.pixelSize = Math.max(0.5, this.canvas.width / this.resolution);
  }

  generateTestPattern() {
    this.pixels = [];
    const centerX = this.resolution / 2;
    const centerY = this.resolution / 2;

    console.log(`Generating pattern: ${this.resolution}x${this.resolution}, canvas: ${this.canvas.width}x${this.canvas.height}`);

    for (let x = 0; x < this.resolution; x++) {
      for (let y = 0; y < this.resolution; y++) {
        // Calculate depth based on distance from center
        const dx = x - centerX;
        const dy = y - centerY;
        const distance = Math.sqrt(dx*dx + dy*dy) / (this.resolution / 2);
        const z = Math.sin(distance * Math.PI) * 0.3 * this.depthScale;

        const pixel = new HDSTEPPSPixel(`hd_${x}_${y}`, x, y, z);

        // Bright, visible colors
        const hue = (x / this.resolution) * 360;
        const sat = 80;
        const lightness = 60 - (z * 20);

        pixel.data.pixel.color = `hsl(${hue}, ${sat}%, ${lightness}%)`;
        pixel.data.pixel.brightness = 1;

        // Direct screen coordinates
        pixel.data.space.worldX = (x / this.resolution) * this.canvas.width;
        pixel.data.space.worldY = (y / this.resolution) * this.canvas.height;

        this.pixels.push(pixel);
      }
    }

    this.stats.pixelCount = this.pixels.length;
    this.stats.depthLayers = 10;
    this.updateStats();
    console.log(`Created ${this.pixels.length} pixels`);
  }

  generate3DField() {
    this.pixels = [];
    const noise = (x, y) => {
      return Math.sin(x * 0.1) * Math.cos(y * 0.1) * Math.sin((x + y) * 0.05);
    };

    for (let x = 0; x < this.resolution; x++) {
      for (let y = 0; y < this.resolution; y++) {
        const z = noise(x, y) * this.depthScale * 0.5;
        const pixel = new HDSTEPPPSPixel(`field_${x}_${y}`, x, y, z);

        // Terrain-like coloring
        if (z > 0.3) {
          pixel.data.pixel.color = '#8B4513'; // Brown (mountains)
        } else if (z > 0.1) {
          pixel.data.pixel.color = '#228B22'; // Green (hills)
        } else if (z > -0.1) {
          pixel.data.pixel.color = '#9ACD32'; // Light green (plains)
        } else {
          pixel.data.pixel.color = '#4169E1'; // Blue (water)
        }

        pixel.data.space.worldX = (x / this.resolution) * this.canvas.width;
        pixel.data.space.worldY = (y / this.resolution) * this.canvas.height;

        this.pixels.push(pixel);
      }
    }

    this.updateStats();
  }

  render(currentTime) {
    if (!this.isAnimating || !this.pixels || this.pixels.length === 0) return;

    const deltaTime = currentTime - this.lastTime;
    this.lastTime = currentTime;

    // Clear canvas with white background
    this.ctx.fillStyle = '#ffffff';
    this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);

    // Update and render pixels
    const startTime = performance.now();

    for (const pixel of this.pixels) {
      pixel.update(deltaTime);
      pixel.render(this.ctx, this.depthMode);
    }

    this.stats.renderTime = performance.now() - startTime;

    // Update FPS
    this.frameCount++;
    if (this.frameCount % 60 === 0) {
      this.fps = Math.round(60000 / (currentTime - this.lastFPSTime || currentTime));
      this.lastFPSTime = currentTime;
      this.updateStats();
    }
  }

  startRenderLoop() {
    const animate = (currentTime) => {
      this.render(currentTime);
      if (this.isAnimating) {
        requestAnimationFrame(animate);
      }
    };
    requestAnimationFrame(animate);
  }

  setupEventListeners() {
    let hoveredPixel = null;

    this.canvas.addEventListener('mousemove', (e) => {
      const rect = this.canvas.getBoundingClientRect();
      const x = Math.floor(((e.clientX - rect.left) / this.canvas.width) * this.resolution);
      const y = Math.floor(((e.clientY - rect.top) / this.canvas.height) * this.resolution);

      const pixel = this.pixels.find(p =>
        Math.floor(p.data.space.x) === x && Math.floor(p.data.space.y) === y
      );

      if (pixel !== hoveredPixel) {
        if (hoveredPixel) {
          hoveredPixel.handleEvent('mouseleave', {});
        }
        if (pixel) {
          pixel.handleEvent('hover', { x, y });
          hoveredPixel = pixel;
          this.displayPixelInfo(pixel);
        }
      }
    });

    this.canvas.addEventListener('click', (e) => {
      const rect = this.canvas.getBoundingClientRect();
      const x = Math.floor(((e.clientX - rect.left) / this.canvas.width) * this.resolution);
      const y = Math.floor(((e.clientY - rect.top) / this.canvas.height) * this.resolution);

      const pixel = this.pixels.find(p =>
        Math.floor(p.data.space.x) === x && Math.floor(p.data.space.y) === y
      );

      if (pixel) {
        // Clear other selections
        this.pixels.forEach(p => p.isSelected = false);
        pixel.handleEvent('click', { x, y });
        this.displayPixelInfo(pixel);
      }
    });
  }

  displayPixelInfo(pixel) {
    const infoDiv = document.getElementById('pixelInfo');
    const jsonDiv = document.getElementById('pixelJsonData');
    const depthMarker = document.getElementById('depthMarker');

    if (infoDiv) {
      infoDiv.innerHTML = `
        <h4>Pixel: ${pixel.id}</h4>
        <p><strong>Position:</strong> (${pixel.data.space.x.toFixed(1)}, ${pixel.data.space.y.toFixed(1)}, ${pixel.data.space.z.toFixed(3)})</p>
        <p><strong>Color:</strong> ${pixel.data.pixel.color}</p>
        <p><strong>Depth Layer:</strong> ${pixel.data.depth.layer}</p>
        <p><strong>Lightness:</strong> ${pixel.data.depth.lightness.toFixed(2)}</p>
        <p><strong>Occlusion:</strong> ${pixel.data.depth.occlusion.toFixed(2)}</p>
        <p><strong>Scale:</strong> ${pixel.data.rendering.scale.toFixed(2)}</p>
        <p><strong>Emotion:</strong> ${pixel.data.psychology.emotion}</p>
        <p><strong>Attention:</strong> ${pixel.data.psychology.attentionScore.toFixed(1)}</p>
      `;
    }

    // Display full STEPPPS JSON data
    if (jsonDiv) {
      jsonDiv.textContent = JSON.stringify(pixel.data, null, 2);
    }

    if (depthMarker) {
      const depthPercent = (pixel.data.space.z + 1) * 50; // Convert -1 to 1 range to 0-100%
      depthMarker.style.left = `${Math.max(0, Math.min(100, depthPercent))}%`;
    }
  }

  updateStats() {
    const statsDiv = document.getElementById('renderStats');
    if (statsDiv) {
      statsDiv.innerHTML = `
        Pixels: ${this.stats.pixelCount.toLocaleString()}<br>
        FPS: ${this.fps}<br>
        Depth Layers: ${this.stats.depthLayers}<br>
        Memory: ${Math.round(this.stats.pixelCount * 0.001)} KB<br>
        Render Time: ${this.stats.renderTime.toFixed(1)}ms
      `;
    }
  }

  updateResolution(newResolution) {
    this.resolution = parseInt(newResolution);
    this.updatePixelSize();
    this.generateTestPattern();
  }

  updateCanvasSize(newSize) {
    this.canvas.width = parseInt(newSize);
    this.canvas.height = parseInt(newSize);
    this.updatePixelSize();

    // Update world coordinates for all pixels
    for (const pixel of this.pixels) {
      pixel.data.space.worldX = (pixel.data.space.x / this.resolution) * this.canvas.width;
      pixel.data.space.worldY = (pixel.data.space.y / this.resolution) * this.canvas.height;
    }
  }

  exportCanvas() {
    const link = document.createElement('a');
    link.download = `steppps_hd_${this.resolution}x${this.resolution}_${Date.now()}.png`;
    link.href = this.canvas.toDataURL();
    link.click();
  }

  reset() {
    this.generateTestPattern();
  }
}

// Global renderer instance
let hdRenderer;

// UI Control Functions
function updateResolution() {
  const select = document.getElementById('resolutionSelect');
  hdRenderer.updateResolution(select.value);
  updateStatsDisplay();
}

function updateCanvasSize() {
  const select = document.getElementById('canvasSizeSelect');
  hdRenderer.updateCanvasSize(select.value);
}

function updateDepthMode() {
  const select = document.getElementById('depthModeSelect');
  hdRenderer.depthMode = select.value;
}

function updateDepthScale() {
  const slider = document.getElementById('depthScale');
  const display = document.getElementById('depthScaleValue');
  hdRenderer.depthScale = parseFloat(slider.value);
  display.textContent = slider.value;
  hdRenderer.generateTestPattern();
}

function updateAntiAliasing() {
  const checkbox = document.getElementById('antiAlias');
  hdRenderer.antiAliasing = checkbox.checked;
}

function updateBlendMode() {
  const select = document.getElementById('blendMode');
  hdRenderer.pixels.forEach(pixel => {
    pixel.data.rendering.blendMode = select.value;
  });
}

function loadTestImage() {
  hdRenderer.generateTestPattern();
}

function generateDepthField() {
  hdRenderer.generate3DField();
}

function toggleAnimation() {
  hdRenderer.isAnimating = !hdRenderer.isAnimating;
  if (hdRenderer.isAnimating) {
    hdRenderer.startRenderLoop();
  }
}

function exportCanvas() {
  hdRenderer.exportCanvas();
}

function resetCanvas() {
  hdRenderer.reset();
}

function updateStatsDisplay() {
  const statsDiv = document.getElementById('statsDisplay');
  const resolution = hdRenderer.resolution;
  const pixelCount = resolution * resolution;

  statsDiv.innerHTML = `
    Resolution: ${resolution}x${resolution} |
    Total Pixels: ${pixelCount.toLocaleString()} |
    Canvas: ${hdRenderer.canvas.width}x${hdRenderer.canvas.height}px |
    Pixel Size: ${hdRenderer.pixelSize.toFixed(2)}px
  `;
}

// Initialize when DOM is ready
document.addEventListener('DOMContentLoaded', function() {
  console.log('DOM loaded, initializing HD canvas...');

  try {
    const canvas = document.getElementById('hdCanvas');
    if (!canvas) {
      console.error('Canvas element not found!');
      return;
    }
    console.log('Canvas element found:', canvas);

    hdRenderer = new HDSTEPPPSRenderer('hdCanvas');
    updateStatsDisplay();
    console.log('🎨 HD STEPPPS Canvas initialized with true depth rendering');

    // Test basic rendering
    const ctx = canvas.getContext('2d');
    ctx.fillStyle = 'red';
    ctx.fillRect(10, 10, 50, 50);
    console.log('Test red square drawn');

  } catch (error) {
    console.error('Initialization error:', error);
    const statsDiv = document.getElementById('statsDisplay');
    if (statsDiv) {
      statsDiv.textContent = `Error: ${error.message}`;
      statsDiv.style.color = 'red';
    }
  }
});