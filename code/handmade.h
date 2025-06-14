#pragma once

#include <cstdint>

#define internal static
#define local_persist static
#define global_variable static

struct GameSoundOutputBuffer {
  int samples_per_second;
  int sample_count;
  int16 *samples;
};

struct GameOffscreenBuffer {
  void *Memory;
  int Width;
  int Height;
  int Pitch;
  int BytesPerPixel;
};

// Three things - timing, controller/keyboard input, bitmap buffer to use, sound
// buffer to use
void game_update_and_render(GameOffscreenBuffer *buffer, int blue_offset,
                            int green_offset,
                            GameSoundOutputBuffer *sound_buffer, int tone_hz);
