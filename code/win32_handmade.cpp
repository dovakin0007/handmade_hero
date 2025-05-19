#include <windows.h>
#include <Xinput.h>
#include <cstdint>


#define internal static
#define local_persist static
#define global_variable static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

global_variable bool GLOBALRUNNING;

struct Win32OffscreenBuffer {
  BITMAPINFO Info;
  void *Memory;
  int Width;
  int Height;
  int Pitch;
  int BytesPerPixel;
};

global_variable Win32OffscreenBuffer global_back_buffer;

struct Win32WindowDimension {
  int Width;
  int Height;
};

// Xinput Get state
#define X_INPUT_GET_STATE(name)                                                \
  DWORD WINAPI name(DWORD dwUserIndex, _XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub) { return 0; }
#define XInputGetState XInputGetState_
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;

// Xinput Set state
#define X_INPUT_SET_STATE(name)                                                \
  DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub) { return 0; }
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

internal void win32_load_xinput(void) {
  HMODULE xinput_library = LoadLibrary("Xinput1_4.dll");
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
  buffer->Memory =
      VirtualAlloc(0, bitmap_memory_size, MEM_COMMIT, PAGE_READWRITE);
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
    bool was_down = ((lParam & (1 << 30)) != 0);
    bool is_down = ((lParam & (1 << 31)) == 0);
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

      break;
    }
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
      int x_offset = 0;
      int y_offset = 0;
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

            if (a_button) {
              y_offset += 2;
            }

          } else {
          }
        }
        XINPUT_VIBRATION vibration;
        vibration.wLeftMotorSpeed = 60000;
        vibration.wRightMotorSpeed = 60000;
        XInputSetState(0, &vibration);
        RenderWeirdGradient(&global_back_buffer, x_offset, y_offset);
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
