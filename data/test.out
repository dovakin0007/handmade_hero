#include "handmade.h"
#include <cmath>
#include <cstdio>
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
      uint8 blue = (uint8)(x + x_offset);
      uint8 green = (uint8)(y + y_offset);
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

void game_update_and_render(GameMemory *memory, GameInput *input,
                            GameOffscreenBuffer *buffer,
                            GameSoundOutputBuffer *sound_buffer) {
  Assert(sizeof(GameState) <= memory->permanent_storage_space);
  GameState *game_state = (GameState *)memory->permanent_storage;
  if (!memory->is_initialized) {
    game_state->tone_hz = 256;

#if HANDMADE_INTERNAL
    char *file_name = __FILE__;
    DebugReadFileResult result = debug_platform_read_entire_file(file_name);

    if (result.contents) {
      debug_platform_write_entire_file("C:\\dev\\handmade\\data\\test.out",
                                       result.contents, result.content_size);
      debug_platform_free_file_memory(result.contents);
    }
#endif
    // TODO: this may be more appropriate to do in platform layer
    memory->is_initialized = true;
  }
  for (int controller_idx = 0; controller_idx < ArrayCount(input->controllers);
       ++controller_idx) {
    GameControllerInput *controller = get_controller(input, controller_idx);
    if (controller->is_analog) {

      game_state->blue_offset += (int)(4.0f * (controller->stick_average_x));
      game_state->tone_hz = 256 + (int)(128.0f * (controller->stick_average_y));
    } else {
      if (controller->move_left.ended_down) {
        game_state->blue_offset -= 1;
      }
      if (controller->move_right.ended_down) {
        game_state->blue_offset += 1;
      }
    }
    if (controller->action_down.ended_down) {
      game_state->green_offset += 1;
    }
    if (controller->action_down.ended_down) {

      game_state->green_offset += 1;
    }
  }

  // TODO Allow sample offsets for more robust platform options
  game_output_sound(sound_buffer, game_state->tone_hz);
  RenderWeirdGradient(buffer, game_state->blue_offset,
                      game_state->green_offset);
}
