
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#include <windows.h>    // for Sleep()
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "include/uvccamera.h"

#define _LATENCY_CHECK_
#define _DUMP_FRAME_


extern const char wsAddress[];
extern const uint32_t wsPort;

extern void print_error(uint64_t err);
extern void print_error(uint64_t err, const char *msg);

void uvccamera_notification(const hippo::UVCCameraNotificationParam &param,
                            void *data);

extern uint64_t TestCameraStreams(hippo::HippoCamera *cam,
                                  hippo::CameraStreams st);


uint64_t TestUVCCamera(hippo::UVCCamera *uvccamera) {
  uint64_t err;
  fprintf(stderr, "##################################\n");
  fprintf(stderr, "    Now Testing UVC Camera\n");
  fprintf(stderr, "##################################\n");

  ADD_FILE_TO_MAP();   // will add this file to the file/error map

  // subscribe
  uint32_t num_subscribe = 0;
  if (err = uvccamera->subscribe(&uvccamera_notification,
                                 reinterpret_cast<void*>(uvccamera),
                                 &num_subscribe)) {
    print_error(err);
  } else {
    fprintf(stderr, "uvccamera.subscribe: count: %d\n", num_subscribe);
  }

  bool bConnected;
  if (err = uvccamera->is_device_connected(&bConnected)) {
    return err;
  }
  if (bConnected) {
    fprintf(stderr, "UVCCamera is connected\n");
  } else {
    fprintf(stderr, "No uvccamera is connected -- not running test\n");
    return MAKE_HIPPO_ERROR(hippo::HIPPO_DEVICE,
                            hippo::HIPPO_DEV_NOT_FOUND);
  }

  // open
  uint32_t open_count = 0;
  if (err = uvccamera->open(&open_count)) {
    return err;
  }
  fprintf(stderr, "uvccamera.open(): open_count: %d\n", open_count);

  uint32_t index;
  if (err = uvccamera->camera_index(&index)) {
    return err;
  }
  fprintf(stderr, "uvccamera.camera_index(): %d\n", index);

  // test all streams combinations
  hippo::CameraStreams st = { 1 };
  if (err = TestCameraStreams(uvccamera, st)) {
    print_error(err);
  }

  // close
  if (err = uvccamera->close(&open_count)) {
    return err;
  } else {
    fprintf(stderr, "uvccamera.close(): open_count: %d\n", open_count);
  }
  // unsubscribe
  if (err = uvccamera->unsubscribe(&num_subscribe)) {
    print_error(err);
  } else {
    fprintf(stderr, "uvccamera.unsubscribe(): count: %d\n", num_subscribe);
  }
  return 0LL;
}

void uvccamera_notification(const hippo::UVCCameraNotificationParam &param,
                            void *data) {
  // and print the notification
  switch (param.type) {
    case hippo::UVCCameraNotification::on_close:
      fprintf(stderr, "[SIGNAL]: uvccamera.on_close\n");
      break;
    case hippo::UVCCameraNotification::on_device_connected:
      fprintf(stderr, "[SIGNAL]: uvccamera.on_device_connected\n");
      break;
    case hippo::UVCCameraNotification::on_device_disconnected:
      fprintf(stderr, "[SIGNAL]: uvccamera.on_device_disconnected\n");
      break;
    case hippo::UVCCameraNotification::on_factory_default:
      fprintf(stderr, "[SIGNAL]: uvccamera.on_factory_default\n");
      break;
    case hippo::UVCCameraNotification::on_open:
      fprintf(stderr, "[SIGNAL]: uvccamera.on_open\n");
      break;
    case hippo::UVCCameraNotification::on_open_count:
      fprintf(stderr, "[SIGNAL]: uvccamera.on_open_count: %d\n",
              param.on_open_count);
      break;
    case hippo::UVCCameraNotification::on_resume:
      fprintf(stderr, "[SIGNAL]: uvccamera.on_resume\n");
      break;
    case hippo::UVCCameraNotification::on_suspend:
      fprintf(stderr, "[SIGNAL]: uvccamera.on_suspend\n");
      break;
    case hippo::UVCCameraNotification::on_sohal_disconnected:
      fprintf(stderr, "[SIGNAL]: uvccamera.on_sohal_disconnected\n");
      break;
    case hippo::UVCCameraNotification::on_sohal_connected:
      fprintf(stderr, "[SIGNAL]: uvccamera.on_sohal_connected\n");
      break;
    default:
      break;
  }
#if 0
  // sample on how to use the void *data parameter to pass objects (the
  // this pointer in this case) to the callback function
  uint64_t err = 0LL;
  hippo::HiResCamera *hirescamera = (hippo::HiResCamera*)data;
  bool bool_get;
  // in this case we just get auto_exposure
  if (err = hirescamera->auto_exposure(&bool_get)) {
    print_error(err);
  }
  fprintf(stderr, "[SIGNAL+] uvccamera.auto_exposure(): %d\n", bool_get);
#endif
}
