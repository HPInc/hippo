
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include "../include/system.h"
#include "../include/json.hpp"

namespace nl = nlohmann;

namespace hippo {

const char *PowerState_str[] = {
  "display_on", "display_off",
  "display_dimmed", "suspend",
  "resume", "log_off",
  "shut_down",
};

const char *SessionState_str[] = {
  "locked", "unlocked", "unknown",
};

const char *SessionChange_str[] = {
  "console_connect", "console_disconnect",
  "session_logon", "session_logoff",
  "session_lock", "session_unlock",
};

const char *CameraMapType_str[] = { "depthRGB_to_hires", "hires_to_depthRGB"};
const char *CamNames_str[] = { "depthcamera", "hirescamera" };
const char *StreamNames_str[] = { "rgb", "depth", "ir", "points" };

uint64_t System::camera_stream_c2json(const hippo::CameraStream &camStream,
                                      void *obj) {
  if (obj == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  const int NUM_CAMERAS = 2;
  const int NUM_STREAMS = 4;

  int camName = static_cast<uint32_t>(camStream.name);
  int streamName = static_cast<uint32_t>(camStream.stream);

  if (camName > NUM_CAMERAS || streamName > NUM_STREAMS) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
  }

  nl::json params = {
    {"index", camStream.index },
    {"name", CamNames_str[camName]},
    {"stream", StreamNames_str[streamName]}
  };
  *reinterpret_cast<nl::json*>(obj) = params;
  return HIPPO_OK;
}

uint64_t System::cam_3d_mapping_c2json(
  const hippo::Camera3DMappingParameter &camMapParam,
  void *obj) {
  if (obj == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  uint64_t err;
  nl::json param, from, to;
  if ((err = camera_stream_c2json(camMapParam.from, &from))
          != HIPPO_OK) {
    return err;
  }
  if ((err = camera_stream_c2json(camMapParam.to, &to)) != HIPPO_OK) {
    return err;
  }
  param.push_back({{ "from", from}, { "to", to}});
  *(reinterpret_cast<nl::json*>(obj)) = param;

  return HIPPO_OK;
}

uint64_t System::cam_3d_map_json2c(const void *obj,
                                   hippo::Camera3DMapping *cameraMapping) {
  if (obj == NULL || cameraMapping == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *jsonCamMap = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!jsonCamMap->is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    auto jsonFromParams = jsonCamMap->at("from");
    if (!jsonFromParams.is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    uint64_t err = HIPPO_OK;
    if ((err = camera_parameters_json2c(&jsonFromParams,
                                &cameraMapping->from)) != HIPPO_OK) {
      return err;
    }

    auto jsonToParams = jsonCamMap->at("to");
    if (!jsonToParams.is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    if ((err = camera_parameters_json2c(&jsonToParams,
                                  &cameraMapping->to)) != HIPPO_OK) {
      return err;
    }

    // get the transformation matrix item (a 4x4 matrix stored
    // in 4 lists with each list containing 4 items)
    auto xformMatrix = jsonCamMap->at("matrix_transformation");

    if (!xformMatrix.is_array()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    //  // get the number of rows in the list (should be four)
    uint64_t num_rows = xformMatrix.size();
    if (num_rows <= 0 || num_rows > 4) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    for (int i = 0; i < num_rows; i++) {
      auto curr_row = xformMatrix.at(i);
      if (!curr_row.is_array()) {
        return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
      }
      // get the number of items in the current row (should be four)
      uint64_t num_row_items = curr_row.size();
      if (num_row_items <= 0 || num_row_items > 4) {
        return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
      }
      // get each item in a row and put i in the output
      for (int j = 0; j < num_row_items; j++) {
        auto curr_item = curr_row.at(j);
        if (!curr_item.is_number_float() && !curr_item.is_number_integer()) {
          return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
        }
        cameraMapping->matrix_transformation[i][j] = curr_item.get<float>();
      }  // end for(j) loop
    }  // end for(i) loop
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }

  return HIPPO_OK;
}

uint64_t System::camera_parameters_json2c(const void *obj,
                                      hippo::CameraParameters *cameraParams) {
  if (obj == NULL || cameraParams == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *jsonCamParams = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!jsonCamParams->is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // Get the calibration resolution item
    auto calResolution = jsonCamParams->at("calibration_resolution");
    if (!calResolution.is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    auto resH = calResolution.at("height");
    auto resW = calResolution.at("width");
    if (!resH.is_number_integer() || !resW.is_number_integer()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    cameraParams->calibration_resolution.height = resH.get<uint32_t>();
    cameraParams->calibration_resolution.width = resW.get<uint32_t>();

    // get the camera stream
    auto cameraStream = jsonCamParams->at("camera");
    if (!cameraStream.is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    uint64_t err = HIPPO_OK;
    if ((err = camera_stream_json2c(&cameraStream,
                                &cameraParams->camera)) != HIPPO_OK) {
      return err;
    }

    // get the focal length item
    auto focal_length = jsonCamParams->at("focal_length");
    if (!focal_length.is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    auto focalX = focal_length.at("x");
    auto focalY = focal_length.at("y");
    if (!focalX.is_number_float() || !focalY.is_number_float()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    cameraParams->focal_length.x = focalX.get<float>();
    cameraParams->focal_length.y = focalY.get<float>();

    auto lens_distortion = jsonCamParams->at("lens_distortion");

    if (!lens_distortion.is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // get lens distortion
    auto center_pt = lens_distortion.at("center");
    if (!center_pt.is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // get the x and y
    auto distortion_x = center_pt.at("x");
    auto distortion_y = center_pt.at("y");
    if (!distortion_x.is_number_float() || !distortion_y.is_number_float()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // put the x,y into the output
    cameraParams->lens_distortion.center.x = distortion_x.get<float>();
    cameraParams->lens_distortion.center.y = distortion_y.get<float>();

    // memset the kappa and p values to zero
    memset(cameraParams->lens_distortion.kappa, 0, 6 * sizeof(float));
    memset(cameraParams->lens_distortion.p, 0, 2 * sizeof(float));

    // get the kappa
    auto distortion_kappa_list = lens_distortion.at("kappa");

    if (!distortion_kappa_list.is_array()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    // there can be up to six kappa parameters
    // now get the number of items for the distortion kappa
    uint64_t num_k_items = distortion_kappa_list.size();
    if (num_k_items <= 0 || num_k_items > 5) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // iterate over the row items and convert them to the c array
    for (int j = 0; j < num_k_items; j++) {
      auto currItem = distortion_kappa_list.at(j);
      if (!currItem.is_number_float()) {
        return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
      }
      // Store the data in the xform array on the c side
      cameraParams->lens_distortion.kappa[j] = currItem.get<float>();
    }

    auto distortion_p_list = lens_distortion.at("p");
    if (!distortion_p_list.is_array()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    uint64_t num_p_items = distortion_p_list.size();
    // max of two items are contained in the p distortion param
    if (num_p_items <= 0 || num_p_items > 2) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // iterate over the row items and convert them to the c array
    for (int j = 0; j < num_p_items; j++) {
      auto currItem = distortion_p_list.at(j);
      if (!currItem.is_number_float()) {
        return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
      }
      // Store the data in the xform array on the c side
      cameraParams->lens_distortion.p[j] = currItem.get<float>();
    }
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }

  return HIPPO_OK;
}


uint64_t System::camera_stream_json2c(const void *obj,
                                      hippo::CameraStream *cameraStream) {
  if (obj == NULL || cameraStream == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *jsonCamStream = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!jsonCamStream->is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    auto jsonIndex = jsonCamStream->at("index");
    if (!jsonIndex.is_number_integer()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    cameraStream->index = jsonIndex.get<uint32_t>();
    auto camName = jsonCamStream->at("name");
    auto camStream = jsonCamStream->at("stream");
    if (!camName.is_string() || !camStream.is_string()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    // convert the camera name to its enum
    std::string cam_name = camName.get<std::string>();
    int32_t idx = str_to_idx(
      CamNames_str, cam_name.c_str(),
      static_cast<uint32_t>(hippo::CameraNameType::depthcamera),
      static_cast<uint32_t>(hippo::CameraNameType::hirescamera));
    if (idx < 0 ||
        idx > static_cast<uint32_t>(hippo::CameraNameType::hirescamera)) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    cameraStream->name = static_cast<hippo::CameraNameType>(idx);

    // convert the camera stream to its enum
    std::string cam_stream = camStream.get<std::string>();
    idx = str_to_idx(
      StreamNames_str, cam_stream.c_str(),
      static_cast<uint32_t>(hippo::CameraStreamType::rgb),
      static_cast<uint32_t>(hippo::CameraStreamType::points));
    if (idx < 0 ||
      idx > static_cast<uint32_t>(hippo::CameraStreamType::points)) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    cameraStream->stream = static_cast<hippo::CameraStreamType>(idx);
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }

  return HIPPO_OK;
}

uint64_t System::devices_json2c(const void *obj, DeviceInfo **info,
                                uint64_t *num_devices) {
  *num_devices = 0;
  // assign *info to nullptr as soon as possible
  if (info == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  *info = nullptr;
  if (obj == NULL) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *jsonDevInfo = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!jsonDevInfo->is_array()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // get the number of devices in the list
    uint64_t num_items = jsonDevInfo->size();
    if (num_items <= 0) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    // allocate the memory to store the device info
    *info = reinterpret_cast<DeviceInfo*>(
      calloc(num_items, sizeof(DeviceInfo)));
    if (!info) {
      fprintf(stderr, "** Error allocating device ID array\n");
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MEM_ALLOC);
    }

    // iterate over the list to parse the individual devices
    for (int i = 0; i < num_items; i++) {
      auto curr_device = jsonDevInfo->at(i);
      if (!curr_device.is_object()) {
        return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
      }

      auto fw_version = curr_device.at("fw_version");
      auto name = curr_device.at("name");
      auto serial = curr_device.at("serial");
      auto index = curr_device.at("index");
      auto vendor_id = curr_device.at("vendor_id");
      auto product_id = curr_device.at("product_id");

      if (!fw_version.is_string() || !name.is_string() ||
        !serial.is_string() || !index.is_number_integer() ||
        !vendor_id.is_number_integer() || !product_id.is_number_integer()) {
        return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
      }

      (*info)[i].fw_version = strdup(fw_version.get<std::string>().c_str());
      (*info)[i].name = strdup(name.get<std::string>().c_str());
      (*info)[i].serial = strdup(serial.get<std::string>().c_str());
      (*info)[i].index = index.get<uint32_t>();
      (*info)[i].vendor_id = vendor_id.get<uint32_t>();
      (*info)[i].product_id = product_id.get<uint32_t>();
      *num_devices = i + 1;
    }
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return HIPPO_OK;
}

uint64_t System::device_id_json2c(const void *obj, DeviceID *id_info) {
  if (obj == NULL || id_info == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *jsonDevInfo = reinterpret_cast<const nl::json*>(obj);
  try {
    auto curr_device_id = *jsonDevInfo;
    if (!curr_device_id.is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    auto name = curr_device_id.at("name");
    auto index = curr_device_id.at("index");
    auto vendor_id = curr_device_id.at("vendor_id");
    auto product_id = curr_device_id.at("product_id");
    if (!name.is_string() || !index.is_number_integer() ||
      !vendor_id.is_number_integer() || !product_id.is_number_integer()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    (*id_info).name = strdup(name.get<std::string>().c_str());
    (*id_info).index = index.get<uint32_t>();
    (*id_info).vendor_id = vendor_id.get<uint32_t>();
    (*id_info).product_id = product_id.get<uint32_t>();
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return HIPPO_OK;
}

uint64_t System::device_ids_json2c(const void *obj, DeviceID **id_info,
                                   uint64_t *num_devices) {
  *num_devices = 0;
  // assign *id_info to nullptr as soon as possible
  if (id_info == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  *id_info = nullptr;

  if (obj == NULL) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *jsonDevInfo = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!jsonDevInfo->is_array()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // get the number of devices in the list
    uint64_t num_items = jsonDevInfo->size();
    if (num_items <= 0) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    // allocate the memory to store the device ID info
    *id_info = reinterpret_cast<DeviceID*>(
      calloc(num_items, sizeof(DeviceID)));
    if (!id_info) {
      fprintf(stderr, "** Error allocating device ID array\n");
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MEM_ALLOC);
    }

    // iterate over the list to parse the individual devices
    for (int i = 0; i < num_items; i++) {
      auto curr_device_id = jsonDevInfo->at(i);
      device_id_json2c(&curr_device_id, &(*id_info)[i]);
      *num_devices = i + 1;
    }
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return HIPPO_OK;
}

uint64_t System::echo_json2c(const void *obj, char **echo_return_str) {
  if (echo_return_str == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  *echo_return_str = nullptr;
  if (obj == NULL) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *jsStr = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!jsStr->is_string()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    (*echo_return_str) = strdup(jsStr->get<std::string>().c_str());
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }

  return HIPPO_OK;
}

uint64_t System::list_displays_json2c(const void *obj,
                                      DisplayInfo **display_info,
                                      uint64_t *num_displays) {
  *num_displays = 0;
  uint64_t err = HIPPO_OK;
  if (display_info == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  *display_info = nullptr;
  if (obj == NULL) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *jsonDisplayList = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!jsonDisplayList->is_array()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // get the number of displays in the list
    uint64_t num_items = jsonDisplayList->size();
    if (num_items <= 0) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    // allocate the memory to store the display info
    *display_info = reinterpret_cast<DisplayInfo*>(calloc(num_items,
                                                         sizeof(DisplayInfo)));
    if (!display_info) {
      fprintf(stderr, "** Error allocating display info array\n");
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MEM_ALLOC);
    }
    // iterate over the list to parse the individual displays
    for (int i = 0; i < num_items; i++) {
      auto curr_info = jsonDisplayList->at(i);
      if (!curr_info.is_object()) {
        return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
      }
      auto hardware_id = curr_info.at("hardware_id");
      auto is_primary = curr_info.at("primary_display");
      auto coordinates = curr_info.at("coordinates");

      if (!hardware_id.is_string() || !is_primary.is_boolean() ||
          !coordinates.is_object()) {
        return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
      }

      auto dispX = coordinates.at("x");
      auto dispY = coordinates.at("y");
      auto dispWidth = coordinates.at("width");
      auto dispHeight = coordinates.at("height");

      // check the types of the display area objects
      if (!dispX.is_number_integer() ||
          !dispY.is_number_integer() ||
          !dispWidth.is_number_integer() ||
          !dispHeight.is_number_integer()) {
        return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
      }

      // assign the values
      (*display_info)[i].hardware_id = strdup(
                              hardware_id.get<std::string>().c_str());
      (*display_info)[i].primary_display = is_primary.get<bool>();
      (*display_info)[i].coordinates.height = dispHeight.get<uint16_t>();
      (*display_info)[i].coordinates.width = dispWidth.get<uint16_t>();
      (*display_info)[i].coordinates.x = dispX.get<uint16_t>();
      (*display_info)[i].coordinates.y = dispY.get<uint16_t>();

      if (err != HIPPO_OK) {
        return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
      }
      *num_displays = i + 1;
    }
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }

  return HIPPO_OK;
}

uint64_t System::supported_devices_json2c(const void *obj,
                                          SupportedDevice **devices,
                                          uint64_t *num_devices) {
  *num_devices = 0;

  if (devices == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  *devices = nullptr;
  if (obj == NULL) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *jsonDevices = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!jsonDevices->is_array()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // get the number of devices in the list
    uint64_t num_items = jsonDevices->size();
    if (num_items <= 0) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    // allocate the memory to store the device info
    *devices = reinterpret_cast<SupportedDevice*>(
      calloc(num_items, sizeof(SupportedDevice)));
    if (!devices) {
      fprintf(stderr, "** Error allocating supported devices array\n");
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MEM_ALLOC);
    }

    // iterate over the list to parse the individual devices
    for (int i = 0; i < num_items; i++) {
      auto curr_device_name = jsonDevices->at(i);
      if (!curr_device_name.is_string()) {
        return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
      }

      (*devices)[i].name = strdup(curr_device_name.get<std::string>().c_str());

      *num_devices = i + 1;
    }
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }

  return HIPPO_OK;
}

uint64_t System::is_locked_json2c(const void *obj,
                                  SessionState *session_state) {
  if (obj == NULL || session_state == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *jsonStateInfo = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!jsonStateInfo->is_string()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    std::string state = jsonStateInfo->get<std::string>();
    // get the proper enum value for the sensor name
    int32_t idx = str_to_idx(SessionState_str, state.c_str(),
                             static_cast<uint32_t>(
                               SessionState::locked),
                             static_cast<uint32_t>(
                               SessionState::unknown));
    if (idx < 0) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    (*session_state) = static_cast<hippo::SessionState>(idx);
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }

  return HIPPO_OK;
}

uint64_t System::hardware_ids_json2c(const void *obj, HardwareIDs *get,
                             uint64_t *num_projectors,
                             uint64_t *num_touchscreens) {
  *num_projectors = 0;
  *num_touchscreens = 0;

  if (obj == NULL || get == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }

  const nl::json *jsonIDs = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!jsonIDs->is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    auto jsonProjector = jsonIDs->at("sprout_projector");
    auto jsonTouchscreen = jsonIDs->at("sprout_touchscreen");
    if (!jsonProjector.is_array() || !jsonTouchscreen.is_array()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    uint64_t numProj = jsonProjector.size();
    uint64_t numTS = jsonTouchscreen.size();

    // allocate the memory to store the info
    get->sprout_projector = reinterpret_cast<char**>(
                          calloc(numProj, sizeof(char*)));
    if (!get->sprout_projector) {
      fprintf(stderr, "** Error allocating projector info array\n");
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MEM_ALLOC);
    }
    get->sprout_touchscreen = reinterpret_cast<char**>(
                         calloc(numTS, sizeof(char*)));
    if (!get->sprout_touchscreen) {
      fprintf(stderr, "** Error allocating touchscreen info array\n");
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MEM_ALLOC);
    }

    for (int i = 0; i < numProj; i++) {
      auto currProj = jsonProjector.at(i);
      if (!currProj.is_string()) {
        return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
      }
      get->sprout_projector[i] = strdup(currProj.get<std::string>().c_str());
      *num_projectors = i + 1;
    }

    for (int i = 0; i < numTS; i++) {
      auto currTS = jsonTouchscreen.at(i);
      if (!currTS.is_string()) {
        return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
      }
      get->sprout_touchscreen[i] = strdup(currTS.get<std::string>().c_str());
      *num_touchscreens = i + 1;
    }
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }

return HIPPO_OK;
}

void System::free_supported_devices(SupportedDevice *devices,
                                    uint64_t num_devices) {
  for (uint64_t i = 0; i < num_devices; i++) {
    free(devices[i].name);
    devices[i].name = nullptr;
  }
  free(devices);
  devices = nullptr;
}

void System::free_device_id(DeviceID *id_to_free) {
  free(id_to_free->name);
  id_to_free->name = nullptr;
}

void System::free_device_ids(DeviceID *ids_info_to_free,
                             uint64_t num_devices) {
  for (uint64_t i = 0; i < num_devices; i++) {
    free_device_id(&ids_info_to_free[i]);
  }
  free(ids_info_to_free);
  ids_info_to_free = nullptr;
}

void System::free_devices(DeviceInfo *info_to_free, uint64_t num_devices) {
  for (uint64_t i = 0; i < num_devices; i++) {
    free(info_to_free[i].fw_version);
    free(info_to_free[i].name);
    free(info_to_free[i].serial);
    info_to_free[i].fw_version = nullptr;
    info_to_free[i].name = nullptr;
    info_to_free[i].serial = nullptr;
  }
  free(info_to_free);
  info_to_free = nullptr;
}

void System::free_echo_string(char *echo_string_to_free) {
  free(echo_string_to_free);
  echo_string_to_free = nullptr;
}

void System::free_display_info(DisplayInfo *display_info) {
  free(display_info->hardware_id);
  display_info->hardware_id = nullptr;
}

void System::free_display_list(DisplayInfo *list_to_free,
                               uint64_t num_to_free) {
  for (uint64_t i = 0; i < num_to_free; i++) {
    free_display_info(&list_to_free[i]);
  }
  free(list_to_free);
  list_to_free = nullptr;
}

uint64_t System::powerstate_json2c(const void *obj,
                                   hippo::PowerStateType *power_state) {
  if (obj == NULL || power_state == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *jsonPowerStateInfo = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!jsonPowerStateInfo->is_string()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    std::string pwr_state = jsonPowerStateInfo->get<std::string>();
    // get the proper enum value for the sensor name
    int32_t idx = str_to_idx(PowerState_str, pwr_state.c_str(),
                             static_cast<uint32_t>(
                               PowerStateType::display_on),
                             static_cast<uint32_t>(
                               PowerStateType::shut_down));
    if (idx < 0) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    (*power_state) = static_cast<hippo::PowerStateType>(idx);
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }

  return HIPPO_OK;
}

uint64_t System::sessionchange_json2c(const void *obj,
                                   hippo::SessionChange *session_change) {
  if (obj == NULL || session_change == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *jsonSessionChangeInfo =
                                     reinterpret_cast<const nl::json*>(obj);
  try {
    if (!jsonSessionChangeInfo->is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    auto jsonevent = jsonSessionChangeInfo->at("event");
    auto jsonsessionid = jsonSessionChangeInfo->at("session_id");
    if (!jsonevent.is_string() ||
        !jsonsessionid.is_number_integer()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    std::string session_chng = jsonevent.get<std::string>();
    // get the proper enum value for the sensor name
    int32_t idx = str_to_idx(SessionChange_str, session_chng.c_str(),
                             static_cast<uint32_t>(
                               SessionChangeEvent::console_connect),
                             static_cast<uint32_t>(
                               SessionChangeEvent::session_unlock));
    if (idx < 0) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    (*session_change).session_id = jsonsessionid.get<uint32_t>();
    (*session_change).change_event =
                        static_cast<hippo::SessionChangeEvent>(idx);
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }

  return HIPPO_OK;
}

}   // namespace hippo
