
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#ifndef INCLUDE_COMMON_TYPES_H_
#define INCLUDE_COMMON_TYPES_H_

#include <stdint.h>

#if COMPILING_DLL
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif

namespace hippo {

typedef struct PointFloats {
  float x;
  float y;
} PointFloats;

typedef struct Point {
  int16_t x;
  int16_t y;
} Point;

typedef struct Resolution {
  uint32_t width;   // The width of the image in pixels
  uint32_t height;  // The height of the image in pixels
} Resolution;

typedef struct Rectangle {
  int16_t x;        // The left-most pixel coordinate
  int16_t y;        // The top-most pixel coordinate
  int16_t width;    // The width size in pixels
  int16_t height;   // The height size in pixels
} Rectangle;

}   // namespace hippo

#endif   // INCLUDE_COMMON_TYPES_H_
