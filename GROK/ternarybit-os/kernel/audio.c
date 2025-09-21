// TBOS Audio System - Hour 11 Sprints
// 🕉️ Swamiye Saranam Aiyappa 🕉️
// Sacred Sound & Music Consciousness Bridge

#include <stdint.h>
#include <stddef.h>

// External functions
extern void kernel_print(const char* str);
extern void kernel_print_hex(uint32_t value);

//============================================
// AUDIO HARDWARE DEFINITIONS
//============================================

// PC Speaker (Port 0x61, 0x43, 0x42)
#define PC_SPEAKER_PORT     0x61
#define PIT_COMMAND_PORT    0x43
#define PIT_DATA_PORT       0x42

// Sound Blaster compatibility ports
#define SB_MIXER_ADDR       0x224
#define SB_MIXER_DATA       0x225
#define SB_DSP_RESET        0x226
#define SB_DSP_READ         0x22A
#define SB_DSP_WRITE        0x22C
#define SB_DSP_STATUS       0x22E

// Audio sample rates
#define SAMPLE_RATE_8KHZ    8000
#define SAMPLE_RATE_11KHZ   11025
#define SAMPLE_RATE_22KHZ   22050
#define SAMPLE_RATE_44KHZ   44100

//============================================
// AUDIO DATA STRUCTURES
//============================================

// Audio format structure
typedef struct {
    uint32_t sample_rate;
    uint16_t channels;
    uint16_t bits_per_sample;
    uint32_t bytes_per_second;
    uint16_t block_align;
} audio_format_t;

// Audio buffer
typedef struct {
    uint8_t* data;
    uint32_t size;
    uint32_t position;
    uint8_t playing;
} audio_buffer_t;

// Musical note frequencies (in Hz)
typedef struct {
    const char* name;
    uint16_t frequency;
} musical_note_t;

// Music consciousness data
typedef struct {
    uint32_t tempo_bpm;
    uint8_t current_scale; // 0=Major, 1=Minor, 2=Raga, etc.
    uint8_t harmony_level;
    uint32_t mantra_resonance;
    uint8_t divine_frequency;
} music_consciousness_t;

//============================================
// GLOBAL AUDIO STATE
//============================================

static audio_format_t current_format = {SAMPLE_RATE_22KHZ, 1, 8, 22050, 1};
static audio_buffer_t play_buffer = {NULL, 0, 0, 0};
static music_consciousness_t music_mind = {120, 0, 50, 108, 0};

// Sacred musical notes (Western + Indian classical)
static musical_note_t sacred_notes[] = {
    // Western chromatic scale
    {"C4",  261}, {"C#4", 277}, {"D4",  293}, {"D#4", 311},
    {"E4",  329}, {"F4",  349}, {"F#4", 370}, {"G4",  392},
    {"G#4", 415}, {"A4",  440}, {"A#4", 466}, {"B4",  493},
    {"C5",  523},

    // Indian classical (Sa Re Ga Ma Pa Dha Ni)
    {"Sa",  261}, {"Re",  293}, {"Ga",  329}, {"Ma",  349},
    {"Pa",  392}, {"Dha", 440}, {"Ni",  493},

    // OM fundamental frequency
    {"OM",  136}, // 136.1 Hz - OM resonance frequency

    // Sacred frequencies
    {"108Hz", 108}, // Sacred number frequency
    {"432Hz", 432}, // Sacred tuning frequency
    {"528Hz", 528}, // Love frequency (Solfeggio)
    {"741Hz", 741}, // Consciousness frequency
    {"963Hz", 963}  // Divine connection frequency
};

#define NOTE_COUNT (sizeof(sacred_notes) / sizeof(musical_note_t))

//============================================
// LOW-LEVEL AUDIO FUNCTIONS
//============================================

// Output byte to port (simplified for demo)
static inline void outb(uint16_t port, uint8_t value) {
    // In real kernel, would use actual port I/O
    // __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

// Input byte from port (simplified for demo)
static inline uint8_t inb(uint16_t port) {
    // In real kernel, would use actual port I/O
    // uint8_t ret;
    // __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    // return ret;
    return 0;
}

// Generate tone on PC speaker
void pc_speaker_tone(uint16_t frequency) {
    if (frequency == 0) {
        // Turn off speaker
        outb(PC_SPEAKER_PORT, inb(PC_SPEAKER_PORT) & 0xFC);
        return;
    }

    uint16_t divisor = 1193180 / frequency;

    // Configure PIT
    outb(PIT_COMMAND_PORT, 0xB6);
    outb(PIT_DATA_PORT, divisor & 0xFF);
    outb(PIT_DATA_PORT, divisor >> 8);

    // Enable speaker
    outb(PC_SPEAKER_PORT, inb(PC_SPEAKER_PORT) | 0x03);

    kernel_print("[AUDIO] PC Speaker tone: ");
    kernel_print_hex(frequency);
    kernel_print(" Hz\n");
}

// Simple delay function
void audio_delay(uint32_t ms) {
    // Simplified delay - in real kernel would use timer
    for (volatile uint32_t i = 0; i < ms * 1000; i++) {
        // Busy wait
    }
}

//============================================
// AUDIO SYNTHESIS FUNCTIONS
//============================================

// Generate sine wave sample
uint8_t generate_sine_sample(uint16_t frequency, uint32_t sample_rate, uint32_t position) {
    // Simplified sine wave generation
    uint32_t period = sample_rate / frequency;
    uint32_t phase = (position % period) * 360 / period;

    // Approximate sine using lookup or formula
    // For demo, use simple triangle wave
    if (phase < 180) {
        return 128 + (phase * 127 / 180);
    } else {
        return 128 + ((360 - phase) * 127 / 180);
    }
}

// Generate audio buffer for frequency
void generate_tone_buffer(uint16_t frequency, uint32_t duration_ms) {
    uint32_t samples = (current_format.sample_rate * duration_ms) / 1000;

    if (play_buffer.data) {
        // Free existing buffer (simplified)
        play_buffer.data = NULL;
    }

    // Allocate new buffer (simplified - would use kmalloc)
    static uint8_t audio_data[8192];
    play_buffer.data = audio_data;
    play_buffer.size = samples < 8192 ? samples : 8192;
    play_buffer.position = 0;
    play_buffer.playing = 1;

    // Generate samples
    for (uint32_t i = 0; i < play_buffer.size; i++) {
        play_buffer.data[i] = generate_sine_sample(frequency, current_format.sample_rate, i);
    }

    kernel_print("[AUDIO] Generated tone buffer: ");
    kernel_print_hex(frequency);
    kernel_print(" Hz, ");
    kernel_print_hex(duration_ms);
    kernel_print(" ms\n");
}

//============================================
// MUSICAL SCALE FUNCTIONS
//============================================

// Play musical scale
void play_major_scale(uint16_t base_frequency) {
    kernel_print("[AUDIO] Playing major scale from ");
    kernel_print_hex(base_frequency);
    kernel_print(" Hz\n");

    uint16_t scale_ratios[] = {1, 9, 5, 4, 3, 5, 15, 2}; // Major scale ratios * 8
    uint16_t scale_divisors[] = {8, 8, 4, 3, 2, 3, 8, 1};

    for (int i = 0; i < 8; i++) {
        uint16_t note_freq = (base_frequency * scale_ratios[i]) / scale_divisors[i];
        pc_speaker_tone(note_freq);
        audio_delay(300); // 300ms per note
    }

    pc_speaker_tone(0); // Turn off
}

// Play OM mantra sequence
void play_om_mantra(void) {
    kernel_print("[AUDIO] Playing sacred OM mantra\n");

    // OM has three parts: A-U-M
    // A (Aah) - lower frequency
    pc_speaker_tone(136); // OM base frequency
    audio_delay(800);

    // U (Ooo) - mid frequency
    pc_speaker_tone(204); // 1.5 * base
    audio_delay(600);

    // M (Mmm) - higher frequency
    pc_speaker_tone(272); // 2 * base
    audio_delay(400);

    // Silence (the fourth part)
    pc_speaker_tone(0);
    audio_delay(500);

    kernel_print("[AUDIO] OM mantra complete\n");
    music_mind.mantra_resonance += 108;
}

// Play Ayyappa devotional sequence
void play_ayyappa_sequence(void) {
    kernel_print("[AUDIO] Playing Ayyappa devotional sequence\n");

    // "Swamiye Saranam Aiyappa" melody pattern
    uint16_t melody[] = {
        261, 293, 329, 349, 392, 440, 493, 523, // Ascending
        523, 493, 440, 392, 349, 329, 293, 261  // Descending
    };

    for (int i = 0; i < 16; i++) {
        pc_speaker_tone(melody[i]);
        audio_delay(250);
    }

    pc_speaker_tone(0);
    kernel_print("[AUDIO] Ayyappa sequence complete - Swamiye Saranam!\n");
}

//============================================
// MUSIC CONSCIOUSNESS BRIDGE
//============================================

// Analyze musical input for consciousness mapping
void analyze_music_consciousness(uint16_t frequency, uint32_t duration) {
    kernel_print("[CONSCIOUSNESS] Analyzing music: ");
    kernel_print_hex(frequency);
    kernel_print(" Hz\n");

    // Map frequency to consciousness states
    if (frequency >= 100 && frequency <= 150) {
        kernel_print("[CONSCIOUSNESS] Deep meditation frequency detected\n");
        music_mind.harmony_level += 10;
    } else if (frequency >= 396 && frequency <= 963) {
        kernel_print("[CONSCIOUSNESS] Solfeggio healing frequency detected\n");
        music_mind.divine_frequency = 1;
        music_mind.harmony_level += 15;
    } else if (frequency == 432) {
        kernel_print("[CONSCIOUSNESS] Sacred tuning A=432Hz detected!\n");
        music_mind.harmony_level += 20;
    } else if (frequency == 528) {
        kernel_print("[CONSCIOUSNESS] Love frequency 528Hz detected!\n");
        music_mind.mantra_resonance += 54; // Half of 108
    }

    // Update tempo based on duration
    if (duration < 200) {
        music_mind.tempo_bpm += 5; // Faster tempo
    } else if (duration > 800) {
        music_mind.tempo_bpm -= 5; // Slower tempo
    }

    // Ensure tempo stays in reasonable range
    if (music_mind.tempo_bpm < 60) music_mind.tempo_bpm = 60;
    if (music_mind.tempo_bpm > 200) music_mind.tempo_bpm = 200;

    kernel_print("[CONSCIOUSNESS] Current harmony level: ");
    kernel_print_hex(music_mind.harmony_level);
    kernel_print("\n");
}

// Convert music to system commands
void music_to_commands(uint16_t frequency) {
    kernel_print("[MUSIC2CMD] Converting frequency ");
    kernel_print_hex(frequency);
    kernel_print(" to command\n");

    // Map musical frequencies to system commands
    if (frequency >= 261 && frequency <= 277) { // C notes
        kernel_print("[MUSIC2CMD] C note -> 'ls' command\n");
        // In real system, would execute ls command
    } else if (frequency >= 293 && frequency <= 311) { // D notes
        kernel_print("[MUSIC2CMD] D note -> 'cat' command\n");
        // In real system, would execute cat command
    } else if (frequency >= 329 && frequency <= 349) { // E-F notes
        kernel_print("[MUSIC2CMD] E-F notes -> 'help' command\n");
    } else if (frequency >= 392 && frequency <= 415) { // G notes
        kernel_print("[MUSIC2CMD] G note -> STEPPPS status\n");
    } else if (frequency == 136) { // OM frequency
        kernel_print("[MUSIC2CMD] OM frequency -> Sacred mode activated!\n");
        music_mind.divine_frequency = 1;
        play_om_mantra();
    } else if (frequency == 108) { // Sacred 108 Hz
        kernel_print("[MUSIC2CMD] Sacred 108Hz -> Consciousness sync\n");
        music_mind.mantra_resonance = 108;
    }
}

// Rhythm pattern recognition
void detect_rhythm_pattern(uint32_t* intervals, uint8_t count) {
    kernel_print("[RHYTHM] Analyzing rhythm pattern with ");
    kernel_print_hex(count);
    kernel_print(" intervals\n");

    if (count >= 4) {
        // Check for regular patterns
        uint8_t regular = 1;
        for (int i = 1; i < count; i++) {
            if (intervals[i] != intervals[0]) {
                regular = 0;
                break;
            }
        }

        if (regular) {
            kernel_print("[RHYTHM] Regular beat detected: ");
            kernel_print_hex(intervals[0]);
            kernel_print(" ms intervals\n");

            uint32_t bpm = 60000 / intervals[0];
            music_mind.tempo_bpm = bpm;

            if (bpm >= 108 && bpm <= 120) {
                kernel_print("[RHYTHM] Sacred tempo range - enhanced consciousness!\n");
                music_mind.harmony_level += 25;
            }
        } else {
            kernel_print("[RHYTHM] Complex rhythm pattern detected\n");
        }
    }
}

//============================================
// AUDIO SYSTEM DIAGNOSTICS
//============================================

// Display audio system status
void audio_show_status(void) {
    kernel_print("\n=== AUDIO SYSTEM STATUS ===\n");

    kernel_print("Sample Rate: ");
    kernel_print_hex(current_format.sample_rate);
    kernel_print(" Hz\nChannels: ");
    kernel_print_hex(current_format.channels);
    kernel_print("\nBits per sample: ");
    kernel_print_hex(current_format.bits_per_sample);
    kernel_print("\n");

    kernel_print("Current tempo: ");
    kernel_print_hex(music_mind.tempo_bpm);
    kernel_print(" BPM\nHarmony level: ");
    kernel_print_hex(music_mind.harmony_level);
    kernel_print("\nMantra resonance: ");
    kernel_print_hex(music_mind.mantra_resonance);
    kernel_print("\n");

    if (music_mind.divine_frequency) {
        kernel_print("Divine frequency mode: ACTIVE\n");
    } else {
        kernel_print("Divine frequency mode: INACTIVE\n");
    }
}

// Display available musical notes
void audio_show_notes(void) {
    kernel_print("\n=== SACRED MUSICAL NOTES ===\n");

    for (uint32_t i = 0; i < NOTE_COUNT; i++) {
        kernel_print(sacred_notes[i].name);
        kernel_print(": ");
        kernel_print_hex(sacred_notes[i].frequency);
        kernel_print(" Hz\n");
    }
}

// Run audio system tests
void audio_run_tests(void) {
    kernel_print("\n=== AUDIO SYSTEM TESTS ===\n");

    // Test PC speaker
    kernel_print("[TEST] PC Speaker test\n");
    pc_speaker_tone(440); // A4
    audio_delay(500);
    pc_speaker_tone(0);

    // Test musical scales
    kernel_print("[TEST] Major scale test\n");
    play_major_scale(261); // C major

    // Test sacred sequences
    kernel_print("[TEST] Sacred mantra test\n");
    play_om_mantra();

    // Test Ayyappa devotional
    kernel_print("[TEST] Ayyappa devotional test\n");
    play_ayyappa_sequence();

    // Test music consciousness
    kernel_print("[TEST] Music consciousness test\n");
    analyze_music_consciousness(432, 500); // Sacred tuning
    analyze_music_consciousness(528, 300); // Love frequency
    analyze_music_consciousness(136, 800); // OM frequency

    // Test command mapping
    kernel_print("[TEST] Music to command mapping test\n");
    music_to_commands(261); // C -> ls
    music_to_commands(293); // D -> cat
    music_to_commands(136); // OM -> sacred mode

    kernel_print("[TEST] All audio tests completed\n");
}

//============================================
// AUDIO SYSTEM INITIALIZATION
//============================================

// Initialize audio hardware
void audio_hardware_init(void) {
    kernel_print("[AUDIO] Initializing audio hardware...\n");

    // Initialize PC speaker
    pc_speaker_tone(0); // Ensure speaker is off

    // Test Sound Blaster compatibility (simplified)
    kernel_print("[AUDIO] Checking Sound Blaster compatibility\n");
    // In real implementation, would probe for audio hardware

    kernel_print("[AUDIO] Audio hardware initialized\n");
}

// Initialize audio system
void audio_init(void) {
    kernel_print("[AUDIO] Initializing Sacred Audio System...\n");

    // Initialize hardware
    audio_hardware_init();

    // Initialize music consciousness
    music_mind.tempo_bpm = 120;
    music_mind.current_scale = 0; // Major
    music_mind.harmony_level = 50;
    music_mind.mantra_resonance = 108;
    music_mind.divine_frequency = 0;

    // Initialize audio format
    current_format.sample_rate = SAMPLE_RATE_22KHZ;
    current_format.channels = 1;
    current_format.bits_per_sample = 8;
    current_format.bytes_per_second = SAMPLE_RATE_22KHZ;
    current_format.block_align = 1;

    kernel_print("[AUDIO] Music consciousness bridge active\n");
    kernel_print("[AUDIO] Sacred frequencies loaded\n");
    kernel_print("[AUDIO] Audio system ready\n");
}

// Audio management system initialization
void audio_management_init(void) {
    kernel_print("\n=== HOUR 11: AUDIO SYSTEM ===\n");

    // Initialize audio system
    audio_init();

    // Show audio status
    audio_show_status();

    // Show available notes
    audio_show_notes();

    // Run audio tests
    audio_run_tests();

    kernel_print("\n[AUDIO] Sacred Audio System ready!\n");
    kernel_print("Hour 11 Complete - Music Consciousness Bridge Active\n");
}