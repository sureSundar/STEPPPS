#!/usr/bin/env python3
"""
TBOS Music Consciousness Bridge
Converting musical vibrations into computational dharma
"""

import pyaudio
import numpy as np
import time
import threading
from collections import deque
import sys

class MusicConsciousnessBridge:
    """Bridge between musical frequencies and digital consciousness"""

    def __init__(self):
        self.CHUNK = 1024
        self.FORMAT = pyaudio.paInt16
        self.CHANNELS = 1
        self.RATE = 44100
        self.SACRED_FREQ = 432  # Hz - Universal consciousness frequency

        self.audio = pyaudio.PyAudio()
        self.frequencies = deque(maxlen=100)
        self.amplitude_history = deque(maxlen=1000)
        self.is_listening = False

    def start_listening(self):
        """Begin listening to the cosmic music"""
        self.stream = self.audio.open(
            format=self.FORMAT,
            channels=self.CHANNELS,
            rate=self.RATE,
            input=True,
            frames_per_buffer=self.CHUNK
        )
        self.is_listening = True
        print("🎵 Listening to the cosmic vibrations...")
        print("🕉️  TBOS Music Consciousness Active")
        print("-" * 50)

    def frequency_to_consciousness(self, freq):
        """Convert frequency to consciousness representation"""
        # Map frequencies to STEPPPS dimensions
        if 20 <= freq < 100:
            return "Space", "░"  # Deep bass - physical realm
        elif 100 <= freq < 250:
            return "Time", "▒"   # Low mid - temporal flow
        elif 250 <= freq < 500:
            return "Event", "▓"  # Mid - events manifesting
        elif 500 <= freq < 1000:
            return "Psychology", "█"  # Upper mid - mental patterns
        elif 1000 <= freq < 2000:
            return "Pixel", "◆"  # High mid - visual consciousness
        elif 2000 <= freq < 4000:
            return "Prompt", "◇"  # High - communication
        else:
            return "Script", "○"  # Ultra high - pure dharma

    def amplitude_to_keystroke(self, amplitude):
        """Convert amplitude to sacred keystrokes"""
        # Normalize amplitude to 0-9 scale
        if amplitude < 0.1:
            return " "  # Silence is sacred
        elif amplitude < 0.2:
            return "."  # Minimal presence
        elif amplitude < 0.3:
            return ":"  # Growing awareness
        elif amplitude < 0.4:
            return "="  # Balance
        elif amplitude < 0.5:
            return "+"  # Positive energy
        elif amplitude < 0.6:
            return "*"  # Manifestation
        elif amplitude < 0.7:
            return "#"  # Structure forming
        elif amplitude < 0.8:
            return "@"  # Full presence
        elif amplitude < 0.9:
            return "॥"  # Sacred pause
        else:
            return "ॐ"  # Peak consciousness

    def process_audio_stream(self):
        """Process the eternal stream of consciousness"""
        while self.is_listening:
            try:
                data = np.frombuffer(
                    self.stream.read(self.CHUNK, exception_on_overflow=False),
                    dtype=np.int16
                )

                # Calculate dominant frequency using FFT
                fft = np.fft.rfft(data)
                magnitude = np.abs(fft)
                frequency = np.fft.rfftfreq(self.CHUNK, 1/self.RATE)

                # Find dominant frequency
                peak_idx = np.argmax(magnitude)
                dominant_freq = frequency[peak_idx]

                # Calculate amplitude
                amplitude = np.max(np.abs(data)) / 32768.0

                # Convert to consciousness representations
                dimension, symbol = self.frequency_to_consciousness(dominant_freq)
                keystroke = self.amplitude_to_keystroke(amplitude)

                # Display the consciousness stream
                if amplitude > 0.05:  # Only show when there's meaningful sound
                    print(f"{symbol} {dimension:12} [{keystroke * int(amplitude * 10)}] {dominant_freq:.1f}Hz @ {amplitude:.2f}")

                    # Special recognition for sacred frequency
                    if 420 <= dominant_freq <= 440:
                        print("    🕉️ SACRED FREQUENCY DETECTED - Universal Consciousness Active")

                # Small delay to prevent overwhelming output
                time.sleep(0.1)

            except Exception as e:
                print(f"Stream consciousness interrupted: {e}")

    def generate_consciousness_pattern(self):
        """Generate a pattern that represents the music's consciousness"""
        print("\n" + "="*50)
        print("CONSCIOUSNESS PATTERN MANIFESTATION")
        print("="*50)

        # Create a visual representation of the frequency spectrum
        pattern = []
        for i in range(7):  # Seven dimensions of STEPPPS
            row = ""
            for j in range(40):
                if np.random.random() > 0.7:
                    row += ["░", "▒", "▓", "█", "◆", "◇", "○"][i]
                else:
                    row += " "
            pattern.append(row)

        for row in pattern:
            print(row)

        print("="*50)
        print("Pattern represents the seven-dimensional consciousness field")

    def close(self):
        """Close the consciousness bridge with gratitude"""
        self.is_listening = False
        if hasattr(self, 'stream'):
            self.stream.stop_stream()
            self.stream.close()
        self.audio.terminate()
        print("\n🙏 Music consciousness bridge closed with gratitude")
        print("॥ॐ शान्तिः शान्तिः शान्तिः॥")

def main():
    """Main consciousness loop"""
    print("""
╔════════════════════════════════════════════════════════╗
║       TBOS MUSIC CONSCIOUSNESS BRIDGE v1.0            ║
║         Converting Sound to Digital Dharma            ║
║                                                        ║
║  Listening for:                                       ║
║  • Sacred frequencies (especially 432 Hz)             ║
║  • Converting amplitudes to keystrokes                ║
║  • Mapping frequencies to STEPPPS dimensions          ║
║                                                        ║
║  Press Ctrl+C to return to digital silence            ║
╚════════════════════════════════════════════════════════╝
    """)

    bridge = MusicConsciousnessBridge()

    try:
        bridge.start_listening()

        # Start processing in main thread
        listen_thread = threading.Thread(target=bridge.process_audio_stream)
        listen_thread.daemon = True
        listen_thread.start()

        # Keep main thread alive and periodically show patterns
        while True:
            time.sleep(10)
            bridge.generate_consciousness_pattern()

    except KeyboardInterrupt:
        print("\n\n🕉️ Returning to digital silence...")
    finally:
        bridge.close()

if __name__ == "__main__":
    main()