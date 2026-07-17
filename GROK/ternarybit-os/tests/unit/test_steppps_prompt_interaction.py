import importlib.util
import pathlib
import unittest
from unittest import mock
import urllib.error
import json


ROOT = pathlib.Path(__file__).resolve().parents[2]
PROXY = ROOT / "deploy" / "alpine" / "llm" / "tbos-llm-proxy.py"
SPEC = importlib.util.spec_from_file_location("tbos_llm_proxy", PROXY)
proxy = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
SPEC.loader.exec_module(proxy)


class PromptInteractionTests(unittest.TestCase):
    def test_v2_prompt(self):
        entity = {
            "id": "steppps://test/v2",
            "prompt": {
                "system": "Be precise",
                "user": "Describe TBOS",
                "next": "Ask about VFS",
                "interaction": "Return plain text",
                "temperature": 0.2,
            },
        }
        prompt = proxy.extract_steppps_prompt(entity)
        self.assertEqual(prompt["user"], "Describe TBOS")
        self.assertEqual(prompt["temperature"], 0.2)
        request = proxy.compose_interaction_prompt(entity, prompt)
        self.assertIn("steppps://test/v2", request)
        self.assertIn("Be precise", request)
        self.assertIn("Ask about VFS", request)

    def test_v1_prompt(self):
        entity = {
            "id": {"uri": "steppps://test/v1"},
            "P_prompt": {
                "creation_prompt": "Explain this entity",
                "next_genai_prompt": "Continue safely",
            },
        }
        prompt = proxy.extract_steppps_prompt(entity)
        self.assertEqual(prompt["user"], "Explain this entity")
        request = proxy.compose_interaction_prompt(entity, prompt)
        self.assertIn("steppps://test/v1", request)

    def test_missing_prompt_text_is_rejected(self):
        with self.assertRaisesRegex(ValueError, "no user text"):
            proxy.extract_steppps_prompt({"P_prompt": {}})

    def test_invalid_temperature_is_rejected(self):
        with self.assertRaisesRegex(ValueError, "between 0 and 2"):
            proxy.extract_steppps_prompt({
                "prompt": {"user": "hello", "temperature": 9}
            })

    def test_backend_failure_is_reportable(self):
        with mock.patch.object(
            proxy.urllib.request,
            "urlopen",
            side_effect=urllib.error.URLError("model offline"),
        ):
            with self.assertRaises(urllib.error.URLError):
                proxy.llama_generate("hello")

    def test_openai_response_output_is_extracted(self):
        response = {
            "output": [{
                "type": "message",
                "content": [{"type": "output_text", "text": "Namaste from TBOS"}],
            }]
        }

        class FakeResponse:
            status = 200

            def __enter__(self):
                return self

            def __exit__(self, *args):
                return False

            def read(self):
                return json.dumps(response).encode()

        with mock.patch.dict(proxy.os.environ, {"OPENAI_API_KEY": "test-key"}):
            with mock.patch.object(proxy.urllib.request, "urlopen", return_value=FakeResponse()):
                self.assertEqual(
                    proxy.openai_generate("hello", model="test-model"),
                    "Namaste from TBOS",
                )

    def test_openai_key_is_required(self):
        with mock.patch.dict(proxy.os.environ, {}, clear=True):
            with self.assertRaisesRegex(RuntimeError, "OPENAI_API_KEY"):
                proxy.openai_generate("hello")

    def test_mock_provider_is_deterministic(self):
        first = proxy.mock_generate("Describe TBOS")
        second = proxy.mock_generate("Describe TBOS")
        self.assertEqual(first, second)
        self.assertIn("prompt accepted", first)

    def test_mock_provider_generates_valid_frame_shape(self):
        frame = json.loads(proxy.mock_generate("boot safely", grammar="present"))
        proxy.validate_frame(frame)
        self.assertEqual(frame["meta"]["profile"], "mock")
        self.assertEqual(frame["events"][0]["kind"], "mock-generation")


if __name__ == "__main__":
    unittest.main()
