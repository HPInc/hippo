
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#include "../include/projector.h"
#include <stdio.h>
#include "../include/json.hpp"

namespace nl = nlohmann;

namespace hippo {

uint64_t Projector::calibrationData_json2c(void *obj,
                                           hippo::CalibrationData *cal) {
  // test inputs to ensure non-null pointers
  if (obj == NULL || cal == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const int KEYS_LEN = 4;
  char *keys[KEYS_LEN] = { "cam_cal", "cam_cal_hd", "proj_cal", "proj_cal_hd" };
  const nl::json *calData = reinterpret_cast<const nl::json*>(obj);
  // set the inputs to null initially
  cal->cam_cal = NULL;
  cal->cam_cal_hd = NULL;
  cal->proj_cal = NULL;
  cal->proj_cal_hd = NULL;

  if (!calData->is_object()) {
    return MAKE_HIPPO_ERROR(facility_,
                            HIPPO_MESSAGE_ERROR);
  }
  // iterate over the returned array
  for (std::size_t i = 0; i < calData->size(); i++) {
    try {
      const nl::json *tmp = &calData->at(keys[i]);
      if (tmp->is_string()) {
        std::string val = tmp->get<std::string>();
        switch (i) {
        case 0:
          cal->cam_cal = strdup(val.c_str());
          break;
        case 1:
          cal->cam_cal_hd = strdup(val.c_str());
          break;
        case 2:
          cal->proj_cal = strdup(val.c_str());
          break;
        case 3:
          cal->proj_cal_hd = strdup(val.c_str());
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

void Projector::free_calibration_data(
    hippo::CalibrationData *cal_data_to_free) {
  free(cal_data_to_free->cam_cal);
  free(cal_data_to_free->cam_cal_hd);
  free(cal_data_to_free->proj_cal);
  free(cal_data_to_free->proj_cal_hd);

  cal_data_to_free->cam_cal = NULL;
  cal_data_to_free->cam_cal_hd = NULL;
  cal_data_to_free->proj_cal = NULL;
  cal_data_to_free->proj_cal_hd = NULL;
}

uint64_t Projector::dppversion_json2c(void *obj,
                                      hippo::DPPVersion *dppversion) {
  uint64_t err = HIPPO_OK;
  if (obj == NULL || dppversion == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *jsonDPPVersion = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!jsonDPPVersion->is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    auto major = jsonDPPVersion->at("major");
    auto minor = jsonDPPVersion->at("minor");
    auto patch_lsb = jsonDPPVersion->at("patch_lsb");
    auto patch_msb = jsonDPPVersion->at("patch_msb");

    if (!major.is_number_integer() || !minor.is_number_integer() ||
        !patch_lsb.is_number_integer() || !patch_msb.is_number_integer()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    dppversion->major = major.get<uint32_t>();
    dppversion->minor = minor.get<uint32_t>();
    dppversion->patch_lsb = patch_lsb.get<uint32_t>();
    dppversion->patch_msb = patch_msb.get<uint32_t>();
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return err;
}

uint64_t Projector::geoversion_json2c(void *obj,
                                      hippo::GeoFWVersion *geoversion) {
  uint64_t err = HIPPO_OK;
  if (obj == NULL || geoversion == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *jsonGeoVersion = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!jsonGeoVersion->is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    auto major = jsonGeoVersion->at("major");
    auto minor = jsonGeoVersion->at("minor");
    auto package = jsonGeoVersion->at("package");
    auto test_release = jsonGeoVersion->at("test_release");

    if (!major.is_number_integer() || !minor.is_number_integer() ||
        !package.is_string() || !test_release.is_number_integer()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    // make sure that the package string is valid
    std::string package_str = package.get<std::string>();
    if (package_str.size() <= 0) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    geoversion->major = major.get<uint32_t>();
    geoversion->minor = minor.get<uint32_t>();
    geoversion->package = package_str.c_str()[0];
    geoversion->test_release = test_release.get<uint32_t>();
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return err;
}

uint64_t Projector::projector_specific_info_json2c(void *obj,
                                       hippo::ProjectorSpecificInfo *info) {
  uint64_t err = HIPPO_OK;
  if (obj == NULL || info == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *jsonProjInfo = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!jsonProjInfo->is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // set the strings to NULL
    info->column_serial = NULL;
    info->manufacturing_time = NULL;

    auto jsonAsicVersion = jsonProjInfo->at("asic_version");
    auto jsonColSerial = jsonProjInfo->at("column_serial");
    auto jsonEepromVer = jsonProjInfo->at("eeprom_version");
    auto jsonFlashVer = jsonProjInfo->at("flash_version");
    auto jsonGeoVer = jsonProjInfo->at("geo_fw_version");
    auto jsonHWVer = jsonProjInfo->at("hw_version");
    auto jsonMfgTime = jsonProjInfo->at("manufacturing_time");

    // check the types
    if (!jsonAsicVersion.is_object() || !jsonColSerial.is_string() ||
        !jsonEepromVer.is_number_integer() || !jsonFlashVer.is_object() ||
        !jsonGeoVer.is_object() || !jsonHWVer.is_number_integer() ||
        !jsonMfgTime.is_string()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    // fill the output
    if (HIPPO_OK != (err = dppversion_json2c(&jsonAsicVersion,
                                             &info->asic_version))) {
      return err;
    }
    info->column_serial = strdup(jsonColSerial.get<std::string>().c_str());
    info->eeprom_version = jsonEepromVer.get<uint32_t>();
    if (HIPPO_OK != (err = dppversion_json2c(&jsonFlashVer,
                                             &info->flash_version))) {
      return err;
    }
    if (HIPPO_OK != (err = geoversion_json2c(&jsonGeoVer,
                                             &info->geo_fw_version))) {
      return err;
    }
    info->hw_version = jsonHWVer.get<uint32_t>();
    info->manufacturing_time = strdup(jsonMfgTime.get<std::string>().c_str());
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return err;
}
void Projector::free_projector_specific_info(
                                  hippo::ProjectorSpecificInfo *info_to_free) {
  free(info_to_free->column_serial);
  free(info_to_free->manufacturing_time);
}

uint64_t Projector::hardwareInfo_json2c(void * obj,
                                        hippo::HardwareInfo *info) {
  uint64_t err = HIPPO_OK;
  if (obj == NULL || info == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *hwInfoData = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!hwInfoData->is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    auto jsonInputRes = hwInfoData->at("input_resolution");
    auto jsonOutputRes = hwInfoData->at("output_resolution");
    auto jsonPixDens = hwInfoData->at("pixel_density");
    auto jsonRefresh = hwInfoData->at("refresh_rate");

    // validate that json objects got returned for input
    // and output resolution
    if (!jsonInputRes.is_object() ||
      !jsonOutputRes.is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    // get the actual height and width values from their parents
    auto input_height = jsonInputRes.at("height");
    auto input_width = jsonInputRes.at("width");
    auto output_height = jsonOutputRes.at("height");
    auto output_width = jsonOutputRes.at("width");

    // validate that they are integers
    if (!input_height.is_number_integer() ||
      !input_width.is_number_integer() ||
      !output_height.is_number_integer() ||
      !output_width.is_number_integer()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    // convert the values out of json and put them in the ouput variable
    info->input_resolution.height = input_height.get<uint32_t>();
    info->input_resolution.width = input_width.get<uint32_t>();
    info->output_resolution.height = output_height.get<uint32_t>();
    info->output_resolution.width = output_width.get<uint32_t>();

    // validate that pixel density and refresh rate are
    // integers - then put them into the output
    if (!jsonPixDens.is_number_integer() ||
      !jsonRefresh.is_number_integer()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    // convert the values out of json and put them in the ouput variable
    info->pixel_density = jsonPixDens.get<uint32_t>();
    info->refresh_rate = jsonRefresh.get<uint32_t>();
  } catch (nl::json::exception) {     // out_of_range or type_error
      return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return err;
}


uint64_t Projector::keystone_json2c(void *obj, hippo::Keystone *ks) {
  uint64_t err = HIPPO_OK;
  if (obj == NULL || ks == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *ksData = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!ksData->is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    auto type = ksData->at("type");
    auto ksValue = ksData->at("value");
    if (!type.is_string() || !ksValue.is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    std::string typeStr = type.get<std::string>();
    if (typeStr.compare("1d") == 0) {
      // set the keystone type in the output variable
      ks->type = hippo::KeystoneType::KEYSTONE_1D;
      // get the pitch and the display area object
      auto pitch = ksValue.at("pitch");
      auto dispArea = ksValue.at("display_area");
      // check the types
      if (!dispArea.is_object() || !pitch.is_number_float()) {
        return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
      }
      // get the items in the display area object
      auto dispX = dispArea.at("x");
      auto dispY = dispArea.at("y");
      auto dispWidth = dispArea.at("width");
      auto dispHeight = dispArea.at("height");

      // check the types of the display area objects
      if (!dispX.is_number_integer() ||
        !dispY.is_number_integer() ||
        !dispWidth.is_number_integer() ||
        !dispHeight.is_number_integer()) {
        return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
      }
      // get the values out of json and put them into the return
      ks->value_1d.pitch = pitch.get<float>();
      ks->value_1d.display_area.height = dispHeight.get<uint16_t>();
      ks->value_1d.display_area.width = dispWidth.get<uint16_t>();
      ks->value_1d.display_area.x = dispX.get<uint16_t>();
      ks->value_1d.display_area.y = dispY.get<uint16_t>();
    } else if (typeStr.compare("2d") == 0) {
      // set the keystone type in the output variable
      ks->type = hippo::KeystoneType::KEYSTONE_2D;
      auto tl = ksValue.at("top_left");
      auto tr = ksValue.at("top_right");
      auto bl = ksValue.at("bottom_left");
      auto br = ksValue.at("bottom_right");
      auto tm = ksValue.at("top_middle");
      auto bm = ksValue.at("bottom_middle");
      auto lm = ksValue.at("left_middle");
      auto rm = ksValue.at("right_middle");
      auto center = ksValue.at("center");

      if (!tl.is_object() || !tr.is_object() ||
          !bl.is_object() || !br.is_object() ||
          !tm.is_object() || !bm.is_object() ||
          !lm.is_object() || !rm.is_object() ||
          !center.is_object()) {
          return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
      }
      if (err = parsePointJson(reinterpret_cast<void*>(&tl),
                               &ks->value_2d.top_left)
        != HIPPO_OK) {
        return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
      }
      if (err = parsePointJson(reinterpret_cast<void*>(&tr),
                               &ks->value_2d.top_right)
        != HIPPO_OK) {
        return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
      }
      if (err = parsePointJson(reinterpret_cast<void*>(&bl),
                               &ks->value_2d.bottom_left)
        != HIPPO_OK) {
        return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
      }
      if (err = parsePointJson(reinterpret_cast<void*>(&br),
                               &ks->value_2d.bottom_right)
        != HIPPO_OK) {
        return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
      }
      if (err = parsePointJson(reinterpret_cast<void*>(&tm),
                               &ks->value_2d.top_middle)
        != HIPPO_OK) {
        return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
      }
      if (err = parsePointJson(reinterpret_cast<void*>(&bm),
                               &ks->value_2d.bottom_middle)
        != HIPPO_OK) {
        return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
      }
      if (err = parsePointJson(reinterpret_cast<void*>(&lm),
                               &ks->value_2d.left_middle)
        != HIPPO_OK) {
        return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
      }
      if (err = parsePointJson(reinterpret_cast<void*>(&rm),
                               &ks->value_2d.right_middle)
        != HIPPO_OK) {
        return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
      }
      if (err = parsePointJson(reinterpret_cast<void*>(&center),
                               &ks->value_2d.center)
        != HIPPO_OK) {
        return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
      }
    } else {          // returned type is not "1d" or "2d" - error
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return err;
}

uint64_t Projector::keystone_c2json(const hippo::Keystone &ks,
                                    void *obj) {
  if (obj == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  nl::json params;
  if (ks.type == KeystoneType::KEYSTONE_1D) {
    params = {
      { "type", "1d" },
      { "value", {
          { "pitch", ks.value_1d.pitch},
          { "display_area", {
              { "x", ks.value_1d.display_area.x},
              { "y", ks.value_1d.display_area.y},
              {"width", ks.value_1d.display_area.width},
              {"height", ks.value_1d.display_area.height}
            }
          }
        }
      }
    };
  } else if (ks.type == KeystoneType::KEYSTONE_2D) {
    params = {
      { "type", "2d" },
      { "value", {
            { "top_left", {
                { "x", ks.value_2d.top_left.x },
                { "y", ks.value_2d.top_left.y },
              }
            },
            { "top_right", {
                { "x", ks.value_2d.top_right.x },
                { "y", ks.value_2d.top_right.y },
              }
            },
            { "bottom_left", {
                { "x", ks.value_2d.bottom_left.x },
                { "y", ks.value_2d.bottom_right.y },
              }
            },
            { "bottom_right", {
                { "x", ks.value_2d.bottom_right.x },
                { "y", ks.value_2d.bottom_right.y },
              }
            },
            { "top_middle", {
                { "x", ks.value_2d.top_middle.x },
                { "y", ks.value_2d.top_middle.y },
              }
            },
            { "bottom_middle", {
                { "x", ks.value_2d.bottom_middle.x },
                { "y", ks.value_2d.bottom_middle.y },
              }
            },
            { "left_middle", {
                { "x", ks.value_2d.left_middle.x },
                { "y", ks.value_2d.left_middle.y },
              }
            },
            { "right_middle", {
                { "x", ks.value_2d.right_middle.x },
                { "y", ks.value_2d.right_middle.y },
              }
            },
            { "center", {
                { "x", ks.value_2d.center.x },
                { "y", ks.value_2d.center.y },
              }
            }
        }
      }
    };
  } else {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
  }
  reinterpret_cast<nl::json*>(obj)->push_back(params);
  return HIPPO_OK;
}

uint64_t Projector::ledtimes_json2c(void *obj,
  hippo::ProjectorLedTimes *ledtimes) {
  if (obj == NULL || ledtimes == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *ledTimeData = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!ledTimeData->is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // get the individual items from json
    auto on = ledTimeData->at("on");
    auto flash = ledTimeData->at("flash");
    auto grayscale = ledTimeData->at("grayscale");
    // check for proper type
    if (!on.is_number_float() ||
        !flash.is_number_float() ||
        !grayscale.is_number_float()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // put the items into the return
    ledtimes->on = on.get<float>();
    ledtimes->flash = flash.get<float>();
    ledtimes->grayscale = grayscale.get<float>();
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return HIPPO_OK;
}

uint64_t Projector::mfgData_json2c(void *obj,
                                   hippo::ManufacturingData *mfgdata) {
  uint64_t err = HIPPO_OK;
  if (obj == NULL || mfgdata == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *jsonMfgData = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!jsonMfgData->is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // get the individual items from json
    auto blue = jsonMfgData->at("blue");
    auto green = jsonMfgData->at("green");
    auto red = jsonMfgData->at("red");
    auto keystone = jsonMfgData->at("keystone");
    auto irCorners = jsonMfgData->at("ir_corners");
    auto hiresCorners = jsonMfgData->at("hires_corners");
    auto exposure = jsonMfgData->at("exposure");
    auto gain = jsonMfgData->at("gain");

    // check for proper type for the objects
    if (!irCorners.is_object() ||
        !hiresCorners.is_object() ||
        !keystone.is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // parse the ir corners
    if (HIPPO_OK != (err = parseCornersJson(reinterpret_cast<void*>(&irCorners),
                                            &mfgdata->ir_corners))) {
      return err;
    }
    // parse the high res camera corners
    if (HIPPO_OK !=
       (err = parseCornersJson(reinterpret_cast<void*>(&hiresCorners),
                               &mfgdata->hires_corners))) {
      return err;
    }
    // parse the keystone
    if (HIPPO_OK != (err = keystone_json2c(reinterpret_cast<void*>(&keystone),
                                             &mfgdata->keystone))) {
      return err;
    }

    // check for proper type for gain, exposure, r,g,b
    if (!red.is_number_integer() ||
        !green.is_number_integer() ||
        !blue.is_number_integer() ||
        !gain.is_number_integer() ||
        !exposure.is_number_integer()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // put the items into the return
    mfgdata->red = red.get<uint32_t>();
    mfgdata->green = green.get<uint32_t>();
    mfgdata->blue = blue.get<uint32_t>();
    mfgdata->gain = gain.get<uint32_t>();
    mfgdata->exposure = exposure.get<uint32_t>();
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return HIPPO_OK;
}

uint64_t Projector::rectangle_json2c(void *obj, hippo::Rectangle *rect) {
  uint64_t err = HIPPO_OK;
  if (obj == NULL || rect == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *jsonRect = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!jsonRect->is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // get the items in the json object
    auto x = jsonRect->at("x");
    auto y = jsonRect->at("y");
    auto width = jsonRect->at("width");
    auto height = jsonRect->at("height");

    // check the types of the display area objects
    if (!x.is_number_integer() ||
        !y.is_number_integer() ||
        !width.is_number_integer() ||
        !height.is_number_integer()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // get the values out of json and put them into the return
    rect->height = height.get<uint16_t>();
    rect->width = width.get<uint16_t>();
    rect->x = x.get<uint16_t>();
    rect->y = y.get<uint16_t>();
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return HIPPO_OK;
}

uint64_t Projector::state_json2c(void *obj, hippo::ProjectorState *state) {
  const char *ProjectorState_str[
    static_cast<uint32_t>(hippo::ProjectorState::burn_in) + 1] = {
    "off", "standby", "on", "overtemp", "flashing",
    "transition_to_on", "transition_to_st", "hw_fault", "initializing",
    "on_no_source", "transition_to_flash", "transition_to_grayscale",
    "grayscale", "fw_upgrade", "burn_in",
  };

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
    idx = str_to_idx(ProjectorState_str, stateStr.c_str(),
      static_cast<uint32_t>(hippo::ProjectorState::off),
      static_cast<uint32_t>(hippo::ProjectorState::burn_in));
    if (idx < 0) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    *state = static_cast<hippo::ProjectorState>(idx);
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return HIPPO_OK;
}

const char *SolidColor_str[] = { "off", "black", "red", "green", "blue",
"cyan", "magenta", "yellow", "white"
};

uint64_t Projector::solid_color_c2json(const hippo::SolidColor &color,
                                       void *obj) {
  uint64_t err = HIPPO_OK;
  if (obj == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  reinterpret_cast<nl::json*>(obj)->push_back(
                            SolidColor_str[static_cast<uint32_t>(color)]);
  return HIPPO_OK;
}

uint64_t Projector::solid_color_json2c(void *obj,
                                       hippo::SolidColor *color) {
  if (obj == NULL || color == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *jsonColor = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!jsonColor->is_string()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // get the individual items from json
    std::string colorStr = jsonColor->get<std::string>();

    int32_t idx;
    idx = str_to_idx(SolidColor_str, colorStr.c_str(),
      static_cast<uint32_t>(hippo::SolidColor::off),
      static_cast<uint32_t>(hippo::SolidColor::white));
    if (idx < 0) {
      return -1;
    }
    *color = static_cast<hippo::SolidColor>(idx);
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return HIPPO_OK;
}

const char *Illuminant_str[] = { "d50", "d65", "d75", "custom" };

uint64_t Projector::white_point_c2json(const hippo::WhitePoint &wp,
                                       void *obj) {
  if (obj == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }

  nl::json params = {
      { "name", Illuminant_str[static_cast<uint32_t>(wp.name)] },
      { "value", {
          { "x", wp.value.x },
          { "y", wp.value.y },
        }
      }
    };
  reinterpret_cast<nl::json*>(obj)->push_back(params);
  return HIPPO_OK;
}

uint64_t Projector::white_point_json2c(void *obj,
                                     hippo::WhitePoint *wp) {
  if (obj == NULL || wp == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *jsonWp = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!jsonWp->is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    auto name = jsonWp->at("name");
    auto value = jsonWp->at("value");


    if (!name.is_string() || !value.is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }

    // get the individual items from json
    std::string nameStr = name.get<std::string>();

    int32_t idx;
    idx = str_to_idx(Illuminant_str, nameStr.c_str(),
                     static_cast<uint32_t>(hippo::Illuminant::d50),
                     static_cast<uint32_t>(hippo::Illuminant::custom));
    if (idx < 0) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    wp->name = static_cast<hippo::Illuminant>(idx);

    // now get the x and y values
    auto x = value.at("x");
    auto y = value.at("y");
    if (!x.is_number_float() || !y.is_number_float()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    wp->value.x = x.get<float>();
    wp->value.y = y.get<float>();
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return HIPPO_OK;
}

bool Projector::HasRegisteredCallback() {
  return (NULL != callback_);
}

const char *ProjectorNotification_str[] = {
  "on_close", "on_device_connected", "on_device_disconnected",
  "on_factory_default", "on_open", "on_open_count", "on_resume", "on_suspend",
  "on_sohal_disconnected", "on_sohal_connected",
  "on_brightness", "on_keystone",
  "on_solid_color", "on_state",  "on_structured_light_mode",
  "on_white_point",
};

void Projector::ProcessSignal(char *method, void *obj) {
  // fprintf(stderr, "[projector]: %s, %p\n", method, obj);

  if (NULL == callback_) {
    return;
  }
  uint64_t err = 0LL;
  int32_t idx = 0;
  idx = str_to_idx(ProjectorNotification_str,
                   method,
                   static_cast<uint32_t>(
                       hippo::ProjectorNotification::on_close),
                   static_cast<uint32_t>(
                       hippo::ProjectorNotification::on_white_point));
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
  ProjectorNotificationParam param;
  param.type = static_cast<hippo::ProjectorNotification>(idx);

  switch (static_cast<hippo::ProjectorNotification>(idx)) {
    case ProjectorNotification::on_brightness:
      param.on_brightness = v.get<uint32_t>();
      break;

    case ProjectorNotification::on_keystone:
      err = keystone_json2c(obj, &param.on_keystone);
      break;

    case ProjectorNotification::on_solid_color:
      err = solid_color_json2c(obj, &param.on_solid_color);
      break;

    case ProjectorNotification::on_state:
      err = state_json2c(obj, &param.on_state);
      break;

    case ProjectorNotification::on_structured_light_mode:
      param.on_structured_light_mode = v.get<bool>();
      break;

    case ProjectorNotification::on_white_point:
      err = white_point_json2c(obj, &param.on_white_point);
      break;

    case ProjectorNotification::on_open_count:
      param.on_open_count = v.get<uint32_t>();
      break;

    default:
      break;
  }
  if (!err) {
    (*callback_)(param, callback_data_);
  }
}

uint64_t Projector::parseCornerJson(void *jsonCorner, PointFloats *cCorner) {
  if (jsonCorner == NULL || cCorner == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *jCorner = reinterpret_cast<const nl::json*>(jsonCorner);
  try {
    if (!jCorner->is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    auto jx = jCorner->at("x");
    auto jy = jCorner->at("y");
    if (!jx.is_number_float() || !jy.is_number_float()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    cCorner->x = jx.get<float>();
    cCorner->y = jy.get<float>();
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return HIPPO_OK;
}

uint64_t Projector::parseCornersJson(void *jsonCorners, Corners *cCorners) {
  if (jsonCorners == NULL || cCorners == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  uint64_t err = HIPPO_OK;
  const nl::json *jCorners = reinterpret_cast<const nl::json*>(jsonCorners);
  try {
    if (!jCorners->is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    auto tl = jCorners->at("top_left");
    auto tr = jCorners->at("top_right");
    auto bl = jCorners->at("bottom_left");
    auto br = jCorners->at("bottom_right");
    if (!tl.is_object() || !tr.is_object() ||
      !bl.is_object() || !br.is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    if (HIPPO_OK !=
        (err = parseCornerJson(reinterpret_cast<void*>(&tl),
                               &cCorners->top_left))) {
      return err;
    }
    if (HIPPO_OK !=
      (err = parseCornerJson(reinterpret_cast<void*>(&tr),
        &cCorners->top_right))) {
      return err;
    }
    if (HIPPO_OK !=
      (err = parseCornerJson(reinterpret_cast<void*>(&bl),
        &cCorners->bottom_left))) {
      return err;
    }
    if (HIPPO_OK !=
      (err = parseCornerJson(reinterpret_cast<void*>(&br),
        &cCorners->bottom_right))) {
      return err;
    }
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return HIPPO_OK;
}

uint64_t Projector::parsePointJson(void *jsonPoint, Point *cPoint) {
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

}   // namespace hippo
