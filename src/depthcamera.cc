
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#include <stdio.h>

#include <mutex>   // NOLINT

#include "../include/depthcamera.h"
#include "../include/hippo_ws.h"
#include "../include/json.hpp"

namespace nl = nlohmann;

namespace hippo {

extern std::mutex gHippoDeviceMutex;
const char devName[] = "depthcamera";
extern const char *defaultHost;
extern uint32_t defaultPort;


DepthCamera::DepthCamera() :
    HippoCamera(devName, defaultHost, defaultPort, HIPPO_DEPTHCAMERA, 0) {
}

DepthCamera::DepthCamera(uint32_t device_index) :
    HippoCamera(devName, defaultHost, defaultPort, HIPPO_DEPTHCAMERA,
                device_index) {
}

DepthCamera::DepthCamera(const char *address, uint32_t port) :
    HippoCamera(devName, address, port, HIPPO_DEPTHCAMERA, 0) {
}

DepthCamera::DepthCamera(const char *address, uint32_t port,
                         uint32_t device_index) :
    HippoCamera(devName, address, port, HIPPO_DEPTHCAMERA, device_index) {
}

DepthCamera::~DepthCamera(void) {
}

uint64_t DepthCamera::ir_flood_on(bool set) {
  return bool_set_get("ir_flood_on", set, NULL);
}

uint64_t DepthCamera::ir_flood_on(bool *get) {
  return bool_get("ir_flood_on", get);
}

uint64_t DepthCamera::ir_flood_on(bool set, bool* get) {
  return bool_set_get("ir_flood_on", set, get);
}

uint64_t DepthCamera::ir_to_rgb_calibration(IrRgbCalibration *get) {
  if (NULL == get) {
    return  MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  uint64_t err = HIPPO_OK;
  nl::json jset, jget;
  void *jsetptr = reinterpret_cast<void*>(&jset);
  void *jgetptr = reinterpret_cast<void*>(&jget);
  // add the mode name to the parameters
  if (err = SendRawMsg("ir_to_rgb_calibration", jgetptr)) {
    return err;
  }
  return irRGBcalibration_json2c(jgetptr, get);
}

uint64_t DepthCamera::laser_on(bool set) {
  return bool_set_get("laser_on", set, NULL);
}

uint64_t DepthCamera::laser_on(bool *get) {
  return bool_get("laser_on", get);
}

uint64_t DepthCamera::laser_on(bool set, bool* get) {
  return bool_set_get("laser_on", set, get);
}

uint64_t DepthCamera::parseIntrinsics(
    void *jsonIntrinsics,
    hippo::CalibrationIntrinsics* cIntrinsics) {
  if (jsonIntrinsics == NULL || cIntrinsics == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *intrinsics =
      reinterpret_cast<const nl::json*>(jsonIntrinsics);
  try {
    if (!intrinsics->is_array()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // get the number of items in the list
    uint64_t num_items = intrinsics->size();
    if (num_items != 4) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    auto focal_x = intrinsics->at(0);
    auto focal_y = intrinsics->at(1);
    auto center_x = intrinsics->at(2);
    auto center_y = intrinsics->at(3);

    if (!focal_x.is_number_float() ||
        !focal_y.is_number_float() ||
        !center_x.is_number_float() ||
        !center_y.is_number_float()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    cIntrinsics->fx = focal_x.get<float>();
    cIntrinsics->fy = focal_y.get<float>();
    cIntrinsics->cx = center_x.get<float>();
    cIntrinsics->cy = center_y.get<float>();
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return HIPPO_OK;
}

uint64_t DepthCamera::parseDistortion(
    void *jsonDistortion,
    hippo::CalibrationDistortion *cDistortion) {
  if (jsonDistortion == NULL || cDistortion == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *distortion =
      reinterpret_cast<const nl::json*>(jsonDistortion);
  try {
    if (!distortion->is_array()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // get the number of items in the list
    uint64_t num_items = distortion->size();
    if (num_items != 5) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    auto k1 = distortion->at(0);
    auto k2 = distortion->at(1);
    auto k3 = distortion->at(2);
    auto p1 = distortion->at(3);
    auto p2 = distortion->at(4);

    if (!k1.is_number_float() ||
        !k2.is_number_float() ||
        !k3.is_number_float() ||
        !p1.is_number_float() ||
        !p2.is_number_float()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    cDistortion->k1 = k1.get<float>();
    cDistortion->k2 = k2.get<float>();
    cDistortion->k3 = k3.get<float>();
    cDistortion->p1 = p1.get<float>();
    cDistortion->p2 = p2.get<float>();
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return HIPPO_OK;
}

bool DepthCamera::HasRegisteredCallback() {
  return (NULL != callback_);
}

uint64_t DepthCamera::subscribe(
    void(*callback)(const DepthCameraNotificationParam &param, void *data),
    void *data) {
  return subscribe(callback, data, NULL);
}

uint64_t DepthCamera::subscribe(
    void(*callback)(const DepthCameraNotificationParam &param, void *data),
    void *data, uint32_t *get) {
  uint64_t err = 0LL;

  if (err = HippoDevice::subscribe_raw(data, get)) {
    return err;
  }
  callback_ = callback;

  return err;
}

uint64_t DepthCamera::unsubscribe() {
  callback_ = NULL;
  return HippoDevice::unsubscribe();
}

uint64_t DepthCamera::unsubscribe(uint32_t *get) {
  callback_ = NULL;
  return HippoDevice::unsubscribe(get);
}

uint64_t DepthCamera::irRGBcalibration_json2c(void *obj,
                                              hippo::IrRgbCalibration *cal) {
  if (obj == NULL || cal == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }

  uint64_t err = HIPPO_OK;

  const nl::json *jsonCal = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!jsonCal->is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    auto irIntrinsics = jsonCal->at("ir_intrinsics");
    auto rgbIntrinsics = jsonCal->at("rgb_intrinsics");
    auto irDistortion = jsonCal->at("ir_distortion");
    auto rgbDistortion = jsonCal->at("rgb_distortion");
    auto matrixXform = jsonCal->at("matrix_transformation");
    auto mirror = jsonCal->at("mirror");

    if (!irIntrinsics.is_array() ||
        !rgbIntrinsics.is_array() ||
        !irDistortion.is_array() ||
        !rgbDistortion.is_array()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    if (HIPPO_OK != (err = parseIntrinsics(
            reinterpret_cast<void *>(&irIntrinsics),
            &(cal->ir_intrinsics)))) {
      return err;
    }

    if (HIPPO_OK != (err = parseIntrinsics(
            reinterpret_cast<void *>(&rgbIntrinsics),
            &(cal->rgb_intrinsics)))) {
      return err;
    }

    if (HIPPO_OK != (err = parseDistortion(
            reinterpret_cast<void *>(&rgbDistortion),
            &(cal->rgb_distortion)))) {
      return err;
    }

    if (HIPPO_OK != (err = parseDistortion(
            reinterpret_cast<void *>(&irDistortion),
            &(cal->ir_distortion)))) {
      return err;
    }

    // now parse the transformation matrix item (a 4x4 matrix stored in 4 arays
    // where each array is another array containing 4 items)
    if (!matrixXform.is_array()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    uint64_t num_rows = matrixXform.size();
    if (num_rows <= 0 || num_rows > 4) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // iterate over the rows
    for (int i = 0; i < num_rows; i++) {
      // get the current row
      auto curr_row = matrixXform.at(i);
      if (!curr_row.is_array()) {
        return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
      }
      // get the number of items in the current row (should be four)
      uint64_t num_row_items = curr_row.size();
      if (num_row_items <= 0 || num_row_items > 4) {
        return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
      }
      // iterate over the row items and convert them to the c array
      for (int j = 0; j < num_row_items; j++) {
        auto curr_item = curr_row.at(j);
        if (!curr_item.is_number_float()) {
          return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
        }
        // convert the item
        cal->matrix_transformation[i][j] = curr_item.get<float>();
      }  // end for(j) loop
    }  // end for(i) loop

    // get the mirror item
    if (!mirror.is_boolean()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    cal->mirror = mirror.get<bool>();
  } catch (nl::json::exception e) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return HIPPO_OK;
}

const char *DepthCameraNotification_str[] = {
  "on_close", "on_device_connected", "on_device_disconnected",
  "on_factory_default", "on_open", "on_open_count", "on_resume", "on_suspend",
  "on_sohal_disconnected", "on_sohal_connected",
  "on_enable_streams", "on_disable_streams",
  "on_ir_flood_on", "on_laser_on",
};

void DepthCamera::ProcessSignal(char *method, void *obj) {
  if (NULL == callback_) {
    return;
  }
  uint64_t err = 0LL;
  int32_t idx = 0;
  idx = str_to_idx(DepthCameraNotification_str,
                   method,
                   static_cast<uint32_t>(
                       DepthCameraNotification::on_close),
                   static_cast<uint32_t>(
                       DepthCameraNotification::on_laser_on));
  free(method);
  if (idx < 0) {
    return;
  }
  nl::json v, *params = reinterpret_cast<nl::json*>(obj);
  try {
    v = params->at(0);
  }
  catch (nl::json::exception) {
    // out_of_range or type_error do nothing
  }
  DepthCameraNotificationParam param;
  param.type = static_cast<hippo::DepthCameraNotification>(idx);

  switch (param.type) {
  case DepthCameraNotification::on_open_count:
    param.on_open_count = v.get<uint32_t>();
    break;

  case DepthCameraNotification::on_enable_streams:
    err =  CameraStreams_json2c(reinterpret_cast<const void*>(&v),
                                &param.on_enable_streams);
    break;

  case DepthCameraNotification::on_disable_streams:
    err = CameraStreams_json2c(reinterpret_cast<const void*>(&v),
                               &param.on_disable_streams);
    break;

  case DepthCameraNotification::on_ir_flood_on:
    param.on_ir_flood_on = v.get<bool>();
    break;

  case DepthCameraNotification::on_laser_on:
    param.on_laser_on = v.get<bool>();
    break;

  default:
    break;
  }
  if (!err) {
    (*callback_)(param, callback_data_);
  }
  delete params;
}
}  // namespace hippo
