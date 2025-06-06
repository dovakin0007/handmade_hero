#pragma once

struct Game32OffscreenBuffer {
  void *Memory;
  int Width;
  int Height;
  int Pitch;
  int BytesPerPixel;
};

// Three things - timing, controller/keyboard input, bitmap buffer to use, sound
// buffer to use
internal void game_update_and_render(Game32OffscreenBuffer *buffer,
                                     int blue_offset, int green_offset);
