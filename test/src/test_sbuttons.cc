
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#include <windows.h>    // for Sleep()
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "include/sbuttons.h"

extern const char wsAddress[];
extern const uint32_t wsPort;

extern void print_error(uint64_t err);

void sbuttons_notification(const hippo::SButtonsNotificationParam &param,
                           void *data);
void print_button_led_state(const hippo::ButtonLedStateNotification &led_state);
void print_button_press(const hippo::ButtonPress &button_press);


uint64_t TestSButtons(hippo::SButtons *sbuttons) {
  uint64_t err;
  fprintf(stderr, "#################################\n");
  fprintf(stderr, "   Now Testing Sbutton Commands\n");
  fprintf(stderr, "#################################\n");

  ADD_FILE_TO_MAP();   // will add this file to the file/error map

  // subscribe
  uint32_t num_subscribe = 0;
  if (err = sbuttons->subscribe(&sbuttons_notification,
                                reinterpret_cast<void*>(sbuttons),
                                &num_subscribe)) {
    print_error(err);
  } else {
    fprintf(stderr, "sbuttons.subscribe: count: %d\n", num_subscribe);
  }

  bool bConnected;
  if (err = sbuttons->is_device_connected(&bConnected)) {
    return err;
  }
  if (bConnected) {
    fprintf(stderr, "Sbuttons connected\n");
  } else {
    fprintf(stderr, "No sbuttons connected -- not running test\n");
    return MAKE_HIPPO_ERROR(hippo::HIPPO_DEVICE,
                            hippo::HIPPO_DEV_NOT_FOUND);
  }

  // open
  uint32_t open_count = 0;
  if (err = sbuttons->open(&open_count)) {
    return err;
  }
  fprintf(stderr, "sbuttons.open(): open_count: %d\n", open_count);
  // state
  hippo::ButtonId id(hippo::ButtonId::left);
  hippo::ButtonLedState st_get, st_set;
  st_set = {hippo::ButtonLedColor::orange, hippo::ButtonLedMode::pulse};
  // test the get command
  if (err = sbuttons->led_state(id, &st_get)) {
    return err;
  }
  fprintf(stderr, "sbuttons.led_state(id:%d, color:%d, mode:%d):"
          "  color:%d, mode:%d\n",
          id, st_set.color, st_set.mode, st_get.color, st_get.mode);

  // test the set-get command
  if (err = sbuttons->led_state(id, st_set, &st_get)) {
    return err;
  }
  fprintf(stderr, "sbuttons.led_state(id:%d, color:%d, mode:%d):"
          "  color:%d, mode:%d\n",
          id, st_set.color, st_set.mode, st_get.color, st_get.mode);
  id = hippo::ButtonId::center;
  st_set = {hippo::ButtonLedColor::white, hippo::ButtonLedMode::on};
  if (err = sbuttons->led_state(id, st_set, &st_get)) {
    return err;
  }
  fprintf(stderr, "sbuttons.led_state(id:%d, color:%d, mode:%d):"
          "  color:%d, mode:%d\n",
          id, st_set.color, st_set.mode, st_get.color, st_get.mode);
  id = hippo::ButtonId::right;
  st_set = {hippo::ButtonLedColor::white_orange, hippo::ButtonLedMode::breath};
  if (err = sbuttons->led_state(id, st_set, &st_get)) {
    return err;
  }
  fprintf(stderr, "sbuttons.led_state(id:%d, color:%d, mode:%d):"
          "  color:%d, mode:%d\n",
          id, st_set.color, st_set.mode, st_get.color, st_get.mode);
  // button_press
  fprintf(stderr, "*******\n*\n* Here you have 10 seconds to test "
          "the sbuttons.on_button_press notifications\n"
          "* Please tap/hold the sbuttons\n*\n*******\n");
  Sleep(10000);
  // unsubscribe
  if (err = sbuttons->unsubscribe(&num_subscribe)) {
    print_error(err);
  } else {
    fprintf(stderr, "sbuttons.unsubscribe: count: %d\n", num_subscribe);
  }
  return 0;
}

void sbuttons_notification(const hippo::SButtonsNotificationParam &param,
                           void *data) {
  switch (param.type) {
    case hippo::SButtonsNotification::on_close:
      fprintf(stderr, "[SIGNAL]: sbuttons.on_close\n");
      break;
    case hippo::SButtonsNotification::on_device_connected:
      fprintf(stderr, "[SIGNAL]: sbuttons.on_device_connected\n");
      break;
    case hippo::SButtonsNotification::on_device_disconnected:
      fprintf(stderr, "[SIGNAL]: sbuttons.on_device_disconnected\n");
      break;
    case hippo::SButtonsNotification::on_factory_default:
      fprintf(stderr, "[SIGNAL]: sbuttons.on_factory_default\n");
      break;
    case hippo::SButtonsNotification::on_open:
      fprintf(stderr, "[SIGNAL]: sbuttons.on_open\n");
      break;
    case hippo::SButtonsNotification::on_open_count:
      fprintf(stderr, "[SIGNAL]: sbuttons.on_open_count %d\n",
              param.on_open_count);
      break;
    case hippo::SButtonsNotification::on_resume:
      fprintf(stderr, "[SIGNAL]: sbuttons.on_resume\n");
      break;
    case hippo::SButtonsNotification::on_suspend:
      fprintf(stderr, "[SIGNAL]: sbuttons.on_suspend\n");
      break;
    case hippo::SButtonsNotification::on_sohal_disconnected:
      fprintf(stderr, "[SIGNAL]: sbuttons.on_sohal_disconnected\n");
      break;
    case hippo::SButtonsNotification::on_sohal_connected:
      fprintf(stderr, "[SIGNAL]: sbuttons.on_sohal_connected\n");
      break;
    case hippo::SButtonsNotification::on_hold_threshold:
      fprintf(stderr, "[SIGNAL]: sbuttons.on_hold_threshold: %d\n",
              param.on_hold_threshold);
      break;
    case hippo::SButtonsNotification::on_led_on_off_rate:
      fprintf(stderr, "[SIGNAL]: sbuttons.on_led_on_off_rate: %d\n",
              param.on_led_on_off_rate);
      break;
    case hippo::SButtonsNotification::on_led_pulse_rate:
      fprintf(stderr, "[SIGNAL]: sbuttons.on_led_pulse_rate: %d\n",
              param.on_led_pulse_rate);
      break;
    case hippo::SButtonsNotification::on_led_state:
      fprintf(stderr, "[SIGNAL]: sbuttons.on_led_state: ");
      print_button_led_state(param.on_led_state);
      break;
    case hippo::SButtonsNotification::on_button_press:
      fprintf(stderr, "[SIGNAL]: sbuttons.on_button_press: ");
      print_button_press(param.on_button_press);
      break;
    default:
      break;
  }
}

char *ButtonId_str[] = { "left", "center", "right", };
char *ButtonLedColor_str[] = { "orange", "white", "white_orange", };
char *ButtonLedMode_str[] = { "breath", "controlled_on", "controlled_off",
                              "off", "on", "pulse", };
char *ButtonPressType_str[] = { "tap", "hold", };

void print_button_led_state(
    const hippo::ButtonLedStateNotification &led_state) {
  fprintf(stderr, "'%s' '%s' '%s'\n",
          ButtonId_str[static_cast<uint32_t>(led_state.id)],
          ButtonLedColor_str[static_cast<uint32_t>(led_state.state.color)],
          ButtonLedMode_str[static_cast<uint32_t>(led_state.state.mode)]);
}

void print_button_press(const hippo::ButtonPress &button_press) {
  fprintf(stderr, "'%s' '%s'\n",
          ButtonId_str[static_cast<uint32_t>(button_press.id)],
          ButtonPressType_str[static_cast<uint32_t>(button_press.type)]);
}
