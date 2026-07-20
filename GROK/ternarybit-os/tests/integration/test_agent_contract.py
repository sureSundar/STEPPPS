from __future__ import annotations

import json
import os
import subprocess
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
CLI = ROOT / "tools" / "tbos-agent"
CONTRACT = ROOT / "config" / "agents" / "sundars-agent.contract.json"


class AgentContractTest(unittest.TestCase):
    def setUp(self) -> None:
        self.temp = tempfile.TemporaryDirectory()
        self.runtime = Path(self.temp.name) / "agents"
        self.env = {**os.environ, "TBOS_AGENT_ROOT": str(self.runtime)}

    def tearDown(self) -> None:
        self.temp.cleanup()

    def run_cli(self, *args: str, expected: int = 0) -> subprocess.CompletedProcess[str]:
        result = subprocess.run(
            [str(CLI), *args], cwd=ROOT, env=self.env,
            text=True, capture_output=True, check=False,
        )
        self.assertEqual(result.returncode, expected, result.stdout + result.stderr)
        return result

    def birth(self) -> None:
        result = self.run_cli("birth", "sundars-agent", str(CONTRACT))
        self.assertIn("BORN: Sundar's Agent", result.stdout)

    def test_birth_copy_sleep_awaken_and_continuity(self) -> None:
        self.birth()
        result = self.run_cli(
            "act", "sundars-agent", "copy",
            "fixtures/source.txt", "workspace/copy.txt",
        )
        self.assertIn("Byte equality: PASS", result.stdout)
        self.assertIn("Prana: 995/1000", result.stdout)
        copied = self.runtime / "sundars-agent" / "workspace" / "copy.txt"
        self.assertEqual(copied.read_bytes(), (ROOT / "fixtures" / "source.txt").read_bytes())
        self.assertIn("Lifecycle: AWAKE", self.run_cli("status", "sundars-agent").stdout)
        self.assertIn("SLEEPING", self.run_cli("sleep", "sundars-agent").stdout)
        sleeping = self.run_cli("status", "sundars-agent").stdout
        self.assertIn("Lifecycle: SLEEPING", sleeping)
        self.assertIn("Prana: 995/1000", sleeping)
        self.assertIn("AWAKE", self.run_cli("awaken", "sundars-agent").stdout)
        self.assertIn("Audit chain: PASS (4 entries)", self.run_cli("audit", "sundars-agent").stdout)

    def test_escape_is_denied_and_audited(self) -> None:
        self.birth()
        result = self.run_cli(
            "act", "sundars-agent", "copy",
            "WISDOM.md", "workspace/escape.txt", expected=2,
        )
        self.assertIn("DENIED: read path outside admitted capability scope", result.stderr)
        self.assertFalse((self.runtime / "sundars-agent" / "workspace" / "escape.txt").exists())
        status = self.run_cli("status", "sundars-agent").stdout
        self.assertIn("Prana: 999/1000", status)
        audit = self.run_cli("audit", "sundars-agent").stdout
        self.assertIn('"event": "DENIED"', audit)

    def test_contract_tamper_suspends_awaken(self) -> None:
        self.birth()
        self.run_cli("sleep", "sundars-agent")
        path = self.runtime / "sundars-agent" / "contract.json"
        contract = json.loads(path.read_text(encoding="utf-8"))
        contract["capabilities"]["network"] = True
        path.write_text(json.dumps(contract), encoding="utf-8")
        result = self.run_cli("awaken", "sundars-agent", expected=2)
        self.assertIn("SUSPENDED: contract integrity failed", result.stderr)


if __name__ == "__main__":
    unittest.main()
