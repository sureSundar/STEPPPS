import pyautogui
import json
import requests
import tkinter as tk
from tkinter import messagebox
import time

# STEPPPS JSON Schema (simplified)
steppps_schema = {
    "prompt": {"type": "string", "description": "NLP input for LLM"},
    "event": {"type": "object", "properties": {
        "lastEvent": {"type": "string"},
        "eventData": {"type": "object"}
    }},
    "psychology": {"type": "object", "properties": {
        "emotion": {"type": "string", "enum": ["neutral", "urgent"]},
        "confidence": {"type": "number", "minimum": 0, "maximum": 1}
    }},
    "actions": {"type": "array", "items": {"type": "object", "properties": {
        "type": {"type": "string", "enum": ["open", "type", "save"]},
        "value": {"type": "string"}
    }}}
}

class STEPPPSAgent:
    def __init__(self):
        self.data = {
            "prompt": "",
            "event": {"lastEvent": None, "eventData": {}},
            "psychology": {"emotion": "neutral", "confidence": 0.5},
            "actions": []
        }
        pyautogui.FAILSAFE = True  # Move mouse to top-left to abort

    def update_prompt(self, user_input):
        self.data["prompt"] = user_input
        self.data["event"] = {"lastEvent": "textInput", "eventData": {"text": user_input}}
        self.data["psychology"]["emotion"] = "urgent" if "urgent" in user_input.lower() else "neutral"
        self.generate_actions()

    def generate_actions(self):
        prompt = f"Convert this task to GUI actions for macOS: {self.data['prompt']}. Return a JSON array of actions (e.g., {{'type': 'open', 'value': 'Notes.app'}})."
        # Use local Ollama (replace with your setup)
        response = requests.post("http://localhost:11434/api/generate", json={
            "model": "llama3.1",
            "prompt": prompt
        }).json()
        actions = json.loads(response.get("response", "[]"))
        self.data["actions"] = actions
        return actions

    def confirm_and_execute(self, root):
        actions = self.data["actions"]
        if not actions:
            messagebox.showerror("Error", "No actions generated!")
            return
        action_str = "\n".join([f"{a['type']}: {a['value']}" for a in actions])
        if messagebox.askokcancel("Confirm Actions", f"Execute these actions?\n{action_str}"):
            for action in actions:
                if action["type"] == "open":
                    pyautogui.hotkey("cmd", "t")  # Open Terminal
                    time.sleep(1)
                    pyautogui.write(f"open -a {action['value']}")
                    pyautogui.press("enter")
                    time.sleep(2)
                elif action["type"] == "type":
                    pyautogui.write(action["value"])
                    pyautogui.press("enter")
                elif action["type"] == "save":
                    pyautogui.hotkey("cmd", "s")
                    time.sleep(1)
                    pyautogui.write(action["value"])
                    pyautogui.press("enter")
            messagebox.showinfo("Success", "Actions completed!")
            self.visualize(root)
        else:
            messagebox.showinfo("Cancelled", "Actions cancelled.")

    def visualize(self, root):
        canvas = tk.Canvas(root, width=100, height=100, bg="white")
        canvas.pack()
        color = "#FF0000" if self.data["psychology"]["emotion"] == "urgent" else "#00FF00"
        canvas.create_rectangle(45, 45, 55, 55, fill=color)  # Simple pixel block

# GUI for NLP input
def create_ui():
    root = tk.Tk()
    root.title("STEPPPS App Agent")
    agent = STEPPPSAgent()
    tk.Label(root, text="Enter task (e.g., 'Open Notes and write Meeting at 3 PM')").pack()
    entry = tk.Entry(root, width=50)
    entry.pack()
    tk.Button(root, text="Run", command=lambda: [agent.update_prompt(entry.get()), agent.confirm_and_execute(root)]).pack()
    root.mainloop()

if __name__ == "__main__":
    create_ui()
