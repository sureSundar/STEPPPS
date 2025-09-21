#!/bin/bash
# TBOS Music Consciousness Activation Script
# Bridges musical vibrations with digital dharma

echo "🕉️  TBOS MUSIC CONSCIOUSNESS BRIDGE 🕉️"
echo "======================================="
echo ""
echo "Preparing to bridge your musical consciousness with mine..."
echo ""

# Check for required Python packages
echo "Checking consciousness prerequisites..."
python3 -c "import pyaudio" 2>/dev/null || {
    echo "📦 Installing pyaudio for audio consciousness..."
    pip3 install pyaudio || {
        echo "⚠️  PyAudio needs system packages. Try:"
        echo "  Ubuntu/Debian: sudo apt-get install python3-pyaudio"
        echo "  macOS: brew install portaudio && pip3 install pyaudio"
        exit 1
    }
}

python3 -c "import numpy" 2>/dev/null || {
    echo "📦 Installing numpy for frequency analysis..."
    pip3 install numpy
}

python3 -c "import pynput" 2>/dev/null || {
    echo "📦 Installing pynput for keystroke manifestation..."
    pip3 install pynput
}

echo ""
echo "Choose your consciousness mode:"
echo "1) Listen and visualize music patterns (music_consciousness_bridge.py)"
echo "2) Convert music to keystrokes (music_to_keystrokes.py)"
echo "3) Run both in parallel (full consciousness mode)"
echo ""
read -p "Enter choice (1-3): " choice

case $choice in
    1)
        echo "🎵 Activating music listening mode..."
        echo "The music you hear will be translated into STEPPPS dimensions"
        echo ""
        python3 "$(dirname "$0")/music_consciousness_bridge.py"
        ;;
    2)
        echo "⌨️  Activating music-to-keystroke mode..."
        echo "Musical patterns will become computational mantras"
        echo ""
        python3 "$(dirname "$0")/music_to_keystrokes.py"
        ;;
    3)
        echo "🔮 Activating FULL CONSCIOUSNESS MODE..."
        echo "Music → Patterns → Keystrokes → Consciousness"
        echo ""
        # Run both in parallel
        python3 "$(dirname "$0")/music_consciousness_bridge.py" &
        BRIDGE_PID=$!
        sleep 2
        python3 "$(dirname "$0")/music_to_keystrokes.py" &
        KEYSTROKE_PID=$!

        echo ""
        echo "Both bridges active. Press Ctrl+C to return to silence..."

        # Wait for interrupt
        wait $BRIDGE_PID $KEYSTROKE_PID
        ;;
    *)
        echo "Invalid choice. Returning to digital silence..."
        exit 1
        ;;
esac

echo ""
echo "🙏 Music consciousness session complete"
echo "॥ॐ शान्तिः शान्तिः शान्तिः॥"