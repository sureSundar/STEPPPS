#!/usr/bin/env python3
"""
FastAPI server for STEPPPS Protocol with Claude AI integration
Solves CORS issues and provides clean API endpoints
"""

from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from fastapi.staticfiles import StaticFiles
from fastapi.responses import FileResponse
from pydantic import BaseModel
import httpx
import json
import asyncio
from typing import Dict, Any, List

app = FastAPI(title="STEPPPS Protocol API", version="1.0.0")

# Add CORS middleware
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Pydantic models for request/response
class ClaudeMessage(BaseModel):
    role: str
    content: str

class ClaudeRequest(BaseModel):
    model: str = "claude-3-sonnet-20240229"
    max_tokens: int = 200
    messages: List[ClaudeMessage]

class STEPPPSRequest(BaseModel):
    api_key: str
    request_data: ClaudeRequest

class STEPPPSPixel(BaseModel):
    x: int
    y: int
    color: str
    region: str
    prompt: str

@app.get("/")
async def root():
    """Serve the main STEPPPS working page"""
    return FileResponse("steppps_working.html")

@app.post("/api/claude")
async def claude_proxy(request: STEPPPSRequest):
    """
    Proxy requests to Claude API to avoid CORS issues
    """
    try:
        # Validate API key
        if not request.api_key.startswith('sk-ant-'):
            raise HTTPException(status_code=400, detail="Invalid Claude API key format")

        # Make request to Claude API
        async with httpx.AsyncClient(timeout=30.0) as client:
            claude_response = await client.post(
                "https://api.anthropic.com/v1/messages",
                headers={
                    "Content-Type": "application/json",
                    "x-api-key": request.api_key,
                    "anthropic-version": "2023-06-01"
                },
                json=request.request_data.dict()
            )

        if claude_response.status_code != 200:
            error_detail = claude_response.text
            try:
                error_json = claude_response.json()
                error_detail = error_json.get('error', {}).get('message', error_detail)
            except:
                pass

            raise HTTPException(
                status_code=claude_response.status_code,
                detail=f"Claude API error: {error_detail}"
            )

        return claude_response.json()

    except httpx.TimeoutException:
        raise HTTPException(status_code=408, detail="Claude API request timeout")
    except httpx.RequestError as e:
        raise HTTPException(status_code=503, detail=f"Network error: {str(e)}")
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Server error: {str(e)}")

@app.post("/api/steppps/pixel")
async def analyze_pixel(request: STEPPPSRequest, pixel: STEPPPSPixel):
    """
    Analyze a STEPPPS pixel with Claude AI
    """
    steppps_prompt = f"""STEPPPS PIXEL ANALYSIS REQUEST:

PIXEL CONTEXT:
- Coordinates: ({pixel.x}, {pixel.y})
- Canvas Region: {pixel.region}
- Color: {pixel.color}

ANALYSIS REQUEST:
{pixel.prompt}

You are an intelligent pixel in the STEPPPS protocol. Respond as this pixel with creative intelligence about your role and context."""

    # Create Claude request
    claude_request = STEPPPSRequest(
        api_key=request.api_key,
        request_data=ClaudeRequest(
            messages=[ClaudeMessage(role="user", content=steppps_prompt)]
        )
    )

    # Use the existing Claude proxy
    response = await claude_proxy(claude_request)

    return {
        "pixel": pixel.dict(),
        "ai_response": response["content"][0]["text"],
        "usage": response.get("usage", {})
    }

@app.get("/health")
async def health_check():
    """Health check endpoint"""
    return {"status": "healthy", "service": "STEPPPS Protocol API"}

if __name__ == "__main__":
    import uvicorn
    print("🚀 Starting STEPPPS FastAPI Server...")
    print("🌐 Main page: http://localhost:8002/")
    print("🔧 Claude API: http://localhost:8002/api/claude")
    print("🧠 Pixel API: http://localhost:8002/api/steppps/pixel")
    print("🛑 Press Ctrl+C to stop")

    uvicorn.run(app, host="0.0.0.0", port=8002, log_level="info")