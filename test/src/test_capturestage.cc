
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#include <windows.h>    // for Sleep()
#include <cstdio>
#include "include/capturestage.h"

extern void print_error(uint64_t err);
extern void print_device_info(const hippo::DeviceInfo &info);

void capturestage_notification(
                              const hippo::CaptureStageNotificationParam &param,
                              void *data);

const char *ledStateToString[
  static_cast<uint32_t>(hippo::LedStateEnum::led_blink_off_phase) + 1] = {
  "off", "on", "blink_in_phase", "blink_off_phase"
};

uint64_t TestCaptureStage(hippo::CaptureStage *capturestage) {
  uint64_t err;
  fprintf(stderr, "##################################\n");
  fprintf(stderr, "Now Testing Capture Stage Commands\n");
  fprintf(stderr, "##################################\n");

  ADD_FILE_TO_MAP();   // will add this file to the file/error map

  // subscribe
  if (err = capturestage->subscribe(&capturestage_notification,
                                reinterpret_cast<void*>(capturestage))) {
    print_error(err);
  }

  bool bConnected;
  if (err = capturestage->is_device_connected(&bConnected)) {
    return err;
  }
  if (bConnected) {
    fprintf(stderr, "Capturestage is connected\n");
  } else {
    fprintf(stderr, "No capturestage is connected -- not running test\n");
    return MAKE_HIPPO_ERROR(hippo::HIPPO_DEVICE,
                            hippo::HIPPO_DEV_NOT_FOUND);
  }

  // open
  uint32_t open_count = 0;
  if (err = capturestage->open(&open_count)) {
    return err;
  }
  fprintf(stderr, "capturestage.open(): open_count: %d\n", open_count);

  hippo::CaptureStageSpecificInfo csInfo;
  if (err = capturestage->device_specific_info(&csInfo)) {
    capturestage->free_capturestage_info(&csInfo);
    return err;
  }
  fprintf(stderr, "Capturestage is connected to: %s\n", csInfo.port);
  capturestage->free_capturestage_info(&csInfo);

  // home
  fprintf(stderr, "capturestage home():\n");
  if (err = capturestage->home()) {
    print_error(err);
  }
  // led
  hippo::LedState state;
  state.amber = hippo::LedStateEnum::led_on;
  state.red = hippo::LedStateEnum::led_off;
  state.white = hippo::LedStateEnum::led_off;
  if (err = capturestage->led_state(state)) {
    print_error(err);
  }

  Sleep(1000);
  // rotate
  float set_angle = 36., get_angle = 0.;
  for (uint32_t i = 0; i < 3; i++) {
    if (err = capturestage->rotate(set_angle)) {
      print_error(err);
    }
    fprintf(stderr, "capturestage.rotate(%f)\n", set_angle);
  }
  // led on_off_rate
  hippo::LedOnOffRate ledRate, ledRateGet;
  ledRate.time_off = 100;
  ledRate.time_on = 1000;
  if (err = capturestage->led_on_off_rate(ledRate)) {
    print_error(err);
  }
  if (err = capturestage->led_on_off_rate(ledRate, &ledRateGet)) {
    print_error(err);
  }
  if (ledRate.time_off != ledRateGet.time_off ||
      ledRate.time_on != ledRateGet.time_on) {
    fprintf(stderr, "[ERROR]: LedRate doesn't match\n");
  }

  // led red/white blinking
  state.amber = hippo::LedStateEnum::led_off;
  state.red = hippo::LedStateEnum::led_blink_in_phase;
  state.white = hippo::LedStateEnum::led_blink_off_phase;
  fprintf(stderr, "capturestage.led_state (red in phase)\n");
  if (err = capturestage->led_state(state)) {
    print_error(err);
  }
  Sleep(1000);
  // rotate
  set_angle *= -1.f;
  for (uint32_t i = 0; i < 3; i++) {
    if (err = capturestage->rotate(set_angle, &get_angle)) {
      print_error(err);
    }
    fprintf(stderr, "capturestage.rotate(%f) = %f\n", set_angle, get_angle);
  }
  if (err = capturestage->rotation_angle(&get_angle)) {
    print_error(err);
  }
  fprintf(stderr, "get capturestage.rotation_angle = %f\n", get_angle);
  // led white
  state.amber = hippo::LedStateEnum::led_off;
  state.red = hippo::LedStateEnum::led_off;
  state.white = hippo::LedStateEnum::led_on;
  if (err = capturestage->led_state(state)) {
    print_error(err);
  }
  fprintf(stderr, "capturestage.led_state (white on)\n");
  // tilt
  float tilt_set = 90., tilt_get = -1.;
  if (err = capturestage->tilt(&tilt_get)) {
    print_error(err);
  }
  fprintf(stderr, "capturestage.tilt = %f\n", tilt_get);
  fprintf(stderr, "Tilting...\n");
  if (err = capturestage->tilt(tilt_set)) {
    print_error(err);
  }
  if (err = capturestage->tilt(&tilt_get)) {
    print_error(err);
  }
  fprintf(stderr, "capturestage.tilt = %f\n", tilt_get);
  // led on_off_rate
  ledRate.time_off = 500;
  ledRate.time_on = 300;
  if (err = capturestage->led_on_off_rate(ledRate)) {
    print_error(err);
  }
  // led amber/red blinking
  state.amber = hippo::LedStateEnum::led_blink_in_phase;
  state.red = hippo::LedStateEnum::led_blink_off_phase;
  state.white = hippo::LedStateEnum::led_off;
  if (err = capturestage->led_state(state)) {
    print_error(err);
  }
  fprintf(stderr,
          "capturestage.led_state (amber in phase, red out of phase)\n");
  Sleep(1000);
  tilt_set = 180.;
  if (err = capturestage->tilt(180., &tilt_get)) {
    print_error(err);
  }
  fprintf(stderr, "capturestage.tilt[%f] = %f\n", tilt_set, tilt_get);
  // led off
  state.amber = hippo::LedStateEnum::led_off;
  state.red = hippo::LedStateEnum::led_off;
  state.white = hippo::LedStateEnum::led_off;
  fprintf(stderr, "capturestage.led_state (off):\n");
  if (err = capturestage->led_state(state)) {
    print_error(err);
  }
  Sleep(1000);
  // factory_default
  fprintf(stderr, "capturestage.factory_default():\n");
  if (err = capturestage->factory_default()) {
    print_error(err);
  }
  // info
  hippo::DeviceInfo info;
  fprintf(stderr, "capturestage.device_info():\n");
  if (err = capturestage->info(&info)) {
    print_error(err);
  } else {
    print_device_info(info);
    // free the device info that was allocated in the capturestage->info()
    // function call
    capturestage->free_device_info(&info);
  }
  // close
  if (err = capturestage->close(&open_count)) {
    return err;
  }
  fprintf(stderr, "capturestage.close(): open_count: %d\n", open_count);
  Sleep(1000);
  // unsubscribe
  fprintf(stderr, "capturestage.unsubscribe()\n");
  if (err = capturestage->unsubscribe()) {
    print_error(err);
  }

  return 0LL;
}

void capturestage_notification(
                              const hippo::CaptureStageNotificationParam &param,
                              void *data) {
  switch (param.type) {
  case hippo::CaptureStageNotification::on_close:
    fprintf(stderr, "[SIGNAL]: capturestage.on_close\n");
    break;
  case hippo::CaptureStageNotification::on_device_connected:
    fprintf(stderr, "[SIGNAL]: capturestage.on_device_connected\n");
    break;
  case hippo::CaptureStageNotification::on_device_disconnected:
    fprintf(stderr, "[SIGNAL]: capturestage.on_device_disconnected\n");
    break;
  case hippo::CaptureStageNotification::on_factory_default:
    fprintf(stderr, "[SIGNAL]: capturestage.on_factory_default\n");
    break;
  case hippo::CaptureStageNotification::on_open:
    fprintf(stderr, "[SIGNAL]: capturestage.on_open\n");
    break;
  case hippo::CaptureStageNotification::on_open_count:
    fprintf(stderr, "[SIGNAL]: capturestage.on_open_count %d\n",
            param.on_open_count);
    break;
  case hippo::CaptureStageNotification::on_resume:
    fprintf(stderr, "[SIGNAL]: capturestage.on_resume\n");
    break;
  case hippo::CaptureStageNotification::on_suspend:
    fprintf(stderr, "[SIGNAL]: capturestage.on_suspend\n");
    break;
  case hippo::CaptureStageNotification::on_sohal_disconnected:
    fprintf(stderr, "[SIGNAL]: capturestage.on_sohal_disconnected\n");
    break;
  case hippo::CaptureStageNotification::on_sohal_connected:
    fprintf(stderr, "[SIGNAL]: capturestage.on_sohal_connected\n");
    break;
  case hippo::CaptureStageNotification::on_home:
    fprintf(stderr, "[SIGNAL]: capturestage.on_home\n");
    break;
  case hippo::CaptureStageNotification::on_led_on_off_rate:
    fprintf(stderr, "[SIGNAL]: capturestage.on_led_on_off_rate: "
                    "%i off, %i on\n",
            param.on_off_rate.time_off, param.on_off_rate.time_on);
    break;
  case hippo::CaptureStageNotification::on_led_state:
    fprintf(stderr, "[SIGNAL]: capturestage.on_led_state"
                    "\n\t\tamber\t=\t%s,\n\t\tred\t=\t%s,\n\t\twhite\t=\t%s\n",
            ledStateToString[static_cast<uint32_t>(param.on_led_state.amber)],
            ledStateToString[static_cast<uint32_t>(param.on_led_state.red)],
            ledStateToString[static_cast<uint32_t>(param.on_led_state.white)]);
    break;
  case hippo::CaptureStageNotification::on_rotate:
    fprintf(stderr, "[SIGNAL]: capturestage.on_rotate: %f\n", param.on_rotate);
    break;
  case hippo::CaptureStageNotification::on_tilt:
    fprintf(stderr, "[SIGNAL]: capturestage.on_tilt: %f\n", param.on_tilt);
    break;
  }
}
