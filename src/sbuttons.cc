
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <mutex>   // NOLINT
#include "../include/json.hpp"
#include "../include/sbuttons.h"

namespace nl = nlohmann;

namespace hippo {

extern std::mutex gHippoDeviceMutex;
const char devName[] = "sbuttons";
extern const char *defaultHost;
extern uint32_t defaultPort;

SButtons::SButtons() :
        HippoDevice(devName, defaultHost, defaultPort, HIPPO_SBUTTONS, 0),
    callback_(NULL) {
}

SButtons::SButtons(uint32_t device_index) :
  HippoDevice(devName, defaultHost, defaultPort, HIPPO_SBUTTONS,
              device_index),
  callback_(NULL) {
}

SButtons::SButtons(const char *address, uint32_t port) :
        HippoDevice(devName, address, port, HIPPO_SBUTTONS, 0),
    callback_(NULL) {
}

SButtons::SButtons(const char *address, uint32_t port,
                     uint32_t device_index) :
  HippoDevice(devName, address, port, HIPPO_SBUTTONS, device_index),
  callback_(NULL) {
}

SButtons::~SButtons(void) {
}

uint64_t SButtons::hold_threshold(uint32_t set) {
  return uint32_set_get("hold_threshold", set, NULL);
}

uint64_t SButtons::hold_threshold(uint32_t *get) {
  return uint32_get("hold_threshold", get);
}

uint64_t SButtons::hold_threshold(uint32_t set, uint32_t *get) {
  return uint32_set_get("hold_threshold", set, get);
}

uint64_t SButtons::led_on_off_rate(uint32_t set) {
  return uint32_set_get("led_on_off_rate", set, NULL);
}

uint64_t SButtons::led_on_off_rate(uint32_t *get) {
  return uint32_get("led_on_off_rate", get);
}

uint64_t SButtons::led_on_off_rate(uint32_t set, uint32_t *get) {
  return uint32_set_get("led_on_off_rate", set, get);
}

uint64_t SButtons::led_pulse_rate(uint32_t set) {
  return uint32_set_get("led_pulse_rate", set, NULL);
}

uint64_t SButtons::led_pulse_rate(uint32_t *get) {
  return uint32_get("led_pulse_rate", get);
}

uint64_t SButtons::led_pulse_rate(uint32_t set, uint32_t *get) {
  return uint32_set_get("led_pulse_rate", set, get);
}

// map between button IDs and json strings
const char *ButtonId_str[] = { "left", "center", "right" };
const char *ButtonLedColor_str[] = { "orange", "white", "white_orange" };
const char *ButtonLedMode_str[] = { "breath", "controlled_on", "controlled_off",
"off", "on", "pulse" };

uint64_t SButtons::led_state(ButtonId id, const ButtonLedState &set) {
  return led_state(id, set, NULL);
}

uint64_t SButtons::led_state(ButtonId id, ButtonLedState *get) {
  if (NULL == get) {
    return  MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  uint64_t err = HIPPO_OK;
  nl::json jset, jget;
  void *jsetptr = reinterpret_cast<void*>(&jset);
  void *jgetptr = reinterpret_cast<void*>(&jget);
  // add the button name to the parameters
  jset.push_back(ButtonId_str[static_cast<uint32_t>(id)]);
  if (err = SendRawMsg("led_state", jsetptr, jgetptr)) {
    return err;
  }
  return button_led_state_json2c(jgetptr, get);
}

uint64_t SButtons::led_state(ButtonId id, const ButtonLedState &set,
                             ButtonLedState *get) {
  uint64_t err = HIPPO_OK;
  nl::json jset, jget;
  void *jsetptr = reinterpret_cast<void*>(&jset);
  if (err = button_led_state_c2json(id, set, jsetptr)) {
    return err;
  }
  void *jgetptr = reinterpret_cast<void*>(&jget);
  if (err = SendRawMsg("led_state", jsetptr, jgetptr)) {
    return err;
  }
  if (get != NULL) {
    err = button_led_state_json2c(jgetptr, get);
  }
  return err;
}

uint64_t SButtons::button_led_state_c2json(hippo::ButtonId id,
                                         const hippo::ButtonLedState &state,
                                         void *obj) {
  if (obj == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }

  nl::json *params = reinterpret_cast<nl::json*>(obj);

  params->push_back(ButtonId_str[static_cast<uint32_t>(id)]);
  params->push_back({
          { "color", ButtonLedColor_str[static_cast<uint32_t>(state.color)] },
          { "mode", ButtonLedMode_str[static_cast<uint32_t>(state.mode)] }
        });

  return HIPPO_OK;
}


uint64_t SButtons::button_led_state_json2c(void *obj,
                                           hippo::ButtonLedState *state) {
  // test inputs to ensure non-null pointers
  if (obj == NULL || state == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
  }

  const nl::json *ledStateData = reinterpret_cast<const nl::json*>(obj);
  if (!ledStateData->is_object()) {
    return MAKE_HIPPO_ERROR(facility_,
                            HIPPO_MESSAGE_ERROR);
  }
  try {
    const nl::json *jsonColor = &ledStateData->at("color");
    const nl::json *jsonMode = &ledStateData->at("mode");
    // validate that the next item is a string
    if (!jsonColor->is_string() && !jsonMode->is_string()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // get the values
    std::string colorStr = jsonColor->get<std::string>();
    std::string modeStr = jsonMode->get<std::string>();
    int32_t idx;
    idx = str_to_idx(ButtonLedColor_str, colorStr.c_str(),
                    static_cast<uint32_t>(hippo::ButtonLedColor::orange),
                    static_cast<uint32_t>(hippo::ButtonLedColor::white_orange));
    if (idx < 0) {
      return -1;
    }
    state->color = static_cast<hippo::ButtonLedColor>(idx);
    idx = str_to_idx(ButtonLedMode_str, modeStr.c_str(),
                     static_cast<uint32_t>(hippo::ButtonLedMode::breath),
                     static_cast<uint32_t>(hippo::ButtonLedMode::pulse));
    if (idx < 0) {
      return -1;
    }
    state->mode = static_cast<hippo::ButtonLedMode>(idx);
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return HIPPO_OK;
}

uint64_t SButtons::button_led_state_notification_json2c(
    void *obj,
    hippo::ButtonLedStateNotification *state) {
  uint64_t err = 0LL;
  if (obj == NULL || state == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
  }
  const nl::json *ledStateData = reinterpret_cast<const nl::json*>(obj);
  if (!ledStateData->is_array()) {
    return MAKE_HIPPO_ERROR(facility_,
                            HIPPO_MESSAGE_ERROR);
  }
  try {
    const nl::json *id = &ledStateData->at(0);
    const nl::json *type = &ledStateData->at(1);
    if (!(id->is_string() && type->is_object())) {
      return MAKE_HIPPO_ERROR(facility_,
                              HIPPO_MESSAGE_ERROR);
    }
    int32_t idx;
    idx = str_to_idx(ButtonId_str, id->get<std::string>().c_str(),
                     static_cast<uint32_t>(hippo::ButtonId::left),
                     static_cast<uint32_t>(hippo::ButtonId::right));
    if (idx < 0) {
      err = MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
      goto clean_up;
    }
    state->id = static_cast<hippo::ButtonId>(idx);
    const nl::json *color = &type->at("color");
    idx = str_to_idx(ButtonLedColor_str, color->get<std::string>().c_str(),
                     static_cast<uint32_t>(hippo::ButtonLedColor::orange),
                     static_cast<uint32_t>(
                         hippo::ButtonLedColor::white_orange));
    if (idx < 0) {
      err = MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
      goto clean_up;
    }
    state->state.color = static_cast<hippo::ButtonLedColor>(idx);
    const nl::json *mode = &type->at("mode");
    idx = str_to_idx(ButtonLedMode_str, mode->get<std::string>().c_str(),
                     static_cast<uint32_t>(hippo::ButtonLedMode::breath),
                     static_cast<uint32_t>(hippo::ButtonLedMode::pulse));
    if (idx < 0) {
      err = MAKE_HIPPO_ERROR(facility_,
                             HIPPO_MESSAGE_ERROR);
      goto clean_up;
    }
    state->state.mode = static_cast<hippo::ButtonLedMode>(idx);
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
clean_up:
  return err;
}

uint64_t SButtons::subscribe(
    void (*callback)(const SButtonsNotificationParam &param, void *data),
    void *data) {
  return subscribe(callback, data, NULL);
}

uint64_t SButtons::subscribe(
    void (*callback)(const SButtonsNotificationParam &param, void *data),
    void *data, uint32_t *get) {
  uint64_t err = 0LL;

  if (err = HippoDevice::subscribe_raw(data, get)) {
    return err;
  }
  callback_ = callback;

  return err;
}

uint64_t SButtons::unsubscribe() {
  callback_ = NULL;
  return HippoDevice::unsubscribe();
}

uint64_t SButtons::unsubscribe(uint32_t *get) {
  callback_ = NULL;
  return HippoDevice::unsubscribe(get);
}

const char *SButtonPressType_str[] = { "tap", "hold", };

uint64_t SButtons::button_press_notification_json2c(void *obj,
                                                    hippo::ButtonPress *touch) {
  uint64_t err = 0LL;
  if (obj == NULL || touch == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
  }
  const nl::json *touchData = reinterpret_cast<const nl::json*>(obj);
  if (!touchData->is_object()) {
    return MAKE_HIPPO_ERROR(facility_,
                            HIPPO_MESSAGE_ERROR);
  }
  try {
    const nl::json *id = &touchData->at("id");
    const nl::json *type = &touchData->at("type");
    if (!(id->is_string() && type->is_string())) {
      return MAKE_HIPPO_ERROR(facility_,
                              HIPPO_MESSAGE_ERROR);
    }
    int32_t idx;
    idx = str_to_idx(ButtonId_str, id->get<std::string>().c_str(),
                     static_cast<uint32_t>(hippo::ButtonId::left),
                     static_cast<uint32_t>(hippo::ButtonId::right));
    if (idx < 0) {
      err = MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
      goto clean_up;
    }
    touch->id = static_cast<hippo::ButtonId>(idx);

    idx = str_to_idx(SButtonPressType_str, type->get<std::string>().c_str(),
                     static_cast<uint32_t>(hippo::ButtonPressType::tap),
                     static_cast<uint32_t>(hippo::ButtonPressType::hold));
    if (idx < 0) {
      err = MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
      goto clean_up;
    }
    touch->type = static_cast<hippo::ButtonPressType>(idx);
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
clean_up:
  return err;
}

bool SButtons::HasRegisteredCallback() {
  return (NULL != callback_);
}

const char *SButtonsNotification_str[] = {
  "on_close", "on_device_connected", "on_device_disconnected",
  "on_factory_default", "on_open", "on_open_count", "on_resume", "on_suspend",
  "on_sohal_disconnected", "on_sohal_connected",
  "on_hold_threshold", "on_led_on_off_rate", "on_led_pulse_rate",
  "on_led_state", "on_button_press",
};

void SButtons::ProcessSignal(char *method, void *obj) {
  if (NULL == callback_) {
    return;
  }
  uint64_t err = 0LL;
  int32_t idx = 0;
  idx = str_to_idx(SButtonsNotification_str,
                   method,
                   static_cast<uint32_t>(
                       hippo::SButtonsNotification::on_close),
                   static_cast<uint32_t>(
                       hippo::SButtonsNotification::on_button_press));
  free(method);
  if (idx < 0) {
    return;
  }
  nl::json v, *params = reinterpret_cast<nl::json*>(obj);
  try {
    v = params->at(0);
  } catch (nl::json::exception) {     // out_of_range or type_error
    // do nothing
  }
  int val = 0;
  SButtonsNotificationParam param;
  param.type = static_cast<hippo::SButtonsNotification>(idx);

  switch (static_cast<hippo::SButtonsNotification>(idx)) {
    case SButtonsNotification::on_hold_threshold:
      param.on_hold_threshold = v.get<uint32_t>();
      break;

    case SButtonsNotification::on_led_on_off_rate:
      param.on_led_on_off_rate = v.get<uint32_t>();
      break;

    case SButtonsNotification::on_led_pulse_rate:
      param.on_led_pulse_rate = v.get<uint32_t>();
      break;

    case SButtonsNotification::on_led_state:
      err = button_led_state_notification_json2c(params, &param.on_led_state);
      break;

    case SButtonsNotification::on_button_press:
      err = button_press_notification_json2c(&v, &param.on_button_press);
      break;

    case SButtonsNotification::on_open_count:
      param.on_open_count = v.get<uint32_t>();
      break;

    default:
      break;
  }
  if (!err) {
    (*callback_)(param, callback_data_);
  }
}

}  // namespace hippo
