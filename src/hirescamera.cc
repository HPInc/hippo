
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <mutex>   // NOLINT

#include "../include/hirescamera.h"
#include "../include/json.hpp"

namespace nl = nlohmann;

namespace hippo {

extern std::mutex gHippoDeviceMutex;
const char devName[] = "hirescamera";
extern const char *defaultHost;
extern uint32_t defaultPort;

HiResCamera::HiResCamera() :
    HippoCamera(devName, defaultHost, defaultPort, HIPPO_HIRESCAMERA, 0),
    callback_(NULL) {
}

HiResCamera::HiResCamera(uint32_t device_index) :
    HippoCamera(devName, defaultHost, defaultPort, HIPPO_HIRESCAMERA,
                device_index),
    callback_(NULL) {
}

HiResCamera::HiResCamera(const char *address, uint32_t port) :
    HippoCamera(devName, address, port, HIPPO_HIRESCAMERA, 0),
    callback_(NULL) {
}

HiResCamera::HiResCamera(const char *address, uint32_t port,
                         uint32_t device_index) :
    HippoCamera(devName, address, port, HIPPO_HIRESCAMERA, device_index),
    callback_(NULL) {
}

HiResCamera::~HiResCamera(void) {
}

uint64_t HiResCamera::auto_exposure(bool set) {
  return bool_set_get("auto_exposure", set, NULL);
}

uint64_t HiResCamera::auto_exposure(bool *get) {
  return bool_get("auto_exposure", get);
}

uint64_t HiResCamera::auto_exposure(bool set, bool* get) {
  return bool_set_get("auto_exposure", set, get);
}

uint64_t HiResCamera::auto_gain(bool set) {
  return bool_set_get("auto_gain", set, NULL);
}

uint64_t HiResCamera::auto_gain(bool *get) {
  return bool_get("auto_gain", get);
}

uint64_t HiResCamera::auto_gain(bool set, bool* get) {
  return bool_set_get("auto_gain", set, get);
}

uint64_t HiResCamera::auto_white_balance(bool set) {
  return bool_set_get("auto_white_balance", set, NULL);
}

uint64_t HiResCamera::auto_white_balance(bool *get) {
  return bool_get("auto_white_balance", get);
}

uint64_t HiResCamera::auto_white_balance(bool set, bool *get) {
  return bool_set_get("auto_white_balance", set, get);
}

uint64_t HiResCamera::brightness(uint16_t set) {
  return  uint16_set_get("brightness", set, NULL);
}

uint64_t HiResCamera::brightness(uint16_t *get) {
  return uint16_get("brightness", get);
}

uint64_t HiResCamera::brightness(uint16_t set, uint16_t *get) {
  return uint16_set_get("brightness", set, get);
}

uint64_t HiResCamera::camera_index(uint32_t *get) {
  return uint32_get("camera_index", get);
}

uint64_t HiResCamera::camera_settings(const CameraSettings &set) {
  return camera_settings(set, NULL);
}

uint64_t HiResCamera::camera_settings(CameraSettings *get) {
  uint64_t err = 0LL;

  if (NULL == get) {
    MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  nl::json j;
  void *jptr = reinterpret_cast<void*>(&j);
  if (err = SendRawMsg("camera_settings", jptr)) {
    return err;
  }
  if (err = CameraSettings_json2c(jptr, get)) {
    return err;
  }
  return 0LL;
}

uint64_t HiResCamera::camera_settings(const CameraSettings &set,
                                      CameraSettings *get) {
  uint64_t err = 0LL;
  nl::json jset;

  void *jset_ptr = reinterpret_cast<void*>(&jset);
  if (err = CameraSettings_c2json(set, jset_ptr)) {
    return err;
  }
  nl::json jget;
  void *jget_ptr = reinterpret_cast<void*>(&jget);
  if (err = SendRawMsg("camera_settings", jset_ptr, jget_ptr)) {
    return err;
  }
  if (NULL != get) {
    if (err = CameraSettings_json2c(jget_ptr, get)) {
      return err;
    }
  }
  return 0LL;
}

uint64_t HiResCamera::contrast(uint16_t set) {
  return  uint16_set_get("contrast", set, NULL);
}

uint64_t HiResCamera::contrast(uint16_t *get) {
  return uint16_get("contrast", get);
}

uint64_t HiResCamera::contrast(uint16_t set, uint16_t *get) {
  return uint16_set_get("contrast", set, get);
}

uint64_t HiResCamera::default_config(CameraMode mode, CameraConfig *get) {
  if (NULL == get) {
    return  MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  uint64_t err = HIPPO_OK;
  nl::json jset, jget;
  void *jsetptr = reinterpret_cast<void*>(&jset);
  void *jgetptr = reinterpret_cast<void*>(&jget);
  // add the mode name to the parameters
  const char *mode_str[] = { "4416x3312", "2208x1656", "1104x828" };
  jset.push_back(mode_str[static_cast<uint32_t>(mode)]);
  if (err = SendRawMsg("default_config", jsetptr, jgetptr)) {
    return err;
  }
  return CameraConfig_json2c(jgetptr, get);
}

uint64_t HiResCamera::device_status(CameraDeviceStatus *get) {
  uint64_t err = HIPPO_OK;
  if (NULL == get) {
    MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  nl::json j;
  void *jptr = reinterpret_cast<void*>(&j);
  if (err = SendRawMsg("device_status", jptr)) {
    return err;
  }
  if (err = CameraDeviceStatus_json2c(jptr, get)) {
    return err;
  }
  return err;
}

uint64_t HiResCamera::exposure(uint16_t set) {
  return  uint16_set_get("exposure", set, NULL);
}

uint64_t HiResCamera::exposure(uint16_t *get) {
  return uint16_get("exposure", get);
}

uint64_t HiResCamera::exposure(uint16_t set, uint16_t *get) {
  return uint16_set_get("exposure", set, get);
}

uint64_t HiResCamera::flip_frame(bool set) {
  return bool_set_get("flip_frame", set, NULL);
}

uint64_t HiResCamera::flip_frame(bool *get) {
  return bool_get("flip_frame", get);
}

uint64_t HiResCamera::flip_frame(bool set, bool *get) {
  return bool_set_get("flip_frame", set, get);
}

uint64_t HiResCamera::gain(uint16_t set) {
  return uint16_set_get("gain", set, NULL);
}

uint64_t HiResCamera::gain(uint16_t *get) {
  return uint16_get("gain", get);
}

uint64_t HiResCamera::gain(uint16_t set, uint16_t *get) {
  return uint16_set_get("gain", set, get);
}

uint64_t HiResCamera::gamma_correction(bool set) {
  return bool_set_get("gamma_correction", set, NULL);
}

uint64_t HiResCamera::gamma_correction(bool *get) {
  return bool_get("gamma_correction", get);
}

uint64_t HiResCamera::gamma_correction(bool set, bool *get) {
  return bool_set_get("gamma_correction", set, get);
}

uint64_t HiResCamera::keystone(CameraKeystone *get) {
  uint64_t err = HIPPO_OK;
  if (NULL == get) {
    MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  nl::json j;
  void *jptr = reinterpret_cast<void*>(&j);
  if (err = SendRawMsg("keystone", jptr)) {
    return err;
  }
  if (err = CameraKeystone_json2c(jptr, get)) {
    return err;
  }
  return err;
}

uint64_t HiResCamera::keystone(const CameraKeystone &set) {
  return keystone(set, NULL);
}

uint64_t HiResCamera::keystone(const CameraKeystone &set, CameraKeystone *get) {
  uint64_t err = HIPPO_OK;
  nl::json jset, jget;
  void *jsetptr = reinterpret_cast<void*>(&jset);
  if (err = CameraKeystone_c2json(set, jsetptr)) {
    return err;
  }
  void *jgetptr = reinterpret_cast<void*>(&jget);
  if (err = SendRawMsg("keystone", jsetptr, jgetptr)) {
    return err;
  }
  if (get != NULL) {
    err = CameraKeystone_json2c(jgetptr, get);
  }
  return err;
}

uint64_t HiResCamera::keystone_table(CameraKeystoneTable *get) {
  uint64_t err = HIPPO_OK;
  if (NULL == get) {
    MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  nl::json j;
  void *jptr = reinterpret_cast<void*>(&j);
  if (err = SendRawMsg("keystone_table", jptr)) {
    return err;
  }
  if (err = CameraKeystoneTable_json2c(jptr, get)) {
    return err;
  }
  return err;
}

uint64_t HiResCamera::keystone_table(const CameraKeystoneTable &set) {
  return keystone_table(set, NULL);
}

uint64_t HiResCamera::keystone_table(const CameraKeystoneTable &set,
                                     CameraKeystoneTable *get) {
  uint64_t err = HIPPO_OK;
  nl::json jset, jget;
  void *jsetptr = reinterpret_cast<void*>(&jset);
  if (err = CameraKeystoneTable_c2json(set, jsetptr)) {
    return err;
  }
  void *jgetptr = reinterpret_cast<void*>(&jget);
  if (err = SendRawMsg("keystone_table", jsetptr, jgetptr)) {
    return err;
  }
  if (get != NULL) {
    err = CameraKeystoneTable_json2c(jgetptr, get);
  }
  return err;
}

uint64_t HiResCamera::keystone_table_entries(const CameraKeystoneTable &param,
                                             CameraKeystoneTableEntries *get,
                                             uint32_t *num_entries) {
  uint64_t err = HIPPO_OK;
  nl::json jset, jget;
  void *jsetptr = reinterpret_cast<void*>(&jset);
  if (err = CameraKeystoneTable_c2json(param, jsetptr)) {
    return err;
  }
  void *jgetptr = reinterpret_cast<void*>(&jget);
  if (err = SendRawMsg("keystone_table_entries", jsetptr, jgetptr)) {
    return err;
  }
  if (get != NULL) {
    err = CameraKeystoneTableEntries_json2c(jgetptr, get, num_entries);
  }
  return err;
}

uint64_t HiResCamera::keystone_table_entries(const CameraKeystoneTable &table,
                                             CameraResolution *resoution_list,
                                             uint32_t num_resolutions,
                                             CameraKeystoneTableEntries *get,
                                             uint32_t *num_entries) {
  uint64_t err = HIPPO_OK;
  nl::json jset, jget;
  void *jsetptr = reinterpret_cast<void*>(&jset);

  if (err = CameraKeystoneTableAndResolution_c2json(table,
                                                    resoution_list,
                                                    num_resolutions,
                                                    jsetptr)) {
    return err;
  }
  void *jgetptr = reinterpret_cast<void*>(&jget);
  if (err = SendRawMsg("keystone_table_entries", jsetptr, jgetptr)) {
    return err;
  }
  if (get != NULL) {
    err = CameraKeystoneTableEntries_json2c(jgetptr, get, num_entries);
  }
  return err;
}

// set
uint64_t HiResCamera::keystone_table_entries(
                                       const CameraKeystoneTableEntries &set,
                                       const uint32_t &num_entries) {
  return keystone_table_entries(set, num_entries, NULL, NULL);
}

// set & get
uint64_t HiResCamera::keystone_table_entries(
                                const CameraKeystoneTableEntries &set,
                                const uint32_t &num_set_entries,
                                CameraKeystoneTableEntries *get,
                                uint32_t *num_get_entries) {
  uint64_t err = HIPPO_OK;
  nl::json jset, jget;
  void *jsetptr = reinterpret_cast<void*>(&jset);
  if (err = CameraKeystoneTableEntries_c2json(set, num_set_entries, jsetptr)) {
    return err;
  }
  void *jgetptr = reinterpret_cast<void*>(&jget);
  if (err = SendRawMsg("keystone_table_entries", jsetptr, jgetptr)) {
    return err;
  }
  if (get != NULL) {
    err = CameraKeystoneTableEntries_json2c(jgetptr, get, num_get_entries);
  }
  return err;
}

void HiResCamera::free_keystone_table_entries(
                                           CameraKeystoneTableEntries *entries,
                                           uint32_t num_entries) {
  if (entries && entries->entries) {
    free(entries->entries);
    entries->entries = NULL;
  }
}

uint64_t HiResCamera::led_state(CameraLedState *get) {
  uint64_t err = HIPPO_OK;
  if (NULL == get) {
    MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  nl::json j;
  void *jptr = reinterpret_cast<void*>(&j);
  if (err = SendRawMsg("led_state", jptr)) {
    return err;
  }
  if (err = CameraLedState_json2c(jptr, get)) {
    return err;
  }
  return err;
}

uint64_t HiResCamera::led_state(const CameraLedState &set) {
  return led_state(set, NULL);
}
uint64_t HiResCamera::led_state(const CameraLedState &set,
                                CameraLedState *get) {
  uint64_t err = HIPPO_OK;
  nl::json jset, jget;
  void *jsetptr = reinterpret_cast<void*>(&jset);
  if (err = CameraLedState_c2json(set, jsetptr)) {
    return err;
  }
  void *jgetptr = reinterpret_cast<void*>(&jget);
  if (err = SendRawMsg("led_state", jsetptr, jgetptr)) {
    return err;
  }
  if (get != NULL) {
    err = CameraLedState_json2c(jgetptr, get);
  }
  return err;
}


uint64_t HiResCamera::lens_color_shading(bool set) {
  return bool_set_get("lens_color_shading", set, NULL);
}

uint64_t HiResCamera::lens_color_shading(bool *get) {
  return bool_get("lens_color_shading", get);
}

uint64_t HiResCamera::lens_color_shading(bool set, bool *get) {
  return bool_set_get("lens_color_shading", set, get);
}

uint64_t HiResCamera::lens_shading(bool set) {
  return bool_set_get("lens_shading", set, NULL);
}

uint64_t HiResCamera::lens_shading(bool *get) {
  return bool_get("lens_shading", get);
}

uint64_t HiResCamera::lens_shading(bool set, bool *get) {
  return bool_set_get("lens_shading", set, get);
}

uint64_t HiResCamera::mirror_frame(bool set) {
  return bool_set_get("mirror_frame", set, NULL);
}

uint64_t HiResCamera::mirror_frame(bool *get) {
  return bool_get("mirror_frame", get);
}

uint64_t HiResCamera::mirror_frame(bool set, bool *get) {
  return bool_set_get("mirror_frame", set, get);
}

uint64_t HiResCamera::parent_resolution(CameraResolution *get) {
  uint64_t err = HIPPO_OK;
  if (NULL == get) {
    MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  nl::json j;
  void *jptr = reinterpret_cast<void*>(&j);
  if (err = SendRawMsg("parent_resolution", jptr)) {
    return err;
  }
  if (err = Resolution_json2c(jptr, get)) {
    return err;
  }
  return err;
}

uint64_t HiResCamera::parent_resolution(const CameraResolution &provided,
                                        CameraResolution *get) {
  uint64_t err = HIPPO_OK;
  nl::json jset, jget;
  void *jsetptr = reinterpret_cast<void*>(&jset);
  if (err = Resolution_c2json(provided, jsetptr)) {
    return err;
  }
  void *jgetptr = reinterpret_cast<void*>(&jget);
  if (err = SendRawMsg("parent_resolution", jsetptr, jgetptr)) {
    return err;
  }
  if (get != NULL) {
    err = Resolution_json2c(jgetptr, get);
  }
  return err;
}

uint64_t HiResCamera::power_line_frequency(PowerLineFrequency *get) {
  uint64_t err = HIPPO_OK;
  if (NULL == get) {
    MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  nl::json j;
  void *jptr = reinterpret_cast<void*>(&j);
  if (err = SendRawMsg("power_line_frequency", jptr)) {
    return err;
  }
  if (err = PowerLineFrequency_json2c(jptr, get)) {
    return err;
  }
  return err;
}

uint64_t HiResCamera::power_line_frequency(const PowerLineFrequency &set) {
  return power_line_frequency(set, NULL);
}
uint64_t HiResCamera::power_line_frequency(const PowerLineFrequency &set,
                                           PowerLineFrequency *get) {
  uint64_t err = HIPPO_OK;
  nl::json jset, jget;
  void *jsetptr = reinterpret_cast<void*>(&jset);
  if (err = PowerLineFrequency_c2json(&set, jsetptr)) {
    return err;
  }
  void *jgetptr = reinterpret_cast<void*>(&jget);
  if (err = SendRawMsg("power_line_frequency", jsetptr, jgetptr)) {
    return err;
  }
  if (get != NULL) {
    err = PowerLineFrequency_json2c(jgetptr, get);
  }
  return err;
}

uint64_t HiResCamera::reset() {
  nl::json emptyjson;
  void *jptr = reinterpret_cast<void*>(&emptyjson);
  return SendRawMsg("reset", jptr);
}

uint64_t HiResCamera::saturation(uint16_t set) {
  return uint16_set_get("saturation", set, NULL);
}

uint64_t HiResCamera::saturation(uint16_t *get) {
  return uint16_get("saturation", get);
}

uint64_t HiResCamera::saturation(uint16_t set, uint16_t *get) {
  return uint16_set_get("saturation", set, get);
}

uint64_t HiResCamera::sharpness(uint16_t set) {
  return uint16_set_get("sharpness", set, NULL);
}

uint64_t HiResCamera::sharpness(uint16_t *get) {
  return uint16_get("sharpness", get);
}

uint64_t HiResCamera::sharpness(uint16_t set, uint16_t *get) {
  return uint16_set_get("sharpness", set, get);
}

uint64_t HiResCamera::streaming_resolution(CameraResolution *get) {
  uint64_t err = HIPPO_OK;
  if (NULL == get) {
    MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  nl::json j;
  void *jptr = reinterpret_cast<void*>(&j);
  if (err = SendRawMsg("streaming_resolution", jptr)) {
    return err;
  }
  if (err = Resolution_json2c(jptr, get)) {
    return err;
  }
  return err;
}

uint64_t HiResCamera::strobe(const Strobe &set) {
  uint64_t err = HIPPO_OK;
  nl::json jset, jget;
  void *jsetptr = reinterpret_cast<void*>(&jset);
  if (err = Strobe_c2json(set, jsetptr)) {
    return err;
  }
  void *jgetptr = reinterpret_cast<void*>(&jget);
  return SendRawMsg("strobe", jsetptr, jgetptr);
}

uint64_t HiResCamera::subscribe(
    void (*callback)(const HiResCameraNotificationParam &param, void *data),
    void *data) {
  return subscribe(callback, data, NULL);
}

uint64_t HiResCamera::subscribe(
    void (*callback)(const HiResCameraNotificationParam &param, void *data),
    void *data, uint32_t *get) {
  uint64_t err = 0LL;

  if (err = HippoDevice::subscribe_raw(data, get)) {
    return err;
  }
  callback_ = callback;

  return err;
}

uint64_t HiResCamera::unsubscribe() {
  callback_ = NULL;
  return HippoDevice::unsubscribe();
}

uint64_t HiResCamera::unsubscribe(uint32_t *get) {
  callback_ = NULL;
  return HippoDevice::unsubscribe(get);
}

uint64_t HiResCamera::white_balance(const hippo::Rgb &set) {
  return white_balance(set, NULL);
}

uint64_t HiResCamera::white_balance(hippo::Rgb *get) {
  uint64_t err = HIPPO_OK;
  if (NULL == get) {
    MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  nl::json j;
  void *jptr = reinterpret_cast<void*>(&j);
  if (err = SendRawMsg("white_balance", jptr)) {
    return err;
  }
  if (err = WhiteBalance_json2c(jptr, get)) {
    return err;
  }
  return err;
}

uint64_t HiResCamera::white_balance(const hippo::Rgb &set,
                                    hippo::Rgb *get) {
  uint64_t err = HIPPO_OK;
  nl::json jset, jget;
  void *jsetptr = reinterpret_cast<void*>(&jset);
  if (err = WhiteBalance_c2json(set, jsetptr)) {
    return err;
  }
  void *jgetptr = reinterpret_cast<void*>(&jget);
  if (err = SendRawMsg("white_balance", jsetptr, jgetptr)) {
    return err;
  }
  if (get != NULL) {
    err = WhiteBalance_json2c(jgetptr, get);
  }
  return err;
}

uint64_t HiResCamera::white_balance_temperature(uint16_t set) {
  return  uint16_set_get("white_balance_temperature", set, NULL);
}

uint64_t HiResCamera::white_balance_temperature(uint16_t *get) {
  return uint16_get("white_balance_temperature", get);
}

uint64_t HiResCamera::white_balance_temperature(uint16_t set, uint16_t *get) {
  return uint16_set_get("white_balance_temperature", set, get);
}

/////////////////

uint64_t HiResCamera::CameraConfig_json2c(const void *obj, CameraConfig *cf) {
  uint64_t err = HIPPO_OK;
  if (obj == NULL || cf == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *configData = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!configData->is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    auto jsonExp = configData->at("exposure");
    auto jsonFps = configData->at("fps");
    auto jsonGain = configData->at("gain");
    auto jsonMode = configData->at("mode");
    auto jsonWB = configData->at("white_balance");

    // validate that the json values are the expected type
    if (!jsonExp.is_number_integer() ||
        !jsonFps.is_number_integer() ||
        !jsonGain.is_number_integer() ||
        !jsonMode.is_string() ||
        !jsonWB.is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    cf->exposure = jsonExp.get<uint32_t>();
    cf->fps = jsonFps.get<uint16_t>();
    cf->gain = jsonGain.get<uint16_t>();
    std::string modeStr = jsonMode.get<std::string>();
    if (!strncmp(modeStr.c_str(), "44", 2)) {
      cf->mode = CameraMode::MODE_4416x3312;
    } else if (!strncmp(modeStr.c_str(), "22", 2)) {
      cf->mode = CameraMode::MODE_2208x1656;
    } else if (!strncmp(modeStr.c_str(), "11", 2)) {
      cf->mode = CameraMode::MODE_1104x828;
    } else {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    err = WhiteBalance_json2c(reinterpret_cast<void*>(&jsonWB),
                              &cf->white_balance);
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return err;
}

uint64_t HiResCamera::CameraQuadrilateral_json2c(const void *obj,
                                                 CameraQuadrilateral *get) {
  uint64_t err = HIPPO_OK;
  if (obj == NULL || get == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *quadData = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!quadData->is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    auto tl = quadData->at("top_left");
    auto tr = quadData->at("top_right");
    auto bl = quadData->at("bottom_left");
    auto br = quadData->at("bottom_right");

    // validate that values are the expected type
    if (!tl.is_object() ||
        !tr.is_object() ||
        !bl.is_object() ||
        !br.is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // convert the points
    if ((err = ParsePointJson(&tl, &(get->top_left))) != HIPPO_OK) {
      return err;
    }
    if ((err = ParsePointJson(&tr, &(get->top_right))) != HIPPO_OK) {
      return err;
    }
    if ((err = ParsePointJson(&bl, &(get->bottom_left))) != HIPPO_OK) {
      return err;
    }
    if ((err = ParsePointJson(&br, &(get->bottom_right))) != HIPPO_OK) {
      return err;
    }
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return err;
}

uint64_t HiResCamera::CameraKeystone_c2json(const hippo::CameraKeystone &set,
                                            void *obj) {
  if (obj == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  nl::json params = {
    { "enabled", set.enabled },
    { "value", {
        { "bottom_left", {
            { "x", set.value.bottom_left.x },
            { "y", set.value.bottom_left.y },
          }
        },
        { "bottom_right", {
            { "x", set.value.bottom_right.x },
            { "y", set.value.bottom_right.y },
          }
        },
        { "top_left", {
            { "x", set.value.top_left.x },
            { "y", set.value.top_left.y },
          }
        },
        { "top_right", {
            { "x", set.value.top_right.x },
            { "y", set.value.top_right.y },
          }
        },
      },
    },
  };
  reinterpret_cast<nl::json*>(obj)->push_back(params);
  return HIPPO_OK;
}

uint64_t HiResCamera::CameraKeystone_json2c(const void *obj,
                                            hippo::CameraKeystone *get) {
  uint64_t err = HIPPO_OK;
  if (obj == NULL || get == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *keystoneData = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!keystoneData->is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    auto jsonEnabled = keystoneData->at("enabled");
    auto jsonValue = keystoneData->at("value");
    // validate that values are the expected type
    if (!jsonEnabled.is_boolean() ||
        !jsonValue.is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // convert the bool
    get->enabled = jsonEnabled.get<bool>();
    // convert the camera quadrilaterals
    if (err = CameraQuadrilateral_json2c(&jsonValue, &(get->value))) {
      return err;
    }
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return err;
}

const char *HiResCameraKeystoneTable_str[] = {
  "ram", "default", "flash_max_fov", "flash_fit_to_mat",
};

uint64_t HiResCamera::CameraKeystoneTable_c2json(
    const hippo::CameraKeystoneTable &set,
    void *obj) {
  if (obj == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  nl::json params = HiResCameraKeystoneTable_str[static_cast<uint32_t>(set)];
  reinterpret_cast<nl::json*>(obj)->push_back(params);
  return HIPPO_OK;
}

uint64_t HiResCamera::CameraKeystoneTableAndResolution_c2json(
                                const hippo::CameraKeystoneTable &table,
                                const hippo::CameraResolution *resolution_list,
                                const uint32_t &num_resolutions,
                                void *obj) {
  uint64_t err;
  if (obj == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  if (err = CameraKeystoneTable_c2json(table, obj)) {
    return err;
  }
  nl::json resolutionList;

  for (uint32_t i = 0; i < num_resolutions; i++) {
    nl::json currResolution = {
      { "height", resolution_list[i].height },
      { "width", resolution_list[i].width },
      { "fps", resolution_list[i].fps },
    };


    resolutionList.push_back(currResolution);
  }
  reinterpret_cast<nl::json*>(obj)->push_back(resolutionList);
  return HIPPO_OK;
}

uint64_t HiResCamera::CameraKeystoneTable_json2c(
    const void *obj,
    hippo::CameraKeystoneTable *get) {
  uint64_t err = HIPPO_OK;
  if (obj == NULL || get == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *keystoneTableData = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!keystoneTableData->is_string()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // convert the type
    int32_t idx;
    std::string ksTableData = keystoneTableData->get<std::string>();
    idx = str_to_idx(HiResCameraKeystoneTable_str,
                     ksTableData.c_str(),
                     static_cast<uint32_t>(
                       hippo::CameraKeystoneTable::RAM),
                     static_cast<uint32_t>(
                       hippo::CameraKeystoneTable::FLASH_FIT_TO_MAT));
    if (idx < 0) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    *get = static_cast<hippo::CameraKeystoneTable>(idx);
  }
  catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return err;
}

uint64_t HiResCamera::CameraKeystoneTableEntry_c2json(
    const hippo::CameraKeystoneTableEntry &set,
    void *obj) {
  if (obj == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  nl::json params = {
    { "enabled", set.enabled },
    { "value", {
        { "bottom_left", {
            { "x", set.value.bottom_left.x },
            { "y", set.value.bottom_left.y },
          }
        },
        { "bottom_right", {
            { "x", set.value.bottom_right.x },
            { "y", set.value.bottom_right.y },
          }
        },
        { "top_left", {
            { "x", set.value.top_left.x },
            { "y", set.value.top_left.y },
          }
        },
        { "top_right", {
            { "x", set.value.top_right.x },
            { "y", set.value.top_right.y },
          }
        },
      },
    },
    { "resolution", {
        {"height", set.resolution.height},
        {"width", set.resolution.width},
        {"fps", set.resolution.fps}
      }
    }
  };
  reinterpret_cast<nl::json*>(obj)->push_back(params);
  return HIPPO_OK;
}

uint64_t HiResCamera::CameraKeystoneTableEntry_json2c(
    const void *obj,
    hippo::CameraKeystoneTableEntry *get) {
  uint64_t err = HIPPO_OK;
  if (obj == NULL || get == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *keystoneData = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!keystoneData->is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    auto jsonEnabled = keystoneData->at("enabled");
    auto jsonValue = keystoneData->at("value");
    auto jsonResolution = keystoneData->at("resolution");
    // validate that values are the expected type
    if (!jsonEnabled.is_boolean() ||
        !jsonValue.is_object() ||
        !jsonResolution.is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // convert the bool
    get->enabled = jsonEnabled.get<bool>();
    // convert the resoution
    if (err = Resolution_json2c(&jsonResolution, &(get->resolution))) {
      return err;
    }
    // convert the camera quadrilaterals
    if (err = CameraQuadrilateral_json2c(&jsonValue, &(get->value))) {
      return err;
    }
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return err;
}

uint64_t HiResCamera::CameraKeystoneTableEntries_c2json(
                                        const CameraKeystoneTableEntries &set,
                                        const uint32_t num_entries,
                                        void *obj) {
  uint64_t err;
  if (obj == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }

  if (err = CameraKeystoneTable_c2json(set.type, obj)) {
    return err;
  }

  nl::json ksEntries;

  for (uint32_t i = 0; i < num_entries; i++) {
    nl::json currEntry = {
      { "enabled", set.entries[i].enabled },
      { "value", {
          { "bottom_left", {
              { "x", set.entries[i].value.bottom_left.x },
              { "y", set.entries[i].value.bottom_left.y },
            }
          },
          { "bottom_right", {
              { "x", set.entries[i].value.bottom_right.x },
              { "y", set.entries[i].value.bottom_right.y },
            }
          },
          { "top_left", {
              { "x", set.entries[i].value.top_left.x },
              { "y", set.entries[i].value.top_left.y },
            }
          },
          { "top_right", {
              { "x", set.entries[i].value.top_right.x },
              { "y", set.entries[i].value.top_right.y },
            }
          },
        },
      },
      { "resolution", {
          { "height", set.entries[i].resolution.height },
          { "width", set.entries[i].resolution.width },
          { "fps", set.entries[i].resolution.fps }
        }
      }
    };

    ksEntries.push_back(currEntry);
  }
  reinterpret_cast<nl::json*>(obj)->push_back(ksEntries);
  return HIPPO_OK;
}

uint64_t HiResCamera::CameraKeystoneTableEntries_json2c(const void *obj,
                                           CameraKeystoneTableEntries *get,
                                           uint32_t *num_entries) {
  uint64_t err = HIPPO_OK;
  if (obj == NULL || get == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }

  // initialize the values
  get->entries = NULL;
  *num_entries = 0;

  const nl::json *keystoneEntries = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!keystoneEntries->is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    auto jsonType = keystoneEntries->at("type");
    auto jsonEntries = keystoneEntries->at("entries");
    // validate that values are the expected type
    if (!jsonType.is_string() ||
        !jsonEntries.is_array()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    // fill the type
    if (err = CameraKeystoneTable_json2c(&jsonType, &get->type)) {
      return err;
    }

    // now get the number of keystone entries
    uint64_t num_items = jsonEntries.size();
    if (num_items <= 0) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    // allocate the memory to store the CameraKeystoneTableEntries
    get->entries = reinterpret_cast<CameraKeystoneTableEntry*>(
      calloc(num_items, sizeof(CameraKeystoneTableEntry)));

    for (int i = 0; i < num_items; i++) {
      if (err = CameraKeystoneTableEntry_json2c(&jsonEntries.at(i),
                                                &get->entries[i])) {
        return err;
      }
      *num_entries = i + 1;
    }
  }
  catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return err;
}


uint64_t HiResCamera::ParsePointJson(void *jsonPoint, Point *cPoint) {
  if (jsonPoint == NULL || cPoint == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *jPoint = reinterpret_cast<const nl::json*>(jsonPoint);
  try {
    if (!jPoint->is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    auto jx = jPoint->at("x");
    auto jy = jPoint->at("y");
    if (!jx.is_number_integer() || !jy.is_number_integer()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    cPoint->x = jx.get<int16_t>();
    cPoint->y = jy.get<int16_t>();
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return HIPPO_OK;
}

uint64_t HiResCamera::Resolution_c2json(const hippo::CameraResolution &set,
                                        void *obj) {
  if (obj == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  nl::json params = {
    { "height", set.height },
    { "width", set.width },
    { "fps", set.fps },
  };
  reinterpret_cast<nl::json*>(obj)->push_back(params);
  return HIPPO_OK;
}

uint64_t HiResCamera::Resolution_json2c(const void *obj,
                                        hippo::CameraResolution *get) {
  uint64_t err = HIPPO_OK;
  if (obj == NULL || get == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *jsonRes = reinterpret_cast<const nl::json*>(obj);
  try {
    // validate that json objects got returned for input
    // and output resolution
    if (!jsonRes->is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    // get the actual height and width values from their parents
    auto res_height = jsonRes->at("height");
    auto res_width = jsonRes->at("width");
    auto res_fps = jsonRes->at("fps");

    // validate that they are integers
    if (!res_height.is_number_integer() ||
        !res_height.is_number_integer() ||
        !res_fps.is_number_integer()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // convert the values out of json and put them in the ouput variable
    get->height = res_height.get<uint32_t>();
    get->width = res_width.get<uint32_t>();
    get->fps = res_fps.get<uint32_t>();
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return err;
}

uint64_t HiResCamera::WhiteBalance_json2c(void *obj, Rgb *wb) {
  uint64_t err = HIPPO_OK;
  if (obj == NULL || wb == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *wbData = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!wbData->is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    auto jsonRed = wbData->at("red");
    auto jsonGreen = wbData->at("green");
    auto jsonBlue = wbData->at("blue");

    // validate that the red green and blue values are ints
    if (!jsonRed.is_number_integer() ||
        !jsonGreen.is_number_integer() ||
        !jsonBlue.is_number_integer()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    wb->red = jsonRed.get<uint32_t>();
    wb->green = jsonGreen.get<uint32_t>();
    wb->blue = jsonBlue.get<uint32_t>();
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return err;
}

uint64_t HiResCamera::WhiteBalance_c2json(const hippo::Rgb &wb, void *obj) {
  if (obj == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  nl::json params = {
    { "red", wb.red },
    { "green", wb.green },
    { "blue", wb.blue },
  };
  reinterpret_cast<nl::json*>(obj)->push_back(params);
  return HIPPO_OK;
}

uint64_t HiResCamera::Strobe_c2json(const hippo::Strobe &set, void *obj) {
  if (obj == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }

  nl::json params = {
    { "frames", set.frames },
    { "gain", set.gain },
    { "exposure", set.exposure },
  };
  reinterpret_cast<nl::json*>(obj)->push_back(params);
  return HIPPO_OK;
}

uint64_t HiResCamera::Strobe_json2c(const void *obj,
                                    hippo::Strobe *get) {
  if (obj == NULL || get == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  uint64_t err = 0LL;
  const nl::json *strobeData = reinterpret_cast<const nl::json*>(obj);
  try {
    auto jsonExposure = strobeData->at("exposure");
    auto jsonGain = strobeData->at("gain");
    auto jsonFrames = strobeData->at("frames");

    if (!jsonExposure.is_number_integer() ||
        !jsonGain.is_number_integer() ||
        !jsonFrames.is_number_integer()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    get->exposure = jsonExposure.get<uint32_t>();
    get->gain = jsonGain.get<uint32_t>();
    get->frames = jsonFrames.get<uint32_t>();
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return err;
}

const char *CameraStatusEnum_str[
  static_cast<uint32_t>(hippo::CameraStatus::error) + 1] = {
  "ok", "busy", "error",
};

uint64_t HiResCamera::CameraStatus_json2c(const void *obj,
                                          CameraStatus *get) {
  if (obj == NULL || get == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *jsonStatus = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!jsonStatus->is_string()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // get the individual items from json
    std::string statusStr = jsonStatus->get<std::string>();

    int32_t idx;
    idx = str_to_idx(CameraStatusEnum_str, statusStr.c_str(),
                     static_cast<uint32_t>(hippo::CameraStatus::ok),
                     static_cast<uint32_t>(hippo::CameraStatus::error));
    if (idx < 0) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    *get = static_cast<hippo::CameraStatus>(idx);
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return HIPPO_OK;
}

uint64_t HiResCamera::CameraDeviceStatus_json2c(const void* obj,
                                                CameraDeviceStatus* get) {
  if (obj == NULL || get == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  uint64_t err = HIPPO_OK;
  const nl::json *jsonStatus = reinterpret_cast<const nl::json*>(obj);
  try {
      if (!jsonStatus->is_object()) {
        return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
      }
      auto generic_get = jsonStatus->at("generic_get");
      auto generic_set = jsonStatus->at("generic_set");
      auto isp_colorbar = jsonStatus->at("isp_colorbar");
      auto isp_function = jsonStatus->at("isp_function");
      auto isp_fw_boot = jsonStatus->at("isp_fw_boot");
      auto isp_reset = jsonStatus->at("isp_reset");
      auto isp_restore = jsonStatus->at("isp_restore");
      auto isp_videostream = jsonStatus->at("isp_videostream");
      auto load_lenc_calibration = jsonStatus->at("load_lenc_calibration");
      auto load_white_balance_calibration =
                          jsonStatus->at("load_white_balance_calibration");
      auto special_get = jsonStatus->at("special_get");
      auto special_set = jsonStatus->at("special_set");
      auto thermal_sensor_error = jsonStatus->at("thermal_sensor_error");
      auto thermal_shutdown = jsonStatus->at("thermal_shutdown");

      if (err = CameraStatus_json2c(&generic_get, &get->generic_get)) {
        return err;
      }
      if (err = CameraStatus_json2c(&generic_set, &get->generic_set)) {
        return err;
      }
      if (err = CameraStatus_json2c(&isp_colorbar, &get->isp_colorbar)) {
        return err;
      }
      if (err = CameraStatus_json2c(&isp_function, &get->isp_function)) {
        return err;
      }
      if (err = CameraStatus_json2c(&isp_fw_boot, &get->isp_fw_boot)) {
        return err;
      }
      if (err = CameraStatus_json2c(&isp_reset, &get->isp_reset)) {
        return err;
      }
      if (err = CameraStatus_json2c(&isp_restore, &get->isp_restore)) {
        return err;
      }
      if (err = CameraStatus_json2c(&isp_videostream, &get->isp_videostream)) {
        return err;
      }
      if (err = CameraStatus_json2c(&load_lenc_calibration,
                                    &get->load_lenc_calibration)) {
        return err;
      }
      if (err = CameraStatus_json2c(&load_white_balance_calibration,
                                    &get->load_white_balance_calibration)) {
        return err;
      }
      if (err = CameraStatus_json2c(&special_get, &get->special_get)) {
        return err;
      }
      if (err = CameraStatus_json2c(&special_set, &get->special_set)) {
        return err;
      }
      if (err = CameraStatus_json2c(&thermal_sensor_error,
                                    &get->thermal_sensor_error)) {
        return err;
      }
      if (err = CameraStatus_json2c(&thermal_shutdown,
                                    &get->thermal_shutdown)) {
        return err;
      }
  }catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return HIPPO_OK;
}

const char *CameraLedStateEnum_str[
  static_cast<uint32_t>(hippo::CameraLedStateEnum::automatic) + 1] = {
  "off", "low", "high", "auto",
};

uint64_t HiResCamera::CameraLedState_c2json(const hippo::CameraLedState &set,
                               void *obj) {
  if (obj == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }

  nl::json params = {
    { "capture",
      CameraLedStateEnum_str[static_cast<uint32_t>(set.capture)] },
    { "streaming",
       CameraLedStateEnum_str[static_cast<uint32_t>(set.streaming)] },
  };
  reinterpret_cast<nl::json*>(obj)->push_back(params);
  return HIPPO_OK;
}

uint64_t HiResCamera::CameraLedState_json2c(const void *obj,
                               hippo::CameraLedState *get) {
  if (obj == NULL || get == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  uint64_t err = 0LL;
  const nl::json *jsonState = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!jsonState->is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    auto jsonCaptureState = jsonState->at("capture");
    auto jsonStreamingState = jsonState->at("streaming");
    if (!jsonCaptureState.is_string() || !jsonStreamingState.is_string()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    if (err = CameraLedStateEnum_json2c(static_cast<void*>(&jsonCaptureState),
                                        &get->capture)) {
      return err;
    }
    if (err = CameraLedStateEnum_json2c(static_cast<void*>(&jsonStreamingState),
                                        &get->streaming)) {
      return err;
    }
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return HIPPO_OK;
}

uint64_t HiResCamera::CameraLedStateEnum_json2c(const void *obj,
                                             hippo::CameraLedStateEnum *state) {
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
    idx = str_to_idx(CameraLedStateEnum_str, stateStr.c_str(),
                  static_cast<uint32_t>(hippo::CameraLedStateEnum::off),
                  static_cast<uint32_t>(hippo::CameraLedStateEnum::automatic));
    if (idx < 0) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    *state = static_cast<hippo::CameraLedStateEnum>(idx);
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return HIPPO_OK;
}

uint64_t HiResCamera::CameraSettings_json2c(const void *obj,
                                            hippo::CameraSettings *get) {
  if (obj == NULL || get == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  uint64_t err = 0LL;
  if (err = AutoOrFixed_json2c(obj, "exposure", &(get->exposure))) {
    return err;
  }
  if (err = AutoOrFixed_json2c(obj, "gain", &(get->gain))) {
    return err;
  }
  if (err = AutoOrFixed_json2c(obj, "white_balance", &(get->white_balance))) {
    return err;
  }
  if (err = Bool_json2c(obj, "flip_frame", &(get->flip_frame))) {
    return err;
  }
  if (err = Bool_json2c(obj, "gamma_correction", &(get->gamma_correction))) {
    return err;
  }
  if (err = Bool_json2c(obj, "lens_color_shading",
                        &(get->lens_color_shading))) {
    return err;
  }
  if (err = Bool_json2c(obj, "lens_shading", &(get->lens_shading))) {
    return err;
  }
  if (err = Bool_json2c(obj, "mirror_frame", &(get->mirror_frame))) {
    return err;
  }
  return err;
}

uint64_t HiResCamera::CameraSettings_c2json(const hippo::CameraSettings &set,
                                            void *obj) {
  if (obj == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  uint64_t err = 0LL;
  nl::json params;
  void *pptr = reinterpret_cast<void*>(&params);

  if (err = AutoOrFixed_c2json(set.exposure, "exposure", pptr)) {
    return err;
  }
  if (err = AutoOrFixed_c2json(set.gain, "gain", pptr)) {
    return err;
  }
  if (err = AutoOrFixed_c2json(set.white_balance, "white_balance", pptr)) {
    return err;
  }
  params["flip_frame"] = set.flip_frame;
  params["gamma_correction"] = set.gamma_correction;
  params["lens_color_shading"] = set.lens_color_shading;
  params["lens_shading"] = set.lens_shading;
  params["mirror_frame"] = set.mirror_frame;

  // json['params'] is sent as a list containing the object
  reinterpret_cast<nl::json*>(obj)->push_back(params);

  return err;
}


uint64_t HiResCamera::PowerLineFrequency_c2json(
                                   const hippo::PowerLineFrequency *set,
                                   void *obj) {
  nl::json params;

  switch (*set) {
  case PowerLineFrequency::disabled:
    params =  "disabled";
    break;
  case PowerLineFrequency::hz_50:
    params = 50;
    break;
  case PowerLineFrequency::hz_60:
    params = 60;
    break;
  }

  reinterpret_cast<nl::json*>(obj)->push_back(params);
  return HIPPO_OK;
}

uint64_t HiResCamera::PowerLineFrequency_json2c(const void *obj,
                                   hippo::PowerLineFrequency *get) {
  if (obj == NULL || get == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  *get = PowerLineFrequency::disabled;
  try {
    const nl::json *jsonPLF = reinterpret_cast<const nl::json*>(obj);
    if (jsonPLF->is_number_integer()) {
      if (jsonPLF->get<uint32_t>() == 50) {
        *get = PowerLineFrequency::hz_50;
      } else if (jsonPLF->get<uint32_t>() == 60) {
        *get = PowerLineFrequency::hz_60;
      } else {
        return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
      }
    } else if (jsonPLF->is_string()) {
      // if we get a string, and it's not "disabled" then something is wrong
      if (strcmp(jsonPLF->get<std::string>().c_str(), "disabled")) {
        return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
      }
    }
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  return HIPPO_OK;
}

// nl::basic_json<std::map, std::vector, std::string, bool, int64_t, uint64_t,
//                double, std::allocator, nl::adl_serializer> val;
uint64_t HiResCamera::AutoOrFixed_json2c(const void *obj, const char *key,
                                         hippo::AutoOrFixed *get) {
  if (obj == NULL || key == NULL || get == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  nl::json val;

  try {
    val = reinterpret_cast<const nl::json*>(obj)->at(key);
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }

  return AutoOrFixed_json2c(reinterpret_cast<const void*>(&val), get);
}

uint64_t HiResCamera::AutoOrFixed_json2c(const void *obj,
                                         hippo::AutoOrFixed *get) {
  if (obj == NULL || get == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  uint64_t err = 0LL;
  const char *auto_str = "auto";

  try {
    const nl::json *val = reinterpret_cast<const nl::json*>(obj);

    if (val->is_number_integer()) {
      get->type = AutoOrFixedType::TYPE_UINT;
      get->value.value = val->get<uint32_t>();
    } else if (val->is_string()) {
      std::string sstr = val->get<std::string>();
      const char *str = sstr.c_str();
      if (strlen(str) == strlen(auto_str) &&
          !strncmp(str, auto_str, strlen(auto_str))) {
        get->type = AutoOrFixedType::TYPE_AUTO;
      } else {
        err = MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
      }
    } else if (val->is_object()) {
      get->type = AutoOrFixedType::TYPE_RGB;
      auto red = val->at("red");
      auto green = val->at("green");
      auto blue = val->at("blue");
      if (red.is_number_integer() && green.is_number_integer() &&
          blue.is_number_integer()) {
        get->value.rgb.red = red.get<uint32_t>();
        get->value.rgb.green = green.get<uint32_t>();
        get->value.rgb.blue = blue.get<uint32_t>();
      } else {
        err = MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
      }
    } else {
      err = MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
    }
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  return err;
}

uint64_t HiResCamera::AutoOrFixed_c2json(const hippo::AutoOrFixed &param,
                                         const char *key,
                                         void *obj) {
  uint64_t err = 0LL;
  nl::json *j = reinterpret_cast<nl::json*>(obj);

  switch (param.type) {
    case hippo::AutoOrFixedType::TYPE_NONE:
      break;
    case hippo::AutoOrFixedType::TYPE_AUTO:
      (*j)[key] = "auto";
      break;
    case hippo::AutoOrFixedType::TYPE_UINT:
      (*j)[key] = param.value.value;
      break;
    case hippo::AutoOrFixedType::TYPE_RGB:
      (*j)[key] = {{"red", param.value.rgb.red},
                   {"green", param.value.rgb.green},
                   {"blue", param.value.rgb.blue}};
      break;
    case hippo::AutoOrFixedType::TYPE_MODE:
      switch (param.value.mode) {
        case hippo::CameraMode::MODE_4416x3312:
          (*j)[key] = "4416x3312";
          break;
        case hippo::CameraMode::MODE_2208x1656:
          (*j)[key] = "2208x1656";
          break;
        case hippo::CameraMode::MODE_1104x828:
          (*j)[key] = "1104x828";
          break;
      }
      break;
    default:
      fprintf(stderr, "OPS! Unknown type %d\n", param.type);
  }
  return err;
}

uint64_t HiResCamera::Bool_json2c(const void *obj, const char *key, bool *get) {
  uint64_t err = 0LL;

  try {
    nl::json *j = (nl::json*)(obj);
    auto val = reinterpret_cast<const nl::json*>(obj)->at(key);

    if (!val.is_boolean()) {
      err = MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
    }
    *get = val.get<bool>();
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return err;
}

bool HiResCamera::HasRegisteredCallback() {
  return (NULL != callback_);
}

const char *HiResCameraNotification_str[] = {
  "on_close", "on_device_connected", "on_device_disconnected",
  "on_factory_default", "on_open", "on_open_count", "on_resume", "on_suspend",
  "on_sohal_disconnected", "on_sohal_connected",
  "on_brightness", "on_contrast", "on_exposure", "on_flip_frame", "on_gain",
  "on_gamma_correction", "on_keystone", "on_keystone_table",
  "on_keystone_table_entries", "on_led_state", "on_lens_color_shading",
  "on_lens_shading", "on_mirror_frame", "on_power_line_frequency",
  "on_reset", "on_saturation", "on_sharpness",
  "on_strobe", "on_white_balance", "on_white_balance_temperature",
};

void HiResCamera::ProcessSignal(char *method, void *obj) {
  if (NULL == callback_) {
    return;
  }
  uint64_t err = 0LL;
  int32_t idx = 0;
  idx = str_to_idx(HiResCameraNotification_str,
                   method,
                   static_cast<uint32_t>(
                       HiResCameraNotification::on_close),
                   static_cast<uint32_t>(
                       HiResCameraNotification::on_white_balance_temperature));
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
  HiResCameraNotificationParam param;
  param.type = static_cast<hippo::HiResCameraNotification>(idx);

  // initialize the number of keystone_table_entries to zero
  param.num_keystone_table_entries = 0;

  switch (param.type) {
    case HiResCameraNotification::on_open_count:
      param.on_open_count = v.get<uint32_t>();
      break;

    case HiResCameraNotification::on_brightness:
      param.on_brightness = v.get<uint16_t>();
      break;

    case HiResCameraNotification::on_contrast:
      param.on_contrast = v.get<uint16_t>();
      break;

    case HiResCameraNotification::on_exposure:
      err = AutoOrFixed_json2c(reinterpret_cast<const void*>(&v),
                               &param.on_exposure);
      break;

    case HiResCameraNotification::on_flip_frame:
      param.on_flip_frame = v.get<bool>();
      break;

    case HiResCameraNotification::on_gain:
      err = AutoOrFixed_json2c(reinterpret_cast<const void*>(&v),
                               &param.on_gain);
      break;

    case HiResCameraNotification::on_gamma_correction:
      param.on_gamma_correction = v.get<bool>();
      break;

    case HiResCameraNotification::on_lens_color_shading:
      param.on_lens_color_shading = v.get<bool>();
      break;

    case HiResCameraNotification::on_led_state:
      err = CameraLedState_json2c(reinterpret_cast<const void*>(&v),
                                  &param.on_led_state);
      break;

    case HiResCameraNotification::on_lens_shading:
      param.on_lens_shading = v.get<bool>();
      break;

    case HiResCameraNotification::on_mirror_frame:
      param.on_mirror_frame = v.get<bool>();
      break;

    case HiResCameraNotification::on_power_line_frequency:
      err = PowerLineFrequency_json2c(reinterpret_cast<const void*>(&v),
                                  &param.on_power_line_frequency);
      break;

    case HiResCameraNotification::on_white_balance:
      err = AutoOrFixed_json2c(reinterpret_cast<const void*>(&v),
                         &param.on_white_balance);
      break;

    case HiResCameraNotification::on_keystone:
      err = CameraKeystone_json2c(reinterpret_cast<const void*>(&v),
                            &param.on_keystone);
      break;

    case HiResCameraNotification::on_keystone_table:
      err = CameraKeystoneTable_json2c(reinterpret_cast<const void*>(&v),
                                 &param.on_keystone_table);
      break;

    case HiResCameraNotification::on_keystone_table_entries:
      err = CameraKeystoneTableEntries_json2c(reinterpret_cast<const void*>(&v),
                                        &param.on_keystone_table_entries,
                                        &param.num_keystone_table_entries);
      break;

    case HiResCameraNotification::on_strobe:
      err = Strobe_json2c(reinterpret_cast<const void*>(&v),
                                  &param.on_strobe);
      break;
    case HiResCameraNotification::on_saturation:
      param.on_saturation = v.get<uint16_t>();
      break;

    case HiResCameraNotification::on_sharpness:
      param.on_sharpness = v.get<uint16_t>();
      break;

    case HiResCameraNotification::on_white_balance_temperature:
      param.on_white_balance_temperature = v.get<uint16_t>();
      break;
    default:
      break;
  }
  // call the callback function
  if (!err) {
    (*callback_)(param, callback_data_);
  }

  // and clean up
  switch (param.type) {
  case HiResCameraNotification::on_keystone_table_entries:
    // free the memory allocated in CameraKeystoneTableEntries_json2c
    free_keystone_table_entries(&param.on_keystone_table_entries,
                                param.num_keystone_table_entries);
    break;
  default:
    break;
  }

  delete params;
}

}  // namespace hippo
