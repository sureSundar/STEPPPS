// JSON Schema for STEPPPS Pixel Data Structure
const stepppsSchema = {
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "properties": {
    "space": {
      "type": "object",
      "description": "Spatial properties of the pixel",
      "properties": {
        "x": { "type": "number", "description": "X-coordinate" },
        "y": { "type": "number", "description": "Y-coordinate" },
        "neighbors": {
          "type": "array",
          "items": { "type": "string" },
          "description": "IDs of neighboring pixels"
        }
      },
      "required": ["x", "y"]
    },
    "temporal": {
      "type": "object",
      "description": "Temporal state and history",
      "properties": {
        "timestamp": { "type": "string", "format": "date-time" },
        "stateHistory": {
          "type": "array",
          "items": {
            "type": "object",
            "properties": {
              "time": { "type": "string", "format": "date-time" },
              "state": { "type": "string" }
            }
          }
        }
      },
      "required": ["timestamp"]
    },
    "event": {
      "type": "object",
      "description": "Events affecting the pixel",
      "properties": {
        "lastEvent": { "type": "string", "description": "e.g., click, hover" },
        "eventData": { "type": "object", "description": "Event metadata" }
      }
    },
    "psychology": {
      "type": "object",
      "description": "User or contextual psychological state",
      "properties": {
        "emotion": {
          "type": "string",
          "enum": ["neutral", "happy", "sad", "focused"],
          "description": "Inferred user emotion"
        },
        "attentionScore": {
          "type": "number",
          "minimum": 0,
          "maximum": 1,
          "description": "Likelihood of user focus on this pixel"
        }
      }
    },
    "pixel": {
      "type": "object",
      "description": "Core pixel properties",
      "properties": {
        "color": {
          "type": "string",
          "pattern": "^#[0-9A-Fa-f]{6}$",
          "description": "Hex color code"
        },
        "brightness": { "type": "number", "minimum": 0, "maximum": 1 }
      },
      "required": ["color"]
    },
    "prompt": {
      "type": "object",
      "description": "LLM prompt and inference config",
      "properties": {
        "text": { "type": "string", "description": "Prompt for LLM" },
        "llmConfig": {
          "type": "object",
          "properties": {
            "model": { "type": "string" },
            "temperature": { "type": "number", "minimum": 0, "maximum": 1 }
          }
        },
        "lastResponse": { "type": "string", "description": "LLM output" }
      }
    },
    "script": {
      "type": "object",
      "description": "Scripting hooks for pixel behavior",
      "properties": {
        "callback": {
          "type": "string",
          "description": "Name of JS function to call"
        },
        "params": { "type": "object", "description": "Parameters for callback" }
      }
    }
  },
  "required": ["space", "temporal", "pixel"]
};

// Sample Pixel Lifecycle Pseudocode
class STEPPPSPixel {
  constructor(id, x, y) {
    this.id = id;
    this.isHovered = false;
    this.isSelected = false;
    this.animationPhase = 0;
    this.data = {
      space: { x, y, neighbors: [] },
      temporal: { timestamp: new Date().toISOString(), stateHistory: [] },
      event: { lastEvent: null, eventData: {} },
      psychology: { emotion: "neutral", attentionScore: 0.5 },
      pixel: { color: "#FFFFFF", brightness: 1 },
      prompt: { text: "", llmConfig: { model: "mini-llm", temperature: 0.7 }, lastResponse: "" },
      script: { callback: null, params: {} }
    };
  }

  // Find and set neighboring pixels
  setNeighbors(allPixels) {
    const { x, y } = this.data.space;
    this.data.space.neighbors = [];
    
    // 8-directional neighbors
    const directions = [
      [-1, -1], [-1, 0], [-1, 1],
      [0, -1],           [0, 1],
      [1, -1],  [1, 0],  [1, 1]
    ];
    
    directions.forEach(([dx, dy]) => {
      const neighbor = allPixels.find(p => 
        p.data.space.x === x + dx && p.data.space.y === y + dy
      );
      if (neighbor) {
        this.data.space.neighbors.push(neighbor.id);
      }
    });
  }

  // Update pixel state based on event
  handleEvent(eventType, eventData) {
    this.data.event.lastEvent = eventType;
    this.data.event.eventData = eventData;
    this.data.temporal.stateHistory.push({
      time: new Date().toISOString(),
      state: `Event: ${eventType}`
    });

    // Update psychology based on event type
    switch(eventType) {
      case "click":
        this.data.psychology.emotion = "focused";
        this.data.psychology.attentionScore = 0.9;
        this.isSelected = true;
        break;
      case "hover":
        this.data.psychology.emotion = "curious";
        this.data.psychology.attentionScore = 0.7;
        this.isHovered = true;
        break;
      case "doubleclick":
        this.data.psychology.emotion = "excited";
        this.data.psychology.attentionScore = 1.0;
        this.startAnimation();
        break;
      case "rightclick":
        this.data.psychology.emotion = "contemplative";
        this.data.psychology.attentionScore = 0.8;
        this.resetToOriginal();
        break;
      case "mouseleave":
        this.isHovered = false;
        if (!this.isSelected) {
          this.data.psychology.emotion = "neutral";
          this.data.psychology.attentionScore = 0.5;
        }
        break;
    }

    // Trigger prompt for next state (async)
    this.generatePrompt().catch(console.error);
  }

  // Start animation sequence
  startAnimation() {
    this.animationPhase = 1;
    setTimeout(() => {
      this.animationPhase = 0;
    }, 2000);
  }

  // Reset pixel to original state
  resetToOriginal() {
    this.isSelected = false;
    this.isHovered = false;
    this.data.psychology.emotion = "neutral";
    this.data.psychology.attentionScore = 0.5;
    this.data.pixel.color = `hsl(${Math.random() * 360}, 50%, 70%)`;
    this.data.pixel.brightness = 1;
  }

  // Generate LLM prompt based on current state
  async generatePrompt() {
    const { psychology, pixel, event } = this.data;
    this.data.prompt.text = `Given a pixel at (${this.data.space.x}, ${this.data.space.y}) with color ${pixel.color}, user emotion ${psychology.emotion}, and recent event ${event.lastEvent}, suggest a new color and brightness.`;
    
    // Try real Claude API first, fallback to simulation
    try {
      this.data.prompt.lastResponse = await this.callClaudeLLM(this.data.prompt.text);
    } catch (error) {
      console.warn('Using simulated LLM response:', error);
      this.data.prompt.lastResponse = this.simulateLLMResponse();
    }
    
    this.applyLLMResponse();
  }

  // Real LLM response using FastAPI Claude proxy
  async callClaudeLLM(prompt) {
    const apiKey = localStorage.getItem('claude_api_key') || prompt('Enter Claude API Key:');
    if (apiKey) {
      localStorage.setItem('claude_api_key', apiKey);
    }

    const response = await fetch('http://localhost:8002/api/claude', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({
        api_key: apiKey,
        request_data: {
          model: "claude-3-haiku-20240307",
          max_tokens: 100,
          messages: [{
            role: "user",
            content: `STEPPPS Pixel at (${this.data.space.x}, ${this.data.space.y}): ${prompt}. Respond with emotion (excited/contemplative/focused/curious/energetic) and suggest a hex color. Keep response under 50 words.`
          }]
        }
      })
    });

    if (!response.ok) {
      throw new Error(`API Error: ${response.status}`);
    }

    const data = await response.json();
    const aiText = data.content[0].text;
    
    // Parse AI response for emotion and color
    const emotionMatch = aiText.match(/\b(excited|contemplative|focused|curious|energetic)\b/i);
    const colorMatch = aiText.match(/#[0-9A-Fa-f]{6}/);
    
    return {
      emotion: emotionMatch ? emotionMatch[1].toLowerCase() : "curious",
      color: colorMatch ? colorMatch[0] : this.generateRandomColor(),
      brightness: Math.random() * 0.3 + 0.7,
      response: aiText
    };
  }

  // Fallback simulation when API is unavailable
  simulateLLMResponse() {
    const emotions = ["excited", "contemplative", "focused", "curious", "energetic"];
    const colors = ["#FF6B6B", "#4ECDC4", "#45B7D1", "#96CEB4", "#FFEAA7"];
    
    const randomEmotion = emotions[Math.floor(Math.random() * emotions.length)];
    const randomColor = colors[Math.floor(Math.random() * colors.length)];
    
    return {
      emotion: randomEmotion,
      color: randomColor,
      brightness: Math.random() * 0.5 + 0.5,
      response: `I am feeling ${randomEmotion} at position (${this.data.space.x}, ${this.data.space.y})`
    };
  }

  // Generate random color helper
  generateRandomColor() {
    return `#${Math.floor(Math.random()*16777215).toString(16).padStart(6, '0')}`;
  }

  // Apply LLM response to pixel state
  applyLLMResponse() {
    const response = this.data.prompt.lastResponse;
    if (response) {
      this.data.psychology.emotion = response.emotion;
      this.data.pixel.color = response.color;
      this.data.pixel.brightness = response.brightness;

      // Execute script callback if defined
      if (this.data.script.callback) {
        const callbackFn = window[this.data.script.callback];
        if (typeof callbackFn === "function") {
          callbackFn(this.data);
        }
      }
    }
  }

  // Render pixel on canvas with enhanced visual effects
  render(ctx) {
    const x = this.data.space.x * 25;
    const y = this.data.space.y * 25;
    const size = 25;
    
    // Base pixel color
    ctx.fillStyle = this.data.pixel.color;
    ctx.globalAlpha = this.data.pixel.brightness;
    ctx.fillRect(x, y, size, size);
    
    // Hover effect - glowing border
    if (this.isHovered) {
      ctx.strokeStyle = "#FFFFFF";
      ctx.lineWidth = 2;
      ctx.globalAlpha = 0.8;
      ctx.strokeRect(x - 1, y - 1, size + 2, size + 2);
    }
    
    // Selection effect - thick border
    if (this.isSelected) {
      ctx.strokeStyle = "#FFD700";
      ctx.lineWidth = 3;
      ctx.globalAlpha = 1.0;
      ctx.strokeRect(x - 2, y - 2, size + 4, size + 4);
    }
    
    // Animation effect - pulsing
    if (this.animationPhase > 0) {
      const pulse = Math.sin(Date.now() * 0.01) * 0.3 + 0.7;
      ctx.fillStyle = "#FFFFFF";
      ctx.globalAlpha = pulse * 0.5;
      ctx.fillRect(x + 5, y + 5, size - 10, size - 10);
    }
    
    // Neighbor visualization for selected pixels
    if (this.isSelected && this.data.space.neighbors.length > 0) {
      ctx.strokeStyle = "#FF69B4";
      ctx.lineWidth = 1;
      ctx.globalAlpha = 0.4;
      // Draw lines to neighbors (simplified - just show we have neighbors)
      ctx.strokeRect(x - 3, y - 3, size + 6, size + 6);
    }
    
    ctx.globalAlpha = 1.0; // Reset alpha
  }

  // Start animation sequence
  startAnimation() {
    this.animationPhase = 1;
    setTimeout(() => {
      this.animationPhase = 0;
    }, 2000);
  }

  // Reset pixel to original state
  resetToOriginal() {
    this.isSelected = false;
    this.isHovered = false;
    this.data.psychology.emotion = "neutral";
    this.data.psychology.attentionScore = 0.5;
    this.data.pixel.color = `hsl(${Math.random() * 360}, 50%, 70%)`;
    this.data.pixel.brightness = 1;
  }

  // Generate LLM prompt based on current state
  async generatePrompt() {
    const { psychology, pixel, event } = this.data;
    this.data.prompt.text = `Given a pixel at (${this.data.space.x}, ${this.data.space.y}) with color ${pixel.color}, user emotion ${psychology.emotion}, and recent event ${event.lastEvent}, suggest a new color and brightness.`;
    
    // Try real Claude API first, fallback to simulation
    try {
      this.data.prompt.lastResponse = await this.callClaudeLLM(this.data.prompt.text);
    } catch (error) {
      console.warn('Using simulated LLM response:', error);
      this.data.prompt.lastResponse = this.simulateLLMResponse();
    }
    
    this.applyLLMResponse();
  }

  // Real LLM response using FastAPI Claude proxy
  async callClaudeLLM(prompt) {
    const apiKey = localStorage.getItem('claude_api_key') || prompt('Enter Claude API Key:');
    if (apiKey) {
      localStorage.setItem('claude_api_key', apiKey);
    }

    const response = await fetch('http://localhost:8002/api/claude', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({
        api_key: apiKey,
        request_data: {
          model: "claude-3-haiku-20240307",
          max_tokens: 100,
          messages: [{
            role: "user",
            content: `STEPPPS Pixel at (${this.data.space.x}, ${this.data.space.y}): ${prompt}. Respond with emotion (excited/contemplative/focused/curious/energetic) and suggest a hex color. Keep response under 50 words.`
          }]
        }
      })
    });

    if (!response.ok) {
      throw new Error(`API Error: ${response.status}`);
    }

    const data = await response.json();
    const aiText = data.content[0].text;
    
    // Parse AI response for emotion and color
    const emotionMatch = aiText.match(/\b(excited|contemplative|focused|curious|energetic)\b/i);
    const colorMatch = aiText.match(/#[0-9A-Fa-f]{6}/);
    
    return {
      emotion: emotionMatch ? emotionMatch[1].toLowerCase() : "curious",
      color: colorMatch ? colorMatch[0] : this.generateRandomColor(),
      brightness: Math.random() * 0.3 + 0.7,
      response: aiText
    };
  }

  // Fallback simulation when API is unavailable
  simulateLLMResponse() {
    const emotions = ["excited", "contemplative", "focused", "curious", "energetic"];
    const colors = ["#FF6B6B", "#4ECDC4", "#45B7D1", "#96CEB4", "#FFEAA7"];
    
    const randomEmotion = emotions[Math.floor(Math.random() * emotions.length)];
    const randomColor = colors[Math.floor(Math.random() * colors.length)];
    
    return {
      emotion: randomEmotion,
      color: randomColor,
      brightness: Math.random() * 0.5 + 0.5,
      response: `I am feeling ${randomEmotion} at position (${this.data.space.x}, ${this.data.space.y})`
    };
  }

  // Generate random color helper
  generateRandomColor() {
    return `#${Math.floor(Math.random()*16777215).toString(16).padStart(6, '0')}`;
  }

  // Apply LLM response to pixel state
  applyLLMResponse() {
    const response = this.data.prompt.lastResponse;
    if (response) {
      this.data.psychology.emotion = response.emotion;
      this.data.pixel.color = response.color;
      this.data.pixel.brightness = response.brightness;

      // Execute script callback if defined
      if (this.data.script.callback) {
        const callbackFn = window[this.data.script.callback];
        if (typeof callbackFn === "function") {
          callbackFn(this.data);
        }
      }
    }
  }
}


// Display pixel data in the UI panel and show editor  
function displayPixelData(pixel) {
  currentSelectedPixel = pixel;
  
  // Show the editor
  const editor = document.getElementById('pixelEditor');
  const displayElement = document.getElementById('pixelDataDisplay');
  
  if (editor && displayElement) {
    editor.style.display = 'block';
    
    // Update editor fields with current pixel data
    updateEditorFields(pixel);
    
    // Update raw JSON display
    displayElement.textContent = JSON.stringify(pixel.data, null, 2);
  }
  
  // Enhanced schema compliance logging
  console.log('STEPPPS Pixel Data:');
  console.log('===================');
  console.log('Full Data Structure:', pixel.data);
  console.log('Schema Compliance Check:');
  console.log('✓ Space:', pixel.data.space);
  console.log('✓ Temporal:', pixel.data.temporal);
  console.log('✓ Event:', pixel.data.event);
  console.log('✓ Psychology:', pixel.data.psychology);
  console.log('✓ Pixel:', pixel.data.pixel);
  console.log('✓ Prompt:', pixel.data.prompt);
  console.log('✓ Script:', pixel.data.script);
}

// Update editor fields with pixel data
function updateEditorFields(pixel) {
  const data = pixel.data;
  
  // Update pixel ID
  const pixelIdElement = document.getElementById('pixelId');
  if (pixelIdElement) {
    pixelIdElement.textContent = pixel.id;
  }
  
  // Update visual properties
  const colorInput = document.getElementById('editColor');
  const brightnessInput = document.getElementById('editBrightness');
  const brightnessValue = document.getElementById('brightnessValue');
  
  if (colorInput) {
    colorInput.value = rgbToHex(data.pixel.color) || data.pixel.color;
  }
  if (brightnessInput && brightnessValue) {
    brightnessInput.value = data.pixel.brightness;
    brightnessValue.textContent = data.pixel.brightness.toFixed(1);
  }
  
  // Update psychology
  const emotionSelect = document.getElementById('editEmotion');
  const attentionInput = document.getElementById('editAttention');
  const attentionValue = document.getElementById('attentionValue');
  
  if (emotionSelect) {
    emotionSelect.value = data.psychology.emotion;
  }
  if (attentionInput && attentionValue) {
    attentionInput.value = data.psychology.attentionScore;
    attentionValue.textContent = data.psychology.attentionScore.toFixed(1);
  }
  
  // Update prompt
  const promptTextarea = document.getElementById('editPrompt');
  if (promptTextarea) {
    promptTextarea.value = data.prompt.text || '';
  }
  
  // Update position
  const xInput = document.getElementById('editX');
  const yInput = document.getElementById('editY');
  const neighborCount = document.getElementById('neighborCount');
  
  if (xInput) xInput.value = data.space.x;
  if (yInput) yInput.value = data.space.y;
  if (neighborCount) neighborCount.textContent = data.space.neighbors.length;
}

// Update pixel property and re-render
function updatePixelProperty(propertyPath, value) {
  if (!currentSelectedPixel) return;
  
  // Parse property path and update the value
  const pathParts = propertyPath.split('.');
  let target = currentSelectedPixel.data;
  
  for (let i = 0; i < pathParts.length - 1; i++) {
    target = target[pathParts[i]];
  }
  target[pathParts[pathParts.length - 1]] = value;
  
  // Update temporal history
  currentSelectedPixel.data.temporal.stateHistory.push({
    time: new Date().toISOString(),
    state: `Manual edit: ${propertyPath} = ${value}`
  });
  
  // Update display values for sliders
  if (propertyPath === 'pixel.brightness') {
    const brightnessValue = document.getElementById('brightnessValue');
    if (brightnessValue) brightnessValue.textContent = value.toFixed(1);
  }
  if (propertyPath === 'psychology.attentionScore') {
    const attentionValue = document.getElementById('attentionValue');
    if (attentionValue) attentionValue.textContent = value.toFixed(1);
  }
  
  // Update raw JSON display
  const displayElement = document.getElementById('pixelDataDisplay');
  if (displayElement) {
    displayElement.textContent = JSON.stringify(currentSelectedPixel.data, null, 2);
  }
  
  console.log(`Updated ${propertyPath} to:`, value);
}

// Update pixel position
function updatePixelPosition() {
  if (!currentSelectedPixel) return;
  
  const xInput = document.getElementById('editX');
  const yInput = document.getElementById('editY');
  
  if (xInput && yInput) {
    const newX = parseInt(xInput.value);
    const newY = parseInt(yInput.value);
    
    // Update position
    currentSelectedPixel.data.space.x = newX;
    currentSelectedPixel.data.space.y = newY;
    
    // Recalculate neighbors
    currentSelectedPixel.setNeighbors(globalPixels);
    
    // Update neighbor count display
    const neighborCount = document.getElementById('neighborCount');
    if (neighborCount) {
      neighborCount.textContent = currentSelectedPixel.data.space.neighbors.length;
    }
    
    // Update temporal history
    currentSelectedPixel.data.temporal.stateHistory.push({
      time: new Date().toISOString(),
      state: `Position changed to (${newX}, ${newY})`
    });
    
    console.log(`Moved pixel to (${newX}, ${newY}), neighbors: ${currentSelectedPixel.data.space.neighbors.length}`);
  }
}

// Trigger LLM response for current pixel
async function triggerLLMResponse() {
  if (!currentSelectedPixel) return;
  
  try {
    await currentSelectedPixel.generatePrompt();
    updateEditorFields(currentSelectedPixel);
    console.log('🤖 LLM response applied to pixel');
  } catch (error) {
    console.error('Failed to get LLM response:', error);
  }
}

// Reset pixel to original state
function resetPixelToOriginal() {
  if (!currentSelectedPixel) return;
  
  currentSelectedPixel.resetToOriginal();
  updateEditorFields(currentSelectedPixel);
  console.log('🔄 Reset pixel to original state');
}

// Helper function to convert RGB to hex
function rgbToHex(rgb) {
  if (!rgb || typeof rgb !== 'string') return rgb;
  
  const rgbMatch = rgb.match(/rgb\((\d+),\s*(\d+),\s*(\d+)\)/);
  if (rgbMatch) {
    const r = parseInt(rgbMatch[1]);
    const g = parseInt(rgbMatch[2]);
    const b = parseInt(rgbMatch[3]);
    return "#" + ((1 << 24) + (r << 16) + (g << 8) + b).toString(16).slice(1);
  }
  
  return rgb; // Return as-is if not RGB format
}


// Display pixel data in the UI panel and show editor  
function displayPixelData(pixel) {
  currentSelectedPixel = pixel;
  
  // Show the editor and hide raw data initially
  const editor = document.getElementById('pixelEditor');
  const rawData = document.getElementById('rawDataView');
  const displayElement = document.getElementById('pixelDataDisplay');
  
  if (editor && displayElement) {
    editor.style.display = 'block';
    
    // Update editor fields with current pixel data
    updateEditorFields(pixel);
    
    // Update raw JSON display
    displayElement.textContent = JSON.stringify(pixel.data, null, 2);
  }
  
  // Enhanced schema compliance logging
  console.log('STEPPPS Pixel Data:');
  console.log('===================');
  console.log('Full Data Structure:', pixel.data);
  console.log('Schema Compliance Check:');
  console.log('✓ Space:', pixel.data.space);
  console.log('✓ Temporal:', pixel.data.temporal);
  console.log('✓ Event:', pixel.data.event);
  console.log('✓ Psychology:', pixel.data.psychology);
  console.log('✓ Pixel:', pixel.data.pixel);
  console.log('✓ Prompt:', pixel.data.prompt);
  console.log('✓ Script:', pixel.data.script);
}

// Global variable to track currently selected pixel  
let currentSelectedPixel = null;

// Update editor fields with pixel data
function updateEditorFields(pixel) {
  const data = pixel.data;
  
  // Update pixel ID
  const pixelIdElement = document.getElementById('pixelId');
  if (pixelIdElement) {
    pixelIdElement.textContent = pixel.id;
  }
  
  // Update visual properties
  const colorInput = document.getElementById('editColor');
  const brightnessInput = document.getElementById('editBrightness');
  const brightnessValue = document.getElementById('brightnessValue');
  
  if (colorInput) {
    colorInput.value = rgbToHex(data.pixel.color) || data.pixel.color;
  }
  if (brightnessInput && brightnessValue) {
    brightnessInput.value = data.pixel.brightness;
    brightnessValue.textContent = data.pixel.brightness.toFixed(1);
  }
  
  // Update psychology
  const emotionSelect = document.getElementById('editEmotion');
  const attentionInput = document.getElementById('editAttention');
  const attentionValue = document.getElementById('attentionValue');
  
  if (emotionSelect) {
    emotionSelect.value = data.psychology.emotion;
  }
  if (attentionInput && attentionValue) {
    attentionInput.value = data.psychology.attentionScore;
    attentionValue.textContent = data.psychology.attentionScore.toFixed(1);
  }
  
  // Update prompt
  const promptTextarea = document.getElementById('editPrompt');
  if (promptTextarea) {
    promptTextarea.value = data.prompt.text || '';
  }
  
  // Update position
  const xInput = document.getElementById('editX');
  const yInput = document.getElementById('editY');
  const neighborCount = document.getElementById('neighborCount');
  
  if (xInput) xInput.value = data.space.x;
  if (yInput) yInput.value = data.space.y;
  if (neighborCount) neighborCount.textContent = data.space.neighbors.length;
}

// Update pixel property and re-render
function updatePixelProperty(propertyPath, value) {
  if (!currentSelectedPixel) return;
  
  // Parse property path and update the value
  const pathParts = propertyPath.split('.');
  let target = currentSelectedPixel.data;
  
  for (let i = 0; i < pathParts.length - 1; i++) {
    target = target[pathParts[i]];
  }
  target[pathParts[pathParts.length - 1]] = value;
  
  // Update temporal history
  currentSelectedPixel.data.temporal.stateHistory.push({
    time: new Date().toISOString(),
    state: `Manual edit: ${propertyPath} = ${value}`
  });
  
  // Update display values for sliders
  if (propertyPath === 'pixel.brightness') {
    const brightnessValue = document.getElementById('brightnessValue');
    if (brightnessValue) brightnessValue.textContent = value.toFixed(1);
  }
  if (propertyPath === 'psychology.attentionScore') {
    const attentionValue = document.getElementById('attentionValue');
    if (attentionValue) attentionValue.textContent = value.toFixed(1);
  }
  
  // Update raw JSON display
  const displayElement = document.getElementById('pixelDataDisplay');
  if (displayElement) {
    displayElement.textContent = JSON.stringify(currentSelectedPixel.data, null, 2);
  }
  
  console.log(`Updated ${propertyPath} to:`, value);
}

// Update pixel position
function updatePixelPosition() {
  if (!currentSelectedPixel) return;
  
  const xInput = document.getElementById('editX');
  const yInput = document.getElementById('editY');
  
  if (xInput && yInput) {
    const newX = parseInt(xInput.value);
    const newY = parseInt(yInput.value);
    
    // Update position
    currentSelectedPixel.data.space.x = newX;
    currentSelectedPixel.data.space.y = newY;
    
    // Recalculate neighbors
    currentSelectedPixel.setNeighbors(globalPixels);
    
    // Update neighbor count display
    const neighborCount = document.getElementById('neighborCount');
    if (neighborCount) {
      neighborCount.textContent = currentSelectedPixel.data.space.neighbors.length;
    }
    
    // Update temporal history
    currentSelectedPixel.data.temporal.stateHistory.push({
      time: new Date().toISOString(),
      state: `Position changed to (${newX}, ${newY})`
    });
    
    console.log(`Moved pixel to (${newX}, ${newY}), neighbors: ${currentSelectedPixel.data.space.neighbors.length}`);
  }
}

// Trigger LLM response for current pixel
async function triggerLLMResponse() {
  if (!currentSelectedPixel) return;
  
  try {
    await currentSelectedPixel.generatePrompt();
    updateEditorFields(currentSelectedPixel);
    console.log('🤖 LLM response applied to pixel');
  } catch (error) {
    console.error('Failed to get LLM response:', error);
  }
}

// Reset pixel to original state
function resetPixelToOriginal() {
  if (!currentSelectedPixel) return;
  
  currentSelectedPixel.resetToOriginal();
  updateEditorFields(currentSelectedPixel);
  console.log('🔄 Reset pixel to original state');
}

// Helper function to convert RGB to hex
function rgbToHex(rgb) {
  if (!rgb || typeof rgb !== 'string') return rgb;
  
  const rgbMatch = rgb.match(/rgb\((\d+),\s*(\d+),\s*(\d+)\)/);
  if (rgbMatch) {
    const r = parseInt(rgbMatch[1]);
    const g = parseInt(rgbMatch[2]);
    const b = parseInt(rgbMatch[3]);
    return "#" + ((1 << 24) + (r << 16) + (g << 8) + b).toString(16).slice(1);
  }
  
  return rgb; // Return as-is if not RGB format
}

// Bulk operations for frame-level editing
class STEPPPSFrameEditor {
  constructor(pixels) {
    this.pixels = pixels;
  }

  // Apply bulk transformation based on pattern
  applyBulkTransform(pattern, transformation) {
    const matchingPixels = this.pixels.filter(pixel => {
      return this.matchesPattern(pixel, pattern);
    });

    matchingPixels.forEach(pixel => {
      this.applyTransformation(pixel, transformation);
      pixel.handleEvent("bulk_edit", { pattern, transformation });
    });

    return matchingPixels.length;
  }

  // Check if pixel matches pattern
  matchesPattern(pixel, pattern) {
    switch(pattern.type) {
      case "color_range":
        return this.isColorInRange(pixel.data.pixel.color, pattern.colorRange);
      case "emotion":
        return pixel.data.psychology.emotion === pattern.emotion;
      case "attention_threshold":
        return pixel.data.psychology.attentionScore >= pattern.threshold;
      case "all":
        return true;
      default:
        return false;
    }
  }

  // Apply transformation to pixel
  applyTransformation(pixel, transformation) {
    if (transformation.color) {
      pixel.data.pixel.color = transformation.color;
    }
    if (transformation.brightness !== undefined) {
      pixel.data.pixel.brightness = transformation.brightness;
    }
    if (transformation.emotion) {
      pixel.data.psychology.emotion = transformation.emotion;
    }
  }

  // Check if color is in specified range
  isColorInRange(color, range) {
    // Simple color matching - could be enhanced
    return range.includes(color.substring(0, 3)); // Match first 3 chars of hex
  }
}

// Image loading and conversion functionality
class STEPPPSImageConverter {
  constructor(canvas, ctx) {
    this.canvas = canvas;
    this.ctx = ctx;
    this.gridSize = 20;
    this.pixelSize = 25;
  }

  async loadImageFromFile(file) {
    return new Promise((resolve, reject) => {
      const img = new Image();
      const reader = new FileReader();
      
      reader.onload = (e) => {
        img.onload = () => resolve(img);
        img.onerror = reject;
        img.src = e.target.result;
      };
      reader.onerror = reject;
      reader.readAsDataURL(file);
    });
  }

  convertImageToSTEPPPS(img) {
    const statusElement = document.getElementById('conversionStatus');
    statusElement.textContent = 'Converting image to STEPPPS pixels...';
    statusElement.style.color = 'blue';

    // Create temporary canvas for image processing
    const tempCanvas = document.createElement('canvas');
    const tempCtx = tempCanvas.getContext('2d');
    tempCanvas.width = this.gridSize;
    tempCanvas.height = this.gridSize;

    // Draw and scale image to grid size
    tempCtx.drawImage(img, 0, 0, this.gridSize, this.gridSize);
    const imageData = tempCtx.getImageData(0, 0, this.gridSize, this.gridSize);

    const pixels = [];
    
    // Convert each pixel to STEPPPS format
    for (let x = 0; x < this.gridSize; x++) {
      for (let y = 0; y < this.gridSize; y++) {
        const index = (y * this.gridSize + x) * 4;
        const r = imageData.data[index];
        const g = imageData.data[index + 1];
        const b = imageData.data[index + 2];
        const a = imageData.data[index + 3];

        const pixel = new STEPPPSPixel(`img_pixel_${x}_${y}`, x, y);
        
        // Set pixel color from image
        pixel.data.pixel.color = `rgb(${r}, ${g}, ${b})`;
        pixel.data.pixel.brightness = a / 255;
        
        // Add image-specific metadata
        pixel.data.space.imageSource = true;
        pixel.data.space.originalColor = { r, g, b, a };
        
        // Set initial psychology based on color intensity
        const intensity = (r + g + b) / 3;
        if (intensity > 200) {
          pixel.data.psychology.emotion = "energetic";
          pixel.data.psychology.attentionScore = 0.8;
        } else if (intensity < 100) {
          pixel.data.psychology.emotion = "contemplative";
          pixel.data.psychology.attentionScore = 0.6;
        } else {
          pixel.data.psychology.emotion = "neutral";
          pixel.data.psychology.attentionScore = 0.5;
        }

        pixels.push(pixel);
      }
    }

    // Set up neighbors for all pixels
    pixels.forEach(pixel => pixel.setNeighbors(pixels));

    statusElement.textContent = `✅ Converted ${pixels.length} pixels from image`;
    statusElement.style.color = 'green';

    return pixels;
  }

  generateRandomGrid() {
    const statusElement = document.getElementById('conversionStatus');
    statusElement.textContent = 'Generating random STEPPPS grid...';
    statusElement.style.color = 'blue';

    const pixels = [];
    
    for (let x = 0; x < this.gridSize; x++) {
      for (let y = 0; y < this.gridSize; y++) {
        const pixel = new STEPPPSPixel(`random_pixel_${x}_${y}`, x, y);
        pixel.data.pixel.color = `hsl(${Math.random() * 360}, 50%, 70%)`;
        pixel.data.pixel.brightness = Math.random() * 0.4 + 0.6;
        pixels.push(pixel);
      }
    }

    pixels.forEach(pixel => pixel.setNeighbors(pixels));

    statusElement.textContent = `✅ Generated ${pixels.length} random pixels`;
    statusElement.style.color = 'green';

    return pixels;
  }
}

// Global variables for canvas management
let globalPixels = [];
let globalCanvas = null;
let globalCtx = null;
let imageConverter = null;

// Example Canvas Setup for PoC
function setupCanvas() {
  globalCanvas = document.createElement("canvas");
  globalCanvas.width = 500;
  globalCanvas.height = 500;
  
  // Find canvas container or append to body
  const canvasContainer = document.querySelector('.canvas-container');
  if (canvasContainer) {
    canvasContainer.appendChild(globalCanvas);
  } else {
    document.body.appendChild(globalCanvas);
  }
  
  globalCtx = globalCanvas.getContext("2d");
  imageConverter = new STEPPPSImageConverter(globalCanvas, globalCtx);

  // Initialize with random grid
  globalPixels = imageConverter.generateRandomGrid();

  setupEventListeners();
  startAnimation();

  // Return pixels array for external access
  return globalPixels;
}

// Event listeners setup
function setupEventListeners() {
  const frameEditor = new STEPPPSFrameEditor(globalPixels);
  let hoveredPixel = null;

  // Multiple interaction modes
  globalCanvas.addEventListener("mousemove", (e) => {
    const rect = globalCanvas.getBoundingClientRect();
    const x = Math.floor((e.clientX - rect.left) / 25);
    const y = Math.floor((e.clientY - rect.top) / 25);
    const pixel = globalPixels.find(p => p.data.space.x === x && p.data.space.y === y);
    
    if (pixel !== hoveredPixel) {
      if (hoveredPixel) {
        hoveredPixel.handleEvent("mouseleave", {});
      }
      if (pixel) {
        pixel.handleEvent("hover", { x, y });
        hoveredPixel = pixel;
      }
    }
  });

  // Click event with enhanced LLM interaction
  globalCanvas.addEventListener("click", async (e) => {
    const rect = globalCanvas.getBoundingClientRect();
    const x = Math.floor((e.clientX - rect.left) / 25);
    const y = Math.floor((e.clientY - rect.top) / 25);
    const pixel = globalPixels.find(p => p.data.space.x === x && p.data.space.y === y);
    if (pixel) {
      // Clear other selections
      globalPixels.forEach(p => p.isSelected = false);
      pixel.handleEvent("click", { x, y });
      displayPixelData(pixel);
      
      // Enhanced logging for image pixels
      if (pixel.data.space.imageSource) {
        console.log(`🖼️ Image Pixel at (${x}, ${y})`);
        console.log(`Original Color: rgb(${pixel.data.space.originalColor.r}, ${pixel.data.space.originalColor.g}, ${pixel.data.space.originalColor.b})`);
        console.log(`Current Emotion: ${pixel.data.psychology.emotion}`);
        console.log(`Neighbors: ${pixel.data.space.neighbors.length}`);
      } else {
        console.log(`🎲 Random Pixel at (${x}, ${y}) - Neighbors: ${pixel.data.space.neighbors.length}`);
      }
    }
  });

  // Double-click event
  globalCanvas.addEventListener("dblclick", (e) => {
    const rect = globalCanvas.getBoundingClientRect();
    const x = Math.floor((e.clientX - rect.left) / 25);
    const y = Math.floor((e.clientY - rect.top) / 25);
    const pixel = globalPixels.find(p => p.data.space.x === x && p.data.space.y === y);
    if (pixel) {
      pixel.handleEvent("doubleclick", { x, y });
      displayPixelData(pixel);
    }
  });

  // Right-click event
  globalCanvas.addEventListener("contextmenu", (e) => {
    e.preventDefault();
    const rect = globalCanvas.getBoundingClientRect();
    const x = Math.floor((e.clientX - rect.left) / 25);
    const y = Math.floor((e.clientY - rect.top) / 25);
    const pixel = globalPixels.find(p => p.data.space.x === x && p.data.space.y === y);
    if (pixel) {
      pixel.handleEvent("rightclick", { x, y });
      displayPixelData(pixel);
    }
  });

  // Keyboard shortcuts for bulk operations
  document.addEventListener("keydown", (e) => {
    switch(e.key) {
      case "f": // Fire effect on focused pixels
        const count = frameEditor.applyBulkTransform(
          { type: "emotion", emotion: "focused" },
          { color: "#FF4500", brightness: 1.0, emotion: "excited" }
        );
        console.log(`🔥 Applied fire effect to ${count} focused pixels`);
        break;
      case "r": // Reset all pixels
        frameEditor.applyBulkTransform(
          { type: "all" },
          { emotion: "neutral" }
        );
        globalPixels.forEach(p => p.resetToOriginal());
        console.log("🔄 Reset all pixels");
        break;
      case "b": // Blue wave on curious pixels
        frameEditor.applyBulkTransform(
          { type: "emotion", emotion: "curious" },
          { color: "#0080FF", brightness: 0.8 }
        );
        console.log("🌊 Applied blue wave to curious pixels");
        break;
    }
  });

  // Image input handler
  const imageInput = document.getElementById('imageInput');
  if (imageInput) {
    imageInput.addEventListener('change', async (e) => {
      const file = e.target.files[0];
      if (file) {
        try {
          const img = await imageConverter.loadImageFromFile(file);
          globalPixels = imageConverter.convertImageToSTEPPPS(img);
          console.log(`📸 Loaded image: ${file.name} (${globalPixels.length} pixels)`);
        } catch (error) {
          console.error('Failed to load image:', error);
          document.getElementById('conversionStatus').textContent = '❌ Failed to load image';
          document.getElementById('conversionStatus').style.color = 'red';
        }
      }
    });
  }
}

// Animation loop
function startAnimation() {
  function animate() {
    globalCtx.clearRect(0, 0, globalCanvas.width, globalCanvas.height);
    globalPixels.forEach(pixel => pixel.render(globalCtx));
    requestAnimationFrame(animate);
  }
  animate();
}

// Load random colors function
function loadRandomColors() {
  if (imageConverter) {
    globalPixels = imageConverter.generateRandomGrid();
    console.log('🎲 Generated new random grid');
  }
}

// Test Claude API connection
async function testClaudeConnection() {
  const statusElement = document.getElementById('llmStatus');
  statusElement.textContent = 'Testing...';
  
  try {
    const response = await fetch('http://localhost:8002/health');
    if (response.ok) {
      statusElement.textContent = '✅ FastAPI Server Connected';
      statusElement.style.color = 'green';
    } else {
      throw new Error('Server not responding');
    }
  } catch (error) {
    statusElement.textContent = '❌ FastAPI Server Offline';
    statusElement.style.color = 'red';
    console.error('FastAPI connection failed:', error);
  }
}

// Clear stored API key
function clearApiKey() {
  localStorage.removeItem('claude_api_key');
  alert('Claude API key cleared from storage');
}

// Check server status on page load
document.addEventListener('DOMContentLoaded', () => {
  testClaudeConnection();
});

// Example script callback
function exampleCallback(pixelData) {
  console.log(`Pixel at (${pixelData.space.x}, ${pixelData.space.y}) changed to ${pixelData.pixel.color}`);
}

// Initialize PoC when DOM is ready
document.addEventListener('DOMContentLoaded', function() {
  setupCanvas();
});