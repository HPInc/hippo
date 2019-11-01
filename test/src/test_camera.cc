
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#include <windows.h>    // for Sleep()
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "include/hippo_camera.h"

#define _LATENCY_CHECK_
#define _DUMP_FRAME_


extern void print_error(uint64_t err);
extern void print_error(uint64_t err, const char *msg);

void print_camera_frame(const hippo::CameraFrame &frame);


uint64_t TestCameraStreams(hippo::HippoCamera *cam,
                           hippo::CameraStreams st) {
  uint64_t err;
  // test enable_stream
  fprintf(stderr, "Enabling streams = [%x]\n", st.value);

  hippo::EnableStream en;
  if (err = cam->enable_streams(st, &en)) {
    return err;
  }
  fprintf(stderr, "port: %d, streams = [%d]\n", en.port, en.streams.value);
  // read enabled streams
  if (err = cam->enable_streams(&en)) {
    return err;
  }
  fprintf(stderr, "port: %d, streams = [%d]\n", en.port, en.streams.value);

  // grab frames
  hippo::CameraFrame frame = { 0 };
  for (uint32_t i = 0; i < 10; i++) {
    fprintf(stderr, "Grabing frame %d\n", i);
    if (err = cam->grab_frame(st, &frame)) {
      return err;
    } else {
      print_camera_frame(frame);
    }
  }
  free(frame.raw_data_);

  fprintf(stderr, "1!\n");

  // test disable_stream
  hippo::CameraStreams st2 = { 0 };
  if (err = cam->disable_streams(st, &st2)) {
    fprintf(stderr, "Oooops!\n");
    return err;
  }
  fprintf(stderr, "Ok!\n");
  fprintf(stderr, "disable streams = [%d]\n", st2.value);

  return 0LL;
}

void print_camera_frame(const hippo::CameraFrame &frame) {
#ifdef _LATENCY_CHECK_
  LARGE_INTEGER frequency, time;
  QueryPerformanceFrequency(&frequency);
  QueryPerformanceCounter(&time);
  time.QuadPart *= 1000000;
  time.QuadPart /= frequency.QuadPart;
#endif

  fprintf(stderr, "-> Camera Frame\n");
  fprintf(stderr, " |-> Frame header\n");
  fprintf(stderr, "   |-> magic: {0x%02x, 0x%02x}\n",
          frame.header->magic[0], frame.header->magic[1]);
  fprintf(stderr, "   |-> device: {0x%02x, 0x%02x}\n",
          frame.header->device[0], frame.header->device[1]);
  fprintf(stderr, "   |-> version: %d\n", frame.header->version);
  fprintf(stderr, "   |-> stream: %d\n", frame.header->stream.value);
  fprintf(stderr, "   \\-> error: %d\n", frame.header->error);
  if (frame.header->error) {
    // print error code
    fprintf(stderr, " \\-> Error code\n");
    fprintf(stderr, "   |-> code: %016llx\n", frame.streams[0].error->code);
    fprintf(stderr, "   \\-> id: '%s'\n", frame.streams[0].error->id);
  } else {
    // print frame information
    for (uint32_t i = frame.header->stream.value, j = 0; i > 0; i>>=1, j++) {
      if (i & 0x01) {
        fprintf(stderr, " |-> Stream header\n");
        fprintf(stderr, "   |-> width: %d\n", frame.streams[j].header->width);
        fprintf(stderr, "   |-> height: %d\n", frame.streams[j].header->height);
        fprintf(stderr, "   |-> index: %d\n", frame.streams[j].header->index);
        fprintf(stderr, "   |-> stream: %d\n",
                frame.streams[j].header->stream.value);
        fprintf(stderr, "   |-> format: %d\n", frame.streams[j].header->format);
#ifdef _LATENCY_CHECK_
        fprintf(stderr, "   \\-> timestamp: %lld  [now: %lld, latency: %lld]\n",
                frame.streams[j].header->timestamp, time.QuadPart,
                time.QuadPart - frame.streams[j].header->timestamp);
#else
        fprintf(stderr, "   \\-> timestamp: %lld\n",
                frame.streams[j].header->timestamp);
#endif
        fprintf(stderr, " \\-> data: %p\n", frame.streams[j].data);
      }
    }
  }
#ifdef _DUMP_FRAME_
  FILE *fp;
  char name[MAX_PATH];
  snprintf(name, MAX_PATH, "frame-%d.raw", frame.header->stream.value);
  if (NULL != (fp = fopen(name, "wb"))) {
    fwrite(frame.raw_data_, 1, frame.raw_length_, fp);
    fclose(fp);
  }
#endif
}
