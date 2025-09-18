#!/usr/bin/env python3
"""
Test script to verify API key loading and JSON parsing fixes
"""

import json
import os

def test_api_key_loading():
    """Test the get_api_key function"""

    # Check environment first
    api_key = os.getenv("ANTHROPIC_API_KEY")
    if api_key:
        print("✅ Found API key in environment")
        return api_key

    # Check config file
    try:
        if os.path.exists("claude_config.json"):
            with open("claude_config.json", "r") as f:
                config = json.load(f)
                api_key = config.get("anthropic_api_key")
                if api_key:
                    print("✅ Found API key in claude_config.json")
                    return api_key
    except Exception as e:
        print(f"⚠️  Error reading config file: {e}")
        return None

    print("❌ No API key found")
    return None

def test_json_parsing():
    """Test the extract_json_from_response function"""
    import re

    def extract_json_from_response(response: str):
        """Extract JSON from Claude's response, handling extra text"""

        # Method 1: Look for JSON block markers
        json_match = re.search(r'```json\s*(\{.*?\})\s*```', response, re.DOTALL)
        if json_match:
            json_str = json_match.group(1)
            return json.loads(json_str)

        # Method 2: Look for JSON without markers
        json_match = re.search(r'(\{.*\})', response, re.DOTALL)
        if json_match:
            json_str = json_match.group(1)
            try:
                return json.loads(json_str)
            except json.JSONDecodeError:
                pass

        # Method 3: Try to parse the entire response as JSON
        try:
            return json.loads(response.strip())
        except json.JSONDecodeError:
            pass

        # Method 4: Look for JSON starting with { and ending with }
        start_idx = response.find('{')
        end_idx = response.rfind('}')

        if start_idx != -1 and end_idx != -1 and end_idx > start_idx:
            json_str = response[start_idx:end_idx + 1]
            try:
                return json.loads(json_str)
            except json.JSONDecodeError:
                pass

        # If all methods fail, raise an exception
        raise ValueError(f"Could not extract valid JSON from Claude response: {response[:200]}...")

    # Test cases
    test_cases = [
        # Case 1: JSON in markdown blocks
        '```json\n{"generation": 2, "test": "success"}\n```',

        # Case 2: JSON with extra text
        'Here is the evolution result: {"generation": 3, "improvements": {"accuracy": 0.05}}. This represents the next generation.',

        # Case 3: Pure JSON
        '{"generation": 4, "data": "pure json"}',

        # Case 4: Malformed case
        'Some text { "generation": 5, "status": "working" } more text'
    ]

    for i, test_case in enumerate(test_cases, 1):
        try:
            result = extract_json_from_response(test_case)
            print(f"✅ Test case {i} passed: {result}")
        except Exception as e:
            print(f"❌ Test case {i} failed: {e}")

if __name__ == "__main__":
    print("🧪 TESTING API FIX")
    print("=" * 40)

    print("\n1. Testing API key loading:")
    api_key = test_api_key_loading()
    if api_key:
        print(f"   API key found: {api_key[:20]}...{api_key[-10:]}")

    print("\n2. Testing JSON parsing:")
    test_json_parsing()

    print("\n✅ All tests completed!")