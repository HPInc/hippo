
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#include <stdio.h>

#include <mutex>   // NOLINT

#include "../include/capturestage.h"
#include "../include/json.hpp"


namespace nl = nlohmann;

namespace hippo {

extern std::mutex gHippoDeviceMutex;
const char devName[] = "capturestage";
extern const char *defaultHost;
extern uint32_t defaultPort;

CaptureStage::CaptureStage() :
    HippoDevice(devName, defaultHost, defaultPort, HIPPO_CAPTURESTAGE, 0) {
}

CaptureStage::CaptureStage(uint32_t device_index) :
    HippoDevice(devName, defaultHost, defaultPort, HIPPO_CAPTURESTAGE,
                device_index),
    callback_(NULL) {
}

CaptureStage::CaptureStage(const char *address, uint32_t port) :
    HippoDevice(devName, address, port, HIPPO_CAPTURESTAGE, 0) {
}

CaptureStage::CaptureStage(const char *address, uint32_t port,
                           uint32_t device_index) :
    HippoDevice(devName, address, port, HIPPO_CAPTURESTAGE, device_index),
    callback_(NULL) {
}

CaptureStage::~CaptureStage(void) {
}

uint64_t CaptureStage::device_specific_info(CaptureStageSpecificInfo *get) {
  uint64_t err = HIPPO_OK;
  if (NULL == get) {
    MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  nl::json j;
  void *jptr = reinterpret_cast<void*>(&j);
  if (err = SendRawMsg("device_specific_info", jptr)) {
    return err;
  }
  return captureStageInfo_json2c(jptr, get);
}

void CaptureStage::free_capturestage_info(
    CaptureStageSpecificInfo *info_to_free) {
  free(info_to_free->port);
  info_to_free->port = nullptr;
}

uint64_t CaptureStage::home() {
  nl::json j;
  void *jptr = reinterpret_cast<void*>(&j);
  return SendRawMsg("home", jptr);
}

uint64_t CaptureStage::led_on_off_rate(const hippo::LedOnOffRate &set) {
  return led_on_off_rate(set, NULL);
}

uint64_t CaptureStage::led_on_off_rate(hippo::LedOnOffRate *get) {
  uint64_t err = HIPPO_OK;
  if (NULL == get) {
    MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  nl::json j;
  void *jptr = reinterpret_cast<void*>(&j);
  if (err = SendRawMsg("led_on_off_rate", jptr)) {
    return err;
  }
  return ledOnOffRate_json2c(jptr, get);
}

uint64_t CaptureStage::led_on_off_rate(const hippo::LedOnOffRate &set,
                                       hippo::LedOnOffRate *get) {
  uint64_t err = HIPPO_OK;
  nl::json jset;

  void *jset_ptr = reinterpret_cast<void*>(&jset);
  if (err = ledOnOffRate_c2json(set, jset_ptr)) {
    return err;
  }
  nl::json jget;
  void *jget_ptr = reinterpret_cast<void*>(&jget);
  if (err = SendRawMsg("led_on_off_rate", jset_ptr, jget_ptr)) {
    return err;
  }
  if (NULL != get) {
    if (err = ledOnOffRate_json2c(jget_ptr, get)) {
      return err;
    }
  }
  return err;
}

uint64_t CaptureStage::captureStageInfo_json2c(void *obj,
                                               CaptureStageSpecificInfo *info) {
  // test inputs to ensure non-null pointers
  if (obj == NULL || info == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
  }

  const nl::json *jsonInfo = reinterpret_cast<const nl::json*>(obj);
  if (!jsonInfo->is_object()) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
  }
  try {
    // get the rate data
    auto port = jsonInfo->at("port");
    info->port = strdup(port.get<std::string>().c_str());
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return HIPPO_OK;
}

uint64_t CaptureStage::ledOnOffRate_c2json(const hippo::LedOnOffRate &ledRate,
                                           void *obj) {
  // test inputs to ensure non-null pointers
  if (obj == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  nl::json params;
  params["time_off"] = ledRate.time_off;
  params["time_on"] = ledRate.time_on;
  // json['params'] is sent as a list containing the object
  reinterpret_cast<nl::json*>(obj)->push_back(params);

  return HIPPO_OK;
}

uint64_t CaptureStage::ledOnOffRate_json2c(void *obj,
                                           hippo::LedOnOffRate *ledRate) {
  // test inputs to ensure non-null pointers
  if (obj == NULL || ledRate == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
  }

  const nl::json *ledRateData = reinterpret_cast<const nl::json*>(obj);

  // convert from array if returned from the notification
  if (ledRateData->is_array()) {
    ledRateData = &ledRateData->at(0);
  }

  if (!ledRateData->is_object()) {
    return MAKE_HIPPO_ERROR(facility_,
                            HIPPO_MESSAGE_ERROR);
  }
  try {
    // get the rate data
    const nl::json *timeoff = &ledRateData->at("time_off");
    const nl::json *timeon = &ledRateData->at("time_on");
    // validate that the items are integers
    if (timeoff->is_number_integer() && timeon->is_number_integer()) {
      // get the values
      ledRate->time_off = timeoff->get<int>();
      ledRate->time_on = timeon->get<int>();
    } else {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return HIPPO_OK;
}

uint64_t CaptureStage::led_state(const hippo::LedState &set) {
  return led_state(set, NULL);
}

uint64_t CaptureStage::led_state(hippo::LedState *get) {
  uint64_t err = HIPPO_OK;
  if (NULL == get) {
    MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  nl::json j;
  void *jptr = reinterpret_cast<void*>(&j);
  if (err = SendRawMsg("led_state", jptr)) {
    return err;
  }
  if (err = ledState_json2c(jptr, get)) {
    return err;
  }
  return err;
}

uint64_t CaptureStage::led_state(const hippo::LedState &set,
                                 hippo::LedState *get) {
  uint64_t err = HIPPO_OK;
  nl::json jset;

  void *jset_ptr = reinterpret_cast<void*>(&jset);
  if (err = ledState_c2json(set, jset_ptr)) {
    return err;
  }
  nl::json jget;
  void *jget_ptr = reinterpret_cast<void*>(&jget);
  if (err = SendRawMsg("led_state", jset_ptr, jget_ptr)) {
    return err;
  }
  if (NULL != get) {
    if (err = ledState_json2c(jget_ptr, get)) {
      return err;
    }
  }
  return err;
}

const char *ledStateToString[
    static_cast<uint32_t>(
        hippo::LedStateEnum::led_blink_off_phase) + 1] = {
  "off", "on", "blink_in_phase", "blink_off_phase"
};

uint64_t CaptureStage::ledState_c2json(const hippo::LedState &ledSt,
                                       void *obj) {
  // test inputs to ensure non-null pointers
  if (obj == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  nl::json params;
  params["amber"] = ledStateToString[static_cast<uint32_t>(ledSt.amber)];
  params["red"] = ledStateToString[static_cast<uint32_t>(ledSt.red)];
  params["white"] = ledStateToString[static_cast<uint32_t>(ledSt.white)];
  // json['params'] is sent as a list containing the object
  reinterpret_cast<nl::json*>(obj)->push_back(params);

  return HIPPO_OK;
}

uint64_t CaptureStage::ledState_json2c(void *obj,
                                       hippo::LedState *state) {
  // test inputs to ensure non-null pointersg269
  if (obj == NULL || state == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
  }
  const int KEYS_LEN = 3;
  char *keys[KEYS_LEN] = {
    "amber", "red", "white"
  };
  char *ledStateStr[KEYS_LEN] = { NULL, NULL, NULL };
  const nl::json *stateData = reinterpret_cast<const nl::json*>(obj);

  // convert from array if returned from the notification
  if (stateData->is_array()) {
    stateData = &stateData->at(0);
  }

  if (!stateData->is_object()) {
    return MAKE_HIPPO_ERROR(facility_,
                            HIPPO_MESSAGE_ERROR);
  }

  // iterate over the returned array
  for (std::size_t i = 0; i < stateData->size(); i++) {
    try {
      // get the next item of state data
      const nl::json *tmp = &stateData->at(keys[i]);
      // validate that the next item is a string
      if (tmp->is_string()) {
        // get the string
        std::string statestr = tmp->get<std::string>();
        hippo::LedStateEnum currState;
        bool found = false;
        // compare it to the valid strings
        for (uint32_t j = static_cast<uint32_t>(hippo::LedStateEnum::led_off);
             j <= static_cast<uint32_t>(
                 hippo::LedStateEnum::led_blink_off_phase);
             j++) {
          // compare the strings
          if (!statestr.compare(ledStateToString[j])) {
            found = true;
            currState = static_cast<hippo::LedStateEnum>(j);
          }
        }
        // make sure we actually found the state
        if (!found) {
          return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
        }
        // now assign the state to the proper color
        switch (i) {
          case 0:
            state->amber = currState;
            break;
          case 1:
            state->red = currState;
            break;
          case 2:
            state->white = currState;
            break;
        }
      } else {
        return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
      }
    } catch (nl::json::exception) {     // out_of_range or type_error
      return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
    }
  }
  return HIPPO_OK;
}

uint64_t CaptureStage::rotate(float set) {
  return float_set_get("rotate", set, NULL);
}

uint64_t CaptureStage::rotate(float *get) {
  return float_get("rotate", get);
}

uint64_t CaptureStage::rotate(float set, float *get) {
  return float_set_get("rotate", set, get);
}

uint64_t CaptureStage::rotation_angle(float *get) {
  return float_get("rotation_angle", get);
}

uint64_t CaptureStage::subscribe(
    void (*callback)(const CaptureStageNotificationParam &param, void *data),
    void *data) {
  return subscribe(callback, data, NULL);
}

uint64_t CaptureStage::subscribe(
    void(*callback)(const CaptureStageNotificationParam &param, void *data),
    void *data, uint32_t *get) {
  uint64_t err = HIPPO_OK;

  if (err = HippoDevice::subscribe_raw(data, get)) {
    return err;
  }
  callback_ = callback;

  return err;
}

uint64_t CaptureStage::tilt(float set) {
  return float_set_get("tilt", set, NULL);
}

uint64_t CaptureStage::tilt(float *get) {
  return float_get("tilt", get);
}

uint64_t CaptureStage::tilt(float set, float *get) {
  return float_set_get("tilt", set, get);
}

uint64_t CaptureStage::unsubscribe() {
  callback_ = NULL;
  return HippoDevice::unsubscribe();
}

uint64_t CaptureStage::unsubscribe(uint32_t *get) {
  callback_ = NULL;
  return HippoDevice::unsubscribe(get);
}

// Notifications

bool CaptureStage::HasRegisteredCallback() {
  return (NULL != callback_);
}

const char *CaptureStageNotification_str[] = {
  "on_close", "on_device_connected", "on_device_disconnected",
  "on_factory_default", "on_open", "on_open_count", "on_resume", "on_suspend",
  "on_sohal_disconnected", "on_sohal_connected",
  "on_home", "on_led_on_off_rate", "on_led_state",
  "on_rotate", "on_tilt",
};

void CaptureStage::ProcessSignal(char *method, void *obj) {
  if (NULL == callback_) {
    return;
  }
  uint64_t err = HIPPO_OK;
  int32_t idx = 0;
  idx = str_to_idx(CaptureStageNotification_str,
                   method,
                   static_cast<uint32_t>(
                       hippo::CaptureStageNotification::on_close),
                   static_cast<uint32_t>(
                       hippo::CaptureStageNotification::on_tilt));
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
  float val = 0;
  CaptureStageNotificationParam param;
  param.type = static_cast<hippo::CaptureStageNotification>(idx);

  switch (static_cast<hippo::CaptureStageNotification>(idx)) {
    case CaptureStageNotification::on_led_on_off_rate:
      err = ledOnOffRate_json2c(obj, &param.on_off_rate);
      break;

    case CaptureStageNotification::on_led_state:
      err = ledState_json2c(obj, &param.on_led_state);
      break;

    case CaptureStageNotification::on_rotate:
      param.on_rotate = v.get<float>();
      break;

    case CaptureStageNotification::on_tilt:
      param.on_tilt = v.get<float>();
      break;

    case CaptureStageNotification::on_open_count:
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
