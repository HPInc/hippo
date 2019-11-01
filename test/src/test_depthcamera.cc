
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#include <windows.h>    // for Sleep()
#include <cstdio>
#include "include/depthcamera.h"


#define _LATENCY_CHECK_
#define _DUMP_FRAME_

extern void print_error(uint64_t err);
extern void print_device_info(const hippo::DeviceInfo &info);

void depthcam_notification(const hippo::DepthCameraNotificationParam &param,
                           void *data);

extern uint64_t TestCameraStreams(hippo::HippoCamera *cam,
                                  hippo::CameraStreams st);

void print_ir_rgb_calibration(hippo::IrRgbCalibration * cal) {
  fprintf(stderr, "IR to RGB Calibration\n");
  fprintf(stderr, "IR Intrinsics:\n");
  fprintf(stderr, "\tFocal Length (%f,%f)\n", cal->ir_intrinsics.fx,
          cal->ir_intrinsics.fy);
  fprintf(stderr, "\tCenter Point (%f,%f)\n", cal->ir_intrinsics.cx,
          cal->ir_intrinsics.cy);
  fprintf(stderr, "RGB Intrinsics:\n");
  fprintf(stderr, "\tFocal Length (%f,%f)\n", cal->rgb_intrinsics.fx,
          cal->rgb_intrinsics.fy);
  fprintf(stderr, "\tCenter Point (%f,%f)\n", cal->rgb_intrinsics.cx,
          cal->rgb_intrinsics.cy);
  fprintf(stderr, "IR Distortion:\n");
  fprintf(stderr, "\tk1,k2,k3 (%f,%f,%f)\n\tp1,p2 (%f,%f)\n",
          cal->ir_distortion.k1, cal->ir_distortion.k2, cal->ir_distortion.k3,
          cal->ir_distortion.p1, cal->ir_distortion.p2);
  fprintf(stderr, "RGB Distortion:\n");
  fprintf(stderr, "\tk1,k2,k3 (%f,%f,%f)\n\tp1,p2 (%f,%f)\n",
          cal->rgb_distortion.k1, cal->rgb_distortion.k2,
          cal->rgb_distortion.k3,
          cal->rgb_distortion.p1, cal->rgb_distortion.p2);

  fprintf(stderr, "Depthcam IR to RGB Transformation Matrix:\n\t"
          "[[%f,\t%f,\t%f,\t%f]\n\t [%f,\t%f,\t%f,\t%f]\n\t "
          "[%f,\t%f,\t%f,\t%f]\n\t [%f,\t%f,\t%f,\t%f]]\n",
          cal->matrix_transformation[0][0], cal->matrix_transformation[0][1],
          cal->matrix_transformation[0][2], cal->matrix_transformation[0][3],
          cal->matrix_transformation[1][0], cal->matrix_transformation[1][1],
          cal->matrix_transformation[1][2], cal->matrix_transformation[1][3],
          cal->matrix_transformation[2][0], cal->matrix_transformation[2][1],
          cal->matrix_transformation[2][2], cal->matrix_transformation[2][3],
          cal->matrix_transformation[3][0], cal->matrix_transformation[3][1],
          cal->matrix_transformation[3][2], cal->matrix_transformation[3][3]);
  if (cal->mirror) {
    fprintf(stderr, "Camera is mirrored\n");
  } else {
    fprintf(stderr, "Camera is not mirrored\n");
  }
}

uint64_t TestDepthCamera(hippo::DepthCamera *depthcam) {
  uint64_t err;
  fprintf(stderr, "##################################\n");
  fprintf(stderr, "    Now Testing Depth Camera\n");
  fprintf(stderr, "##################################\n");

  ADD_FILE_TO_MAP();   // will add this file to the file/error map

  // subscribe
  uint32_t get;
  if (err = depthcam->subscribe(&depthcam_notification,
                                reinterpret_cast<void*>(depthcam),
                                &get)) {
    print_error(err);
  } else {
    fprintf(stderr, "depthcam.subscribe(): %d\n", get);
  }
  // is connected
  bool bConnected;
  if (err = depthcam->is_device_connected(&bConnected)) {
    return err;
  }
  if (bConnected) {
    fprintf(stderr, "Depthcam is connected\n");
  } else {
    fprintf(stderr, "No depthcam is connected -- not running test\n");
    return MAKE_HIPPO_ERROR(hippo::HIPPO_DEVICE,
                            hippo::HIPPO_DEV_NOT_FOUND);
  }
  // open
  uint32_t open_count = 0;
  if (err = depthcam->open(&open_count)) {
    return err;
  }
  fprintf(stderr, "depthcamera.open(): open_count: %d\n", open_count);
  // open_count
  if (err = depthcam->open_count(&open_count)) {
    print_error(err);
  }
  fprintf(stderr, "depthcam.open_count(): %d\n", open_count);
  // test all streams combinations
  for (uint8_t i = 1; i <= 15; i++) {
    hippo::CameraStreams st = { i };
    if (!(st.color && st.ir)) {
      if (!(st.points && !st.depth)) {
        if (err = TestCameraStreams(depthcam, st)) {
          print_error(err);
        }
      }
    }
  }
  // test IR floodlight
  if (err = depthcam->ir_flood_on(true)) {
    print_error(err);
  }
  bool bFloodOn = false;
  if (err = depthcam->ir_flood_on(&bFloodOn)) {
    print_error(err);
  }
  if (!bFloodOn) {
    fprintf(stderr, "Error - IR flood is off when commanded to on\n");
  } else {
    fprintf(stderr, "IR flood successfully turned on\n");
  }
  Sleep(2000);
  if (err = depthcam->ir_flood_on(false, &bFloodOn)) {
    print_error(err);
  }
  if (bFloodOn) {
    fprintf(stderr, "Error - IR flood is on when commanded to off\n");
  } else {
    fprintf(stderr, "IR flood successfully turned off\n");
  }
  // test depthcamera laser
  if (err = depthcam->laser_on(false)) {
    print_error(err);
  }
  bool bLaserOn;
  if (err = depthcam->laser_on(&bLaserOn)) {
    print_error(err);
  }
  if (bLaserOn) {
    fprintf(stderr, "Error - Depthcamera laser is on when commanded to off\n");
  } else {
    fprintf(stderr, "Depthcamera laser successfully turned off\n");
  }
  Sleep(2000);
  if (err = depthcam->laser_on(true, &bLaserOn)) {
    print_error(err);
  }
  if (!bLaserOn) {
    fprintf(stderr, "Error - Depthcamera laser is off when commanded to on\n");
  } else {
    fprintf(stderr, "Depthcamera laser successfully turned on\n");
  }
  // Get Orbbec to OV580 calibration
  hippo::IrRgbCalibration irToDepthCal;
  if (err = depthcam->ir_to_rgb_calibration(&irToDepthCal)) {
    print_error(err);
  } else {
    print_ir_rgb_calibration(&irToDepthCal);
  }
  // info
  hippo::DeviceInfo info;
  if (err = depthcam->info(&info)) {
    print_error(err);
  } else {
    print_device_info(info);
    // free the device info that was allocated in the depthcam->info()
    // function call
    depthcam->free_device_info(&info);
  }
  // close
  if (err = depthcam->close(&open_count)) {
    return err;
  }
  fprintf(stderr, "depthcam.close(): open_count: %d\n", open_count);

  // unsubscribe
  if (err = depthcam->unsubscribe(&get)) {
    print_error(err);
  } else {
    fprintf(stderr, "depthcam.unsubscribe(): %d\n", get);
  }

  return 0LL;
}

void depthcam_notification(const hippo::DepthCameraNotificationParam &param,
                           void *data) {
  switch (param.type) {
    case hippo::DepthCameraNotification::on_close:
      fprintf(stderr, "[SIGNAL]: depthcamera.on_close\n");
      break;
    case hippo::DepthCameraNotification::on_device_connected:
      fprintf(stderr, "[SIGNAL]: depthcamera.on_device_connected\n");
      break;
    case hippo::DepthCameraNotification::on_device_disconnected:
      fprintf(stderr, "[SIGNAL]: depthcamera.on_device_disconnected\n");
      break;
    case hippo::DepthCameraNotification::on_factory_default:
      fprintf(stderr, "[SIGNAL]: depthcamera.on_factory_default\n");
      break;
    case hippo::DepthCameraNotification::on_open:
      fprintf(stderr, "[SIGNAL]: depthcamera.on_open\n");
      break;
    case hippo::DepthCameraNotification::on_open_count:
      fprintf(stderr, "[SIGNAL]: depthcamera.on_open_count %d\n",
              param.on_open_count);
      break;
    case hippo::DepthCameraNotification::on_resume:
      fprintf(stderr, "[SIGNAL]: depthcamera.on_resume\n");
      break;
    case hippo::DepthCameraNotification::on_suspend:
      fprintf(stderr, "[SIGNAL]: depthcamera.on_suspend\n");
      break;
    case hippo::DepthCameraNotification::on_sohal_disconnected:
      fprintf(stderr, "[SIGNAL]: depthcamera.on_sohal_disconnected\n");
      break;
    case hippo::DepthCameraNotification::on_sohal_connected:
      fprintf(stderr, "[SIGNAL]: depthcamera.on_sohal_connected\n");
      break;
    case hippo::DepthCameraNotification::on_ir_flood_on:
      fprintf(stderr, "[SIGNAL]: depthcamera.on_ir_flood_on: %s\n",
              param.on_ir_flood_on? "True":"False");
      break;
    case hippo::DepthCameraNotification::on_laser_on:
      fprintf(stderr, "[SIGNAL]: depthcamera.on_laser_on: %s\n",
              param.on_laser_on ? "True" : "False");
      break;
    case hippo::DepthCameraNotification::on_enable_streams:
      fprintf(stderr,
              "[SIGNAL]: depthcamera.on_enable_streams: [%d]\n",
              param.on_enable_streams.value);
      break;
    case hippo::DepthCameraNotification::on_disable_streams:
      fprintf(stderr,
              "[SIGNAL]: depthcamera.on_disable_streams: [%d]\n",
              param.on_disable_streams.value);
      break;
    default:
      break;
  }
}
