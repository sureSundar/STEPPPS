// STEPPPS Pixel Editor JavaScript
// Global state
let currentStepppsData = null;
let selectedPixel = null;
let canvasZoom = 1;
let editHistory = [];
let originalImageData = null;

// Initialize
document.addEventListener('DOMContentLoaded', function() {
    console.log('STEPPPS Pixel Editor initialized');
});

async function handleImageUpload(input) {
    const file = input.files[0];
    if (!file) return;

    showStatus('loadStatus', '🔄 Loading and converting image to STEPPPS...', 'loading');

    try {
        // Convert image to STEPPPS
        const stepppsData = await processImageToSteppps(file);
        currentStepppsData = stepppsData;
        
        // Render to canvas
        await renderStepppsToCanvas();
        
        // Update stats
        updateStats();
        
        showStatus('loadStatus', '✅ Image loaded and converted to STEPPPS successfully!', 'success');
        document.getElementById('canvasInfo').textContent = 
            `${file.name} - ${stepppsData.canvas.width}×${stepppsData.canvas.height} - Click pixels to edit`;

    } catch (error) {
        console.error('Image processing error:', error);
        showStatus('loadStatus', `❌ Failed to process image: ${error.message}`, 'error');
    }
}

async function processImageToSteppps(file) {
    return new Promise((resolve, reject) => {
        const img = new Image();
        img.onload = function() {
            try {
                const canvas = document.createElement('canvas');
                canvas.width = img.width;
                canvas.height = img.height;
                const ctx = canvas.getContext('2d');
                ctx.drawImage(img, 0, 0);
                
                const imageData = ctx.getImageData(0, 0, img.width, img.height);
                originalImageData = imageData;
                
                const stepppsData = extractFullStepppsFromImageData(imageData, file.name);
                resolve(stepppsData);
            } catch (error) {
                reject(error);
            }
        };
        img.onerror = () => reject(new Error('Failed to load image'));
        img.src = URL.createObjectURL(file);
    });
}

function extractFullStepppsFromImageData(imageData, filename) {
    const { width, height, data } = imageData;
    const totalPixels = width * height;
    
    // Extract ALL pixels for true STEPPPS
    const pixels = {};
    
    for (let y = 0; y < height; y++) {
        for (let x = 0; x < width; x++) {
            const index = (y * width + x) * 4;
            const r = data[index];
            const g = data[index + 1];
            const b = data[index + 2];
            const a = data[index + 3];

            pixels[`${x},${y}`] = {
                space: {
                    coordinates: [x, y],
                    region: determineRegion(x, y, width, height)
                },
                pixel: {
                    rgba: [r, g, b, a],
                    hex: `#${r.toString(16).padStart(2,'0')}${g.toString(16).padStart(2,'0')}${b.toString(16).padStart(2,'0')}`,
                    luminance: (0.299 * r + 0.587 * g + 0.114 * b) / 255,
                    saturation: calculateSaturation(r, g, b)
                },
                time: {
                    extracted: new Date().toISOString(),
                    last_edited: null
                },
                intelligence: {
                    ai_analyzed: false,
                    ai_response: null,
                    edit_prompt: null,
                    edit_count: 0
                }
            };
        }
    }

    return {
        metadata: {
            version: "1.0.0",
            source_type: "pixel_editor",
            source_path: filename,
            resolution: `${width}x${height}`,
            analysis_mode: "full",
            created_at: new Date().toISOString(),
            total_pixels: totalPixels,
            analyzed_pixels: totalPixels,
            format: "STEPPPS-JSON-v1.0"
        },
        canvas: {
            width: width,
            height: height,
            color_space: "RGBA"
        },
        pixels: pixels
    };
}

function determineRegion(x, y, width, height) {
    const thirdW = Math.floor(width / 3);
    const thirdH = Math.floor(height / 3);
    
    if (x < thirdW && y < thirdH) return 'top-left';
    if (x >= 2 * thirdW && y < thirdH) return 'top-right';
    if (x < thirdW && y >= 2 * thirdH) return 'bottom-left';
    if (x >= 2 * thirdW && y >= 2 * thirdH) return 'bottom-right';
    if (y < thirdH) return 'top-center';
    if (y >= 2 * thirdH) return 'bottom-center';
    if (x < thirdW) return 'left-center';
    if (x >= 2 * thirdW) return 'right-center';
    return 'center';
}

function calculateSaturation(r, g, b) {
    const max = Math.max(r, g, b) / 255;
    const min = Math.min(r, g, b) / 255;
    const diff = max - min;
    
    if (diff === 0) return 0;
    
    const lightness = (max + min) / 2;
    return lightness > 0.5 ? diff / (2 - max - min) : diff / (max + min);
}

async function renderStepppsToCanvas() {
    if (!currentStepppsData) return;

    const canvas = document.getElementById('stepppsCanvas');
    const ctx = canvas.getContext('2d');
    
    canvas.width = currentStepppsData.canvas.width;
    canvas.height = currentStepppsData.canvas.height;
    
    // Render all pixels
    Object.entries(currentStepppsData.pixels).forEach(([coord, pixelData]) => {
        const [x, y] = coord.split(',').map(Number);
        const rgba = pixelData.pixel.rgba;
        
        ctx.fillStyle = `rgba(${rgba[0]}, ${rgba[1]}, ${rgba[2]}, ${rgba[3]/255})`;
        ctx.fillRect(x, y, 1, 1);
    });

    // Apply zoom
    canvas.style.transform = `scale(${canvasZoom})`;
}

function handlePixelClick(event) {
    if (!currentStepppsData) return;

    const canvas = document.getElementById('stepppsCanvas');
    const rect = canvas.getBoundingClientRect();
    const scaleX = canvas.width / rect.width;
    const scaleY = canvas.height / rect.height;
    
    const x = Math.floor((event.clientX - rect.left) * scaleX);
    const y = Math.floor((event.clientY - rect.top) * scaleY);
    
    const coord = `${x},${y}`;
    const pixelData = currentStepppsData.pixels[coord];
    
    if (pixelData) {
        selectPixel(coord, pixelData);
        highlightPixel(x, y);
    }
}

function selectPixel(coord, pixelData) {
    selectedPixel = { coord, data: pixelData };
    
    // Update pixel info display
    const infoDiv = document.getElementById('selectedPixelInfo');
    const [x, y] = coord.split(',').map(Number);
    
    infoDiv.innerHTML = `
        <strong>Selected Pixel:</strong> (${x}, ${y})<br>
        <strong>Region:</strong> ${pixelData.space.region}<br>
        <strong>Color:</strong> ${pixelData.pixel.hex} - rgba(${pixelData.pixel.rgba.join(', ')})<br>
        <strong>Luminance:</strong> ${(pixelData.pixel.luminance * 100).toFixed(1)}%<br>
        <strong>AI Edited:</strong> ${pixelData.intelligence.ai_analyzed ? '✅ Yes' : '❌ No'}<br>
        ${pixelData.intelligence.ai_response ? `<strong>Last AI Edit:</strong> "${pixelData.intelligence.ai_response}"` : ''}
    `;

    // Update JSON editor
    document.getElementById('pixelJsonEditor').value = JSON.stringify(pixelData, null, 2);

    // Enable edit buttons
    document.getElementById('aiEditBtn').disabled = false;
    document.querySelectorAll('.edit-controls button').forEach(btn => {
        if (btn.onclick && btn.onclick.toString().includes('disabled')) return;
        btn.disabled = false;
    });
}

function highlightPixel(x, y) {
    const overlay = document.getElementById('pixelOverlay');
    const canvas = document.getElementById('stepppsCanvas');
    const rect = canvas.getBoundingClientRect();
    
    const pixelSize = Math.max(4, 20 / canvasZoom);
    
    overlay.style.left = `${(x / canvas.width) * rect.width - pixelSize/2}px`;
    overlay.style.top = `${(y / canvas.height) * rect.height - pixelSize/2}px`;
    overlay.style.width = `${pixelSize}px`;
    overlay.style.height = `${pixelSize}px`;
    overlay.style.display = 'block';
}

async function aiEditPixel() {
    if (!selectedPixel) return;

    const apiKey = document.getElementById('claudeApiKey').value.trim();
    if (!apiKey || !apiKey.startsWith('sk-ant-')) {
        showStatus('aiStatus', '❌ Please enter a valid Claude API key', 'error');
        return;
    }

    const prompt = document.getElementById('aiPrompt').value.trim();
    if (!prompt) {
        showStatus('aiStatus', '❌ Please enter a prompt describing how to edit the pixel', 'error');
        return;
    }

    showStatus('aiStatus', '🤖 Sending pixel to Claude for AI editing...', 'loading');

    try {
        const aiResponse = await callClaudeForPixelEdit(selectedPixel, prompt, apiKey);
        
        // Apply AI response to pixel
        applyAiEditToPixel(aiResponse, prompt);
        
        // Re-render canvas
        await renderStepppsToCanvas();
        
        // Update display
        selectPixel(selectedPixel.coord, selectedPixel.data);
        updateStats();
        
        showStatus('aiStatus', '✅ Pixel successfully edited by Claude AI!', 'success');

    } catch (error) {
        console.error('AI edit error:', error);
        showStatus('aiStatus', `❌ AI edit failed: ${error.message}`, 'error');
    }
}

async function callClaudeForPixelEdit(pixel, prompt, apiKey) {
    const pixelPrompt = `STEPPPS PIXEL EDIT REQUEST:

CURRENT PIXEL DATA:
- Position: (${pixel.coord}) in ${pixel.data.space.region} region
- Current Color: ${pixel.data.pixel.hex} - rgba(${pixel.data.pixel.rgba.join(', ')})
- Luminance: ${(pixel.data.pixel.luminance * 100).toFixed(1)}%
- Saturation: ${(pixel.data.pixel.saturation * 100).toFixed(1)}%

EDIT REQUEST: ${prompt}

Please respond with a JSON object containing the new pixel values. Format:
{
  "rgba": [r, g, b, a],
  "reasoning": "explanation of the change"
}

The rgba values should be integers 0-255 for RGB and 0-255 for alpha.`;

    // Use the existing FastAPI server for Claude API calls
    const response = await fetch('http://localhost:8005/api/claude', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify({
            api_key: apiKey,
            request_data: {
                model: 'claude-3-5-sonnet-20241022',
                max_tokens: 200,
                messages: [{
                    role: 'user',
                    content: pixelPrompt
                }]
            }
        })
    });

    if (!response.ok) {
        const errorText = await response.text();
        throw new Error(`Claude API request failed: ${response.status} - ${errorText}`);
    }

    const data = await response.json();
    const aiText = data.content[0].text;
    
    // Parse JSON response from Claude
    try {
        const jsonMatch = aiText.match(/\{[\s\S]*\}/);
        if (jsonMatch) {
            return JSON.parse(jsonMatch[0]);
        } else {
            throw new Error('No valid JSON found in Claude response');
        }
    } catch (e) {
        throw new Error('Failed to parse Claude response as JSON');
    }
}

function applyAiEditToPixel(aiResponse, originalPrompt) {
    if (!selectedPixel || !aiResponse.rgba) return;

    const [r, g, b, a] = aiResponse.rgba;
    
    // Update pixel data
    selectedPixel.data.pixel.rgba = [r, g, b, a];
    selectedPixel.data.pixel.hex = `#${r.toString(16).padStart(2,'0')}${g.toString(16).padStart(2,'0')}${b.toString(16).padStart(2,'0')}`;
    selectedPixel.data.pixel.luminance = (0.299 * r + 0.587 * g + 0.114 * b) / 255;
    selectedPixel.data.pixel.saturation = calculateSaturation(r, g, b);
    
    // Update intelligence data
    selectedPixel.data.intelligence.ai_analyzed = true;
    selectedPixel.data.intelligence.ai_response = aiResponse.reasoning || 'AI edited';
    selectedPixel.data.intelligence.edit_prompt = originalPrompt;
    selectedPixel.data.intelligence.edit_count += 1;
    selectedPixel.data.time.last_edited = new Date().toISOString();
    
    // Update in main data structure
    currentStepppsData.pixels[selectedPixel.coord] = selectedPixel.data;
    
    // Re-render canvas to show changes
    renderStepppsToCanvas();
    
    // Update pixel selection display
    selectPixel(selectedPixel.coord, selectedPixel.data);
    
    // Update stats
    updateStats();
}

function applyJsonEdit() {
    if (!selectedPixel) return;

    try {
        const jsonText = document.getElementById('pixelJsonEditor').value;
        const editedPixelData = JSON.parse(jsonText);
        
        // Validate the structure
        if (!editedPixelData.pixel || !editedPixelData.pixel.rgba) {
            throw new Error('Invalid pixel data structure');
        }
        
        // Update the pixel
        currentStepppsData.pixels[selectedPixel.coord] = editedPixelData;
        selectedPixel.data = editedPixelData;
        
        // Re-render
        renderStepppsToCanvas();
        selectPixel(selectedPixel.coord, editedPixelData);
        updateStats();
        
        showStatus('aiStatus', '✅ JSON edit applied successfully!', 'success');
        
    } catch (error) {
        showStatus('aiStatus', `❌ JSON edit failed: ${error.message}`, 'error');
    }
}

function resetPixel() {
    if (!selectedPixel || !originalImageData) return;

    const [x, y] = selectedPixel.coord.split(',').map(Number);
    const width = originalImageData.width;
    const index = (y * width + x) * 4;
    
    const r = originalImageData.data[index];
    const g = originalImageData.data[index + 1];
    const b = originalImageData.data[index + 2];
    const a = originalImageData.data[index + 3];
    
    // Reset to original values
    selectedPixel.data.pixel.rgba = [r, g, b, a];
    selectedPixel.data.pixel.hex = `#${r.toString(16).padStart(2,'0')}${g.toString(16).padStart(2,'0')}${b.toString(16).padStart(2,'0')}`;
    selectedPixel.data.pixel.luminance = (0.299 * r + 0.587 * g + 0.114 * b) / 255;
    selectedPixel.data.pixel.saturation = calculateSaturation(r, g, b);
    selectedPixel.data.intelligence.ai_analyzed = false;
    selectedPixel.data.intelligence.ai_response = null;
    selectedPixel.data.intelligence.edit_prompt = null;
    selectedPixel.data.intelligence.edit_count = 0;
    
    // Update in main data
    currentStepppsData.pixels[selectedPixel.coord] = selectedPixel.data;
    
    // Re-render
    renderStepppsToCanvas();
    selectPixel(selectedPixel.coord, selectedPixel.data);
    updateStats();
    
    showStatus('aiStatus', '✅ Pixel reset to original value', 'success');
}

// Unified AI editing function
async function performUnifiedAiEdit() {
    const editMode = document.querySelector('input[name="editMode"]:checked').value;
    
    if (editMode === 'pixel') {
        await performAiEdit();
    } else {
        await performFrameAiEdit();
    }
}

function performReset() {
    const editMode = document.querySelector('input[name="editMode"]:checked').value;
    
    if (editMode === 'pixel') {
        resetPixel();
    } else {
        resetFrame();
    }
}

// Frame-level AI editing function
async function performFrameAiEdit() {
    const prompt = document.getElementById('aiPrompt').value.trim();
    const apiKey = document.getElementById('claudeApiKey').value.trim();
    
    if (!prompt) {
        showStatus('aiStatus', '❌ Please enter a frame edit prompt', 'error');
        return;
    }
    
    if (!apiKey) {
        showStatus('aiStatus', '❌ Please enter Claude API key', 'error');
        return;
    }
    
    if (!currentStepppsData) {
        showStatus('aiStatus', '❌ No STEPPPS data loaded', 'error');
        return;
    }
    
    showStatus('aiStatus', '🤖 Processing frame-level AI edit...', 'processing');
    
    try {
        const aiResponse = await callClaudeForFrameEdit(prompt, apiKey);
        
        // Apply frame-level changes
        await applyFrameEdit(aiResponse, prompt);
        
        showStatus('aiStatus', '✅ Frame edit completed successfully!', 'success');
        
    } catch (error) {
        showStatus('aiStatus', `❌ Frame edit failed: ${error.message}`, 'error');
    }
}

async function callClaudeForFrameEdit(prompt, apiKey) {
    // Create a summary of the current STEPPPS data for Claude
    const pixelCount = Object.keys(currentStepppsData.pixels).length;
    const colorAnalysis = analyzeImageColors();
    
    const framePrompt = `You are editing an entire STEPPPS image frame with ${pixelCount} pixels.

Current image analysis:
- Dimensions: ${currentStepppsData.canvas.width}x${currentStepppsData.canvas.height}
- Total pixels: ${pixelCount}
- Color distribution: ${JSON.stringify(colorAnalysis, null, 2)}

User request: "${prompt}"

IMPORTANT: You must respond with ONLY a valid JSON object, no other text. Format:
{
  "modifications": [
    {
      "condition": "description of which pixels to modify",
      "action": "what to do",
      "rgba_transform": [r_change, g_change, b_change, a_change],
      "color_filter": {"r_min": 0, "r_max": 255, "g_min": 0, "g_max": 255, "b_min": 0, "b_max": 255}
    }
  ],
  "reasoning": "explanation of the changes"
}

The rgba_transform should be additive values (-255 to +255).
The color_filter defines which pixels match the condition (RGB ranges). Set to null to affect all pixels.`;

    const response = await fetch('http://localhost:8005/api/claude', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify({
            api_key: apiKey,
            request_data: {
                model: 'claude-3-5-sonnet-20241022',
                max_tokens: 500,
                messages: [{
                    role: 'user',
                    content: framePrompt
                }]
            }
        })
    });

    if (!response.ok) {
        const errorText = await response.text();
        throw new Error(`Claude API request failed: ${response.status} - ${errorText}`);
    }

    const data = await response.json();
    const aiText = data.content[0].text;
    
    console.log('Claude frame response:', aiText);
    
    // Parse JSON response from Claude with better error handling
    try {
        // Clean the response text
        let cleanText = aiText.trim();
        
        // Remove any markdown code blocks
        cleanText = cleanText.replace(/```json\s*/gi, '').replace(/```\s*/g, '');
        
        // Try to find JSON in the response
        const jsonMatch = cleanText.match(/\{[\s\S]*\}/);
        if (jsonMatch) {
            const jsonStr = jsonMatch[0];
            console.log('Extracted JSON:', jsonStr);
            return JSON.parse(jsonStr);
        } else {
            // If no JSON found, create a simple transformation based on the prompt
            console.log('No JSON found, creating fallback response');
            return createFallbackFrameResponse(prompt);
        }
    } catch (e) {
        console.error('JSON parse error:', e);
        // Create fallback response
        return createFallbackFrameResponse(prompt);
    }
}

function createFallbackFrameResponse(prompt) {
    // Create a simple fallback response based on common patterns
    const lowerPrompt = prompt.toLowerCase();
    
    let modifications = [];
    
    if (lowerPrompt.includes('fire') || lowerPrompt.includes('flame')) {
        modifications.push({
            condition: "all pixels",
            action: "change to fire colors",
            rgba_transform: [100, 50, -50, 0],
            color_filter: null
        });
    } else if (lowerPrompt.includes('blue')) {
        modifications.push({
            condition: "blue pixels",
            action: "modify blue pixels",
            rgba_transform: [0, 0, 100, 0],
            color_filter: {"r_min": 0, "r_max": 100, "g_min": 0, "g_max": 100, "b_min": 100, "b_max": 255}
        });
    } else if (lowerPrompt.includes('bright')) {
        modifications.push({
            condition: "all pixels",
            action: "brighten",
            rgba_transform: [50, 50, 50, 0],
            color_filter: null
        });
    } else if (lowerPrompt.includes('red')) {
        modifications.push({
            condition: "red pixels",
            action: "enhance red",
            rgba_transform: [50, 0, 0, 0],
            color_filter: {"r_min": 100, "r_max": 255, "g_min": 0, "g_max": 150, "b_min": 0, "b_max": 150}
        });
    } else {
        // Generic color enhancement
        modifications.push({
            condition: "all pixels",
            action: "enhance colors",
            rgba_transform: [20, 20, 20, 0],
            color_filter: null
        });
    }
    
    return {
        modifications: modifications,
        reasoning: `Applied fallback transformation based on prompt: "${prompt}"`
    };
}

function analyzeImageColors() {
    const colorCounts = {};
    const pixels = currentStepppsData.pixels;
    
    Object.values(pixels).forEach(pixel => {
        const [r, g, b] = pixel.pixel.rgba;
        const colorKey = `${Math.floor(r/32)*32},${Math.floor(g/32)*32},${Math.floor(b/32)*32}`;
        colorCounts[colorKey] = (colorCounts[colorKey] || 0) + 1;
    });
    
    // Get top 5 color ranges
    const sortedColors = Object.entries(colorCounts)
        .sort(([,a], [,b]) => b - a)
        .slice(0, 5)
        .map(([color, count]) => ({color, count}));
    
    return sortedColors;
}

async function applyFrameEdit(aiResponse, originalPrompt) {
    if (!aiResponse.modifications) return;
    
    let modifiedCount = 0;
    
    aiResponse.modifications.forEach(mod => {
        Object.entries(currentStepppsData.pixels).forEach(([coord, pixelData]) => {
            const [r, g, b, a] = pixelData.pixel.rgba;
            
            // Check if pixel matches the condition
            const filter = mod.color_filter;
            if (filter && (
                r < filter.r_min || r > filter.r_max ||
                g < filter.g_min || g > filter.g_max ||
                b < filter.b_min || b > filter.b_max
            )) {
                return; // Skip this pixel
            }
            
            // Apply transformation
            let newR = r, newG = g, newB = b, newA = a;
            
            if (mod.action.includes('fire')) {
                // Fire colors: red-orange-yellow gradient
                newR = Math.min(255, r + 100);
                newG = Math.min(255, Math.max(g, 100));
                newB = Math.max(0, b - 50);
            } else if (mod.rgba_transform) {
                const [rChange, gChange, bChange, aChange] = mod.rgba_transform;
                newR = Math.max(0, Math.min(255, r + rChange));
                newG = Math.max(0, Math.min(255, g + gChange));
                newB = Math.max(0, Math.min(255, b + bChange));
                newA = Math.max(0, Math.min(255, a + aChange));
            }
            
            // Update pixel
            pixelData.pixel.rgba = [newR, newG, newB, newA];
            pixelData.pixel.hex = `#${newR.toString(16).padStart(2,'0')}${newG.toString(16).padStart(2,'0')}${newB.toString(16).padStart(2,'0')}`;
            pixelData.pixel.luminance = (0.299 * newR + 0.587 * newG + 0.114 * newB) / 255;
            pixelData.pixel.saturation = calculateSaturation(newR, newG, newB);
            
            // Update intelligence
            pixelData.intelligence.ai_analyzed = true;
            pixelData.intelligence.ai_response = aiResponse.reasoning;
            pixelData.intelligence.edit_prompt = originalPrompt;
            pixelData.intelligence.edit_count += 1;
            pixelData.time.last_edited = new Date().toISOString();
            
            modifiedCount++;
        });
    });
    
    // Force canvas re-render with debug info
    console.log(`Frame edit applied to ${modifiedCount} pixels`);
    console.log('Sample modified pixel:', Object.values(currentStepppsData.pixels)[0]);
    
    await renderStepppsToCanvas();
    updateStats();
    
    // Force a complete canvas refresh
    const canvas = document.getElementById('stepppsCanvas');
    const ctx = canvas.getContext('2d');
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    
    // Re-render immediately
    await renderStepppsToCanvas();
}

function resetFrame() {
    if (!originalImageData || !currentStepppsData) return;
    
    // Reset all pixels to original values
    Object.entries(currentStepppsData.pixels).forEach(([coord, pixelData]) => {
        const [x, y] = coord.split(',').map(Number);
        const width = originalImageData.width;
        const index = (y * width + x) * 4;
        
        const r = originalImageData.data[index];
        const g = originalImageData.data[index + 1];
        const b = originalImageData.data[index + 2];
        const a = originalImageData.data[index + 3];
        
        pixelData.pixel.rgba = [r, g, b, a];
        pixelData.pixel.hex = `#${r.toString(16).padStart(2,'0')}${g.toString(16).padStart(2,'0')}${b.toString(16).padStart(2,'0')}`;
        pixelData.pixel.luminance = (0.299 * r + 0.587 * g + 0.114 * b) / 255;
        pixelData.pixel.saturation = calculateSaturation(r, g, b);
        pixelData.intelligence.ai_analyzed = false;
        pixelData.intelligence.ai_response = null;
        pixelData.intelligence.edit_prompt = null;
        pixelData.intelligence.edit_count = 0;
    });
    
    // Re-render
    renderStepppsToCanvas();
    updateStats();
    
    showStatus('aiStatus', '✅ All pixels reset to original values', 'success');
}

function updateStats() {
    if (!currentStepppsData) return;

    const totalPixels = Object.keys(currentStepppsData.pixels).length;
    const editedPixels = Object.values(currentStepppsData.pixels).filter(p => p.intelligence.ai_analyzed).length;
    const jsonSize = Math.round(JSON.stringify(currentStepppsData).length / 1024);

    document.getElementById('totalPixels').textContent = totalPixels.toLocaleString();
    document.getElementById('editedPixels').textContent = editedPixels.toLocaleString();
    document.getElementById('imageSize').textContent = `${currentStepppsData.canvas.width}×${currentStepppsData.canvas.height}`;
    document.getElementById('jsonSize').textContent = jsonSize + 'KB';
}

async function saveEditedImage() {
    if (!currentStepppsData) return;

    const canvas = document.createElement('canvas');
    canvas.width = currentStepppsData.canvas.width;
    canvas.height = currentStepppsData.canvas.height;
    const ctx = canvas.getContext('2d');
    
    // Render all pixels to canvas
    Object.entries(currentStepppsData.pixels).forEach(([coord, pixelData]) => {
        const [x, y] = coord.split(',').map(Number);
        const rgba = pixelData.pixel.rgba;
        
        ctx.fillStyle = `rgba(${rgba[0]}, ${rgba[1]}, ${rgba[2]}, ${rgba[3]/255})`;
        ctx.fillRect(x, y, 1, 1);
    });

    // Download image
    canvas.toBlob(function(blob) {
        const link = document.createElement('a');
        link.href = URL.createObjectURL(blob);
        link.download = `steppps_edited_${Date.now()}.png`;
        link.click();
    });
}

function exportStepppsJson() {
    if (!currentStepppsData) return;

    const dataStr = JSON.stringify(currentStepppsData, null, 2);
    const dataBlob = new Blob([dataStr], {type: 'application/json'});

    const link = document.createElement('a');
    link.href = URL.createObjectURL(dataBlob);
    link.download = `steppps_data_${Date.now()}.json`;
    link.click();
}

function showStatus(elementId, message, type) {
    const element = document.getElementById(elementId);
    element.textContent = message;
    element.className = `status ${type}`;
}
