
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <mutex>   // NOLINT
#include "../include/touchmat.h"
#include "../include/json.hpp"

namespace nl = nlohmann;

namespace hippo {

extern std::mutex gHippoDeviceMutex;
const char devName[] = "touchmat";
extern const char *defaultHost;
extern uint32_t defaultPort;

TouchMat::TouchMat() :
    HippoDevice(devName, defaultHost, defaultPort, HIPPO_TOUCHMAT, 0) {
}

TouchMat::TouchMat(uint32_t device_index) :
  HippoDevice(devName, defaultHost, defaultPort, HIPPO_TOUCHMAT,
              device_index),
  callback_(NULL) {
}

TouchMat::TouchMat(const char *address, uint32_t port) :
    HippoDevice(devName, address, port, HIPPO_TOUCHMAT, 0) {
}

TouchMat::TouchMat(const char *address, uint32_t port,
                   uint32_t device_index) :
  HippoDevice(devName, address, port, HIPPO_TOUCHMAT, device_index),
  callback_(NULL) {
}

TouchMat::~TouchMat(void) {
}

uint64_t TouchMat::active_area(ActiveArea *get) {
  if (NULL == get) {
    return  MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  uint64_t err = HIPPO_OK;
  nl::json j;
  void *jptr = reinterpret_cast<void*>(&j);

  if (err = SendRawMsg("active_area", jptr)) {
    return err;
  }
  return active_area_json2c(jptr, get);
}

uint64_t TouchMat::active_area(const ActiveArea &set) {
  return active_area(set, NULL);
}

uint64_t TouchMat::active_area(const ActiveArea &set, ActiveArea *get) {
  uint64_t err = HIPPO_OK;
  nl::json jset, jget;
  void *jsetptr = reinterpret_cast<void*>(&jset);
  if (err = active_area_c2json(set, jsetptr)) {
    return err;
  }

  void *jgetptr = reinterpret_cast<void*>(&jget);
  if (err = SendRawMsg("active_area", jsetptr, jgetptr)) {
    return err;
  }
  if (get != NULL) {
    err = active_area_json2c(jgetptr, get);
  }
  return err;
}

uint64_t TouchMat::active_pen_range(const ActivePenRange &set) {
  return active_pen_range(set, NULL);
}

uint64_t TouchMat::active_pen_range(ActivePenRange *get) {
  if (NULL == get) {
    return  MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  uint64_t err = HIPPO_OK;
  nl::json j;
  void *jptr = reinterpret_cast<void*>(&j);

  if (err = SendRawMsg("active_pen_range", jptr)) {
    return err;
  }
  return active_pen_range_json2c(jptr, get);
}

const char *ActivePenRange_str[] = {
  "five_mm", "ten_mm", "fifteen_mm", "twenty_mm",
};

uint64_t TouchMat::active_pen_range(const ActivePenRange &set,
                                    ActivePenRange *get) {
  uint64_t err = HIPPO_OK;
  nl::json jset, jget;
  void *jsetptr = reinterpret_cast<void*>(&jset);
  if (err = active_pen_range_c2json(set, jsetptr)) {
    return err;
  }

  void *jgetptr = reinterpret_cast<void*>(&jget);
  if (err = SendRawMsg("active_pen_range", jsetptr, jgetptr)) {
    return err;
  }
  if (get != NULL) {
    err = active_pen_range_json2c(jgetptr, get);
  }
  return err;
}

uint64_t TouchMat::calibrate() {
  nl::json emptyjson;
  void *jptr = reinterpret_cast<void*>(&emptyjson);
  return SendRawMsg("calibrate", jptr);
}

uint64_t TouchMat::device_palm_rejection(bool set) {
  return bool_set_get("device_palm_rejection", set, NULL);
}

uint64_t TouchMat::device_palm_rejection(bool *get) {
  return bool_get("device_palm_rejection", get);
}

uint64_t TouchMat::device_palm_rejection(bool set, bool *get) {
  return bool_set_get("device_palm_rejection", set, get);
}

uint64_t TouchMat::hardware_info(TouchmatHardwareInfo *get) {
  if (NULL == get) {
    return  MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  uint64_t err = HIPPO_OK;
  nl::json j;
  void *jptr = reinterpret_cast<void*>(&j);

  if (err = SendRawMsg("hardware_info", jptr)) {
    return err;
  }
  return hardware_info_json2c(jptr, get);
}

uint64_t TouchMat::palm_rejection_timeout(uint32_t *get) {
  return uint32_get("palm_rejection_timeout", get);
}

uint64_t TouchMat::palm_rejection_timeout(uint32_t set) {
  return uint32_set_get("palm_rejection_timeout", set, NULL);
}

uint64_t TouchMat::palm_rejection_timeout(uint32_t set, uint32_t *get) {
  return uint32_set_get("palm_rejection_timeout", set, get);
}

uint64_t TouchMat::reset() {
  nl::json emptyjson;
  void *jptr = reinterpret_cast<void*>(&emptyjson);
  return SendRawMsg("reset", jptr);
}

uint64_t TouchMat::state(hippo::TouchMatState *get) {
  if (NULL == get) {
    return  MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  uint64_t err = HIPPO_OK;
  nl::json j;
  void *jptr = reinterpret_cast<void*>(&j);

  if (err = SendRawMsg("state", jptr)) {
    return err;
  }
  return touchMatState_json2c(jptr, get);
}

uint64_t TouchMat::state(const hippo::TouchMatState &set) {
  return state(set, NULL);
}

uint64_t TouchMat::state(const hippo::TouchMatState &set,
                         hippo::TouchMatState *get) {
  uint64_t err = HIPPO_OK;
  nl::json jset, jget;
  void *jsetptr = reinterpret_cast<void*>(&jset);
  if (err = touchMatState_c2json(set, jsetptr)) {
    return err;
  }

  void *jgetptr = reinterpret_cast<void*>(&jget);
  if (err = SendRawMsg("state", jsetptr, jgetptr)) {
    return err;
  }
  if (get != NULL) {
    err = touchMatState_json2c(jgetptr, get);
  }
  return err;
}

uint64_t TouchMat::subscribe(
    void (*callback)(const TouchMatNotificationParam &param, void *data),
    void *data) {
  return subscribe(callback, data, NULL);
}

uint64_t TouchMat::subscribe(
    void(*callback)(const TouchMatNotificationParam &param, void *data),
    void *data, uint32_t *get) {
  uint64_t err = 0LL;

  if (err = HippoDevice::subscribe_raw(data, get)) {
    return err;
  }
  callback_ = callback;

  return err;
}

uint64_t TouchMat::unsubscribe() {
  callback_ = NULL;
  return HippoDevice::unsubscribe();
}

uint64_t TouchMat::unsubscribe(uint32_t *get) {
  callback_ = NULL;
  return HippoDevice::unsubscribe(get);
}

uint64_t TouchMat::touchMatState_json2c(void *obj, TouchMatState *state) {
  if (obj == NULL || state == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *jsonTMstate = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!jsonTMstate->is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // get the individual items from json
    auto touch = jsonTMstate->at("touch");
    auto active_pen = jsonTMstate->at("active_pen");

    if (!active_pen.is_boolean() || !touch.is_boolean()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    state->active_pen = active_pen.get<bool>() ? true : false;
    state->touch = touch.get<bool>() ? true : false;
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return HIPPO_OK;
}

uint64_t TouchMat::touchMatState_c2json(const hippo::TouchMatState &state,
                                        void *obj) {
  if (obj == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }

  nl::json params = {
    { "touch", state.touch ? true : false },
    { "active_pen", state.active_pen ? true : false}
  };
  reinterpret_cast<nl::json*>(obj)->push_back(params);
  return HIPPO_OK;
}

uint64_t TouchMat::active_area_json2c(void *obj, ActiveArea *area) {
  if (obj == NULL || area == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *jsonActiveArea = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!jsonActiveArea->is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // get the individual items from json
    auto enabled = jsonActiveArea->at("enabled");
    auto tl = jsonActiveArea->at("top_left");
    auto br = jsonActiveArea->at("bottom_right");

    if (!tl.is_object() || !br.is_object() || !enabled.is_boolean()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    auto tlx = tl.at("x");
    auto tly = tl.at("y");
    auto brx = br.at("x");
    auto bry = br.at("y");

    if (!tlx.is_number_integer() || !tly.is_number_integer() ||
        !brx.is_number_integer() || !bry.is_number_integer()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    area->enabled = enabled.get<int>() ? true : false;
    area->top_left.x = tlx.get<uint32_t>();
    area->top_left.y = tly.get<uint32_t>();
    area->bottom_right.x = brx.get<uint32_t>();
    area->bottom_right.y = bry.get<uint32_t>();
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return HIPPO_OK;
}

uint64_t TouchMat::active_area_c2json(const hippo::ActiveArea &area,
                                      void *obj) {
  if (obj == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  nl::json params = {
    { "enabled", area.enabled ? true : false },
    { "start", {
        { "x", area.top_left.x },
        { "y", area.top_left.y }
      }
    },
    { "stop", {
        { "x", area.bottom_right.x },
        { "y", area.bottom_right.y },
      }
    }
  };

  reinterpret_cast<nl::json*>(obj)->push_back(params);
  return HIPPO_OK;
}

uint64_t TouchMat::active_pen_range_json2c(void *obj,
                                           hippo::ActivePenRange *range) {
  if (obj == NULL || range == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *jsonRange = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!jsonRange->is_string()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
    }
    // get the individual items from json
    std::string rangeStr = jsonRange->get<std::string>();

    int32_t idx;
    idx = str_to_idx(ActivePenRange_str, rangeStr.c_str(),
                     static_cast<uint32_t>(hippo::ActivePenRange::five_mm),
                     static_cast<uint32_t>(hippo::ActivePenRange::twenty_mm));
    if (idx < 0) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    *range = static_cast<hippo::ActivePenRange>(idx);
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return HIPPO_OK;
}

uint64_t TouchMat::active_pen_range_c2json(const hippo::ActivePenRange &range,
                                           void *obj) {
  uint64_t err = HIPPO_OK;
  if (obj == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  reinterpret_cast<nl::json*>(obj)->push_back(
      ActivePenRange_str[static_cast<uint32_t>(range)]);
  return HIPPO_OK;
}

uint64_t TouchMat::hardware_info_json2c(void *obj,
                                        hippo::TouchmatHardwareInfo *info) {
  if (obj == NULL || info == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *jsonInfo = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!jsonInfo->is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
    }
    auto jsonSize = jsonInfo->at("size");
    if (!jsonSize.is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
    }
    auto width = jsonSize.at("width");
    auto height = jsonSize.at("height");

    if (!width.is_number_float() || !height.is_number_float()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
    }

    // get the individual items from json
    info->size.width = width.get<float>();
    info->size.height = height.get<float>();
  }
  catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return HIPPO_OK;
}

// Notifications

bool TouchMat::HasRegisteredCallback() {
  return (NULL != callback_);
}

const char *TouchMatNotification_str[] = {
  "on_close", "on_device_connected", "on_device_disconnected",
  "on_factory_default", "on_open", "on_open_count", "on_resume", "on_suspend",
  "on_sohal_disconnected", "on_sohal_connected",
  "on_active_area", "on_active_pen_range",
  "on_calibrate", "on_device_palm_rejection", "on_palm_rejection_timeout",
  "on_reset", "on_state",
};

void TouchMat::ProcessSignal(char *method, void *obj) {
  if (NULL == callback_) {
    return;
  }
  uint64_t err = 0LL;
  int32_t idx = 0;
  idx = str_to_idx(TouchMatNotification_str,
                   method,
                   static_cast<uint32_t>(
                       hippo::TouchMatNotification::on_close),
                   static_cast<uint32_t>(
                       hippo::TouchMatNotification::on_state));
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
  TouchMatNotificationParam param;
  param.type = static_cast<hippo::TouchMatNotification>(idx);

  switch (static_cast<hippo::TouchMatNotification>(idx)) {
    case TouchMatNotification::on_active_area:
      err = active_area_json2c(obj, &param.on_active_area);
      break;

    case TouchMatNotification::on_active_pen_range:
      err = active_pen_range_json2c(obj, &param.on_active_pen_range);
      break;

    case TouchMatNotification::on_device_palm_rejection:
      param.on_device_palm_rejection = v.get<bool>();
      break;

    case TouchMatNotification::on_state:
      err = touchMatState_json2c(obj, &param.on_state);
      break;

    case TouchMatNotification::on_palm_rejection_timeout:
      param.on_palm_rejection_timeout = v.get<uint32_t>();
      break;

    case TouchMatNotification::on_open_count:
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
