
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <mutex>   // NOLINT
#include "../include/projector.h"
#include "../include/json.hpp"

namespace nl = nlohmann;

namespace hippo {

extern std::mutex gHippoDeviceMutex;
const char devName[] = "projector";
extern const char *defaultHost;
extern uint32_t defaultPort;

Projector::Projector() :
    HippoDevice(devName, defaultHost, defaultPort, HIPPO_PROJECTOR, 0),
    callback_(NULL) {
}

Projector::Projector(uint32_t device_index) :
  HippoDevice(devName, defaultHost, defaultPort, HIPPO_PROJECTOR,
              device_index),
  callback_(NULL) {
}

Projector::Projector(const char *address, uint32_t port) :
    HippoDevice(devName, address, port, HIPPO_PROJECTOR, 0),
    callback_(NULL) {
}

Projector::Projector(const char *address, uint32_t port,
                     uint32_t device_index) :
  HippoDevice(devName, address, port, HIPPO_PROJECTOR, device_index),
  callback_(NULL) {
}

Projector::~Projector(void) {
}

uint64_t Projector::brightness(uint32_t set) {
  return uint32_set_get("brightness", set, NULL);
}

uint64_t Projector::brightness(uint32_t *get) {
  return uint32_get("brightness", get);
}

uint64_t Projector::brightness(uint32_t set, uint32_t *get) {
  return uint32_set_get("brightness", set, get);
}

uint64_t Projector::calibration_data(hippo::CalibrationData *get) {
  if (NULL == get) {
    return  MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  uint64_t err = HIPPO_OK;
  nl::json j;
  void *jptr = reinterpret_cast<void*>(&j);

  if (err = SendRawMsg("calibration_data", jptr)) {
    return err;
  }
  err = calibrationData_json2c(jptr, get);

  return err;
}

uint64_t Projector::device_specific_info(hippo::ProjectorSpecificInfo *get) {
  if (NULL == get) {
    return  MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  uint64_t err = HIPPO_OK;
  nl::json j;
  void *jptr = reinterpret_cast<void*>(&j);

  if (err = SendRawMsg("device_specific_info", jptr)) {
    return err;
  }
  err = projector_specific_info_json2c(jptr, get);

  return err;
}

uint64_t Projector::flash(bool set) {
  return bool_set_get("flash", set, NULL);
}

// the get returns the number of seconds left that the projector will
// remain in flash mode for
uint64_t Projector::flash(bool set, uint32_t *get) {
  if (NULL == get) {
    return flash(set);
  }
  uint64_t err = HIPPO_OK;
  nl::json jset;
  nl::json jget;
  jset.push_back(set);
  void *jsetptr = reinterpret_cast<void*>(&jset);
  void *jgetptr = reinterpret_cast<void*>(&jget);
  if (err = SendRawMsg("flash", jsetptr, jgetptr)) {
    return err;
  }
  if (get != NULL) {
    try {
      *get = reinterpret_cast<const nl::json*>(jgetptr)->get<uint32_t>();
    } catch (nl::json::exception e) {     // out_of_range or type_error
      return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
    }
  }
  return HIPPO_OK;
}

uint64_t Projector::grayscale() {
  nl::json emptyjson;
  void *jptr = reinterpret_cast<void*>(&emptyjson);
  return SendRawMsg("grayscale", jptr);
}

uint64_t Projector::hardware_info(HardwareInfo *get) {
  if (NULL == get) {
    return  MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  uint64_t err = HIPPO_OK;
  nl::json j;
  void *jptr = reinterpret_cast<void*>(&j);

  if (err = SendRawMsg("hardware_info", jptr)) {
    return err;
  }
  return hardwareInfo_json2c(jptr, get);
}

uint64_t Projector::keystone(const hippo::Keystone &set) {
  return keystone(set, NULL);
}

uint64_t Projector::keystone(hippo::Keystone *get) {
  if (NULL == get) {
    return  MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  uint64_t err = HIPPO_OK;
  nl::json jget;
  void *jgetptr = reinterpret_cast<void*>(&jget);
  if (err = SendRawMsg("keystone", jgetptr)) {
    return err;
  }
  return keystone_json2c(jgetptr, get);
}

uint64_t Projector::keystone(const hippo::Keystone &set, hippo::Keystone *get) {
  uint64_t err = HIPPO_OK;
  nl::json jset, jget;
  void *jsetptr = reinterpret_cast<void*>(&jset);
  if (err = keystone_c2json(set, jsetptr)) {
    return err;
  }

  void *jgetptr = reinterpret_cast<void*>(&jget);
  if (err = SendRawMsg("keystone", jsetptr, jgetptr)) {
    return err;
  }
  if (get != NULL) {
    err = keystone_json2c(jgetptr, get);
  }
  return err;
}

uint64_t Projector::led_times(hippo::ProjectorLedTimes *get) {
  if (NULL == get) {
    return  MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  uint64_t err = HIPPO_OK;
  nl::json jget;
  void *jgetptr = reinterpret_cast<void*>(&jget);
  if (err = SendRawMsg("led_times", jgetptr)) {
    return err;
  }
  return ledtimes_json2c(jgetptr, get);
}

uint64_t Projector::manufacturing_data(hippo::ManufacturingData *get) {
  if (NULL == get) {
    return  MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  uint64_t err = HIPPO_OK;
  nl::json jget;
  void *jgetptr = reinterpret_cast<void*>(&jget);
  if (err = SendRawMsg("manufacturing_data", jgetptr)) {
    return err;
  }
  return mfgData_json2c(jgetptr, get);
}

uint64_t Projector::monitor_coordinates(hippo::Rectangle *get) {
  if (NULL == get) {
    return  MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  uint64_t err = HIPPO_OK;
  nl::json jget;
  void *jgetptr = reinterpret_cast<void*>(&jget);
  if (err = SendRawMsg("monitor_coordinates", jgetptr)) {
    return err;
  }
  return rectangle_json2c(jgetptr, get);
}

uint64_t Projector::off(void) {
  nl::json jget;
  void *jgetptr = reinterpret_cast<void*>(&jget);
  return SendRawMsg("off", jgetptr);
}

uint64_t Projector::on(void) {
  nl::json jget;
  void *jgetptr = reinterpret_cast<void*>(&jget);
  return SendRawMsg("on", jgetptr);
}

uint64_t Projector::solid_color(hippo::SolidColor set) {
  return solid_color(set, NULL);
}

uint64_t Projector::solid_color(hippo::SolidColor *get) {
  if (NULL == get) {
    return  MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  uint64_t err = HIPPO_OK;
  nl::json jget;
  void *jgetptr = reinterpret_cast<void*>(&jget);
  if (err = SendRawMsg("solid_color", jgetptr)) {
    return err;
  }
  return solid_color_json2c(jgetptr, get);
}

uint64_t Projector::solid_color(hippo::SolidColor set, hippo::SolidColor *get) {
  uint64_t err = HIPPO_OK;
  nl::json jset, jget;
  void *jsetptr = reinterpret_cast<void*>(&jset);
  if (err = solid_color_c2json(set, jsetptr)) {
    return err;
  }

  void *jgetptr = reinterpret_cast<void*>(&jget);
  if (err = SendRawMsg("solid_color", jsetptr, jgetptr)) {
    return err;
  }
  if (get != NULL) {
    err = solid_color_json2c(jgetptr, get);
  }
  return err;
}

uint64_t Projector::state(hippo::ProjectorState *get) {
  if (NULL == get) {
    return  MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  uint64_t err = HIPPO_OK;
  nl::json jget;
  void *jgetptr = reinterpret_cast<void*>(&jget);
  if (err = SendRawMsg("state", jgetptr)) {
    return err;
  }
  return state_json2c(jgetptr, get);
}

uint64_t Projector::structured_light_mode(const bool set) {
  return bool_set_get("structured_light_mode", set, NULL);
}

uint64_t Projector::structured_light_mode(bool *get) {
  return bool_get("structured_light_mode", get);
}

uint64_t Projector::structured_light_mode(const bool set, bool *get) {
  return bool_set_get("structured_light_mode", set, get);
}

uint64_t Projector::subscribe(
    void (*callback)(const ProjectorNotificationParam &param, void *data),
    void *data) {
  return subscribe(callback, data, NULL);
}

uint64_t Projector::subscribe(
    void(*callback)(const ProjectorNotificationParam &param, void *data),
    void *data, uint32_t *get) {
  uint64_t err = HIPPO_OK;

  if (err = HippoDevice::subscribe_raw(data, get)) {
    return err;
  }
  callback_ = callback;

  return err;
}

uint64_t Projector::unsubscribe() {
  callback_ = NULL;
  return HippoDevice::unsubscribe();
}

uint64_t Projector::unsubscribe(uint32_t *get) {
  callback_ = NULL;
  return HippoDevice::unsubscribe(get);
}

uint64_t Projector::white_point(const hippo::WhitePoint &set) {
  return white_point(set, NULL);
}

uint64_t Projector::white_point(hippo::WhitePoint *get) {
  if (NULL == get) {
    return  MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  uint64_t err = HIPPO_OK;
  nl::json jget;
  void *jgetptr = reinterpret_cast<void*>(&jget);
  if (err = SendRawMsg("white_point", jgetptr)) {
    return err;
  }
  return white_point_json2c(jgetptr, get);
}

uint64_t Projector::white_point(const hippo::WhitePoint &set,
                                hippo::WhitePoint *get) {
  uint64_t err = HIPPO_OK;
  nl::json jset, jget;
  void *jsetptr = reinterpret_cast<void*>(&jset);
  if (err = white_point_c2json(set, jsetptr)) {
    return err;
  }
  void *jgetptr = reinterpret_cast<void*>(&jget);
  if (err = SendRawMsg("white_point", jsetptr, jgetptr)) {
    return err;
  }
  if (get != NULL) {
    err = white_point_json2c(jgetptr, get);
  }
  return err;
}


}   // namespace hippo
