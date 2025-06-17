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

struct GameButtonState {
  int half_transition_count;
  bool32 ended_down;
};

struct GameControllerInput {
  bool32 is_analog;

  real32 start_x;
  real32 start_y;

  real32 min_x;
  real32 min_y;

  real32 max_x;
  real32 max_y;

  real32 end_x;
  real32 end_y;

  union {
    GameButtonState buttons[6];
    struct {
      GameButtonState up;
      GameButtonState down;
      GameButtonState left;
      GameButtonState right;
      GameButtonState left_shoulder;
      GameButtonState right_shoulder;
    };
  };
};
struct GameInput {
  GameControllerInput controllers[4];
};

// Three things - timing, controller/keyboard input, bitmap buffer to use, sound
// buffer to use
internal void game_update_and_render(GameInput *input,
                                     GameOffscreenBuffer *buffer,
                                     GameSoundOutputBuffer *sound_buffer);
