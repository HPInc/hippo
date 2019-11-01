
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#include <windows.h>    // for Sleep()
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "include/touchmat.h"


const char wsAddress[] = "localhost";
const uint32_t wsPort = 20641;

extern void print_error(uint64_t err);

void print_active_area(const hippo::ActiveArea &area);
void touchmat_notification(const hippo::TouchMatNotificationParam &param,
                           void *data);

const char *ActivePenRange_str[] = {
  "five_mm", "ten_mm", "fifteen_mm", "twenty_mm",
};

uint64_t TestTouchMat(hippo::TouchMat *touchmat) {
  uint64_t err;
  fprintf(stderr, "#################################\n");
  fprintf(stderr, "  Now Testing Touchmat Commands\n");
  fprintf(stderr, "#################################\n");

  ADD_FILE_TO_MAP();   // will add this file to the file/error map

  // subscribe
  uint32_t num_subscribe = 0;
  if (err = touchmat->subscribe(&touchmat_notification,
                                reinterpret_cast<void*>(touchmat),
                                &num_subscribe)) {
    print_error(err);
  } else {
    fprintf(stderr, "touchmat.subscribe: count: %d\n", num_subscribe);
  }

  bool bConnected;
  if (err = touchmat->is_device_connected(&bConnected)) {
    return err;
  }
  if (bConnected) {
    fprintf(stderr, "Touchmat is connected\n");
  } else {
    fprintf(stderr, "No touchmat is connected -- not running test\n");
    return MAKE_HIPPO_ERROR(hippo::HIPPO_DEVICE,
                            hippo::HIPPO_DEV_NOT_FOUND);
  }

  // open
  uint32_t open_count = 0;
  if (err = touchmat->open(&open_count)) {
    return err;
  }
  fprintf(stderr, "touchmat.open(): open_count: %d\n", open_count);

  // touchmat hardware info
  hippo::TouchmatHardwareInfo tmHWInfo;
  if (err = touchmat->hardware_info(&tmHWInfo)) {
    return err;
  }
  fprintf(stderr, "Touchmat Hardware Info: (%f,%f)\n",
          tmHWInfo.size.width, tmHWInfo.size.height);

  // state
  hippo::TouchMatState st1, st2;
  if (err = touchmat->state(&st1)) {
    return err;
  }
  fprintf(stderr, "touchmat.state(): 'touch' : %d, 'active_pen' : %d\n",
          st1.touch, st1.active_pen);
  // opposite values values
  st2.touch = st1.touch ? false : true;
  st2.active_pen = st1.active_pen ? false : true;
  if (err = touchmat->state(st2)) {
    return err;
  }
  if (err = touchmat->state(&st1)) {
    return err;
  }
  fprintf(stderr, "touchmat.state(): 'touch' : %d, 'active_pen' : %d\n",
          st1.touch, st1.active_pen);
  // and default to false
  st2.touch = false;
  st2.active_pen = false;
  if (err = touchmat->state(st2, &st1)) {
    return err;
  }
  fprintf(stderr, "touchmat.state(): 'touch' : %d, 'active_pen' : %d\n",
          st1.touch, st1.active_pen);
  // active_area
  hippo::ActiveArea aa_1, aa_2;
  if (err = touchmat->active_area(&aa_1)) {
    return err;
  }
  fprintf(stderr, "touchmat.active_area(): ");
  print_active_area(aa_1);
  aa_1.enabled = aa_1.enabled ? false : true;
  if (err = touchmat->active_area(aa_1)) {
    return err;
  }
  fprintf(stderr, "touchmat.active_area(area)\n");
  if (err = touchmat->active_area(&aa_1)) {
    return err;
  }
  fprintf(stderr, "touchmat.active_area(): ");
  print_active_area(aa_1);
  aa_1.enabled = aa_1.enabled ? false : true;
  if (err = touchmat->active_area(aa_1, &aa_2)) {
    return err;
  }
  fprintf(stderr, "touchmat.active_area(area): ");
  print_active_area(aa_2);
  // active_pen_range
  hippo::ActivePenRange apr_1, apr_2;
  if (err = touchmat->active_pen_range(&apr_1)) {
    return err;
  }
  fprintf(stderr, "touchmat.active_pen_range(): %s\n",
          ActivePenRange_str[static_cast<uint32_t>(apr_1)]);
  if (err = touchmat->active_pen_range(hippo::ActivePenRange::ten_mm)) {
    return err;
  }
  fprintf(stderr, "touchmat.active_pen_range(10_mm)\n");
  if (err = touchmat->active_pen_range(&apr_1)) {
    return err;
  }
  fprintf(stderr, "touchmat.active_pen_range():  %s\n",
          ActivePenRange_str[static_cast<uint32_t>(apr_1)]);
  if (err = touchmat->active_pen_range(hippo::ActivePenRange::five_mm,
                                       &apr_2)) {
    return err;
  }
  fprintf(stderr, "touchmat.active_pen_range(5mm) %s\n",
          ActivePenRange_str[static_cast<uint32_t>(apr_2)]);
  // device palm rejection
  bool b, b2;
  if (err = touchmat->device_palm_rejection(&b)) {
    return err;
  }
  fprintf(stderr, "touchmat.device_palm_rejection():  %d\n", b);
  if (err = touchmat->device_palm_rejection(!b)) {
    return err;
  }
  fprintf(stderr, "touchmat.device_palm_rejection(%d)\n", !b);
  if (err = touchmat->device_palm_rejection(&b)) {
    return err;
  }
  fprintf(stderr, "touchmat.device_palm_rejection(): %d\n", b);
  if (err = touchmat->device_palm_rejection(!b, &b2)) {
    return err;
  }
  fprintf(stderr, "touchmat.device_palm_rejection(%d): %d\n", !b, b2);

  // palm_rejection_timeout
  uint32_t currtimeout, nexttimeout;
  if (err = touchmat->palm_rejection_timeout(&currtimeout)) {
    return err;
  }
  fprintf(stderr, "Current palm rejection timeout is %d\n", currtimeout);
  nexttimeout = currtimeout + 123;
  if (err = touchmat->palm_rejection_timeout(nexttimeout)) {
    return err;
  }
  nexttimeout = currtimeout;
  // now get the current
  if (err = touchmat->palm_rejection_timeout(&currtimeout)) {
    return err;
  }
  fprintf(stderr, "New palm rejection timeout is %d\n", currtimeout);
  if (err = touchmat->palm_rejection_timeout(nexttimeout, &currtimeout)) {
    return err;
  }
  fprintf(stderr, "Reset palm rejection timeout is %d\n", currtimeout);

  // calibrate
  fprintf(stderr, "Now testing touchmat.calibrate().\n");
  fprintf(stderr, "Please remove all objects from touchmat and press enter.\n");
  getc(stdin);
  if (err = touchmat->calibrate()) {
    return err;
  }

  // reset
  fprintf(stderr, "Now resetting touchmat\n");
  if (err = touchmat->reset()) {
    return err;
  }
  fprintf(stderr, "Sleeping for 10 seconds while touchmat reconnects\n");
  Sleep(10000);


  // close
  if (err = touchmat->close(&open_count)) {
    return err;
  }

  fprintf(stderr, "touchmat.close(): open_count: %d\n", open_count);
  // unsubscribe
  if (err = touchmat->unsubscribe(&num_subscribe)) {
    print_error(err);
  } else {
    fprintf(stderr, "touchmat.unsubscribe(): count: %d\n", num_subscribe);
  }
  return 0LL;
}

void print_active_area(const hippo::ActiveArea &area) {
  fprintf(stderr, "enabled: %d, top_left: [%d, %d], bottom_right: [%d, %d]\n",
          area.enabled, area.top_left.x, area.top_left.y,
          area.bottom_right.x, area.bottom_right.y);
}

void touchmat_notification(const hippo::TouchMatNotificationParam &param,
                           void *data) {
  switch (param.type) {
    case hippo::TouchMatNotification::on_close:
      fprintf(stderr, "[SIGNAL]: touchmat.on_close\n");
      break;
    case hippo::TouchMatNotification::on_device_connected:
      fprintf(stderr, "[SIGNAL]: touchmat.on_device_connected\n");
      break;
    case hippo::TouchMatNotification::on_device_disconnected:
      fprintf(stderr, "[SIGNAL]: touchmat.on_device_disconnected\n");
      break;
    case hippo::TouchMatNotification::on_factory_default:
      fprintf(stderr, "[SIGNAL]: touchmat.on_factory_default\n");
      break;
    case hippo::TouchMatNotification::on_open:
      fprintf(stderr, "[SIGNAL]: touchmat.on_open\n");
      break;
    case hippo::TouchMatNotification::on_open_count:
      fprintf(stderr, "[SIGNAL]: touchmat.on_open_count: %d\n",
              param.on_open_count);
      break;
    case hippo::TouchMatNotification::on_resume:
      fprintf(stderr, "[SIGNAL]: touchmat.on_resume\n");
      break;
    case hippo::TouchMatNotification::on_suspend:
      fprintf(stderr, "[SIGNAL]: touchmat.on_suspend\n");
      break;
    case hippo::TouchMatNotification::on_sohal_disconnected:
      fprintf(stderr, "[SIGNAL]: touchmat.on_sohal_disconnected\n");
      break;
    case hippo::TouchMatNotification::on_sohal_connected:
      fprintf(stderr, "[SIGNAL]: touchmat.on_sohal_connected\n");
      break;
    case hippo::TouchMatNotification::on_active_area:
      fprintf(stderr, "[SIGNAL]: touchmat.on_active_area: ");
      print_active_area(param.on_active_area);
      break;
    case hippo::TouchMatNotification::on_active_pen_range:
      fprintf(stderr, "[SIGNAL]: touchmat.on_active_pen_range: %s\n",
              ActivePenRange_str[
                  static_cast<uint32_t>(param.on_active_pen_range)]);
      break;
    case hippo::TouchMatNotification::on_calibrate:
      fprintf(stderr, "[SIGNAL]: touchmat.on_calibrate\n");
      break;
    case hippo::TouchMatNotification::on_device_palm_rejection:
      fprintf(stderr, "[SIGNAL]: touchmat.on_device_palm_rejection: %d\n",
              param.on_device_palm_rejection);
      break;
    case hippo::TouchMatNotification::on_palm_rejection_timeout:
      fprintf(stderr, "[SIGNAL]: touchmat.on_palm_rejection_timeout: %d\n",
              param.on_palm_rejection_timeout);
      break;
    case hippo::TouchMatNotification::on_reset:
      fprintf(stderr, "[SIGNAL]: touchmat.on_reset\n");
      break;
    case hippo::TouchMatNotification::on_state:
      fprintf(stderr, "[SIGNAL]: touchmat.on_state: 'touch' : %d, "
              "'active_pen' : %d\n", param.on_state.touch,
              param.on_state.active_pen);
      break;
  }
}
