// STEPPPS Pixel class for AI-generated scenes
class AISTEPPPSPixel {
  constructor(data) {
    this.data = {
      space: {
        x: data.x || 0,
        y: data.y || 0,
        z: data.z || 0,
        neighbors: [],
        element: data.element || "unknown"
      },
      temporal: {
        timestamp: new Date().toISOString(),
        stateHistory: [],
        birthPrompt: data.birthPrompt || ""
      },
      event: {
        lastEvent: null,
        eventData: {}
      },
      psychology: {
        emotion: data.emotion || "neutral",
        attentionScore: data.attention || 0.5,
        mood: data.mood || "balanced",
        energy: data.energy || 0.5
      },
      pixel: {
        color: data.color || "#FFFFFF",
        brightness: data.brightness || 1,
        saturation: data.saturation || 0.5,
        temperature: data.temperature || "neutral"
      },
      depth: {
        z: data.z || 0,
        layer: Math.floor((data.z || 0) * 10),
        perspective: data.perspective || 1,
        atmospheric: data.atmospheric || 0
      },
      semantics: {
        meaning: data.meaning || "pixel",
        role: data.role || "background",
        importance: data.importance || 0.5,
        context: data.context || ""
      },
      ai: {
        reasoning: data.reasoning || "",
        confidence: data.confidence || 0.5,
        alternatives: data.alternatives || [],
        prompt: data.prompt || ""
      },
      prompt: {
        text: data.promptText || "",
        llmConfig: { model: "claude", temperature: 0.7 },
        lastResponse: ""
      },
      script: {
        callback: null,
        params: {}
      }
    };

    this.isSelected = false;
    this.isHovered = false;
  }

  // Render pixel with AI-informed properties and smooth anti-aliasing
  render(ctx, pixelSize) {
    const x = this.data.space.x * pixelSize;
    const y = this.data.space.y * pixelSize;

    // Apply depth and atmospheric effects with color validation
    let finalColor = this.validateColor(this.data.pixel.color);
    let finalAlpha = Math.max(0, Math.min(1, this.data.pixel.brightness || 1));

    // Atmospheric perspective
    if (this.data.depth.atmospheric > 0) {
      finalColor = this.applyAtmosphericHaze(finalColor, this.data.depth.atmospheric);
      finalAlpha *= (1 - this.data.depth.atmospheric * 0.3);
    }

    // Perspective scaling
    const scale = this.data.depth.perspective;
    const scaledSize = Math.max(0.5, pixelSize * scale);

    // Save context for restoration
    ctx.save();

    // Enable high-quality smoothing
    ctx.imageSmoothingEnabled = true;
    ctx.imageSmoothingQuality = 'high';

    // Set alpha for main pixel
    ctx.globalAlpha = finalAlpha;
    ctx.fillStyle = finalColor;

    // Render smooth pixel based on size
    if (scaledSize >= 4) {
      // For larger pixels, use rounded rectangles for smoother appearance
      this.drawRoundedRect(ctx, x, y, scaledSize, scaledSize, Math.min(scaledSize * 0.1, 2));
    } else if (scaledSize >= 2) {
      // For medium pixels, use anti-aliased circles
      this.drawSmoothCircle(ctx, x + scaledSize/2, y + scaledSize/2, scaledSize/2);
    } else {
      // For very small pixels, use sub-pixel positioning
      this.drawSubPixel(ctx, x, y, scaledSize);
    }

    // Importance glow effect with smooth gradient
    if (this.data.semantics.importance > 0.7) {
      const glowRadius = scaledSize * (1 + this.data.semantics.importance * 0.5);
      this.drawGlowEffect(ctx, x + scaledSize/2, y + scaledSize/2, glowRadius, finalColor, finalAlpha * 0.3);
    }

    // Selection highlight with smooth outline
    if (this.isSelected) {
      ctx.strokeStyle = "#FFD700";
      ctx.lineWidth = Math.max(1, pixelSize * 0.08);
      ctx.globalAlpha = 0.9;
      ctx.lineCap = 'round';
      ctx.lineJoin = 'round';
      this.drawRoundedRect(ctx, x - 2, y - 2, scaledSize + 4, scaledSize + 4, 3, true);
    }

    // Hover effect with subtle glow
    if (this.isHovered) {
      ctx.strokeStyle = "#FFFFFF";
      ctx.lineWidth = 0.8;
      ctx.globalAlpha = 0.6;
      ctx.lineCap = 'round';
      this.drawRoundedRect(ctx, x - 1, y - 1, scaledSize + 2, scaledSize + 2, 2, true);
    }

    // Restore context
    ctx.restore();
  }

  // Draw rounded rectangle for smooth appearance
  drawRoundedRect(ctx, x, y, width, height, radius, strokeOnly = false) {
    ctx.beginPath();
    ctx.moveTo(x + radius, y);
    ctx.lineTo(x + width - radius, y);
    ctx.quadraticCurveTo(x + width, y, x + width, y + radius);
    ctx.lineTo(x + width, y + height - radius);
    ctx.quadraticCurveTo(x + width, y + height, x + width - radius, y + height);
    ctx.lineTo(x + radius, y + height);
    ctx.quadraticCurveTo(x, y + height, x, y + height - radius);
    ctx.lineTo(x, y + radius);
    ctx.quadraticCurveTo(x, y, x + radius, y);
    ctx.closePath();

    if (strokeOnly) {
      ctx.stroke();
    } else {
      ctx.fill();
    }
  }

  // Draw smooth anti-aliased circle
  drawSmoothCircle(ctx, centerX, centerY, radius) {
    ctx.beginPath();
    ctx.arc(centerX, centerY, radius, 0, 2 * Math.PI);
    ctx.fill();
  }

  // Draw sub-pixel positioned small pixel
  drawSubPixel(ctx, x, y, size) {
    // Use sub-pixel positioning for smoother small pixels
    const offsetX = (size % 1) * 0.5;
    const offsetY = (size % 1) * 0.5;

    ctx.beginPath();
    ctx.arc(x + offsetX + size/2, y + offsetY + size/2, Math.max(0.3, size/2), 0, 2 * Math.PI);
    ctx.fill();
  }

  // Draw smooth glow effect with radial gradient
  drawGlowEffect(ctx, centerX, centerY, radius, color, alpha) {
    const gradient = ctx.createRadialGradient(centerX, centerY, 0, centerX, centerY, radius);
    gradient.addColorStop(0, color);
    gradient.addColorStop(0.5, this.addAlphaToColor(color, alpha * 0.5));
    gradient.addColorStop(1, this.addAlphaToColor(color, 0));

    ctx.save();
    ctx.globalCompositeOperation = 'screen';
    ctx.fillStyle = gradient;
    ctx.beginPath();
    ctx.arc(centerX, centerY, radius, 0, 2 * Math.PI);
    ctx.fill();
    ctx.restore();
  }

  // Add alpha channel to hex color with validation
  addAlphaToColor(hexColor, alpha) {
    // Validate hex color format
    if (!hexColor || typeof hexColor !== 'string' || !hexColor.match(/^#[0-9A-Fa-f]{6}$/)) {
      console.warn('Invalid hex color:', hexColor, 'using fallback #FFFFFF');
      hexColor = '#FFFFFF';
    }

    // Ensure alpha is valid
    alpha = Math.max(0, Math.min(1, alpha || 0));

    const r = parseInt(hexColor.slice(1, 3), 16);
    const g = parseInt(hexColor.slice(3, 5), 16);
    const b = parseInt(hexColor.slice(5, 7), 16);

    // Validate parsed values
    if (isNaN(r) || isNaN(g) || isNaN(b)) {
      console.warn('Failed to parse color components from:', hexColor);
      return `rgba(255, 255, 255, ${alpha})`;
    }

    return `rgba(${r}, ${g}, ${b}, ${alpha})`;
  }

  // Validate and fix color values
  validateColor(color) {
    if (!color || typeof color !== 'string') {
      return '#FFFFFF';
    }

    // Handle various color formats
    if (color.startsWith('#')) {
      // Hex color validation
      if (color.match(/^#[0-9A-Fa-f]{6}$/)) {
        return color;
      } else if (color.match(/^#[0-9A-Fa-f]{3}$/)) {
        // Convert 3-digit hex to 6-digit
        const r = color[1];
        const g = color[2];
        const b = color[3];
        return `#${r}${r}${g}${g}${b}${b}`;
      }
    } else if (color.startsWith('rgb')) {
      // RGB/RGBA format - return as is if valid
      if (color.match(/^rgba?\(\s*\d+\s*,\s*\d+\s*,\s*\d+\s*(?:,\s*[\d.]+\s*)?\)$/)) {
        return color;
      }
    } else if (color.startsWith('hsl')) {
      // HSL format - return as is if valid
      if (color.match(/^hsla?\(\s*\d+\s*,\s*\d+%\s*,\s*\d+%\s*(?:,\s*[\d.]+\s*)?\)$/)) {
        return color;
      }
    }

    // Fallback for invalid colors
    console.warn('Invalid color format:', color, 'using fallback #FFFFFF');
    return '#FFFFFF';
  }

  // Apply atmospheric haze effect
  applyAtmosphericHaze(color, hazeAmount) {
    const hazeColor = '#87CEEB'; // Sky blue
    return this.blendColors(color, hazeColor, hazeAmount);
  }

  // Blend two colors with validation
  blendColors(color1, color2, blend) {
    // Validate input colors
    color1 = this.validateColor(color1);
    color2 = this.validateColor(color2);
    blend = Math.max(0, Math.min(1, blend || 0));

    // Convert to hex if needed
    const hex1 = color1.startsWith('#') ? color1.replace('#', '') : 'FFFFFF';
    const hex2 = color2.startsWith('#') ? color2.replace('#', '') : '87CEEB';

    // Ensure hex colors are 6 characters
    const normalizedHex1 = hex1.length === 6 ? hex1 : 'FFFFFF';
    const normalizedHex2 = hex2.length === 6 ? hex2 : '87CEEB';

    const r1 = parseInt(normalizedHex1.substr(0, 2), 16);
    const g1 = parseInt(normalizedHex1.substr(2, 2), 16);
    const b1 = parseInt(normalizedHex1.substr(4, 2), 16);

    const r2 = parseInt(normalizedHex2.substr(0, 2), 16);
    const g2 = parseInt(normalizedHex2.substr(2, 2), 16);
    const b2 = parseInt(normalizedHex2.substr(4, 2), 16);

    // Validate parsed values
    if (isNaN(r1) || isNaN(g1) || isNaN(b1) || isNaN(r2) || isNaN(g2) || isNaN(b2)) {
      console.warn('Color blending failed, using fallback');
      return '#FFFFFF';
    }

    const r = Math.floor(r1 * (1 - blend) + r2 * blend);
    const g = Math.floor(g1 * (1 - blend) + g2 * blend);
    const b = Math.floor(b1 * (1 - blend) + b2 * blend);

    return `#${r.toString(16).padStart(2, '0')}${g.toString(16).padStart(2, '0')}${b.toString(16).padStart(2, '0')}`;
  }

  // Handle click events
  handleClick() {
    this.data.psychology.emotion = "focused";
    this.data.psychology.attentionScore = 0.9;
    this.data.event.lastEvent = "click";
    this.data.temporal.stateHistory.push({
      time: new Date().toISOString(),
      state: "user_clicked",
      context: this.data.semantics.meaning
    });
  }
}

// AI STEPPPS Generator and Renderer
class AISTEPPPSGenerator {
  constructor() {
    this.canvas = document.getElementById('generatedCanvas');
    this.ctx = this.canvas.getContext('2d');
    this.pixels = [];
    this.selectedPixel = null;
    this.resolution = 256; // Much higher default resolution for smoother rendering
    this.apiKey = '';

    // Setup high-DPI canvas for crisp rendering
    this.setupHighDPICanvas();

    // Enable maximum quality canvas smoothing
    this.ctx.imageSmoothingEnabled = true;
    this.ctx.imageSmoothingQuality = 'high';

    this.setupEventListeners();
    this.loadApiKey();
  }

  // Setup high-DPI canvas for crisp rendering on all devices
  setupHighDPICanvas() {
    const devicePixelRatio = window.devicePixelRatio || 1;

    // Get computed style dimensions to avoid 0 width/height
    const computedStyle = window.getComputedStyle(this.canvas);
    const displayWidth = parseInt(computedStyle.width) || 600;
    const displayHeight = parseInt(computedStyle.height) || 600;

    // Set actual canvas size based on device pixel ratio
    this.canvas.width = displayWidth * devicePixelRatio;
    this.canvas.height = displayHeight * devicePixelRatio;

    // Scale context to match device pixel ratio
    this.ctx.scale(devicePixelRatio, devicePixelRatio);

    // Set CSS size to maintain visual size
    this.canvas.style.width = displayWidth + 'px';
    this.canvas.style.height = displayHeight + 'px';

    console.log(`Canvas setup: ${displayWidth}x${displayHeight} (display) -> ${this.canvas.width}x${this.canvas.height} (actual)`);
  }

  setupEventListeners() {
    // Generate button
    document.getElementById('generateBtn').addEventListener('click', () => {
      this.generateFromPrompt();
    });

    // Canvas click for pixel selection
    this.canvas.addEventListener('click', (e) => {
      this.selectPixel(e);
    });

    // Canvas hover
    this.canvas.addEventListener('mousemove', (e) => {
      this.hoverPixel(e);
    });

    // API key input
    document.getElementById('apiKey').addEventListener('input', (e) => {
      this.apiKey = e.target.value;
      localStorage.setItem('claude_api_key', this.apiKey);
    });

    // Resolution change
    document.getElementById('resolution').addEventListener('change', (e) => {
      this.resolution = parseInt(e.target.value);
    });
  }

  loadApiKey() {
    const stored = localStorage.getItem('claude_api_key');
    if (stored) {
      this.apiKey = stored;
      document.getElementById('apiKey').value = stored;
    }
  }

  // Main generation function
  async generateFromPrompt() {
    const prompt = document.getElementById('promptInput').value.trim();
    if (!prompt) {
      this.showStatus('error', 'Please enter a prompt');
      return;
    }

    if (!this.apiKey || !this.apiKey.startsWith('sk-ant-api03-')) {
      this.showStatus('loading', 'No valid API key - using smart fallback generation...');
      // Continue with fallback generation instead of stopping
    }

    this.showStatus('loading', 'Generating STEPPPS scene...');
    this.showProgress(0);

    try {
      // Step 1: Generate STEPPPS structure from Claude
      this.showProgress(25);
      const stepppsData = await this.callClaudeForSTEPPPS(prompt);

      // Step 2: Process and create pixel objects
      this.showProgress(50);
      this.pixels = this.createPixelsFromData(stepppsData);

      // Step 3: Render to canvas
      this.showProgress(75);
      this.renderScene();

      // Step 4: Display JSON and complete
      this.showProgress(100);
      this.displayJSON(stepppsData);
      this.showStatus('success', `✅ Generated ${this.pixels.length} STEPPPS pixels`);

      document.getElementById('canvasInfo').textContent =
        `${this.resolution}x${this.resolution} AI-generated scene - Click pixels to inspect`;

    } catch (error) {
      console.error('Generation failed:', error);
      this.showStatus('error', `Generation failed: ${error.message}`);
    }

    this.hideProgress();
  }

  // Call Claude API to generate STEPPPS structure
  async callClaudeForSTEPPPS(prompt) {
    const style = document.getElementById('style').value;
    const depthMode = document.getElementById('depthMode').value;
    const psychology = document.getElementById('psychology').value;

    // Check if we have a valid API key before trying Claude
    if (!this.apiKey || !this.apiKey.startsWith('sk-ant-api03-')) {
      console.log('No valid API key, using smart fallback generation directly');
      this.showStatus('loading', 'Using smart generation (no API key)...');
      return this.generateSmartSTEPPPS(prompt, style, depthMode, psychology);
    }

    // Try Claude API first if we have a valid key
    try {
      return await this.tryClaudeAPI(prompt, style, depthMode, psychology);
    } catch (apiError) {
      console.warn('Claude API failed, using smart fallback generation:', apiError.message);
      this.showStatus('loading', 'Claude API unavailable, using smart generation...');
      return this.generateSmartSTEPPPS(prompt, style, depthMode, psychology);
    }
  }

  // Try Claude API (with CORS proxy if available)
  async tryClaudeAPI(prompt, style, depthMode, psychology) {
    const systemPrompt = `# STEPPPS Meta-Prompt Generator

You are an expert STEPPPS (Spatial Temporal Event Psychology Pixel Prompt Script) generator. Your task is to create intelligent, semantically-aware pixel data structures that transform text prompts into rich, interactive visual scenes.

## STEPPPS Schema Deep Dive

STEPPPS is a revolutionary pixel format where each pixel is an intelligent entity containing comprehensive metadata. Every pixel understands its role, context, and relationships within the scene.

### OFFICIAL STEPPPS Schema Structure:

\`\`\`json
{
  "space": {
    "x": 0,                    // X coordinate (0 to ${this.resolution-1})
    "y": 0,                    // Y coordinate (0 to ${this.resolution-1})
    "neighbors": []            // Array of neighboring pixel IDs
  },
  "temporal": {
    "timestamp": "2024-01-01T12:00:00.000Z",
    "stateHistory": []         // Array of state change events
  },
  "event": {
    "lastEvent": null,         // Last interaction type
    "eventData": {}            // Event metadata
  },
  "psychology": {
    "emotion": "neutral",      // One of: neutral, happy, sad, focused, peaceful, energetic, mysterious, dramatic, curious, contemplative, excited
    "attentionScore": 0.5      // 0.0 to 1.0
  },
  "pixel": {
    "color": "#FFFFFF",        // Hex color code
    "brightness": 1.0          // 0.0 to 1.0
  },
  "prompt": {
    "text": "",                 // LLM prompt text
    "llmConfig": {
      "model": "claude",
      "temperature": 0.7
    },
    "lastResponse": ""         // AI response
  },
  "script": {
    "callback": null,          // Function name for pixel behavior
    "params": {}              // Parameters for callback
  }
}
\`\`\`

### Detailed Property Explanations:

#### 1. SPACE OBJECT
- **space.x, space.y**: Grid coordinates within ${this.resolution}x${this.resolution} canvas
- **space.neighbors**: Array of neighboring pixel IDs for relationship mapping

#### 2. TEMPORAL OBJECT
- **temporal.timestamp**: ISO datetime when pixel was created/updated
- **temporal.stateHistory**: Array of state changes with timestamps and descriptions

#### 3. EVENT OBJECT
- **event.lastEvent**: Last interaction type (click, hover, generate, etc.)
- **event.eventData**: Metadata about the event (coordinates, user actions, etc.)

#### 4. PSYCHOLOGY OBJECT
- **psychology.emotion**: Pixel's emotional resonance (MUST be one of):
  - "neutral": Default balanced state
  - "happy": Bright, cheerful, positive elements
  - "sad": Dark, melancholy, somber areas
  - "focused": Sharp, important, attention-drawing elements
  - "peaceful": Calm scenes, nature, soft lighting
  - "energetic": Bright colors, action areas, light sources
  - "mysterious": Dark areas, shadows, unknown elements
  - "dramatic": High contrast, storm clouds, intense moments
  - "curious": Interesting details, focal points, unusual elements
  - "contemplative": Quiet areas, reflective surfaces, serene spaces
  - "excited": Vibrant colors, dynamic elements, celebration

- **psychology.attentionScore**: 0.0-1.0 visual weight:
  - 0.9-1.0: Primary focal points (sun, faces, main subjects)
  - 0.7-0.9: Secondary interest (buildings, large objects)
  - 0.4-0.7: Supporting elements (trees, clouds, details)
  - 0.0-0.4: Background fill (sky, water, distant elements)

#### 5. PIXEL OBJECT (CORE VISUAL PROPERTIES)
- **pixel.color**: Hex color code considering:
  - Scene lighting (golden hour, blue hour, harsh noon)
  - Atmospheric perspective (distant = cooler/lighter)
  - Local illumination (shadows, reflections, highlights)
  - MUST be valid hex format: #RRGGBB

- **pixel.brightness**: 0.0-1.0 affected by:
  - Time of day
  - Light sources
  - Material properties
  - Atmospheric conditions

#### 6. PROMPT OBJECT (AI INTEGRATION)
- **prompt.text**: Descriptive text about this pixel's role and context
- **prompt.llmConfig.model**: AI model used (default: "claude")
- **prompt.llmConfig.temperature**: 0.0-1.0 creativity setting
- **prompt.lastResponse**: AI's reasoning for pixel properties

#### 7. SCRIPT OBJECT (BEHAVIOR HOOKS)
- **script.callback**: Function name for dynamic pixel behavior (can be null)
- **script.params**: Parameters object for callback function (can be empty {})

## Example STEPPPS Samples (CORRECT HIERARCHICAL FORMAT):

### Sample 1: Sunset Sky Pixel
\`\`\`json
{
  "space": {
    "x": 32,
    "y": 10,
    "neighbors": ["pixel_31_10", "pixel_33_10", "pixel_32_9", "pixel_32_11"]
  },
  "temporal": {
    "timestamp": "2024-01-01T18:30:00.000Z",
    "stateHistory": [
      {"time": "2024-01-01T18:30:00.000Z", "state": "Generated as sunset sky"}
    ]
  },
  "event": {
    "lastEvent": "generate",
    "eventData": {"prompt": "sunset over ocean"}
  },
  "psychology": {
    "emotion": "peaceful",
    "attentionScore": 0.6
  },
  "pixel": {
    "color": "#FF6B35",
    "brightness": 0.9
  },
  "prompt": {
    "text": "Orange sky pixel during golden hour sunset",
    "llmConfig": {
      "model": "claude",
      "temperature": 0.7
    },
    "lastResponse": "Warm orange sky during golden hour creates peaceful mood"
  },
  "script": {
    "callback": null,
    "params": {}
  }
}
\`\`\`

### Sample 2: Sun Center Pixel
\`\`\`json
{
  "space": {
    "x": 45,
    "y": 15,
    "neighbors": ["pixel_44_15", "pixel_46_15", "pixel_45_14", "pixel_45_16"]
  },
  "temporal": {
    "timestamp": "2024-01-01T18:30:00.000Z",
    "stateHistory": [
      {"time": "2024-01-01T18:30:00.000Z", "state": "Generated as sun center"}
    ]
  },
  "event": {
    "lastEvent": "generate",
    "eventData": {"prompt": "sunset over ocean"}
  },
  "psychology": {
    "emotion": "energetic",
    "attentionScore": 0.95
  },
  "pixel": {
    "color": "#FFD700",
    "brightness": 1.0
  },
  "prompt": {
    "text": "Bright sun center as primary light source",
    "llmConfig": {
      "model": "claude",
      "temperature": 0.7
    },
    "lastResponse": "Bright sun center draws maximum attention as primary light source"
  },
  "script": {
    "callback": null,
    "params": {}
  }
}
\`\`\`

### Sample 3: Ocean Water Pixel
\`\`\`json
{
  "space": {
    "x": 20,
    "y": 45,
    "neighbors": ["pixel_19_45", "pixel_21_45", "pixel_20_44", "pixel_20_46"]
  },
  "temporal": {
    "timestamp": "2024-01-01T18:30:00.000Z",
    "stateHistory": [
      {"time": "2024-01-01T18:30:00.000Z", "state": "Generated as ocean water"}
    ]
  },
  "event": {
    "lastEvent": "generate",
    "eventData": {"prompt": "sunset over ocean"}
  },
  "psychology": {
    "emotion": "contemplative",
    "attentionScore": 0.4
  },
  "pixel": {
    "color": "#4169E1",
    "brightness": 0.7
  },
  "prompt": {
    "text": "Ocean water reflecting sunset sky",
    "llmConfig": {
      "model": "claude",
      "temperature": 0.7
    },
    "lastResponse": "Blue ocean water reflects sky, creates calming contemplative mood"
  },
  "script": {
    "callback": null,
    "params": {}
  }
}
\`\`\`

## Generation Guidelines:

### Scene Analysis Process:
1. **Parse prompt** for scene type, time, weather, objects
2. **Establish lighting** - determine primary light sources and quality
3. **Plan composition** - identify focal points and visual hierarchy
4. **Map depth layers** - background, midground, foreground elements
5. **Apply psychology** - emotional resonance for each scene area
6. **Consider style** - adjust colors and properties for artistic style

### Style Interpretations:
- **Realistic**: Natural colors, accurate lighting, detailed textures
- **Impressionist**: Softer edges, visible brushstrokes, color mixing
- **Abstract**: Bold colors, simplified forms, emotional emphasis
- **Cyberpunk**: Neon colors, high contrast, urban technology themes
- **Minimalist**: Reduced palette, simple forms, negative space

### Depth Mode Applications:
- **Atmospheric**: Distance creates lighter, cooler, hazier colors
- **Geometric**: Perspective scaling based on distance from center
- **Layered**: Discrete depth layers with clear foreground/background

### Psychology Mapping:
- **Auto**: Infer emotions from scene content and lighting
- **Peaceful**: Emphasize calm, harmonious, serene qualities
- **Energetic**: Boost vibrant, dynamic, exciting elements
- **Mysterious**: Enhance shadows, unknowns, intriguing aspects
- **Dramatic**: Increase contrast, tension, emotional intensity

## Your Task:

Generate a ${this.resolution}x${this.resolution} STEPPPS pixel array for the scene: "${prompt}"

**Requirements:**
- Style: ${style}
- Depth Mode: ${depthMode}
- Psychology: ${psychology}
- Each pixel must follow the complete STEPPPS schema
- Create cohesive scene with proper lighting, depth, and composition
- Apply emotional intelligence to every pixel
- Ensure semantic accuracy for all elements
- Provide clear reasoning for pixel properties

**Output Format:**
Return ONLY a valid JSON array of pixel objects. No additional text or explanation.

**Critical Success Factors:**
✓ Visual coherence across all pixels
✓ Realistic lighting and shadows
✓ Proper depth and perspective
✓ Emotional resonance matching psychology setting
✓ Semantic accuracy for all identified elements
✓ Style consistency throughout scene`;

    const userPrompt = `Create a ${this.resolution}x${this.resolution} STEPPPS pixel scene: "${prompt}"`;

    // Try local proxy first (if running)
    const proxyUrls = [
      'http://localhost:8002/api/claude',  // Local FastAPI proxy
      'http://localhost:3001/api/claude', // Local Node proxy
      'https://api.anthropic.com/v1/messages' // Direct (will fail due to CORS)
    ];

    for (const url of proxyUrls) {
      try {
        const isProxy = !url.includes('anthropic.com');
        const headers = isProxy ? {
          'Content-Type': 'application/json'
        } : {
          'Content-Type': 'application/json',
          'X-API-Key': this.apiKey,
          'anthropic-version': '2023-06-01'
        };

        const body = isProxy ? {
          api_key: this.apiKey,
          request_data: {
            model: 'claude-3-5-sonnet-20241022',
            max_tokens: 4000,
            temperature: 0.7,
            system: systemPrompt,
            messages: [{ role: 'user', content: userPrompt }]
          }
        } : {
          model: 'claude-3-5-sonnet-20241022',
          max_tokens: 4000,
          temperature: 0.7,
          system: systemPrompt,
          messages: [{ role: 'user', content: userPrompt }]
        };

        const response = await fetch(url, {
          method: 'POST',
          headers,
          body: JSON.stringify(body)
        });

        if (response.ok) {
          const data = await response.json();
          const content = isProxy ?
            (data.content?.[0]?.text || data.response || data.text) :
            data.content[0].text;

          try {
            return JSON.parse(content);
          } catch (parseError) {
            const jsonMatch = content.match(/\[[\s\S]*\]/);
            if (jsonMatch) {
              return JSON.parse(jsonMatch[0]);
            }
          }
        }
      } catch (urlError) {
        console.warn(`Failed to connect to ${url}:`, urlError.message);
        continue;
      }
    }

    throw new Error('All API endpoints failed - CORS restrictions prevent direct browser access');
  }

  // Create pixel objects from Claude data (expects hierarchical STEPPPS format)
  createPixelsFromData(data) {
    if (!Array.isArray(data)) {
      // If Claude returned object instead of array, generate pixel grid
      return this.generateFallbackPixels();
    }

    return data.map(pixelData => {
      // Handle both flat and hierarchical formats
      if (pixelData.space && pixelData.temporal && pixelData.pixel) {
        // Hierarchical STEPPPS format (correct)
        return new AISTEPPPSPixel({
          x: pixelData.space.x || 0,
          y: pixelData.space.y || 0,
          z: 0, // Will be calculated from other properties
          color: pixelData.pixel.color || '#FFFFFF',
          brightness: pixelData.pixel.brightness || 1,
          emotion: pixelData.psychology?.emotion || 'neutral',
          attention: pixelData.psychology?.attentionScore || 0.5,
          element: this.extractElementFromPrompt(pixelData.prompt?.text) || 'unknown',
          meaning: pixelData.prompt?.text || 'pixel',
          role: this.inferRoleFromPosition(pixelData.space.x, pixelData.space.y),
          importance: pixelData.psychology?.attentionScore || 0.5,
          reasoning: pixelData.prompt?.lastResponse || 'Generated by Claude',
          atmospheric: this.calculateAtmosphericFromPosition(pixelData.space.y),
          perspective: 1,
          birthPrompt: document.getElementById('promptInput').value,
          stepppsData: pixelData // Store full STEPPPS structure
        });
      } else {
        // Flat format (fallback)
        return new AISTEPPPSPixel({
          x: pixelData.x || 0,
          y: pixelData.y || 0,
          z: pixelData.z || 0,
          color: pixelData.color || '#FFFFFF',
          brightness: pixelData.brightness || 1,
          emotion: pixelData.emotion || 'neutral',
          attention: pixelData.attention || 0.5,
          element: pixelData.element || 'unknown',
          meaning: pixelData.meaning || 'pixel',
          role: pixelData.role || 'background',
          importance: pixelData.importance || 0.5,
          reasoning: pixelData.reasoning || '',
          atmospheric: pixelData.atmospheric || 0,
          perspective: pixelData.perspective || 1,
          birthPrompt: document.getElementById('promptInput').value
        });
      }
    });
  }

  // Helper functions for hierarchical format processing
  extractElementFromPrompt(promptText) {
    if (!promptText) return 'unknown';
    const lowerText = promptText.toLowerCase();

    if (lowerText.includes('sky')) return 'sky';
    if (lowerText.includes('sun')) return 'sun';
    if (lowerText.includes('water') || lowerText.includes('ocean')) return 'water';
    if (lowerText.includes('tree') || lowerText.includes('forest')) return 'tree';
    if (lowerText.includes('building') || lowerText.includes('structure')) return 'building';
    if (lowerText.includes('ground') || lowerText.includes('earth')) return 'ground';
    if (lowerText.includes('cloud')) return 'cloud';

    return 'unknown';
  }

  inferRoleFromPosition(x, y) {
    const normalizedY = y / this.resolution;
    if (normalizedY < 0.3) return 'background';
    if (normalizedY > 0.7) return 'foreground';
    return 'midground';
  }

  calculateAtmosphericFromPosition(y) {
    return (y / this.resolution) * 0.3; // Atmospheric perspective based on Y position
  }

  // Smart STEPPPS generation based on prompt analysis (hierarchical format)
  generateSmartSTEPPPS(prompt, style, depthMode, psychology) {
    const pixels = [];
    const promptLower = prompt.toLowerCase();

    // Analyze prompt for scene elements
    const sceneAnalysis = this.analyzePrompt(promptLower);

    for (let x = 0; x < this.resolution; x++) {
      for (let y = 0; y < this.resolution; y++) {
        const pixelData = this.generatePixelFromScene(x, y, sceneAnalysis, style, depthMode, psychology);
        pixels.push(pixelData);
      }
    }

    return pixels;
  }

  // Analyze prompt to understand scene composition
  analyzePrompt(prompt) {
    const analysis = {
      sceneType: 'landscape',
      timeOfDay: 'day',
      weather: 'clear',
      elements: [],
      colors: [],
      mood: 'peaceful',
      style: 'realistic'
    };

    // Scene type detection
    if (prompt.includes('city') || prompt.includes('urban') || prompt.includes('building')) {
      analysis.sceneType = 'urban';
    } else if (prompt.includes('underwater') || prompt.includes('ocean') || prompt.includes('sea')) {
      analysis.sceneType = 'underwater';
    } else if (prompt.includes('space') || prompt.includes('star') || prompt.includes('galaxy')) {
      analysis.sceneType = 'space';
    } else if (prompt.includes('forest') || prompt.includes('tree') || prompt.includes('jungle')) {
      analysis.sceneType = 'forest';
    }

    // Time of day
    if (prompt.includes('sunset') || prompt.includes('dusk')) {
      analysis.timeOfDay = 'sunset';
      analysis.colors = ['#FF6B35', '#FF8C42', '#FFD700', '#4169E1'];
    } else if (prompt.includes('sunrise') || prompt.includes('dawn')) {
      analysis.timeOfDay = 'sunrise';
      analysis.colors = ['#FFA500', '#FFB84D', '#87CEEB', '#F0E68C'];
    } else if (prompt.includes('night') || prompt.includes('dark')) {
      analysis.timeOfDay = 'night';
      analysis.colors = ['#191970', '#000080', '#483D8B', '#2F4F4F'];
    }

    // Weather and atmosphere
    if (prompt.includes('storm') || prompt.includes('rain')) {
      analysis.weather = 'stormy';
      analysis.mood = 'dramatic';
    } else if (prompt.includes('fog') || prompt.includes('mist')) {
      analysis.weather = 'foggy';
      analysis.mood = 'mysterious';
    }

    // Specific elements
    const elements = {
      'sun': { type: 'light_source', importance: 0.9, emotion: 'energetic' },
      'moon': { type: 'light_source', importance: 0.8, emotion: 'peaceful' },
      'mountain': { type: 'terrain', importance: 0.7, emotion: 'contemplative' },
      'water': { type: 'surface', importance: 0.6, emotion: 'peaceful' },
      'tree': { type: 'vegetation', importance: 0.5, emotion: 'contemplative' },
      'cloud': { type: 'atmosphere', importance: 0.4, emotion: 'peaceful' },
      'building': { type: 'structure', importance: 0.6, emotion: 'focused' },
      'car': { type: 'object', importance: 0.5, emotion: 'energetic' },
      'flower': { type: 'decoration', importance: 0.4, emotion: 'peaceful' }
    };

    for (const [element, props] of Object.entries(elements)) {
      if (prompt.includes(element)) {
        analysis.elements.push({ name: element, ...props });
      }
    }

    return analysis;
  }

  // Generate a single pixel based on scene analysis (hierarchical STEPPPS format)
  generatePixelFromScene(x, y, scene, style, depthMode, psychology) {
    const normalizedX = x / this.resolution;
    const normalizedY = y / this.resolution;
    const element = this.determineElement(normalizedX, normalizedY, scene);
    const emotion = this.calculateEmotion(normalizedX, normalizedY, scene, psychology);
    const attention = this.calculateAttention(normalizedX, normalizedY, scene);
    const color = this.calculateColor(normalizedX, normalizedY, scene, style);
    const brightness = this.calculateBrightness(normalizedX, normalizedY, scene);
    const reasoning = `Generated ${element} for ${scene.sceneType} scene at position (${x},${y})`;

    // Return hierarchical STEPPPS structure
    return {
      space: {
        x: x,
        y: y,
        neighbors: [] // Will be calculated later if needed
      },
      temporal: {
        timestamp: new Date().toISOString(),
        stateHistory: [
          {
            time: new Date().toISOString(),
            state: `Generated as ${element} pixel`
          }
        ]
      },
      event: {
        lastEvent: "generate",
        eventData: {
          prompt: document.getElementById('promptInput').value,
          scene: scene.sceneType,
          style: style,
          depthMode: depthMode
        }
      },
      psychology: {
        emotion: emotion,
        attentionScore: attention
      },
      pixel: {
        color: color,
        brightness: brightness
      },
      prompt: {
        text: `${element} pixel in ${scene.sceneType} scene`,
        llmConfig: {
          model: "claude-fallback",
          temperature: 0.7
        },
        lastResponse: reasoning
      },
      script: {
        callback: null,
        params: {}
      }
    };
  }

  calculateDepth(x, y, scene, depthMode) {
    switch (depthMode) {
      case 'atmospheric':
        return y * 0.8 + Math.random() * 0.2; // Top = far, bottom = near
      case 'geometric':
        const centerDist = Math.sqrt((x - 0.5) ** 2 + (y - 0.5) ** 2);
        return centerDist * 0.6 + Math.random() * 0.4;
      case 'layered':
        if (y < 0.3) return 0.8 + Math.random() * 0.2; // Sky
        if (y < 0.7) return 0.4 + Math.random() * 0.4; // Middle
        return 0.1 + Math.random() * 0.3; // Foreground
      default:
        return Math.random() * 0.8;
    }
  }

  calculateColor(x, y, scene, style) {
    if (scene.colors.length > 0) {
      // Use scene-specific colors
      const gradientPos = (x + y) / 2;
      const colorIndex = Math.floor(gradientPos * scene.colors.length);
      return scene.colors[Math.min(colorIndex, scene.colors.length - 1)];
    }

    // Default color calculation
    switch (scene.sceneType) {
      case 'sunset':
        if (y < 0.3) return '#FF6B35'; // Orange sky
        if (y < 0.7) return '#4169E1'; // Blue sky
        return '#228B22'; // Green ground
      case 'underwater':
        const depth = y;
        return `hsl(${200 + depth * 60}, 70%, ${70 - depth * 30}%)`;
      case 'space':
        return Math.random() < 0.05 ? '#FFFFFF' : '#000011'; // Stars or space
      case 'forest':
        return y < 0.3 ? '#87CEEB' : '#228B22'; // Sky or trees
      case 'urban':
        return `hsl(${Math.random() * 60 + 180}, 30%, ${40 + Math.random() * 40}%)`; // Gray tones
      default:
        return `hsl(${200 + x * 160}, ${50 + y * 30}%, ${50 + Math.random() * 30}%)`;
    }
  }

  calculateBrightness(x, y, scene) {
    let brightness = 0.7 + Math.random() * 0.3;

    if (scene.timeOfDay === 'night') {
      brightness *= 0.3;
    } else if (scene.timeOfDay === 'sunset' || scene.timeOfDay === 'sunrise') {
      brightness *= (0.6 + y * 0.4); // Brighter at horizon
    }

    return Math.max(0.1, Math.min(1.0, brightness));
  }

  calculateEmotion(x, y, scene, psychology) {
    if (psychology !== 'auto') return psychology;

    const emotions = {
      'sunset': 'peaceful',
      'storm': 'dramatic',
      'space': 'mysterious',
      'forest': 'contemplative',
      'urban': 'energetic',
      'underwater': 'curious'
    };

    return emotions[scene.sceneType] || 'peaceful';
  }

  determineElement(x, y, scene) {
    if (y < 0.2) return 'sky';
    if (y > 0.8) return 'ground';
    if (scene.sceneType === 'underwater') return 'water';
    if (scene.sceneType === 'space') return 'space';
    if (scene.sceneType === 'urban') return x < 0.5 ? 'building' : 'street';
    return y < 0.5 ? 'sky' : 'landscape';
  }

  calculateAttention(x, y, scene) {
    // Center pixels get more attention
    const centerDist = Math.sqrt((x - 0.5) ** 2 + (y - 0.5) ** 2);
    const baseAttention = 1 - centerDist;

    // Add scene-specific attention modifiers
    if (scene.timeOfDay === 'sunset' && Math.abs(y - 0.3) < 0.1) {
      return Math.min(1.0, baseAttention + 0.4); // Horizon line
    }

    return Math.max(0.1, Math.min(1.0, baseAttention * 0.6 + Math.random() * 0.4));
  }

  calculateImportance(x, y, scene) {
    // Key elements get higher importance
    if (scene.elements.some(e => e.name === 'sun') &&
        Math.sqrt((x - 0.5) ** 2 + (y - 0.3) ** 2) < 0.1) {
      return 0.95; // Sun position
    }

    const centerDist = Math.sqrt((x - 0.5) ** 2 + (y - 0.5) ** 2);
    return Math.max(0.1, 0.8 - centerDist * 0.6);
  }

  calculateAtmospheric(x, y, scene) {
    if (scene.weather === 'foggy') return y * 0.6;
    if (scene.timeOfDay === 'sunset') return Math.max(0, y - 0.5) * 0.4;
    return y * 0.2; // Subtle atmospheric perspective
  }

  calculatePerspective(x, y, scene) {
    return Math.max(0.5, 1 - y * 0.3); // Objects get smaller towards horizon
  }

  determineRole(x, y, scene) {
    if (y < 0.3) return 'background';
    if (y > 0.7) return 'foreground';
    return 'midground';
  }

  // Fallback pixel generation (simplified version)
  generateFallbackPixels() {
    return this.generateSmartSTEPPPS(
      document.getElementById('promptInput').value,
      'realistic',
      'atmospheric',
      'peaceful'
    );
  }

  // Render the scene to canvas with improved quality
  renderScene() {
    const devicePixelRatio = window.devicePixelRatio || 1;
    const computedStyle = window.getComputedStyle(this.canvas);
    const displayWidth = parseInt(computedStyle.width) || 600;
    const displayHeight = parseInt(computedStyle.height) || 600;
    const pixelSize = Math.min(displayWidth, displayHeight) / this.resolution;

    // Save context for global settings
    this.ctx.save();

    // Clear canvas with smooth black background
    this.ctx.fillStyle = '#000000';
    this.ctx.fillRect(0, 0, displayWidth, displayHeight);

    // Enable anti-aliasing for the entire scene
    this.ctx.imageSmoothingEnabled = true;
    this.ctx.imageSmoothingQuality = 'high';

    // Sort pixels by depth for proper rendering order (back to front)
    const sortedPixels = [...this.pixels].sort((a, b) => b.data.depth.z - a.data.depth.z);

    // Render each pixel with quality enhancements
    sortedPixels.forEach(pixel => {
      pixel.render(this.ctx, pixelSize);
    });

    // Restore global context
    this.ctx.restore();
  }

  // Handle pixel selection
  selectPixel(event) {
    const rect = this.canvas.getBoundingClientRect();
    const x = Math.floor(((event.clientX - rect.left) / this.canvas.width) * this.resolution);
    const y = Math.floor(((event.clientY - rect.top) / this.canvas.height) * this.resolution);

    const pixel = this.pixels.find(p => p.data.space.x === x && p.data.space.y === y);

    if (pixel) {
      // Clear previous selection
      if (this.selectedPixel) {
        this.selectedPixel.isSelected = false;
      }

      // Select new pixel
      pixel.isSelected = true;
      pixel.handleClick();
      this.selectedPixel = pixel;

      this.displayPixelInfo(pixel);
      this.renderScene(); // Re-render to show selection
    }
  }

  // Handle pixel hover
  hoverPixel(event) {
    const rect = this.canvas.getBoundingClientRect();
    const x = Math.floor(((event.clientX - rect.left) / this.canvas.width) * this.resolution);
    const y = Math.floor(((event.clientY - rect.top) / this.canvas.height) * this.resolution);

    // Clear previous hovers
    this.pixels.forEach(p => p.isHovered = false);

    const pixel = this.pixels.find(p => p.data.space.x === x && p.data.space.y === y);
    if (pixel) {
      pixel.isHovered = true;
      this.renderScene();
    }
  }

  // Display pixel information (hierarchical STEPPPS format)
  displayPixelInfo(pixel) {
    const infoDiv = document.getElementById('pixelInfo');
    const detailsDiv = document.getElementById('pixelDetails');

    infoDiv.style.display = 'block';

    // Handle both old and new pixel formats
    const stepppsData = pixel.data.stepppsData || this.convertToHierarchical(pixel.data);

    detailsDiv.innerHTML = `
      <div class="detail-item">
        <strong>Position</strong>
        (${stepppsData.space?.x || 0}, ${stepppsData.space?.y || 0})
      </div>
      <div class="detail-item">
        <strong>Element</strong>
        ${this.extractElementFromPrompt(stepppsData.prompt?.text) || 'unknown'}
      </div>
      <div class="detail-item">
        <strong>Color</strong>
        <div style="background: ${stepppsData.pixel?.color || '#000'}; width: 20px; height: 20px; border: 1px solid #ccc; display: inline-block; margin-left: 5px;"></div>
        ${stepppsData.pixel?.color || '#000'}
      </div>
      <div class="detail-item">
        <strong>Emotion</strong>
        ${stepppsData.psychology?.emotion || 'neutral'}
      </div>
      <div class="detail-item">
        <strong>Brightness</strong>
        ${(stepppsData.pixel?.brightness || 1).toFixed(2)}
      </div>
      <div class="detail-item">
        <strong>Attention Score</strong>
        ${(stepppsData.psychology?.attentionScore || 0.5).toFixed(2)}
      </div>
      <div class="detail-item">
        <strong>Last Event</strong>
        ${stepppsData.event?.lastEvent || 'none'}
      </div>
      <div class="detail-item">
        <strong>AI Reasoning</strong>
        ${stepppsData.prompt?.lastResponse || 'Generated by STEPPPS system'}
      </div>
    `;
  }

  // Convert old flat format to hierarchical for display
  convertToHierarchical(flatData) {
    return {
      space: {
        x: flatData.space?.x || 0,
        y: flatData.space?.y || 0,
        neighbors: flatData.space?.neighbors || []
      },
      temporal: {
        timestamp: flatData.temporal?.timestamp || new Date().toISOString(),
        stateHistory: flatData.temporal?.stateHistory || []
      },
      event: {
        lastEvent: flatData.event?.lastEvent || 'unknown',
        eventData: flatData.event?.eventData || {}
      },
      psychology: {
        emotion: flatData.psychology?.emotion || 'neutral',
        attentionScore: flatData.psychology?.attentionScore || 0.5
      },
      pixel: {
        color: flatData.pixel?.color || '#FFFFFF',
        brightness: flatData.pixel?.brightness || 1
      },
      prompt: {
        text: flatData.prompt?.text || 'Generated pixel',
        llmConfig: flatData.prompt?.llmConfig || { model: 'claude', temperature: 0.7 },
        lastResponse: flatData.prompt?.lastResponse || 'AI generated'
      },
      script: {
        callback: flatData.script?.callback || null,
        params: flatData.script?.params || {}
      }
    };
  }

  // Display generated JSON (hierarchical STEPPPS format)
  displayJSON(data) {
    const jsonDiv = document.getElementById('jsonDisplay');

    // Show first 3 pixels with full hierarchical structure
    const samplePixels = data.slice(0, 3);
    const displayText = `STEPPPS HIERARCHICAL FORMAT:\n\n${JSON.stringify(samplePixels, null, 2)}\n\n... and ${data.length - 3} more pixels\n\nTotal: ${data.length} STEPPPS pixels generated\n\nEach pixel contains:\n- space: coordinates and neighbors\n- temporal: timestamps and history\n- event: interaction data\n- psychology: emotion and attention\n- pixel: color and brightness\n- prompt: AI reasoning and config\n- script: behavior hooks`;

    jsonDiv.textContent = displayText;
  }

  // UI helper functions
  showStatus(type, message) {
    const status = document.getElementById('status');
    status.className = `status ${type}`;
    status.textContent = message;
    status.style.display = 'block';
  }

  showProgress(percent) {
    const progressBar = document.getElementById('progressBar');
    const progressFill = document.getElementById('progressFill');
    progressBar.style.display = 'block';
    progressFill.style.width = `${percent}%`;
  }

  hideProgress() {
    setTimeout(() => {
      document.getElementById('progressBar').style.display = 'none';
    }, 1000);
  }
}

// Example prompt functions
function setPrompt(prompt) {
  document.getElementById('promptInput').value = prompt;
}

// Initialize when DOM loads
document.addEventListener('DOMContentLoaded', function() {
  window.generator = new AISTEPPPSGenerator();
  console.log('🎨 AI STEPPPS Generator initialized');
});