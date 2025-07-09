
/*
- Saved game locations
- Getting a handle to our own executable file
- Asset loading path
- Threading (launch a thread)
- Raw Input (support for multiple keyboards)
- Sleep/timeBeginPeriod
- ClipCursor() (for multimonitor support)
- Fullscreen support
- WM_SETCURSOR (control cursor visibility)
- QueryCancelAutoplay
- WM_ACTIVATEAPP (for when we are not the active application)
- Blit speed improvements (BitBlt)
- Hardware acceleration (OpenGL or Direct3D or BOTH??)
- GetKeyboardLayout (for French keyboards, international WASD support)
*/

#include <DSound.h>
#include <Windows.h>
#include <Xinput.h>
#include <cmath>
#include <cstdint>
#include <ctime>
#include <debugapi.h>
#include <fileapi.h>
#include <handleapi.h>
#include <intrin.h>
#include <memoryapi.h>
#include <minwindef.h>
#include <profileapi.h>
#include <stdio.h>
#include <synchapi.h>
#include <timeapi.h>
#include <winerror.h>
#include <winnt.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winuser.h>

#define internal static
#define local_persist static
#define global_variable static

#define PI32 3.1415927f

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

#include "handmade.cpp"
#include "win32_handmade.h"

global_variable bool GLOBALRUNNING;
global_variable Win32OffscreenBuffer global_back_buffer;
global_variable LPDIRECTSOUNDBUFFER global_secondary_buffer;

// Xinput Get state
#define X_INPUT_GET_STATE(name)                                                \
  DWORD WINAPI name(DWORD dwUserIndex, _XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub) { return ERROR_DEVICE_NOT_CONNECTED; }
#define XInputGetState XInputGetState_
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;

// Xinput Set state
#define X_INPUT_SET_STATE(name)                                                \
  DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub) { return ERROR_DEVICE_NOT_CONNECTED; }
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

#define DIRECT_SOUND_CREATE(name)                                              \
  HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS,               \
                      LPUNKNOWN pUnkOuter);
typedef DIRECT_SOUND_CREATE(direct_sound_create);

#if HANDMADE_INTERNAL
DebugReadFileResult debug_platform_read_entire_file(char *file_name) {
  DebugReadFileResult debug_file_result = {};
  HANDLE file_handle = CreateFile(file_name, GENERIC_READ, FILE_SHARE_READ, 0,
                                  OPEN_EXISTING, 0, 0);

  if (file_handle != INVALID_HANDLE_VALUE) {
    LARGE_INTEGER file_size;
    if (GetFileSizeEx(file_handle, &file_size)) {

      uint32 file_size32 = safe_truncate_uint64(file_size.QuadPart);
      debug_file_result.contents = VirtualAlloc(
          0, file_size32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
      if (debug_file_result.contents) {
        DWORD bytes_read;
        if (ReadFile(file_handle, debug_file_result.contents, file_size32,
                     &bytes_read, 0) &&
            (bytes_read == file_size32)) {
          debug_file_result.content_size = file_size32;
        } else {
          debug_platform_free_file_memory(debug_file_result.contents);
          debug_file_result.contents = 0;
          debug_file_result.content_size = 0;
        }
      } else {
      }
    } else {
    }
    CloseHandle(file_handle);
  } else {
  }

  return debug_file_result;
}
internal void debug_platform_free_file_memory(void *memory) {
  VirtualFree(memory, 0, MEM_RELEASE);
}
internal bool32 debug_platform_write_entire_file(char *file_name, void *memory,
                                                 uint32 memory_size) {
  bool32 result = false;
  HANDLE file_handle =
      CreateFile(file_name, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
  if (file_handle != INVALID_HANDLE_VALUE) {
    DWORD bytes_written;
    if (WriteFile(file_handle, memory, memory_size, &bytes_written, 0)) {
      result = (bytes_written == memory_size);
    } else {
      // TODO: Logging
    }
    CloseHandle(file_handle);
  } else {
    // TODO: Logging
  }

  return result;
}

#endif

internal void win32_init_direct_audio(HWND Window, int32 SamplesPerSec,
                                      int32 BufferSize) {
  HMODULE library = LoadLibraryA("dsound.dll");
  if (library) {
    direct_sound_create *DirectSoundCreate =
        (direct_sound_create *)GetProcAddress(library, "DirectSoundCreate");

    LPDIRECTSOUND direct_sound;

    if (SUCCEEDED(DirectSoundCreate(0, &direct_sound, 0))) {
      if (SUCCEEDED(
              direct_sound->SetCooperativeLevel(Window, DSSCL_PRIORITY))) {
        OutputDebugStringA("Set cooperative level ok\n");
      } else {
        // TODO: logging
      }

      WAVEFORMATEX wave_format = {};
      wave_format.wFormatTag = WAVE_FORMAT_PCM;
      wave_format.nChannels = 2;
      wave_format.nSamplesPerSec = SamplesPerSec;
      wave_format.wBitsPerSample = 16;
      wave_format.nBlockAlign =
          wave_format.nChannels * wave_format.wBitsPerSample / 8;
      wave_format.nAvgBytesPerSec =
          wave_format.nSamplesPerSec * wave_format.nBlockAlign;

      {
        DSBUFFERDESC buffer_desc = {};
        buffer_desc.dwSize = sizeof(buffer_desc);
        buffer_desc.dwFlags = DSBCAPS_PRIMARYBUFFER;
        LPDIRECTSOUNDBUFFER primary_buffer;
        if (SUCCEEDED(direct_sound->CreateSoundBuffer(&buffer_desc,
                                                      &primary_buffer, 0))) {
          OutputDebugStringA("Create primary buffer ok\n");
          if (SUCCEEDED(primary_buffer->SetFormat(&wave_format))) {
            OutputDebugStringA("Primary buffer set format ok\n");
          } else {
            // TDOO: logging
          }
        }
      }

      DSBUFFERDESC buffer_desc = {};
      buffer_desc.dwSize = sizeof(buffer_desc);
      buffer_desc.dwFlags = DSBCAPS_GETCURRENTPOSITION2;
      buffer_desc.dwBufferBytes = BufferSize;
      buffer_desc.lpwfxFormat = &wave_format;
      if (SUCCEEDED(direct_sound->CreateSoundBuffer(
              &buffer_desc, &global_secondary_buffer, 0))) {
        OutputDebugStringA("Secondary buffer created\n");
      } else {
        // TODO: logging
      }

    } else {
      // TODO: logging
    }
  } else {
    // TODO: logging
  }
}

// TODO: DIAGNOSTIC LOGGING
internal void win32_load_xinput(void) {
  HMODULE xinput_library = LoadLibrary("Xinput9_1_0.dll");
  if (!xinput_library) {
    xinput_library = LoadLibrary("Xinput1_4.dll");
  }
  if (xinput_library) {
    XInputGetState =
        (x_input_get_state *)GetProcAddress(xinput_library, "XInputGetState");
    XInputSetState =
        (x_input_set_state *)GetProcAddress(xinput_library, "XInputSetState");
  } else {
    OutputDebugString("Failed to load Xinput dll\n");
  }
}

internal Win32WindowDimension win32_get_window_dimension(HWND window_handle) {
  Win32WindowDimension window_dimensions;
  RECT client_rect;
  GetClientRect(window_handle, &client_rect);

  window_dimensions.Width = client_rect.right - client_rect.left;
  window_dimensions.Height = client_rect.bottom - client_rect.top;

  return window_dimensions;
}

internal void win32_process_digital_xinput_button(DWORD x_input_button_state,
                                                  GameButtonState *old_state,
                                                  DWORD button_bit,
                                                  GameButtonState *new_state) {

  new_state->ended_down = (x_input_button_state & button_bit);
  new_state->half_transition_count =
      old_state->ended_down != new_state->ended_down ? 1 : 0;
}

internal void win32_process_key_board_message(GameButtonState *new_state,
                                              bool32 is_down) {
  Assert(is_down != new_state->ended_down);
  new_state->ended_down = is_down;
  ++new_state->half_transition_count;
}

internal real32 win32_process_input_stick_value(SHORT value,
                                                SHORT dead_zone_threshold) {
  real32 result = 0;
  if (value < -dead_zone_threshold) {
    result = (real32)value / 32768.0f;
  } else if (value > dead_zone_threshold) {
    result = (real32)value / 32767.0f;
  }
  return result;
}

internal void
win32_process_pending_messages(GameControllerInput *keyboard_controller) {
  MSG message = {};
  while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
    switch (message.message) {
    case WM_QUIT: {
      GLOBALRUNNING = false;
      break;
    }

    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP: {
      uint32 vk_code = (uint32)message.wParam;
      bool was_down = ((message.lParam & (1 << 30))) != 0;
      bool is_down = ((message.lParam & (1 << 31))) == 0;
      if (was_down != is_down) {
        if (vk_code == 'W') {
          win32_process_key_board_message(&keyboard_controller->move_up,
                                          is_down);
        } else if (vk_code == 'A') {
          win32_process_key_board_message(&keyboard_controller->move_left,
                                          is_down);
        } else if (vk_code == 'S') {
          win32_process_key_board_message(&keyboard_controller->move_down,
                                          is_down);
        } else if (vk_code == 'D') {
          win32_process_key_board_message(&keyboard_controller->move_right,
                                          is_down);
        } else if (vk_code == 'Q') {
          win32_process_key_board_message(&keyboard_controller->left_shoulder,
                                          is_down);
        } else if (vk_code == 'E') {
          win32_process_key_board_message(&keyboard_controller->right_shoulder,
                                          is_down);
        } else if (vk_code == VK_UP) {
          win32_process_key_board_message(&keyboard_controller->action_up,
                                          is_down);
        } else if (vk_code == VK_LEFT) {
          win32_process_key_board_message(&keyboard_controller->action_left,
                                          is_down);
        } else if (vk_code == VK_DOWN) {
          win32_process_key_board_message(&keyboard_controller->action_down,
                                          is_down);
        } else if (vk_code == VK_RIGHT) {
          win32_process_key_board_message(&keyboard_controller->action_right,
                                          is_down);
        } else if (vk_code == VK_ESCAPE) {
          win32_process_key_board_message(&keyboard_controller->back, is_down);
        } else if (vk_code == VK_SPACE) {
          win32_process_key_board_message(&keyboard_controller->start, is_down);
        }
      }
      bool32 alt_key_was_down = ((message.lParam & (1 << 29)));
      if ((vk_code == VK_F4) && alt_key_was_down) {
        GLOBALRUNNING = false;
      }
      break;
    }
    default: {
      TranslateMessage(&message);
      DispatchMessage(&message);
      break;
    }
    }
  }
}

// Resizes or recreates the DIB section to match the new window dimensions
internal void Win32ResizeDIBSection(Win32OffscreenBuffer *buffer, int width,
                                    int height) {

  if (buffer->Memory) {
    VirtualFree(buffer->Memory, 0, MEM_RELEASE);
  }
  // height is negative which allows to treat the bit map as top-down rather
  // than bottom-up meaning that first three bytes of the pixel are from top
  // left of the bit map rather than bottom left
  buffer->BytesPerPixel = 4;
  buffer->Width = width;
  buffer->Height = height;
  buffer->Info.bmiHeader.biSize = sizeof(buffer->Info.bmiHeader);
  buffer->Info.bmiHeader.biWidth = buffer->Width;
  buffer->Info.bmiHeader.biHeight = -buffer->Height;
  buffer->Info.bmiHeader.biPlanes = 1;
  buffer->Info.bmiHeader.biBitCount = 32;
  buffer->Info.bmiHeader.biCompression = BI_RGB;
  int bitmap_memory_size =
      (buffer->Width * buffer->Height) * buffer->BytesPerPixel;
  buffer->Memory = VirtualAlloc(0, bitmap_memory_size, MEM_COMMIT | MEM_RESERVE,
                                PAGE_READWRITE);
  buffer->Pitch = width * buffer->BytesPerPixel;
}
// hInstance is window instance
internal void Win32DisplayBufferInWindow(HDC device_context,
                                         Win32WindowDimension window_dimension,
                                         Win32OffscreenBuffer *buffer, int x,
                                         int y, int width, int height) {
  // Pitch refers to the number of bytes from one row to another
  // Copies from previous screen buffer to new screen buffer
  StretchDIBits(device_context, 0, 0, window_dimension.Width,
                window_dimension.Height, 0, 0, buffer->Width, buffer->Height,
                buffer->Memory, &buffer->Info, DIB_RGB_COLORS, SRCCOPY);
}

LRESULT Win32MainWindowCallback(HWND hInstance, UINT uMsg, WPARAM wParam,
                                LPARAM lParam) {
  LRESULT result = 0;
  switch (uMsg) {
  case WM_SIZE: {
    break;
  };

  case WM_SYSKEYUP:
  case WM_KEYDOWN:
  case WM_SYSKEYDOWN:
  case WM_KEYUP: {
    Assert(!"Keyboard input came in through a non dispatch event");
    break;
  };

  case WM_DESTROY: {
    GLOBALRUNNING = false;
    break;
  };
  case WM_CLOSE: {
    GLOBALRUNNING = false;
    break;
  };
  case WM_ACTIVATEAPP: {
    OutputDebugString("WM_ACTIVATEAPP\n");
    break;
  };
  case WM_PAINT: {
    PAINTSTRUCT paint = {};
    HDC device_context = BeginPaint(hInstance, &paint);
    int x = paint.rcPaint.left;
    int y = paint.rcPaint.top;
    int height = paint.rcPaint.bottom - paint.rcPaint.top;
    int width = paint.rcPaint.right - paint.rcPaint.left;
    Win32WindowDimension dimension = win32_get_window_dimension(hInstance);
    Win32DisplayBufferInWindow(device_context, dimension, &global_back_buffer,
                               x, y, width, height);
    EndPaint(hInstance, &paint);
    break;
  };
  default: {
    result = DefWindowProc(hInstance, uMsg, wParam, lParam);
    break;
  };
  }

  return result;
}

internal void win32_clear_sound_buffer(Win32_Sound_Output *sound_output) {
  VOID *region1;
  DWORD region1_size;
  VOID *region2;
  DWORD region2_size;

  if (SUCCEEDED(global_secondary_buffer->Lock(
          0, sound_output->secondary_buffer_size, &region1, &region1_size,
          &region2, &region2_size, 0))) {
    uint8 *dest_sample = (uint8 *)region1;
    for (DWORD byte_index = 0; byte_index < region1_size; ++byte_index) {
      *dest_sample++ = 0;
    }
    dest_sample = (uint8 *)region2;
    for (DWORD byte_index = 0; byte_index < region2_size; ++byte_index) {
      *dest_sample++ = 0;
    }
    global_secondary_buffer->Unlock(region1, region1_size, region2,
                                    region2_size);
  }
}

internal void win32_fill_sound_buffer(Win32_Sound_Output *sound_output,
                                      DWORD byte_to_lock, DWORD bytes_to_write,
                                      GameSoundOutputBuffer *source_buffer) {
  VOID *region1;
  DWORD region1_size;
  VOID *region2;
  DWORD region2_size;

  if (SUCCEEDED(global_secondary_buffer->Lock(byte_to_lock, bytes_to_write,
                                              &region1, &region1_size, &region2,
                                              &region2_size, 0))) {
    DWORD region1_sample_count = region1_size / sound_output->bytes_per_sample;
    DWORD region2_sample_count = region2_size / sound_output->bytes_per_sample;

    int16 *dest_sample = (int16 *)region1;
    int16 *source_sample = source_buffer->samples;

    // Fill region 1
    for (DWORD sample_idx = 0; sample_idx < region1_sample_count;
         ++sample_idx) {
      *dest_sample++ = *source_sample++;
      *dest_sample++ = *source_sample++;
      ++sound_output->running_sample_index;
    }

    // Fill region 2
    dest_sample = (int16 *)region2;
    for (DWORD sample_idx = 0; sample_idx < region2_sample_count;
         ++sample_idx) {
      *dest_sample++ = *source_sample++;
      *dest_sample++ = *source_sample++;
      ++sound_output->running_sample_index;
    }

    global_secondary_buffer->Unlock(region1, region1_size, region2,
                                    region2_size);
  }
}

global_variable int64 pref_count_frequency;

inline LARGE_INTEGER win32_get_wall_clock() {
  LARGE_INTEGER result;
  QueryPerformanceCounter(&result);
  return result;
}

inline real32 win32_get_seconds_elapsed(LARGE_INTEGER start,
                                        LARGE_INTEGER end) {
  real32 result =
      ((real32)(end.QuadPart - start.QuadPart) / (real32)pref_count_frequency);
  return result;
}

internal void win32_debug_draw_vertical(Win32OffscreenBuffer *back_buffer,
                                        int x, int top, int bottom,
                                        uint32 color) {
  uint8 *pixel = ((uint8 *)back_buffer->Memory +
                  x * back_buffer->BytesPerPixel + top * back_buffer->Pitch);
  for (int y = top; y < bottom; ++y) {
    *(uint32 *)pixel = color;
    pixel += back_buffer->Pitch;
  }
}

inline void win32_draw_sound_buffer(Win32OffscreenBuffer *back_buffer,
                                    Win32_Sound_Output *sound_output, real32 c,
                                    int pad_x, int top, int bottom, DWORD value,
                                    uint32 color) {
  Assert(value < sound_output->secondary_buffer_size);
  real32 x_real32 = (c * (real32)(value));
  int x = pad_x + (int)x_real32;
  win32_debug_draw_vertical(back_buffer, x, top, bottom, color);
}

internal void win32_debug_sync_display(Win32OffscreenBuffer *back_buffer,
                                       int marker_count,
                                       Win32DebugTimeMarker *markers,
                                       Win32_Sound_Output *sound_output,
                                       real32 target_seconds_per_frame) {
  // TODO: Draw where we're writing out sound
  int pad_x = 16;
  int pad_y = 16;
  int top = pad_y;
  int bottom = back_buffer->Height - pad_y;

  real32 c = (real32)(back_buffer->Width - 2 * pad_x) /
             (real32)sound_output->secondary_buffer_size;
  for (int marker_index = 0; marker_index < marker_count; ++marker_index) {
    Win32DebugTimeMarker *this_marker = &markers[marker_index];
    win32_draw_sound_buffer(back_buffer, sound_output, c, pad_x, top, bottom,
                            this_marker->play_cursor, 0xFFFFFFFF);
    win32_draw_sound_buffer(back_buffer, sound_output, c, pad_x, top, bottom,
                            this_marker->write_cursor, 0xFFFF0000);
  }
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine,
            int nShowCmd) {
  UINT desired_scheduler = 1;
  bool32 sleep_is_granular =
      (timeBeginPeriod(desired_scheduler) == TIMERR_NOERROR);
  win32_load_xinput();
  WNDCLASSEX window_class{};

  Win32ResizeDIBSection(&global_back_buffer, 1280, 720);
  window_class.cbSize = sizeof(WNDCLASSEX);
  window_class.style = CS_HREDRAW | CS_VREDRAW;
  window_class.lpfnWndProc = Win32MainWindowCallback;
  window_class.hInstance = hInstance;
  // window_class.hIcon;
  window_class.lpszClassName = "HandmadeHeroWindowClass";
// TODO: how to query this in windows
#define FramesOfAudioLatency 4
#define MonitorRefreshHz 60
#define GameUpdateHz (MonitorRefreshHz / 2)
  real32 target_seconds_per_frame = 1.0f / (real32)MonitorRefreshHz;

  LARGE_INTEGER pref_count_frequency_result;
  QueryPerformanceFrequency(&pref_count_frequency_result);
  pref_count_frequency = pref_count_frequency_result.QuadPart;

  if (RegisterClassEx(&window_class)) {
    HWND window_handle = CreateWindowEx(
        0, window_class.lpszClassName, "Handmade hero",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, hInstance, 0);
    if (window_handle) {
      GLOBALRUNNING = true;

      // Graphics test

      Win32_Sound_Output sound_output = {};
      sound_output.samples_per_second = 48000;
      sound_output.latency_sample_count =
          FramesOfAudioLatency *
          (sound_output.samples_per_second / GameUpdateHz);
      sound_output.running_sample_index = 0;
      sound_output.bytes_per_sample = sizeof(int16) * 2;
      sound_output.secondary_buffer_size =
          sound_output.bytes_per_sample * sound_output.samples_per_second;

      win32_init_direct_audio(window_handle, sound_output.samples_per_second,
                              sound_output.secondary_buffer_size);
      win32_clear_sound_buffer(&sound_output);
      global_secondary_buffer->Play(0, 0, DSBPLAY_LOOPING);
#if 0
      while (GLOBALRUNNING) {
        DWORD play_cursor;
        DWORD write_cursor;
        global_secondary_buffer->GetCurrentPosition(&play_cursor,
                                                    &write_cursor);
        char text_buffer[256];
        _snprintf_s(text_buffer, sizeof(text_buffer), "PC: %u WC: %u\n",
                    play_cursor, write_cursor);
        OutputDebugStringA(text_buffer);
      }
#endif
      int16 *sound_memory =
          (int16 *)VirtualAlloc(0, sound_output.secondary_buffer_size,
                                MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

#if HANDMADE_INTERNAL
      LPVOID base_address = (LPVOID)Terabytes((uint64)2);
#else
      LPVOID base_address = 0;
#endif

      GameMemory game_memory = {};
      game_memory.permanent_storage_space = Megabytes(64);
      game_memory.transient_storage_space = Gigabytes((uint64)4);
      uint64 total_size = game_memory.permanent_storage_space +
                          game_memory.transient_storage_space;
      game_memory.permanent_storage = VirtualAlloc(
          base_address, total_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

      game_memory.transient_storage = ((uint8 *)game_memory.permanent_storage +
                                       game_memory.permanent_storage_space);

      if (sound_memory && game_memory.permanent_storage &&
          game_memory.transient_storage) {
        GameInput input[2] = {};
        GameInput *new_input = &input[0];
        GameInput *old_input = &input[1];

        LARGE_INTEGER last_counter = win32_get_wall_clock();
        int debug_time_marker_index = 0;
        Win32DebugTimeMarker debug_time_markers[GameUpdateHz / 2] = {0};

        DWORD last_play_cursor = 0;
        bool32 sound_is_valid = false;

        uint64 last_cycle_count = __rdtsc();
        while (GLOBALRUNNING) {
          GameControllerInput *old_keyboard_controller =
              get_controller(old_input, 0);
          GameControllerInput *new_keyboard_controller =
              get_controller(new_input, 0);
          GameControllerInput zero_controller = {};
          *new_keyboard_controller = zero_controller;
          new_keyboard_controller->is_connected = true;

          for (int button_index = 0;
               button_index < ArrayCount(new_keyboard_controller->buttons);
               button_index++) {
            new_keyboard_controller->buttons[button_index] =
                old_keyboard_controller->buttons[button_index];
          }
          win32_process_pending_messages(new_keyboard_controller);
          // TODO: need not poll disconnected controllers to allow xinput
          // framerate hit on older libs
          // TODO: should poll more frequently
          DWORD max_controller_count = XUSER_MAX_COUNT;
          if (max_controller_count > (ArrayCount(new_input->controllers) - 1)) {
            max_controller_count = (ArrayCount(new_input->controllers) - 1);
          }
          for (DWORD controller_index = 0;
               controller_index < max_controller_count; controller_index++) {
            DWORD our_controller_index = controller_index + 1;
            GameControllerInput *old_controller =
                get_controller(old_input, controller_index);
            GameControllerInput *new_controller =
                get_controller(new_input, controller_index);
            ;
            XINPUT_STATE controller_state;
            if (XInputGetState(controller_index, &controller_state) ==
                ERROR_SUCCESS) {
              new_controller->is_connected = true;
              XINPUT_GAMEPAD *pad = &controller_state.Gamepad;
              new_controller->stick_average_x = win32_process_input_stick_value(
                  pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

              new_controller->stick_average_y = win32_process_input_stick_value(
                  pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
              if (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP) {
                new_controller->stick_average_y = 1.0f;
              }
              if (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN) {
                new_controller->stick_average_y = -1.0f;
              }
              if (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT) {
                new_controller->stick_average_x = -1.0f;
              }
              if (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) {
                new_controller->stick_average_x = 1.0f;
              }
              new_controller->is_analog = true;
              // TODO Min Max Macros

              // TODO Min Max Macros

              // TODO: we will do deadzone handling later
              real32 threshold = -0.5f;
              win32_process_digital_xinput_button(
                  (new_controller->stick_average_x < -threshold) ? 1 : 0,
                  &old_controller->move_left, 1, &new_controller->move_left);
              win32_process_digital_xinput_button(
                  (new_controller->stick_average_x > threshold) ? 1 : 0,
                  &old_controller->move_left, 1, &new_controller->move_left);
              win32_process_digital_xinput_button(
                  (new_controller->stick_average_y < -threshold) ? 1 : 0,
                  &old_controller->move_left, 1, &new_controller->move_left);
              win32_process_digital_xinput_button(
                  (new_controller->stick_average_y > threshold) ? 1 : 0,
                  &old_controller->move_left, 1, &new_controller->move_left);
              win32_process_digital_xinput_button(
                  pad->wButtons, &old_controller->action_down, XINPUT_GAMEPAD_A,
                  &new_controller->action_down);
              win32_process_digital_xinput_button(
                  pad->wButtons, &old_controller->action_left, XINPUT_GAMEPAD_B,
                  &new_controller->action_left);
              win32_process_digital_xinput_button(
                  pad->wButtons, &old_controller->action_left, XINPUT_GAMEPAD_X,
                  &new_controller->action_right);
              win32_process_digital_xinput_button(
                  pad->wButtons, &old_controller->action_up, XINPUT_GAMEPAD_Y,
                  &new_controller->action_up);
              win32_process_digital_xinput_button(
                  pad->wButtons, &old_controller->left_shoulder,
                  XINPUT_GAMEPAD_LEFT_SHOULDER, &new_controller->left_shoulder);
              win32_process_digital_xinput_button(
                  pad->wButtons, &old_controller->right_shoulder,
                  XINPUT_GAMEPAD_RIGHT_SHOULDER,
                  &new_controller->right_shoulder);
              win32_process_digital_xinput_button(
                  pad->wButtons, &old_controller->start, XINPUT_GAMEPAD_START,
                  &new_controller->start);
              win32_process_digital_xinput_button(
                  pad->wButtons, &old_controller->back, XINPUT_GAMEPAD_BACK,
                  &new_controller->back);
              // BOOL start = (pad->wButtons & XINPUT_GAMEPAD_START);
              // BOOL back = (pad->wButtons & XINPUT_GAMEPAD_BACK);

            } else {
              new_controller->is_connected = false;
            }
          }
          XINPUT_VIBRATION vibration;
          vibration.wLeftMotorSpeed = 60000;
          vibration.wRightMotorSpeed = 60000;
          XInputSetState(0, &vibration);

          // TODO: Compuite how much to write and where
          DWORD byte_to_lock = 0;
          DWORD target_cursor = 0;
          DWORD bytes_to_write = 0;

          // TODO: tighten up sound logic so that we know where  we should be
          //  writing to and can anticipate the time spent in the game update.
          if (sound_is_valid) {
            // TODO: if first time through, write from write cursor!
            byte_to_lock = ((sound_output.running_sample_index *
                             sound_output.bytes_per_sample) %
                            sound_output.secondary_buffer_size);
            target_cursor =
                ((last_play_cursor + (sound_output.latency_sample_count *
                                      sound_output.bytes_per_sample)) %
                 sound_output.secondary_buffer_size);

            if (byte_to_lock > target_cursor) {
              bytes_to_write =
                  sound_output.secondary_buffer_size - byte_to_lock;
              bytes_to_write += target_cursor;
            } else {
              bytes_to_write = target_cursor - byte_to_lock;
            }
            char text_buffer[256];
            _snprintf_s(text_buffer, sizeof(text_buffer),
                        "PC: %u BTL: %u TC: %u BTW: %u \n", last_play_cursor,
                        byte_to_lock, target_cursor, bytes_to_write);
            sound_is_valid = true;
          }

          GameSoundOutputBuffer sound_buffer = {};

          sound_buffer.samples_per_second = sound_output.samples_per_second;
          sound_buffer.sample_count =
              bytes_to_write / sound_output.bytes_per_sample;
          sound_buffer.samples = sound_memory;

          GameOffscreenBuffer buffer = {};
          buffer.Memory = global_back_buffer.Memory;
          buffer.Width = global_back_buffer.Width;
          buffer.Height = global_back_buffer.Height;
          buffer.BytesPerPixel = global_back_buffer.BytesPerPixel;
          buffer.Pitch = global_back_buffer.Pitch;

          game_update_and_render(&game_memory, new_input, &buffer,
                                 &sound_buffer);
          if (sound_is_valid) {
#if HANDMADE_INTERNAL
            DWORD debug_play_cursor;
            DWORD debug_write_cursor;
            global_secondary_buffer->GetCurrentPosition(&debug_play_cursor,
                                                        &debug_write_cursor);

            char buffer_debug[256];
            _snprintf_s(buffer_debug, sizeof(buffer_debug),
                        "LPC: %u BTL: %u TC: %u BTW: %u PC: %u WC: %u \n",
                        last_play_cursor, byte_to_lock, target_cursor,
                        bytes_to_write, debug_play_cursor, debug_write_cursor);
            OutputDebugStringA(buffer_debug);
#endif
            win32_fill_sound_buffer(&sound_output, byte_to_lock, bytes_to_write,
                                    &sound_buffer);
          }

          HDC device_context = GetDC(window_handle);

          uint64 end_cycle_count = __rdtsc();
          LARGE_INTEGER work_counter = win32_get_wall_clock();
          uint64 cycle_elapsed = end_cycle_count - last_cycle_count;
          int64 counter_elapsed = work_counter.QuadPart - last_counter.QuadPart;
          real32 work_seconds_elapsed =
              win32_get_seconds_elapsed(last_counter, work_counter);
          real32 seconds_elapsed_for_frame = work_seconds_elapsed;

          if (seconds_elapsed_for_frame < target_seconds_per_frame) {

            while (seconds_elapsed_for_frame < target_seconds_per_frame) {

              if (sleep_is_granular) {
                DWORD sleep_ms = (DWORD)(1000.0f * (target_seconds_per_frame -
                                                    seconds_elapsed_for_frame));
                Sleep(sleep_ms);
              }
              seconds_elapsed_for_frame = win32_get_seconds_elapsed(
                  last_counter, win32_get_wall_clock());
            }
          } else {
            // TODO: missed frame rate target
            // TODO: logging
          }
          LARGE_INTEGER end_counter = win32_get_wall_clock();
          real32 ms_per_frame =
              1000.0f * win32_get_seconds_elapsed(last_counter, end_counter);
          last_counter = end_counter;

          Win32WindowDimension dimension =
              win32_get_window_dimension(window_handle);
#if HANDMADE_INTERNAL
          win32_debug_sync_display(
              &global_back_buffer, ArrayCount(debug_time_markers),
              debug_time_markers, &sound_output, target_seconds_per_frame);
#endif
          Win32DisplayBufferInWindow(device_context, dimension,
                                     &global_back_buffer, 0, 0, dimension.Width,
                                     dimension.Height);
          DWORD play_cursor;
          DWORD write_cursor;
          if (global_secondary_buffer->GetCurrentPosition(
                  &play_cursor, &write_cursor) == DS_OK) {

            last_play_cursor = play_cursor;
            if (!sound_is_valid) {
              sound_output.running_sample_index =
                  write_cursor / sound_output.bytes_per_sample;
              sound_is_valid = true;
            }

          } else {
            sound_is_valid = false;
          }
#if HANDMADE_INTERNAL
          {
            Win32DebugTimeMarker *marker =
                &debug_time_markers[debug_time_marker_index++];
            if (debug_time_marker_index > ArrayCount(debug_time_markers)) {
              debug_time_marker_index = 0;
            }
            marker->play_cursor = play_cursor;
            marker->write_cursor = write_cursor;
          }
#endif
          ReleaseDC(window_handle, device_context);
          LARGE_INTEGER end_counter2 = win32_get_wall_clock();

          real32 fps = 0.0;
          real32 mcpf = (real32)((real32)cycle_elapsed / (1000.0f * 1000.0f));
#if 1
          // use this part for debug
          char buffer2[256];
          sprintf_s(buffer2, sizeof(buffer2), "%fms/f / %fFPS/s %fmc/f \n",
                    ms_per_frame, fps, mcpf);
          OutputDebugStringA(buffer2);
          //
#endif

          uint64 end_cycle_count2 = __rdtsc();
          uint64 cycles_elapsed = end_cycle_count2 - last_cycle_count;
          last_cycle_count = end_cycle_count2;

          GameInput *temp = new_input;
          new_input = old_input;
          old_input = temp;
        }
      } else {
        // TODO: logging
      }
    } else {
    }
  } else {
    OutputDebugString("Failed to register WNDCLASS\n");
  }

  return 0;
}
