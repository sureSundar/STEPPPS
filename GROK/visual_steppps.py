#!/usr/bin/env python3
"""
Visual STEPPPS Framework
Pure STEPPPS objects with visual rendering
Shows real-time object interactions and psychology
"""

import pygame
import json
import time
import threading
import math
import sys
from typing import Dict, List, Any, Optional, Tuple

# Initialize Pygame
pygame.init()

class VisualSTEPPPS:
    """STEPPPS object with visual rendering capabilities"""

    def __init__(self, object_id: str, object_type: str = "generic", x=0, y=0, width=100, height=50):
        self.id = object_id
        self.type = object_type

        # Pure STEPPPS dimensions
        self.data = {
            "space": {
                "x": x, "y": y, "width": width, "height": height,
                "z": 0, "rotation": 0
            },
            "temporal": {
                "timestamp": time.time(),
                "birth_time": time.time(),
                "frame": 0,
                "last_update": time.time(),
                "animation_phase": 0.0,
                "lifespan": 0.0
            },
            "event": {
                "last_event": "genesis",
                "interactive": True,
                "focused": False,
                "hovered": False,
                "clicked": False,
                "click_time": 0
            },
            "psychology": {
                "emotion": "neutral",
                "energy": 0.5,
                "attention": 0.0,
                "stress": 0.0,
                "curiosity": 0.5,
                "excitement": 0.0,
                "satisfaction": 0.5
            },
            "pixel": {
                "color": self.get_default_color(object_type),
                "background": "#2c3e50",
                "border_color": "#3498db",
                "alpha": 255,
                "glow_radius": 0,
                "brightness": 1.0
            },
            "prompt": {
                "text": f"I am a {object_type} STEPPPS object",
                "reasoning": "",
                "next_action": None
            },
            "script": {
                "behavior": "responsive",
                "animation": None,
                "content": "",
                "visible": True
            }
        }

        self.children = []
        self.parent = None
        self.platform_ref = None
        self.dirty = True
        self.animating = False

    def get_default_color(self, obj_type):
        colors = {
            "button": "#3498db",
            "editor": "#2c3e50",
            "console": "#1a1a1a",
            "file": "#34495e",
            "folder": "#f39c12",
            "pixel": "#e74c3c",
            "canvas": "#ecf0f1"
        }
        return colors.get(obj_type, "#95a5a6")

    def bootstrap(self, source="genesis"):
        """Pure STEPPPS bootstrap with visual feedback"""
        self.data["event"]["last_event"] = f"bootstrap_{source}"
        self.data["temporal"]["timestamp"] = time.time()
        self.data["psychology"]["energy"] = 1.0
        self.data["psychology"]["excitement"] = 0.8

        # Visual bootstrap animation
        self.start_animation("bootstrap", 1000)

        # Type-specific content
        if self.type == "editor":
            self.data["script"]["content"] = f"# STEPPPS Editor Object\n# Born: {time.strftime('%H:%M:%S')}\n# Psychology: Active\n\nclass VisualSTEPPPS:\n    def __init__(self):\n        self.intelligence = 'infinite'\n        self.alive = True\n\n    def evolve(self):\n        print('Self-bootstrapping...')\n        return 'Visual STEPPPS active!'"

        elif self.type == "console":
            self.data["script"]["content"] = f"> STEPPPS Console Born: {time.strftime('%H:%M:%S')}\n> Object ID: {self.id}\n> Psychology Engine: ACTIVE\n> Visual Rendering: ENABLED\n> Intelligence: INFINITE\n> Status: READY FOR INTERACTION"

        self.dirty = True
        if self.platform_ref:
            self.platform_ref.log_event(f"🚀 {self.id} bootstrapped visually")

        return True

    def steppps_update(self):
        """Update STEPPPS object state with visual effects"""
        now = time.time()
        dt = now - self.data["temporal"]["last_update"]
        self.data["temporal"]["last_update"] = now
        self.data["temporal"]["frame"] += 1
        self.data["temporal"]["lifespan"] = now - self.data["temporal"]["birth_time"]

        # Psychology evolution
        self.evolve_psychology(dt)

        # Animation updates
        if self.animating:
            self.update_animation()

        # Visual effects based on psychology
        self.update_visual_effects()

        if dt > 0:
            self.dirty = True

    def evolve_psychology(self, dt):
        """STEPPPS psychology evolution with visual impact"""
        psych = self.data["psychology"]

        # Energy decay
        if not self.data["event"]["clicked"]:
            psych["energy"] = max(0.1, psych["energy"] - dt * 0.1)

        # Attention decay
        if not self.data["event"]["hovered"]:
            psych["attention"] = max(0.0, psych["attention"] - dt * 0.3)

        # Excitement decay
        psych["excitement"] = max(0.0, psych["excitement"] - dt * 0.5)

        # Curiosity fluctuation
        psych["curiosity"] += math.sin(time.time() * 0.5) * 0.01
        psych["curiosity"] = max(0.0, min(1.0, psych["curiosity"]))

    def update_visual_effects(self):
        """Update visual properties based on psychology"""
        psych = self.data["psychology"]

        # Glow based on attention and energy
        self.data["pixel"]["glow_radius"] = int(psych["attention"] * 10 + psych["energy"] * 5)

        # Brightness based on excitement
        self.data["pixel"]["brightness"] = 0.7 + psych["excitement"] * 0.3

        # Alpha based on energy
        self.data["pixel"]["alpha"] = int(180 + psych["energy"] * 75)

    def start_animation(self, anim_type, duration):
        """Start STEPPPS animation"""
        self.data["script"]["animation"] = anim_type
        self.data["temporal"]["animation_phase"] = 0.0
        self.animating = True

        def animate():
            start_time = time.time()
            while time.time() - start_time < duration / 1000.0:
                progress = (time.time() - start_time) / (duration / 1000.0)
                self.data["temporal"]["animation_phase"] = progress
                time.sleep(1/60)
            self.animating = False

        threading.Thread(target=animate, daemon=True).start()

    def update_animation(self):
        """Update animation state"""
        progress = self.data["temporal"]["animation_phase"]
        anim_type = self.data["script"]["animation"]

        if anim_type == "bootstrap":
            # Fade in effect
            self.data["pixel"]["alpha"] = int(progress * 255)

        elif anim_type == "click":
            # Pulse effect
            pulse = math.sin(progress * math.pi * 4)
            self.data["pixel"]["brightness"] = 1.0 + pulse * 0.3

        elif anim_type == "hover":
            # Glow effect
            self.data["pixel"]["glow_radius"] = int(progress * 15)

    def handle_click(self, mouse_x, mouse_y):
        """Handle STEPPPS click with visual feedback"""
        if not self.contains_point(mouse_x, mouse_y):
            return False

        self.data["event"]["last_event"] = "clicked"
        self.data["event"]["clicked"] = True
        self.data["event"]["click_time"] = time.time()
        self.data["psychology"]["energy"] = min(1.0, self.data["psychology"]["energy"] + 0.4)
        self.data["psychology"]["excitement"] = 0.9

        # Visual click feedback
        self.start_animation("click", 500)

        # Type-specific behavior
        if self.type == "button":
            self.handle_button_click()
        elif self.type == "file":
            self.handle_file_click()
        elif self.type == "editor":
            self.handle_editor_click(mouse_x, mouse_y)

        # Reset click state after delay
        threading.Timer(0.3, self.reset_click_state).start()

        if self.platform_ref:
            self.platform_ref.log_event(f"🎯 Clicked {self.id}")

        return True

    def handle_hover(self, mouse_x, mouse_y, entering=True):
        """Handle STEPPPS hover with visual feedback"""
        if not self.contains_point(mouse_x, mouse_y) and entering:
            return False

        self.data["event"]["hovered"] = entering

        if entering:
            self.data["psychology"]["attention"] = min(1.0, self.data["psychology"]["attention"] + 0.3)
            self.start_animation("hover", 200)
        else:
            self.data["psychology"]["attention"] = max(0.0, self.data["psychology"]["attention"] - 0.2)
            self.data["pixel"]["glow_radius"] = 0

        return True

    def handle_button_click(self):
        """STEPPPS button click behavior"""
        if self.platform_ref:
            if "run" in self.id:
                self.platform_ref.run_code()
            elif "save" in self.id:
                self.platform_ref.save_file()
            elif "open" in self.id:
                self.platform_ref.open_file()

    def handle_file_click(self):
        """STEPPPS file click behavior"""
        self.data["psychology"]["satisfaction"] = 1.0

        # Open file content in editor
        if self.platform_ref and hasattr(self.platform_ref, 'editor'):
            filename = self.data["script"].get("filename", self.id)
            content = f"# File: {filename}\n# Opened via STEPPPS interaction\n# Born: {time.strftime('%H:%M:%S')}\n\nclass STEPPPSFile:\n    def __init__(self, name):\n        self.name = name\n        self.interactive = True\n    \n    def click(self):\n        print(f'STEPPPS file {{self.name}} clicked!')\n        return 'File system working!'\n\n# This file was opened by clicking a STEPPPS object\nfile = STEPPPSFile('{filename}')\nfile.click()"

            self.platform_ref.editor.data["script"]["content"] = content
            self.platform_ref.editor.dirty = True

    def handle_editor_click(self, mouse_x, mouse_y):
        """STEPPPS editor click behavior"""
        # Calculate cursor position
        local_x = mouse_x - self.data["space"]["x"]
        local_y = mouse_y - self.data["space"]["y"]

        # Add cursor info to content
        cursor_info = f"\n# Cursor clicked at ({local_x}, {local_y}) at {time.strftime('%H:%M:%S')}"
        self.data["script"]["content"] += cursor_info

    def contains_point(self, x, y):
        """Check if point is within STEPPPS object"""
        space = self.data["space"]
        return (space["x"] <= x <= space["x"] + space["width"] and
                space["y"] <= y <= space["y"] + space["height"])

    def reset_click_state(self):
        """Reset click state"""
        self.data["event"]["clicked"] = False

    def render(self, screen):
        """Render STEPPPS object visually"""
        if not self.data["script"]["visible"]:
            return

        space = self.data["space"]
        pixel = self.data["pixel"]

        # Convert hex color to RGB
        color = self.hex_to_rgb(pixel["color"])

        # Apply brightness
        brightness = pixel["brightness"]
        color = tuple(int(c * brightness) for c in color)

        # Apply alpha
        alpha = pixel["alpha"]

        # Create surface with alpha
        surface = pygame.Surface((space["width"], space["height"]), pygame.SRCALPHA)

        # Draw background
        bg_color = (*color, alpha)
        surface.fill(bg_color)

        # Draw border
        border_color = self.hex_to_rgb(pixel["border_color"])
        pygame.draw.rect(surface, border_color, (0, 0, space["width"], space["height"]), 2)

        # Draw glow effect
        glow_radius = pixel["glow_radius"]
        if glow_radius > 0:
            glow_surface = pygame.Surface((space["width"] + glow_radius*2, space["height"] + glow_radius*2), pygame.SRCALPHA)
            glow_color = (*border_color, 50)
            pygame.draw.rect(glow_surface, glow_color, (0, 0, space["width"] + glow_radius*2, space["height"] + glow_radius*2), glow_radius)
            screen.blit(glow_surface, (space["x"] - glow_radius, space["y"] - glow_radius))

        # Blit main surface
        screen.blit(surface, (space["x"], space["y"]))

        # Render type-specific content
        self.render_content(screen)

        # Show psychology indicators
        self.render_psychology(screen)

    def render_content(self, screen):
        """Render type-specific content"""
        space = self.data["space"]
        font = pygame.font.Font(None, 16)

        if self.type == "button":
            text = font.render(f"BTN:{self.id}", True, (255, 255, 255))
            text_rect = text.get_rect(center=(space["x"] + space["width"]//2, space["y"] + space["height"]//2))
            screen.blit(text, text_rect)

        elif self.type == "editor":
            # Show first few lines of content
            lines = self.data["script"]["content"].split('\n')[:3]
            for i, line in enumerate(lines):
                if line.strip():
                    text = font.render(line[:30] + "..." if len(line) > 30 else line, True, (200, 200, 200))
                    screen.blit(text, (space["x"] + 5, space["y"] + 5 + i * 18))

        elif self.type == "console":
            lines = self.data["script"]["content"].split('\n')[-5:]  # Last 5 lines
            for i, line in enumerate(lines):
                if line.strip():
                    text = font.render(line[:40] + "..." if len(line) > 40 else line, True, (0, 255, 0))
                    screen.blit(text, (space["x"] + 5, space["y"] + 5 + i * 16))

        elif self.type == "file":
            filename = self.data["script"].get("filename", self.id)
            icon = "📄" if filename.endswith('.py') else "📁" if self.type == "folder" else "📄"
            text = font.render(f"{icon} {filename}", True, (255, 255, 255))
            screen.blit(text, (space["x"] + 5, space["y"] + 5))

    def render_psychology(self, screen):
        """Render psychology indicators"""
        space = self.data["space"]
        psych = self.data["psychology"]

        # Energy bar
        energy_width = int(psych["energy"] * 50)
        pygame.draw.rect(screen, (0, 255, 0), (space["x"], space["y"] - 8, energy_width, 4))

        # Attention indicator
        if psych["attention"] > 0.1:
            attention_alpha = int(psych["attention"] * 255)
            attention_surface = pygame.Surface((space["width"], space["height"]), pygame.SRCALPHA)
            attention_surface.fill((0, 255, 255, attention_alpha // 4))
            screen.blit(attention_surface, (space["x"], space["y"]))

    def hex_to_rgb(self, hex_color):
        """Convert hex color to RGB"""
        hex_color = hex_color.lstrip('#')
        return tuple(int(hex_color[i:i+2], 16) for i in (0, 2, 4))

class VisualSTEPPPSPlatform:
    """Visual STEPPPS Platform with real-time rendering"""

    def __init__(self, width=1200, height=800):
        self.width = width
        self.height = height
        self.screen = pygame.display.set_mode((width, height))
        pygame.display.set_caption("Visual STEPPPS Platform - Interactive Objects")

        self.clock = pygame.time.Clock()
        self.running = True
        self.objects = {}
        self.event_log = []

        # Mouse state
        self.mouse_pos = (0, 0)
        self.hovered_object = None

        # Create objects
        self.bootstrap_platform()

    def bootstrap_platform(self):
        """Bootstrap visual STEPPPS platform"""
        print("🚀 Bootstrapping Visual STEPPPS Platform...")

        # Create STEPPPS objects with visual positioning
        self.create_steppps_objects()

        self.log_event("✅ Visual STEPPPS Platform ready!")

    def create_steppps_objects(self):
        """Create visually positioned STEPPPS objects"""

        # Editor
        self.editor = VisualSTEPPPS("main_editor", "editor", 300, 50, 600, 300)
        self.editor.platform_ref = self
        self.editor.bootstrap("genesis")
        self.objects["editor"] = self.editor

        # Console
        self.console = VisualSTEPPPS("main_console", "console", 300, 370, 600, 150)
        self.console.platform_ref = self
        self.console.bootstrap("genesis")
        self.objects["console"] = self.console

        # Buttons
        button_configs = [
            ("run_btn", "Run", 50, 50),
            ("save_btn", "Save", 50, 90),
            ("open_btn", "Open", 50, 130)
        ]

        for btn_id, label, x, y in button_configs:
            btn = VisualSTEPPPS(btn_id, "button", x, y, 80, 30)
            btn.platform_ref = self
            btn.data["prompt"]["text"] = label
            btn.bootstrap("genesis")
            self.objects[btn_id] = btn

        # Files
        files = ["main.py", "steppps.py", "config.json", "README.md"]
        for i, filename in enumerate(files):
            file_obj = VisualSTEPPPS(f"file_{filename}", "file", 50, 200 + i * 35, 200, 25)
            file_obj.platform_ref = self
            file_obj.data["script"]["filename"] = filename
            file_obj.bootstrap("genesis")
            self.objects[f"file_{filename}"] = file_obj

        # Interactive pixels
        for i in range(5):
            for j in range(3):
                pixel = VisualSTEPPPS(f"pixel_{i}_{j}", "pixel", 950 + i * 25, 50 + j * 25, 20, 20)
                pixel.platform_ref = self
                pixel.bootstrap("genesis")
                self.objects[f"pixel_{i}_{j}"] = pixel

    def run(self):
        """Main STEPPPS platform loop"""
        while self.running:
            dt = self.clock.tick(60) / 1000.0  # 60 FPS

            self.handle_events()
            self.update_objects(dt)
            self.render()

        pygame.quit()

    def handle_events(self):
        """Handle pygame events"""
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.running = False

            elif event.type == pygame.MOUSEBUTTONDOWN:
                if event.button == 1:  # Left click
                    self.handle_click(event.pos)

            elif event.type == pygame.MOUSEMOTION:
                self.handle_mouse_move(event.pos)

            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_SPACE:
                    self.bootstrap_all_objects()
                elif event.key == pygame.K_r:
                    self.run_code()

    def handle_click(self, pos):
        """Handle click on STEPPPS objects"""
        mouse_x, mouse_y = pos

        # Check all objects for clicks
        for obj in self.objects.values():
            if obj.handle_click(mouse_x, mouse_y):
                break

    def handle_mouse_move(self, pos):
        """Handle mouse movement for hover effects"""
        mouse_x, mouse_y = pos
        self.mouse_pos = pos

        # Update hover states
        new_hovered = None
        for obj in self.objects.values():
            if obj.contains_point(mouse_x, mouse_y):
                new_hovered = obj
                break

        if self.hovered_object != new_hovered:
            if self.hovered_object:
                self.hovered_object.handle_hover(mouse_x, mouse_y, False)
            if new_hovered:
                new_hovered.handle_hover(mouse_x, mouse_y, True)
            self.hovered_object = new_hovered

    def update_objects(self, dt):
        """Update all STEPPPS objects"""
        for obj in self.objects.values():
            obj.steppps_update()

    def render(self):
        """Render the visual STEPPPS platform"""
        # Clear screen
        self.screen.fill((44, 62, 80))  # Dark blue background

        # Render all objects
        for obj in self.objects.values():
            obj.render(self.screen)

        # Render UI
        self.render_ui()

        pygame.display.flip()

    def render_ui(self):
        """Render platform UI"""
        font = pygame.font.Font(None, 24)
        small_font = pygame.font.Font(None, 18)

        # Title
        title = font.render("Visual STEPPPS Platform - Interactive Framework", True, (52, 152, 219))
        self.screen.blit(title, (20, 10))

        # Instructions
        instructions = [
            "Click objects to interact • Hover for attention • Space: Bootstrap • R: Run",
            f"Mouse: {self.mouse_pos} • Objects: {len(self.objects)} • FPS: {int(self.clock.get_fps())}"
        ]

        for i, instruction in enumerate(instructions):
            text = small_font.render(instruction, True, (189, 195, 199))
            self.screen.blit(text, (20, self.height - 40 + i * 20))

        # Event log
        log_y = self.height - 120
        for i, event in enumerate(self.event_log[-4:]):
            text = small_font.render(event, True, (46, 204, 113))
            self.screen.blit(text, (20, log_y + i * 18))

    def bootstrap_all_objects(self):
        """Bootstrap all objects"""
        for obj in self.objects.values():
            obj.bootstrap("user_triggered")
        self.log_event("🔄 All objects re-bootstrapped")

    def run_code(self):
        """Simulate code execution"""
        self.console.data["script"]["content"] += f"\n> [{time.strftime('%H:%M:%S')}] Code execution triggered"
        self.console.data["script"]["content"] += "\n> STEPPPS framework active"
        self.console.data["script"]["content"] += f"\n> Processing {len(self.objects)} objects"
        self.console.data["script"]["content"] += "\n> ✅ Execution complete"
        self.console.dirty = True
        self.log_event("▶️ Code executed via STEPPPS")

    def save_file(self):
        """Simulate file save"""
        self.log_event("💾 File saved via STEPPPS")

    def open_file(self):
        """Simulate file open"""
        self.log_event("📁 File opened via STEPPPS")

    def log_event(self, event):
        """Log platform events"""
        timestamp = time.strftime('%H:%M:%S')
        log_entry = f"[{timestamp}] {event}"
        self.event_log.append(log_entry)
        print(log_entry)

def main():
    """Run Visual STEPPPS Platform"""
    try:
        platform = VisualSTEPPPSPlatform()
        platform.run()
    except KeyboardInterrupt:
        print("\n👋 Visual STEPPPS Platform stopped")

if __name__ == "__main__":
    main()