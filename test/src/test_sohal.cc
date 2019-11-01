
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#include <windows.h>    // for Sleep()
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "include/sohal.h"

extern void print_error(uint64_t err);

void sohal_notification(const hippo::SoHalNotificationParam &param,
                        void *data);

uint64_t TestSoHal(hippo::SoHal *sohal) {
  uint64_t err;
  fprintf(stderr, "##################################\n");
  fprintf(stderr, "  Now Testing SoHal Commands\n");
  fprintf(stderr, "##################################\n");

  ADD_FILE_TO_MAP();   // will add this file to the file/error map

  // subscribe
  uint32_t num_subscribe = 0;
  if (err = sohal->subscribe(&sohal_notification,
                             reinterpret_cast<void*>(sohal),
                             &num_subscribe)) {
    print_error(err);
    // return if SoHal is not running
    if (hippo::HIPPO_OPEN == hippo::HippoErrorCode(err)) {
      fprintf(stderr, "Error - Can not connect to SoHal\n");
      return err;
    }
  } else {
    fprintf(stderr, "sohal.subscribe: count: %d\n", num_subscribe);
  }

  hippo::LogInfo li1, li2, testli;
  // get the current log level
  if (err = sohal->log(&li1)) {
    print_error(err);
  }

  if (li1.level + 1 <= 3) {
    li2.level = li1.level + 1;
  } else {
    li2.level = li1.level - 1;
  }
  // change the log level
  if (err = sohal->log(li2, &testli)) {
    print_error(err);
  }
  if (li2.level != testli.level) {
    fprintf(stderr, "Error - log levels don't match\n");
  } else {
    fprintf(stderr, "Success changing log level\n");
  }
  // reset the log level
  if (err = sohal->log(li1)) {
    print_error(err);
  }

  char *version = NULL;
  if (err = sohal->version(&version)) {
    print_error(err);
    sohal->free_version(version);
  } else {
    fprintf(stderr, "sohal.version: '%s'\n", version);
  }

#if 0
  // exit SoHal
  if (err = sohal->exit()) {
    print_error(err);
  }
#endif

  return 0LL;
}

void sohal_notification(const hippo::SoHalNotificationParam &param,
                        void *data) {
  switch (param.type) {
    case hippo::SoHalNotification::on_close:
      fprintf(stderr, "[SIGNAL]: sohal.on_close\n");
      break;
    case hippo::SoHalNotification::on_device_connected:
      fprintf(stderr, "[SIGNAL]: sohal.on_device_connected\n");
      break;
    case hippo::SoHalNotification::on_device_disconnected:
      fprintf(stderr, "[SIGNAL]: sohal.on_device_disconnected\n");
      break;
    case hippo::SoHalNotification::on_factory_default:
      fprintf(stderr, "[SIGNAL]: sohal.on_factory_default\n");
      break;
    case hippo::SoHalNotification::on_open:
      fprintf(stderr, "[SIGNAL]: sohal.on_open\n");
      break;
    case hippo::SoHalNotification::on_resume:
      fprintf(stderr, "[SIGNAL]: sohal.on_resume\n");
      break;
    case hippo::SoHalNotification::on_suspend:
      fprintf(stderr, "[SIGNAL]: sohal.on_suspend\n");
      break;
    case hippo::SoHalNotification::on_sohal_disconnected:
      fprintf(stderr, "[SIGNAL]: sohal.on_sohal_disconnected\n");
      break;
    case hippo::SoHalNotification::on_sohal_connected:
      fprintf(stderr, "[SIGNAL]: sohal.on_sohal_connected\n");
      break;
    case hippo::SoHalNotification::on_exit:
      fprintf(stderr, "[SIGNAL]: sohal.on_exit\n");
      break;
    case hippo::SoHalNotification::on_log:
      fprintf(stderr, "[SIGNAL]: sohal.on_log: %d\n",
              param.on_log.level);
      break;
    default:
      break;
  }
}
