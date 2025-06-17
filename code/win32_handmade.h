#pragma once

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#include <wingdi.h>

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

struct Win32_Sound_Output {
  int samples_per_second;
  int latency_sample_count;
  uint32 running_sample_index;
  int bytes_per_sample;
  int secondary_buffer_size;
};
