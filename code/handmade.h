#pragma once

#include <cstdint>

#define internal static
#define local_persist static
#define global_variable static

/*
    NOTE:
    HANDMADE_INTERNAL:
    0 - build for public release
    1 - build for developer only

    HANDMADE_SLOW:
    0 - No Slow code allowed!
    1 - Slow code allowed!
*/
#if HANDMADE_INTERNAL
/*
    These are not for doing anything in relase version - they are
    blocking and the write doesn't protect against lost data
*/
struct DebugReadFileResult {
  uint32 content_size;
  void *contents;
};
internal DebugReadFileResult debug_platform_read_entire_file(char *file_name);
internal void debug_platform_free_file_memory(void *memory);
internal bool32 debug_platform_write_entire_file(char *file_name, void *memory,
                                                 uint32 memory_size);
#endif

#if HANDMADE_SLOW
#define Assert(expression)                                                     \
  if (!(expression)) {                                                         \
    *(int *)0 = 0;                                                             \
  }
#else
#define Assert(expression)
#endif

#define Kilobytes(x) ((x) * 1024)
#define Megabytes(x) (Kilobytes(x) * 1024)
#define Gigabytes(x) (Megabytes(x) * 1024)
#define Terabytes(x) (Gigabytes(x) * 1024)

inline uint32 safe_truncate_uint64(uint64 value) {
  Assert(value < 0xFFFFFFFF);
  uint32 value32 = (uint32)value;
  return value32;
}

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

struct GameMemory {
  bool32 is_initialized;
  uint64 permanent_storage_space;
  void *permanent_storage; // NOTE: REQUIRED to be cleared to zero at startup

  uint64 transient_storage_space;
  void *transient_storage;
};

// Three things - timing, controller/keyboard input, bitmap buffer to use, sound
// buffer to use
internal void game_update_and_render(GameMemory *memory, GameInput *input,
                                     GameOffscreenBuffer *buffer,
                                     GameSoundOutputBuffer *sound_buffer);

struct GameState {
  int tone_hz;
  int green_offset;
  int blue_offset;
};

struct GameClocks {
  real32 seconds_elapsed;
};
