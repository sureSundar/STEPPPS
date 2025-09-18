#!/usr/bin/env python3
"""
Simple Claude API proxy server for STEPPPS pixel editing
"""

import uvicorn
from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
import httpx
import json
from typing import List

app = FastAPI(title="Claude API Proxy", version="1.0.0")

# Add CORS middleware
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

class ClaudeMessage(BaseModel):
    role: str
    content: str

class ClaudeRequest(BaseModel):
    model: str = "claude-3-5-sonnet-20241022"
    max_tokens: int = 200
    messages: List[ClaudeMessage]

class STEPPPSRequest(BaseModel):
    api_key: str
    request_data: ClaudeRequest

@app.post("/api/claude")
async def claude_proxy(request: STEPPPSRequest):
    """Proxy requests to Claude API"""
    try:
        print(f"Received request with API key: {request.api_key[:10]}...")
        
        # Validate API key format
        if not request.api_key.startswith('sk-ant-'):
            raise HTTPException(status_code=400, detail="Invalid Claude API key format")
        
        # Prepare request data
        request_data = request.request_data.model_dump()
        print(f"Request data: {json.dumps(request_data, indent=2)}")
        
        # Make request to Claude API
        async with httpx.AsyncClient(timeout=30.0) as client:
            claude_response = await client.post(
                "https://api.anthropic.com/v1/messages",
                headers={
                    "Content-Type": "application/json",
                    "x-api-key": request.api_key,
                    "anthropic-version": "2023-06-01"
                },
                json=request_data
            )
        
        print(f"Claude API response status: {claude_response.status_code}")
        
        if claude_response.status_code != 200:
            error_detail = claude_response.text
            print(f"Claude API error: {error_detail}")
            try:
                error_json = claude_response.json()
                error_detail = error_json.get('error', {}).get('message', error_detail)
            except:
                pass
            
            raise HTTPException(
                status_code=claude_response.status_code,
                detail=f"Claude API error: {error_detail}"
            )
        
        response_data = claude_response.json()
        print(f"Claude API success: {len(response_data.get('content', []))} content items")
        return response_data
        
    except httpx.TimeoutException:
        print("Claude API timeout")
        raise HTTPException(status_code=408, detail="Claude API request timeout")
    except httpx.RequestError as e:
        print(f"Network error: {str(e)}")
        raise HTTPException(status_code=503, detail=f"Network error: {str(e)}")
    except Exception as e:
        print(f"Server error: {str(e)}")
        raise HTTPException(status_code=500, detail=f"Server error: {str(e)}")

@app.get("/health")
async def health_check():
    """Health check endpoint"""
    return {"status": "healthy", "service": "Claude API Proxy"}

if __name__ == "__main__":
    print("🚀 Starting Claude API Proxy Server...")
    print("🔧 Claude API: http://localhost:8005/api/claude")
    print("🛑 Press Ctrl+C to stop")
    
    uvicorn.run(app, host="0.0.0.0", port=8005, log_level="info")
