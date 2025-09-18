#!/usr/bin/env python3
"""
Debug script to test Claude API call and see actual response
"""

import json
import requests
import os

def test_claude_api():
    # Load API key
    api_key = None
    try:
        if os.path.exists("claude_config.json"):
            with open("claude_config.json", "r") as f:
                config = json.load(f)
                api_key = config.get("anthropic_api_key")
                if api_key:
                    print("✅ Found API key in claude_config.json")
    except Exception as e:
        print(f"❌ Error reading config file: {e}")
        return

    if not api_key:
        print("❌ No API key found")
        return

    # Simple test prompt
    prompt = """
    Please respond with a simple JSON object for testing:
    {
        "generation": 2,
        "test": "success",
        "message": "This is a test response"
    }
    """

    print("🔄 Making API call to Claude...")

    try:
        response = requests.post(
            "https://api.anthropic.com/v1/messages",
            headers={
                "Content-Type": "application/json",
                "x-api-key": api_key,
                "anthropic-version": "2023-06-01"
            },
            json={
                "model": "claude-3-5-sonnet-20241022",
                "max_tokens": 1000,
                "messages": [{"role": "user", "content": prompt}]
            },
            timeout=30
        )

        print(f"📊 Response Status: {response.status_code}")
        print(f"📋 Response Headers: {dict(response.headers)}")

        if response.status_code == 200:
            result = response.json()
            print("✅ API call successful!")
            print(f"📄 Full Response Structure: {json.dumps(result, indent=2)}")

            # Try to extract the text content
            if 'content' in result and len(result['content']) > 0:
                text = result['content'][0]['text']
                print(f"📝 Text Content: {text}")

                # Test JSON extraction
                try:
                    import re
                    start_idx = text.find('{')
                    end_idx = text.rfind('}') + 1

                    if start_idx != -1 and end_idx != -1:
                        json_str = text[start_idx:end_idx]
                        parsed_json = json.loads(json_str)
                        print(f"✅ Successfully parsed JSON: {parsed_json}")
                    else:
                        print("❌ No JSON braces found in response")

                except Exception as e:
                    print(f"❌ JSON parsing failed: {e}")
            else:
                print("❌ No content found in response")

        else:
            print(f"❌ API Error: {response.status_code}")
            print(f"📄 Error Response: {response.text}")

    except Exception as e:
        print(f"❌ Request failed: {e}")

if __name__ == "__main__":
    print("🐛 CLAUDE API DEBUG TEST")
    print("=" * 40)
    test_claude_api()