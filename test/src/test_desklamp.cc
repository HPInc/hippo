
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#include <windows.h>    // for Sleep()
#include <cstdio>
#include "include/desklamp.h"

extern void print_error(uint64_t err);
void desklamp_notification(const hippo::DeskLampNotificationParam &param,
                           void *data);

uint64_t TestDeskLamp(hippo::DeskLamp *desklamp) {
  uint64_t err;
  fprintf(stderr, "##################################\n");
  fprintf(stderr, "  Now Testing Desklamp Commands\n");
  fprintf(stderr, "##################################\n");

  ADD_FILE_TO_MAP();   // will add this file to the file/error map

  // subscribe
  if (err = desklamp->subscribe(&desklamp_notification,
                                reinterpret_cast<void*>(desklamp))) {
    print_error(err);
  }

  bool bConnected;
  if (err = desklamp->is_device_connected(&bConnected)) {
    return err;
  }
  if (bConnected) {
    fprintf(stderr, "Desklamp is connected\n");
  } else {
    fprintf(stderr, "No desklamp is connected -- not running test\n");
    return MAKE_HIPPO_ERROR(hippo::HIPPO_DEVICE,
                            hippo::HIPPO_DEV_NOT_FOUND);
  }

  // open
  uint32_t open_count = 0;
  if (err = desklamp->open(&open_count)) {
    return err;
  }
  if (err = desklamp->high()) {
    return err;
  }
  fprintf(stderr, "desklamp->high\n");
  Sleep(1000);

  if (err = desklamp->low()) {
    return err;
  }
  fprintf(stderr, "desklamp->low\n");
  Sleep(1000);

  if (err = desklamp->off()) {
    return err;
  }
  fprintf(stderr, "desklamp->off\n");
  Sleep(1000);
  if (err = desklamp->close()) {
    return err;
  }
  fprintf(stderr, "desklamp->close\n");
  Sleep(1000);

  // unsubscribe
  if (err = desklamp->unsubscribe()) {
    print_error(err);
  }
  return 0LL;
}

const char *DeskLampState_str[
    static_cast<uint32_t>(hippo::DeskLampState::high) + 1] = {
  "off", "low", "high",
};

void desklamp_notification(const hippo::DeskLampNotificationParam &param,
                           void *data) {
  switch (param.type) {
    case hippo::DeskLampNotification::on_close:
      fprintf(stderr, "[SIGNAL]: desklamp.on_close\n");
      break;
    case hippo::DeskLampNotification::on_device_connected:
      fprintf(stderr, "[SIGNAL]: desklamp.on_device_connected\n");
      break;
    case hippo::DeskLampNotification::on_device_disconnected:
      fprintf(stderr, "[SIGNAL]: desklamp.on_device_disconnected\n");
      break;
    case hippo::DeskLampNotification::on_factory_default:
      fprintf(stderr, "[SIGNAL]: desklamp.on_factory_default\n");
      break;
    case hippo::DeskLampNotification::on_open:
      fprintf(stderr, "[SIGNAL]: desklamp.on_open\n");
      break;
    case hippo::DeskLampNotification::on_open_count:
      fprintf(stderr, "[SIGNAL]: desklamp.on_open_count %d\n",
              param.on_open_count);
      break;
    case hippo::DeskLampNotification::on_resume:
      fprintf(stderr, "[SIGNAL]: desklamp.on_resume\n");
      break;
    case hippo::DeskLampNotification::on_suspend:
      fprintf(stderr, "[SIGNAL]: desklamp.on_suspend\n");
      break;
    case hippo::DeskLampNotification::on_sohal_disconnected:
      fprintf(stderr, "[SIGNAL]: desklamp.on_sohal_disconnected\n");
      break;
    case hippo::DeskLampNotification::on_sohal_connected:
      fprintf(stderr, "[SIGNAL]: desklamp.on_sohal_connected\n");
      break;
    case hippo::DeskLampNotification::on_state:
      fprintf(stderr, "[SIGNAL]: desklamp.on_state: %s\n",
              DeskLampState_str[static_cast<uint32_t>(param.on_state)]);
      break;

      break;
    default:
      break;
  }
}
