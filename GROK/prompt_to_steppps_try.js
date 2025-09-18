class AISTEPPPSGenerator {
  // ... (existing code)

  // Improved smart STEPPPS generation with Perlin noise
  generateSmartSTEPPPS(prompt, style, depthMode, psychology) {
    const pixels = [];
    const promptLower = prompt.toLowerCase();
    const sceneAnalysis = this.analyzePrompt(promptLower);

    // Simple Perlin noise implementation for smooth gradients
    function perlinNoise(x, y, seed = 0) {
      const n = x + y * 57 + seed * 131;
      return (Math.sin(n * 12.9898) * 43758.5453) % 1;
    }

    for (let x = 0; x < this.resolution; x++) {
      for (let y = 0; y < this.resolution; y++) {
        const normalizedX = x / this.resolution;
        const normalizedY = y / this.resolution;
        const noise = perlinNoise(normalizedX, normalizedY, 42);

        // Calculate color with smooth transitions
        let color;
        if (promptLower.includes('sunset')) {
          const t = noise * 0.3 + normalizedY * 0.7; // Blend noise with Y position
          if (t < 0.3) color = `#FF${Math.floor(107 + t * 148).toString(16)}35`; // Orange sky
          else if (t < 0.7) color = `#41${Math.floor(69 + t * 50).toString(16)}E1`; // Blue sky
          else color = `#22${Math.floor(139 - t * 50).toString(16)}22`; // Green ground
        } else {
          color = this.calculateColor(normalizedX, normalizedY, sceneAnalysis, style);
        }

        const pixelData = {
          space: { x, y, neighbors: [] },
          temporal: {
            timestamp: new Date().toISOString(),
            stateHistory: [{ time: new Date().toISOString(), state: `Generated pixel` }]
          },
          event: {
            lastEvent: "generate",
            eventData: { prompt, scene: sceneAnalysis.sceneType, style, depthMode }
          },
          psychology: {
            emotion: this.calculateEmotion(normalizedX, normalizedY, sceneAnalysis, psychology),
            attentionScore: this.calculateAttention(normalizedX, normalizedY, sceneAnalysis)
          },
          pixel: {
            color,
            brightness: this.calculateBrightness(normalizedX, normalizedY, sceneAnalysis)
          },
          prompt: {
            text: `${this.determineElement(normalizedX, normalizedY, sceneAnalysis)} pixel`,
            llmConfig: { model: "claude-fallback", temperature: 0.7 },
            lastResponse: `Generated with Perlin noise for smooth transitions`
          },
          script: { callback: null, params: {} }
        };
        pixels.push(pixelData);
      }
    }
    return pixels;
  }

  // ... (rest of the existing code)
}