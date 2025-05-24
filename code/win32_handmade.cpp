#include <DSound.h>
#include <Xinput.h>
#include <combaseapi.h>
#include <cstdint>
#include <windows.h>
#include <winerror.h>
#include <winnt.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <xaudio2.h>

#define internal static
#define local_persist static
#define global_variable static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

struct Win32OffscreenBuffer {
  BITMAPINFO Info;
  void *Memory;
  int Width;
  int Height;
  int Pitch;
  int BytesPerPixel;
};

struct Win32WindowDimension {
  int Width;
  int Height;
};

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

// XAUDIO2 CREATE
#define XAUDIO2_CREATE(name)                                                   \
  HRESULT name(IXAudio2 **ppXAudio2, UINT32 Flags,                             \
               XAUDIO2_PROCESSOR XAudio2Processor)
typedef XAUDIO2_CREATE(xaudio2_create);

#define DIRECT_SOUND_CREATE(name)                                              \
  HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS,               \
                      LPUNKNOWN pUnkOuter);
typedef DIRECT_SOUND_CREATE(direct_sound_create);

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

// TODO: DIAGNOSTIC LOGGING
// internal void win32_init_direct_audio(int32 samples_per_second, int32
// buffer_size) {
//   // Load the lib
//   // keep  this as static to work lol
//       internal winrt::com_ptr<IXAudio2> m_xAudio2;
//       internal IXAudio2SourceVoice* m_pXAudio2SourceVoice;
//       internal void* m_buffer;
//   HMODULE x_audio_lib = LoadLibraryA("XAUDIO2_9.DLL");
//   if (!x_audio_lib) {
//     return;
//   }
//   constexpr WORD BITSPERSSAMPLE = 16;
//   constexpr double CYCLESPERSEC =
//       220.0;
//   constexpr double VOLUME = 0.5;
//   constexpr WORD AUDIOBUFFERSIZEINCYCLES = 10;
//   constexpr double PI = 3.14159265358979323846;

//   // Calculated constants.
// DWORD SAMPLESPERCYCLE =
//       (DWORD)(samples_per_second / CYCLESPERSEC); // 200 samples per cycle.
//   DWORD AUDIOBUFFERSIZEINSAMPLES =
//       samples_per_second * AUDIOBUFFERSIZEINCYCLES; // 2,000 samples per
//       buffer.
//   UINT32 AUDIOBUFFERSIZEINBYTES =
//       AUDIOBUFFERSIZEINSAMPLES * BITSPERSSAMPLE / 8; // 4,000 bytes per
//       buffer.

//   HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
//   if (FAILED(hr)) {
//     OutputDebugString("failed to initialize COM\n");
//     return;
//   }
//   xaudio2_create *XAudio2Create =
//       (xaudio2_create *)GetProcAddress(x_audio_lib, "XAudio2Create");

//       if (!XAudio2Create || FAILED(XAudio2Create(m_xAudio2.put(), 0,
//       XAUDIO2_DEFAULT_PROCESSOR))) {
//         OutputDebugStringA("Failed to create XAudio2 instance\n");
//         return;
//       }
//     IXAudio2MasteringVoice *m_pXAudio2MasteringVoice{};
//     m_xAudio2->CreateMasteringVoice(&m_pXAudio2MasteringVoice);
//     // Clean up the buffer lol
//     m_buffer =
//         VirtualAlloc(0, AUDIOBUFFERSIZEINBYTES, MEM_COMMIT, PAGE_READWRITE);
//     WAVEFORMATEX waveFormatEx;
//     waveFormatEx.wFormatTag = WAVE_FORMAT_PCM;
//     waveFormatEx.nChannels = 2;
//     waveFormatEx.nSamplesPerSec = samples_per_second;
//     waveFormatEx.wBitsPerSample = 16;
//     waveFormatEx.nBlockAlign = (waveFormatEx.nChannels *
//     waveFormatEx.wBitsPerSample) / 8; waveFormatEx.nAvgBytesPerSec =
//         waveFormatEx.nSamplesPerSec * waveFormatEx.nBlockAlign;
//     waveFormatEx.cbSize = 0;
//     HRESULT result =
//         m_xAudio2->CreateSourceVoice(&m_pXAudio2SourceVoice, &waveFormatEx);
//     if (FAILED(result)) {
//       OutputDebugStringA("Failed to create source voice");
//       return;
//     }
//     double phase = 0.0;
//     uint16 buf_idx = 0;
//     uint8 *buffer = (uint8 *)m_buffer;
//     for (DWORD i = 0; i < AUDIOBUFFERSIZEINSAMPLES; ++i) {
//       double value = sin(phase) * INT16_MAX * VOLUME;
//       int16_t sample = (int16_t)value;
//       buffer[2 * i] = (uint8_t)sample;
//       buffer[2 * i + 1] = (uint8_t)(sample >> 8);
//       phase += (2 * PI / SAMPLESPERCYCLE);
//       if (phase >= 2 * PI)
//         phase -= 2 * PI;
//     }

//     XAUDIO2_BUFFER audio_buffer = {};
//      audio_buffer.Flags = XAUDIO2_END_OF_STREAM;
//      audio_buffer.AudioBytes = AUDIOBUFFERSIZEINBYTES;
//      audio_buffer.pAudioData = reinterpret_cast<BYTE *>(m_buffer);
//      audio_buffer.LoopCount = XAUDIO2_LOOP_INFINITE;

//      if (FAILED(m_pXAudio2SourceVoice->SubmitSourceBuffer(&audio_buffer))) {
//        OutputDebugStringA("Failed to submit source buffer\n");
//        return;
//      }

//      if (FAILED(m_pXAudio2SourceVoice->Start(0))) {
//        OutputDebugStringA("Failed to start source voice\n");
//        return;
//      }

//      OutputDebugStringA("Audio playback started\n");

//   // Get a xaudio2 object

//   // Create a primary buffer
//   // Create a secondary buffer
//   //
//   // Start playing it
// }

internal Win32WindowDimension win32_get_window_dimension(HWND window_handle) {
  Win32WindowDimension window_dimensions;
  RECT client_rect;
  GetClientRect(window_handle, &client_rect);

  window_dimensions.Width = client_rect.right - client_rect.left;
  window_dimensions.Height = client_rect.bottom - client_rect.top;

  return window_dimensions;
}

internal void RenderWeirdGradient(Win32OffscreenBuffer *buffer, int x_offset,
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
    bool was_down = ((lParam & (1 << 30)));
    bool is_down = ((lParam & (1 << 31)));
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

  if (RegisterClassEx(&window_class)) {
    HWND window_handle = CreateWindowEx(
        0, window_class.lpszClassName, "Handmade hero",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, hInstance, 0);
    if (window_handle) {
      GLOBALRUNNING = true;

      // Graphics test
      int x_offset = 0;
      int y_offset = 0;

      // Sound Test
      int samples_per_second = 48000;
      int tone_hz = 256;
      int tone_volume = 3000;
      uint32 running_sample_index = 0;
      int square_wave_period = samples_per_second / tone_hz;
      int half_square_wave_period = square_wave_period / 2;
      int bytes_per_sample = sizeof(int16) * 2;
      int secondary_buffer_size = bytes_per_sample * samples_per_second;
      win32_init_direct_audio(window_handle, samples_per_second,
                              secondary_buffer_size);
      global_secondary_buffer->Play(0, 0, DSBPLAY_LOOPING);
      while (GLOBALRUNNING) {
        MSG message = {};
        while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
          if (message.message == WM_QUIT) {
            GLOBALRUNNING = false;
          }
          TranslateMessage(&message);
          DispatchMessage(&message);
        }
        for (DWORD controller_index = 0; controller_index < XUSER_MAX_COUNT;
             controller_index++) {
          XINPUT_STATE controller_state = {};
          if (XInputGetState(controller_index, &controller_state)) {
            XINPUT_GAMEPAD *pad = &controller_state.Gamepad;
            BOOL up = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
            BOOL down = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
            BOOL left = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
            BOOL right = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
            BOOL start = (pad->wButtons & XINPUT_GAMEPAD_START);
            BOOL back = (pad->wButtons & XINPUT_GAMEPAD_BACK);
            BOOL left_shoulder = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
            BOOL a_button = (pad->wButtons & XINPUT_GAMEPAD_A);
            BOOL b_button = (pad->wButtons & XINPUT_GAMEPAD_B);
            BOOL x_button = (pad->wButtons & XINPUT_GAMEPAD_X);
            BOOL y_button = (pad->wButtons & XINPUT_GAMEPAD_Y);

            int16 stick_x = pad->sThumbLX;
            int16 stick_y = pad->sThumbLY;
            x_offset += stick_x >> 12;
            y_offset += stick_y >> 12;
          } else {
          }
        }
        XINPUT_VIBRATION vibration;
        vibration.wLeftMotorSpeed = 60000;
        vibration.wRightMotorSpeed = 60000;
        XInputSetState(0, &vibration);
        RenderWeirdGradient(&global_back_buffer, x_offset, y_offset);
        DWORD play_cursor;
        DWORD write_cusror;
        if (SUCCEEDED(global_secondary_buffer->GetCurrentPosition(
                &play_cursor, &write_cusror))) {
          DWORD byte_to_lock =
              running_sample_index * bytes_per_sample % secondary_buffer_size;
          DWORD bytes_to_write;
          if (byte_to_lock > play_cursor) {
            bytes_to_write = secondary_buffer_size - byte_to_lock;
            bytes_to_write += play_cursor;
          } else {
            bytes_to_write = play_cursor - byte_to_lock;
          }
          VOID *region1;
          DWORD region1_size;
          VOID *region2;
          DWORD region2_size;

          if (SUCCEEDED(global_secondary_buffer->Lock(
                  byte_to_lock, bytes_to_write, &region1, &region1_size,
                  &region2, &region2_size, 0))) {
            int16 *sample_out1 = (int16 *)region1;
            DWORD region1_sample_count = region1_size / bytes_per_sample;
            DWORD region2_sample_count = region2_size / bytes_per_sample;
            for (DWORD sample_idx = 0; sample_idx < region1_sample_count;
                 ++sample_idx) {
              // returns releative index of the give sin wave example 0 or 1
              int16 sample_value =
                  ((running_sample_index++ / half_square_wave_period) % 2)
                      ? tone_volume
                      : -tone_volume;
              *sample_out1++ = sample_value;
              *sample_out1++ = sample_value;
            }
            int16 *sample_out2 = (int16 *)region2;
            for (DWORD sample_idx = 0; sample_idx < region2_sample_count;
                 ++sample_idx) {
              int16 sample_value =
                  ((running_sample_index++ / half_square_wave_period) % 2)
                      ? tone_volume
                      : -tone_volume;
              *sample_out2++ = sample_value;
              *sample_out2++ = sample_value;
            }
            global_secondary_buffer->Unlock(&region1, region1_size, &region2,
                                            region2_size);
          }
        }
        HDC device_context = GetDC(window_handle);
        Win32WindowDimension dimension =
            win32_get_window_dimension(window_handle);

        Win32DisplayBufferInWindow(device_context, dimension,
                                   &global_back_buffer, 0, 0, dimension.Width,
                                   dimension.Height);
        ReleaseDC(window_handle, device_context);
        ++x_offset;
      }
    } else {
      // TODO: logging
    }

  } else {
    OutputDebugString("Failed to register WNDCLASS\n");
  }

  return 0;
}
