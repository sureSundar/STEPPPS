#!/usr/bin/env python3
"""
FastAPI proxy server for Claude API to bypass CORS restrictions
"""

from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
import httpx
import json
from typing import Dict, Any, List, Optional

app = FastAPI(title="Claude STEPPPS Proxy", version="1.0.0")

# Enable CORS for all origins
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # In production, specify your domain
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

class ClaudeRequest(BaseModel):
    api_key: str
    request_data: Dict[str, Any]

class HealthResponse(BaseModel):
    status: str
    message: str

@app.get("/health", response_model=HealthResponse)
async def health_check():
    """Health check endpoint"""
    return HealthResponse(status="ok", message="Claude STEPPPS Proxy is running")

@app.post("/api/claude")
async def proxy_claude_request(request: ClaudeRequest):
    """
    Proxy Claude API requests to bypass CORS restrictions
    """
    try:
        print(f"🔍 Received request with API key: {request.api_key[:20]}...")
        print(f"🔍 Request data keys: {list(request.request_data.keys())}")

        # Validate API key format
        if not request.api_key.startswith('sk-ant-api03-'):
            print(f"❌ Invalid API key format: {request.api_key[:20]}")
            raise HTTPException(status_code=400, detail="Invalid API key format")

        # Make request to Claude API
        async with httpx.AsyncClient(timeout=30.0) as client:
            print(f"🚀 Making request to Claude API...")
            response = await client.post(
                "https://api.anthropic.com/v1/messages",
                headers={
                    "Content-Type": "application/json",
                    "X-API-Key": request.api_key,
                    "anthropic-version": "2023-06-01"
                },
                json=request.request_data
            )

            print(f"📡 Claude API response status: {response.status_code}")

            if response.status_code != 200:
                error_detail = f"Claude API error: {response.status_code}"
                try:
                    error_data = response.json()
                    error_detail += f" - {error_data.get('error', {}).get('message', 'Unknown error')}"
                    print(f"❌ Claude API error: {error_data}")
                except:
                    error_detail += f" - {response.text}"
                    print(f"❌ Claude API error text: {response.text}")

                raise HTTPException(status_code=response.status_code, detail=error_detail)

            print(f"✅ Claude API success!")
            return response.json()

    except httpx.TimeoutException:
        print(f"⏰ Request to Claude API timed out")
        raise HTTPException(status_code=408, detail="Request to Claude API timed out")
    except httpx.RequestError as e:
        print(f"🌐 Network error: {str(e)}")
        raise HTTPException(status_code=500, detail=f"Network error: {str(e)}")
    except Exception as e:
        print(f"💥 Unexpected error: {str(e)}")
        raise HTTPException(status_code=500, detail=f"Unexpected error: {str(e)}")

@app.get("/")
async def root():
    """Root endpoint with usage information"""
    return {
        "message": "Claude STEPPPS Proxy Server",
        "version": "1.0.0",
        "endpoints": {
            "/health": "Health check",
            "/api/claude": "POST - Proxy Claude API requests",
            "/docs": "API documentation"
        },
        "usage": {
            "description": "This proxy server allows STEPPPS frontend to call Claude API without CORS issues",
            "cors": "Enabled for all origins",
            "timeout": "30 seconds"
        }
    }

if __name__ == "__main__":
    import uvicorn
    print("🚀 Starting Claude STEPPPS Proxy Server...")
    print("📡 Server will be available at: http://localhost:8002")
    print("🔗 API documentation at: http://localhost:8002/docs")
    print("💡 Use this proxy to enable Claude API calls from your STEPPPS frontend")

    uvicorn.run(
        "claude_proxy:app",
        host="0.0.0.0",
        port=8002,
        reload=True,
        log_level="info"
    )