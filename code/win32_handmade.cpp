
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
    LARGE_INTEGER file_size;

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
      buffer_desc.dwFlags = 0;
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

void *platform_load_file(const char *file_name) { return (0); }

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
    uint32 vk_code = wParam;
    bool was_down = ((lParam & (1 << 30))) != 0;
    bool is_down = ((lParam & (1 << 31))) == 0;
    if (was_down != is_down) {
      if (vk_code == 'W') {
      } else if (vk_code == 'A') {
      } else if (vk_code == 'S') {
      } else if (vk_code == 'D') {
      } else if (vk_code == 'Q') {
      } else if (vk_code == VK_UP) {
      } else if (vk_code == VK_LEFT) {
      } else if (vk_code == VK_DOWN) {
      } else if (vk_code == VK_RIGHT) {
      } else if (vk_code == VK_ESCAPE) {
        OutputDebugStringA("ESCAPE: ");
        if (was_down) {
          OutputDebugString("WAS DOWN ");
        }
        if (is_down) {
          OutputDebugStringA("IS DOWN ");
        }
        OutputDebugStringA("\n");
      } else if (vk_code == VK_SPACE) {
      }
    }
    bool32 alt_key_was_down = ((lParam & (1 << 29)));
    if ((vk_code == VK_F4) && alt_key_was_down) {
      GLOBALRUNNING = false;
    }
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

internal void win32_process_digital_xinput_button(DWORD x_input_button_state,
                                                  GameButtonState *old_state,
                                                  DWORD button_bit,
                                                  GameButtonState *new_state) {

  new_state->ended_down = (x_input_button_state & button_bit);
  new_state->half_transition_count =
      (old_state->ended_down == new_state->ended_down) ? 1 : 0;
}
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine,
            int nShowCmd) {
  win32_load_xinput();
  WNDCLASSEX window_class{};

  Win32ResizeDIBSection(&global_back_buffer, 1280, 720);
  window_class.cbSize = sizeof(WNDCLASSEX);
  window_class.style = CS_HREDRAW | CS_VREDRAW;
  window_class.lpfnWndProc = Win32MainWindowCallback;
  window_class.hInstance = hInstance;
  // window_class.hIcon;
  window_class.lpszClassName = "HandmadeHeroWindowClass";

  LARGE_INTEGER pref_count_frequency_result;
  QueryPerformanceFrequency(&pref_count_frequency_result);
  int64 pref_count_frequency = pref_count_frequency_result.QuadPart;

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
      sound_output.latency_sample_count = sound_output.samples_per_second / 15;
      sound_output.running_sample_index = 0;
      sound_output.bytes_per_sample = sizeof(int16) * 2;
      sound_output.secondary_buffer_size =
          sound_output.bytes_per_sample * sound_output.samples_per_second;

      win32_init_direct_audio(window_handle, sound_output.samples_per_second,
                              sound_output.secondary_buffer_size);
      win32_clear_sound_buffer(&sound_output);
      global_secondary_buffer->Play(0, 0, DSBPLAY_LOOPING);
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

        LARGE_INTEGER last_counter;
        QueryPerformanceCounter(&last_counter);
        uint64 last_cycle_count = __rdtsc();
        while (GLOBALRUNNING) {
          GameInput input = {};
          MSG message = {};
          while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) {
              GLOBALRUNNING = false;
            }
            TranslateMessage(&message);
            DispatchMessage(&message);
          }
          int max_controller_count = XUSER_MAX_COUNT;
          if (max_controller_count > ArrayCount(new_input->controllers)) {
            max_controller_count = ArrayCount(new_input->controllers);
          }
          for (DWORD controller_index = 0;
               controller_index < max_controller_count; controller_index++) {
            GameControllerInput *old_controller =
                &old_input->controllers[controller_index];
            GameControllerInput *new_controller =
                &new_input->controllers[controller_index];
            XINPUT_STATE controller_state;
            if (XInputGetState(controller_index, &controller_state) ==
                ERROR_SUCCESS) {
              XINPUT_GAMEPAD *pad = &controller_state.Gamepad;
              BOOL up = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
              BOOL down = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
              BOOL left = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
              BOOL right = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
              new_controller->is_analog = true;
              new_controller->start_x = old_controller->end_x;
              new_controller->start_y = old_controller->end_y;

              real32 x;
              if (pad->sThumbLX < 0) {
                x = (real32)pad->sThumbLX / 32768.0f;
              } else {
                x = (real32)pad->sThumbLX / 32767.0f;
              }

              // TODO Min Max Macros
              new_controller->min_x = new_controller->max_x =
                  new_controller->end_x = x;

              real32 y;
              if (pad->sThumbLY < 0) {
                y = (real32)pad->sThumbLY / 32768.0f;
              } else {
                y = (real32)pad->sThumbLY / 32767.0f;
              }

              // TODO Min Max Macros
              new_controller->min_y = new_controller->max_y =
                  new_controller->end_y = x;
              new_controller->end_x = y;
              // TODO: we will do deadzone handling later
              win32_process_digital_xinput_button(
                  pad->wButtons, &old_controller->down, XINPUT_GAMEPAD_A,
                  &new_controller->down);
              win32_process_digital_xinput_button(
                  pad->wButtons, &old_controller->right, XINPUT_GAMEPAD_B,
                  &new_controller->right);
              win32_process_digital_xinput_button(
                  pad->wButtons, &old_controller->left, XINPUT_GAMEPAD_X,
                  &new_controller->left);
              win32_process_digital_xinput_button(
                  pad->wButtons, &old_controller->up, XINPUT_GAMEPAD_Y,
                  &new_controller->up);
              win32_process_digital_xinput_button(
                  pad->wButtons, &old_controller->left_shoulder,
                  XINPUT_GAMEPAD_LEFT_SHOULDER, &new_controller->left_shoulder);
              win32_process_digital_xinput_button(
                  pad->wButtons, &old_controller->right_shoulder,
                  XINPUT_GAMEPAD_RIGHT_SHOULDER,
                  &new_controller->right_shoulder);
              // BOOL start = (pad->wButtons & XINPUT_GAMEPAD_START);
              // BOOL back = (pad->wButtons & XINPUT_GAMEPAD_BACK);

            } else {
            }
          }
          XINPUT_VIBRATION vibration;
          vibration.wLeftMotorSpeed = 60000;
          vibration.wRightMotorSpeed = 60000;
          XInputSetState(0, &vibration);

          DWORD byte_to_lock = 0;
          DWORD target_cursor = 0;
          DWORD bytes_to_write = 0;
          DWORD play_cursor = 0;
          DWORD write_cursor = 0;
          bool32 sound_is_valid = false;
          // TODO: tighten up sound logic so that we know where  we should be
          //  writing to and can anticipate the time spent in the game update.
          if (SUCCEEDED(global_secondary_buffer->GetCurrentPosition(
                  &play_cursor, &write_cursor))) {
            byte_to_lock = ((sound_output.running_sample_index *
                             sound_output.bytes_per_sample) %
                            sound_output.secondary_buffer_size);
            target_cursor = ((play_cursor + (sound_output.latency_sample_count *
                                             sound_output.bytes_per_sample)) %
                             sound_output.secondary_buffer_size);

            if (byte_to_lock > target_cursor) {
              bytes_to_write =
                  sound_output.secondary_buffer_size - byte_to_lock;
              bytes_to_write += target_cursor;
            } else {
              bytes_to_write = target_cursor - byte_to_lock;
            }

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
            win32_fill_sound_buffer(&sound_output, byte_to_lock, bytes_to_write,
                                    &sound_buffer);
          }

          HDC device_context = GetDC(window_handle);
          Win32WindowDimension dimension =
              win32_get_window_dimension(window_handle);

          Win32DisplayBufferInWindow(device_context, dimension,
                                     &global_back_buffer, 0, 0, dimension.Width,
                                     dimension.Height);
          ReleaseDC(window_handle, device_context);
          uint64 end_cycle_count = __rdtsc();
          LARGE_INTEGER end_counter;
          QueryPerformanceCounter(&end_counter);
          uint64 cycle_elapsed = end_cycle_count - last_cycle_count;
          int64 counter_elapsed = end_counter.QuadPart - last_counter.QuadPart;
          real32 ms_per_frame = (real32)(((real32)counter_elapsed * 1000.0f) /
                                         pref_count_frequency);
          real32 fps = (real32)pref_count_frequency / (real32)counter_elapsed;
          real32 mcpf = (real32)((real32)cycle_elapsed / (1000.0f * 1000.0f));
#if 0
        // use this part for debug
        char buffer[256];
        sprintf(buffer, "%fms/f / %fFPS/s %fmc/f \n", ms_per_frame, fps, mcpf);
        OutputDebugStringA(buffer);
        //
#endif
          last_counter = end_counter;
          last_cycle_count = end_cycle_count;

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
