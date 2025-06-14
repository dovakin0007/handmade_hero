#include "handmade.h"
#include <cmath>
#include <windows.h>

internal void RenderWeirdGradient(GameOffscreenBuffer *buffer, int x_offset,
                                  int y_offset) {
  uint8 *row = (uint8 *)buffer->Memory;
  for (int y = 0; y < buffer->Height; ++y) {
    // the size is 32 cause of the RGB padding is set to 32
    uint32 *Pixel = (uint32 *)row;
    for (int x = 0; x < buffer->Width; ++x) {
      /*
                          8  8  8  8
          Pixel in memory BB GG RR xx
          Little Endian arch
      */

      /*
        Memory:    BB GG RR xx
        Register:  xx RR GG BB
      */
      uint8 blue = (x + x_offset);
      uint8 green = (y + y_offset);
      *Pixel++ = ((green << 8) | blue);
    }
    row += buffer->Pitch;
  }
}

internal void game_output_sound(GameSoundOutputBuffer *sound_buffer,
                                int tone_hz) {
  local_persist real32 t_sine = 0.0f;
  int16 tone_volume = 3000;
  int wave_period = sound_buffer->samples_per_second / tone_hz;

  int16 *sample_out = sound_buffer->samples;
  for (int sample_idx = 0; sample_idx < sound_buffer->sample_count;
       ++sample_idx) {
    int16 value = (int16)(sinf(t_sine) * tone_volume);
    *sample_out++ = value;
    *sample_out++ = value;
    t_sine += 2.0f * PI32 * (1.0f / (real32)wave_period);
    // ++sound_output->running_sample_index;
  }
}

void game_update_and_render(GameOffscreenBuffer *buffer, int blue_offset,
                            int green_offset,
                            GameSoundOutputBuffer *sound_buffer, int tone_hz) {

  // TODO Allow sample offsets for more robust platform options
  game_output_sound(sound_buffer, tone_hz);
  RenderWeirdGradient(buffer, blue_offset, green_offset);
}
