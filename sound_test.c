#include <stdio.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include <math.h>
#include <stdint.h>

#ifndef M_PI
    #define M_PI 3.14159265358979323846 // Define M_PI if not already defined
#endif

#define SAMPLE_RATE 44100  // Sampling rate (44.1 kHz)
#define BEEP_DURATION 0.2  // Duration of each beep in seconds
#define FREQUENCY 880.0    // Frequency of the beep (Hz)

// Function to generate a tone
void generate_tone(int16_t *buffer, float frequency, float duration, float volume) {
    int samples = (int)(SAMPLE_RATE * duration);
    for (int i = 0; i < samples; i++) {
        buffer[i] = (int16_t)(volume * 32767 * sin(2 * M_PI * frequency * i / SAMPLE_RATE));
    }
}

int beep_sound() {
    // Volume control: value between 0.0 (silent) and 1.0 (full volume)
    float volume = 0.2; // Adjusted to 30% volume

    // PulseAudio sample format
    static const pa_sample_spec sample_spec = {
        .format = PA_SAMPLE_S16LE, // 16-bit little-endian PCM
        .rate = SAMPLE_RATE,       // Sample rate
        .channels = 1              // Mono audio
    };

    // Create a new playback stream
    pa_simple *stream = NULL;
    int error;

    stream = pa_simple_new(
        NULL,               // Use default server
        "DigitalAlarm",     // Application name
        PA_STREAM_PLAYBACK, // Playback stream
        NULL,               // Default device
        "Alarm Sound",      // Stream description
        &sample_spec,       // Sample format
        NULL,               // Default channel map
        NULL,               // Default buffering attributes
        &error              // Error code
    );

    if (!stream) {
        fprintf(stderr, "Failed to connect to PulseAudio: %s\n", pa_strerror(error));
        return 1;
    }

    // Generate the beep sound buffer
    int beep_samples = (int)(SAMPLE_RATE * BEEP_DURATION);
    int16_t beep_buffer[beep_samples];
    generate_tone(beep_buffer, FREQUENCY, BEEP_DURATION, volume);

    // Short silence buffer (100ms)
    int short_pause_samples = SAMPLE_RATE * 0.1;
    int16_t short_pause_buffer[short_pause_samples];
    for (int i = 0; i < short_pause_samples; i++) short_pause_buffer[i] = 0;

    // Play the first beep
    if (pa_simple_write(stream, beep_buffer, sizeof(beep_buffer), &error) < 0) {
        fprintf(stderr, "Failed to write data to PulseAudio stream: %s\n", pa_strerror(error));
        pa_simple_free(stream);
        return 1;
    }

    // Short silence
    if (pa_simple_write(stream, short_pause_buffer, sizeof(short_pause_buffer), &error) < 0) {
        fprintf(stderr, "Failed to write data to PulseAudio stream: %s\n", pa_strerror(error));
        pa_simple_free(stream);
        return 1;
    }

    // Play the second beep
    if (pa_simple_write(stream, beep_buffer, sizeof(beep_buffer), &error) < 0) {
        fprintf(stderr, "Failed to write data to PulseAudio stream: %s\n", pa_strerror(error));
        pa_simple_free(stream);
        return 1;
    }

    // Cleanup
    if (pa_simple_drain(stream, &error) < 0) {
        fprintf(stderr, "Failed to drain PulseAudio stream: %s\n", pa_strerror(error));
        pa_simple_free(stream);
        return 1;
    }

    pa_simple_free(stream);
    // printf("Beep sequence completed.\n");

    return 0;
}
