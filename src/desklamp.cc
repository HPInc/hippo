
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <mutex>   // NOLINT

#include "../include/json.hpp"

#include "../include/desklamp.h"

namespace nl = nlohmann;

namespace hippo {

extern std::mutex gHippoDeviceMutex;
const char devName[] = "desklamp";
extern const char *defaultHost;
extern uint32_t defaultPort;

DeskLamp::DeskLamp() :
    HippoDevice(devName, defaultHost, defaultPort, HIPPO_DESKLAMP, 0) {
}

DeskLamp::DeskLamp(uint32_t device_index) :
  HippoDevice(devName, defaultHost, defaultPort, HIPPO_DESKLAMP,
              device_index) {
}

DeskLamp::DeskLamp(const char *address, uint32_t port) :
    HippoDevice(devName, address, port, HIPPO_DESKLAMP, 0) {
}

DeskLamp::DeskLamp(const char *address, uint32_t port,
                   uint32_t device_index) :
  HippoDevice(devName, address, port, HIPPO_DESKLAMP, device_index) {
}

DeskLamp::~DeskLamp(void) {
}

uint64_t DeskLamp::high(void) {
  nl::json j;
  void *jptr = reinterpret_cast<void*>(&j);
  return SendRawMsg("high", jptr);
}

uint64_t DeskLamp::low(void) {
  nl::json j;
  void *jptr = reinterpret_cast<void*>(&j);
  return SendRawMsg("low", jptr);
}

uint64_t DeskLamp::off(void) {
  nl::json j;
  void *jptr = reinterpret_cast<void*>(&j);
  return SendRawMsg("off", jptr);
}

uint64_t DeskLamp::state(DeskLampState *get) {
  if (NULL == get) {
    return  MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  uint64_t err = HIPPO_OK;
  nl::json j;
  void *jptr = reinterpret_cast<void*>(&j);

  if (err = SendRawMsg("state", jptr)) {
    return err;
  }
  return desklamp_state_json2c(jptr, get);
}

bool DeskLamp::HasRegisteredCallback() {
  return (NULL != callback_);
}

uint64_t DeskLamp::subscribe(
  void(*callback)(const DeskLampNotificationParam &param, void *data),
  void *data) {
  return subscribe(callback, data, NULL);
}

uint64_t DeskLamp::subscribe(
  void(*callback)(const DeskLampNotificationParam &param, void *data),
  void *data, uint32_t *get) {
  uint64_t err = 0LL;

  if (err = HippoDevice::subscribe_raw(data, get)) {
    return err;
  }
  callback_ = callback;

  return err;
}

uint64_t DeskLamp::unsubscribe() {
  callback_ = NULL;
  return HippoDevice::unsubscribe();
}

uint64_t DeskLamp::unsubscribe(uint32_t *get) {
  callback_ = NULL;
  return HippoDevice::unsubscribe(get);
}

uint64_t DeskLamp::desklamp_state_json2c(void *obj,
                                         hippo::DeskLampState *state) {
  const char *DeskLampState_str[] = { "off", "low", "high" };

  if (obj == NULL || state == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *jsonState = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!jsonState->is_string()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // get the individual items from json
    std::string stateStr = jsonState->get<std::string>();

    int32_t idx;
    idx = str_to_idx(DeskLampState_str, stateStr.c_str(),
                     static_cast<uint32_t>(hippo::DeskLampState::off),
                     static_cast<uint32_t>(hippo::DeskLampState::high));
    if (idx < 0) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    *state = static_cast<hippo::DeskLampState>(idx);
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return HIPPO_OK;
}

const char *DeskLampNotification_str[] = {
  "on_close", "on_device_connected", "on_device_disconnected",
  "on_factory_default", "on_open", "on_open_count", "on_resume", "on_suspend",
  "on_sohal_disconnected", "on_sohal_connected",
  "on_state",
};

void DeskLamp::ProcessSignal(char *method, void *obj) {
  if (NULL == callback_) {
    return;
  }
  uint64_t err = 0LL;
  int32_t idx = 0;
  idx = str_to_idx(DeskLampNotification_str,
                   method,
                   static_cast<uint32_t>(
                     hippo::DeskLampNotification::on_close),
                   static_cast<uint32_t>(
                     hippo::DeskLampNotification::on_state));
  free(method);
  if (idx < 0) {
    return;
  }
  nl::json v, *params = reinterpret_cast<nl::json*>(obj);
  try {
    v = params->at(0);
  }
  catch (nl::json::exception) {     // out_of_range or type_error
                                    // do nothing
  }
  int val = 0;
  DeskLampNotificationParam param;
  param.type = static_cast<hippo::DeskLampNotification>(idx);

  switch (static_cast<hippo::DeskLampNotification>(idx)) {
  case DeskLampNotification::on_state:
    err = desklamp_state_json2c(params, &param.on_state);
    break;

  case DeskLampNotification::on_open_count:
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
