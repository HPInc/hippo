
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#include <windows.h>    // for Sleep()
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "include/projector.h"

extern void print_error(uint64_t err);
extern void print_device_info(const hippo::DeviceInfo &info);

void print_keystone(const hippo::Keystone &ks);
void projector_notification(const hippo::ProjectorNotificationParam &param,
                            void *data);
extern void print_temperatures(const hippo::TemperatureInfo * temps,
                               uint64_t num_temperatures);

void print_manufacturing_data(hippo::ManufacturingData *mfgData) {
  fprintf(stderr, "Projector Manufacturing Data:\n");
  fprintf(stderr, "\tR,G,B = (%i,%i,%i)\n", mfgData->red, mfgData->green,
          mfgData->blue);
  fprintf(stderr, "\texposure = %i, gain = %i\n", mfgData->exposure,
          mfgData->gain);
  fprintf(stderr, "\tHighres Corners:\n");
  fprintf(stderr, "\t\tTL = (%f,%f)\tTR = (%f,%f)\n",
          mfgData->hires_corners.top_left.x,
          mfgData->hires_corners.top_left.y,
          mfgData->hires_corners.top_right.x,
          mfgData->hires_corners.top_right.y);
  fprintf(stderr, "\t\tBL = (%f,%f)\tBR = (%f,%f)\n",
          mfgData->hires_corners.bottom_left.x,
          mfgData->hires_corners.bottom_left.y,
          mfgData->hires_corners.bottom_right.x,
          mfgData->hires_corners.bottom_right.y);
  fprintf(stderr, "\tIR Corners:\n");
  fprintf(stderr, "\t\tTL = (%f,%f)\tTR = (%f,%f)\n",
          mfgData->ir_corners.top_left.x, mfgData->ir_corners.top_left.y,
          mfgData->ir_corners.top_right.x, mfgData->ir_corners.top_right.y);
  fprintf(stderr, "\t\tBL = (%f,%f)\tBR = (%f,%f)\n",
          mfgData->ir_corners.bottom_left.x,
          mfgData->ir_corners.bottom_left.y,
          mfgData->ir_corners.bottom_right.x,
          mfgData->ir_corners.bottom_right.y);
  fprintf(stderr, "\tKeystone:\n");
  print_keystone(mfgData->keystone);
}

void print_projector_specific_info(hippo::ProjectorSpecificInfo *projInfo) {
  fprintf(stderr, "Projector Specific Info:\n");
  fprintf(stderr, "  Asic Version: %d.%d.%d.%d\n",
          projInfo->asic_version.major,
          projInfo->asic_version.minor,
          projInfo->asic_version.patch_msb,
          projInfo->asic_version.patch_lsb);
  fprintf(stderr, "  Column Serial: %s\n", projInfo->column_serial);
  fprintf(stderr, "  EEPROM Version: %d\n", projInfo->eeprom_version);
  fprintf(stderr, "  Flash Version: %d.%d.%d.%d\n",
          projInfo->flash_version.major,
          projInfo->flash_version.minor,
          projInfo->flash_version.patch_msb,
          projInfo->flash_version.patch_lsb);
  fprintf(stderr, "  Geo FW Version: %d.%d.%c.%d\n",
          projInfo->geo_fw_version.major,
          projInfo->geo_fw_version.minor,
          projInfo->geo_fw_version.package,
          projInfo->geo_fw_version.test_release);
  fprintf(stderr, "  HW Version: %d\n", projInfo->hw_version);
  fprintf(stderr, "  MFG Time: %s\n", projInfo->manufacturing_time);
}

uint64_t TestProjector(hippo::Projector *projector) {
  uint64_t err;
  fprintf(stderr, "##################################\n");
  fprintf(stderr, "  Now Testing Projector Commands\n");
  fprintf(stderr, "##################################\n");

  ADD_FILE_TO_MAP();   // will add this file to the file/error map

  // subscribe
  if (err = projector->subscribe(&projector_notification,
                                 reinterpret_cast<void*>(projector))) {
    print_error(err);
  }

  bool bConnected;
  if (err = projector->is_device_connected(&bConnected)) {
    return err;
  }
  if (bConnected) {
    fprintf(stderr, "Projector is connected\n");
  } else {
    fprintf(stderr, "No projector is connected -- not running test\n");
    return MAKE_HIPPO_ERROR(hippo::HIPPO_DEVICE,
                            hippo::HIPPO_DEV_NOT_FOUND);
  }

  // open
  uint32_t open_count = 0;
  if (err = projector->open(&open_count)) {
    return err;
  }
  fprintf(stderr, "projector.open(): open_count: %d\n", open_count);
  // open_count
  if (err = projector->open_count(&open_count)) {
    print_error(err);
  }
  fprintf(stderr, "projector.open_count(): %d\n", open_count);
  // info
  hippo::HardwareInfo hwinfo;
  if (err = projector->hardware_info(&hwinfo)) {
    print_error(err);
  } else {
    fprintf(stderr, "Input Resolution: (%i,%i)\n",
            hwinfo.input_resolution.width, hwinfo.input_resolution.height);
    fprintf(stderr, "Output Resolution: (%i,%i)\n",
            hwinfo.output_resolution.width, hwinfo.output_resolution.height);
    fprintf(stderr, "Pixel Density: %i\n", hwinfo.pixel_density);
    fprintf(stderr, "Refresh Rate: %i\n", hwinfo.refresh_rate);
  }

  hippo::TemperatureInfo *tempInfo;
  uint64_t num_temps;
  if (err = projector->temperatures(&tempInfo, &num_temps)) {
    print_error(err);
  } else {
    fprintf(stderr, "There are %u items with temperatures\n",
            static_cast<int>(num_temps));
    // print out the temperatures
    print_temperatures(tempInfo, num_temps);
  }
  projector->free_temperatures(tempInfo);


  // get manufacturing data
  hippo::ManufacturingData mfgData;
  if (err = projector->manufacturing_data(&mfgData)) {
    print_error(err);
  } else {
    print_manufacturing_data(&mfgData);
  }

  hippo::ProjectorSpecificInfo projInfo;
  if (err = projector->device_specific_info(&projInfo)) {
    print_error(err);
  } else {
    print_projector_specific_info(&projInfo);
  }
  projector->free_projector_specific_info(&projInfo);

  hippo::Rectangle monitorCoords;
  if (err = projector->monitor_coordinates(&monitorCoords)) {
    return err;
  }
  fprintf(stderr, "Monitor coordinates are (%d,%d,%d,%d)",
          monitorCoords.x, monitorCoords.y,
          monitorCoords.width, monitorCoords.height);

  // led times
  hippo::ProjectorLedTimes ledtimes;
  if (err = projector->led_times(&ledtimes)) {
    print_error(err);
  } else {
    fprintf(stderr, "LED Times:\n");
    fprintf(stderr, "\tOn:\t\t%f\n", ledtimes.on);
    fprintf(stderr, "\tGrayscale:\t%f\n", ledtimes.grayscale);
    fprintf(stderr, "\tFlash:\t\t%f\n", ledtimes.flash);
    fprintf(stderr, "Refresh Rate: %i\n", hwinfo.refresh_rate);
  }
  // state
  hippo::ProjectorState state;
  if (err = projector->state(&state)) {
    print_error(err);
  }
  fprintf(stderr, "projector.state(): %d\n", state);
  // on
  fprintf(stderr, "*** going to call projector.on\n");
  if (err = projector->on()) {
    print_error(err);
  }
  fprintf(stderr, "projector.on()\n");
  // wait for projector state to go to "on state"
  Sleep(500);
  // get state
  if (err = projector->state(&state)) {
    print_error(err);
  }
  // check state
  if (state != hippo::ProjectorState::on) {
    fprintf(stderr, "ERROR - projector should be in ON state!\n");
    return MAKE_HIPPO_ERROR(hippo::HIPPO_DEVICE,
                            hippo::HIPPO_WRONG_STATE_ERROR);
  }
  fprintf(stderr, "projector.state(): %d\n", state);
  // solid color
  hippo::SolidColor color_set, color_get;
  if (err = projector->solid_color(&color_get)) {
    print_error(err);
  }
  fprintf(stderr, "projector.solid_color(): %d\n", color_get);
  // switch to red
  color_set = hippo::SolidColor::red;
  if (err = projector->solid_color(color_set)) {
    print_error(err);
  }
  fprintf(stderr, "projector.solid_color(%d)\n", color_set);
  if (err = projector->solid_color(&color_get)) {
    print_error(err);
  }
  fprintf(stderr, "projector.solid_color(): %d\n", color_get);
  if (color_set != color_get) {
    fprintf(stderr,
            "ERROR - projector isn't reporting red as the set color!\n");
    return MAKE_HIPPO_ERROR(hippo::HIPPO_DEVICE,
                            hippo::HIPPO_WRONG_STATE_ERROR);
  }
  color_set = hippo::SolidColor::off;
  if (err = projector->solid_color(color_set, &color_get)) {
    print_error(err);
  }
  fprintf(stderr, "projector.solid_color(%d): %d\n", color_set, color_get);
  if (color_set != color_get) {
    fprintf(stderr, "ERROR - projector didn't disable solid_color!\n");
    return MAKE_HIPPO_ERROR(hippo::HIPPO_DEVICE,
                            hippo::HIPPO_WRONG_STATE_ERROR);
  }
  // flash
  bool flash = true;
  if (err = projector->flash(flash)) {
    print_error(err);
  }
  fprintf(stderr, "projector.flash(%d)\n", flash);
  Sleep(1000);
  if (err = projector->on()) {
    print_error(err);
  }
  Sleep(1000);
  uint32_t flash_time = 0;
  if (err = projector->flash(flash, &flash_time)) {
    print_error(err);
  }
  fprintf(stderr, "projector.flash(%d):  %d seconds remain\n",
          flash, flash_time);
  Sleep(3000);
  // this should get the time but not flash again
  if (err = projector->flash(flash, &flash_time)) {
    print_error(err);
  }
  fprintf(stderr, "projector.flash(%d):  %d seconds remain\n",
          flash, flash_time);
  if (err = projector->on()) {
    print_error(err);
  }
  // Sleep(1000);
  // flash = false;
  // if (err = projector->flash(flash, &flash_time)) {
  //  print_error(err);
  // }
  // fprintf(stderr, "projector.flash(%d):  %d seconds remain\n",
  //        flash, flash_time);
  // white point
  hippo::WhitePoint wp_set, wp_get;
  if (err = projector->white_point(&wp_get)) {
    print_error(err);
  }
  fprintf(stderr, "projector.white_point():  %d: x:%f, y:%f\n",
          wp_get.name, wp_get.value.x, wp_get.value.y);
  wp_set = { hippo::Illuminant::d50, {0., 0.}};
  if (err = projector->white_point(wp_set)) {
    print_error(err);
  }
  fprintf(stderr, "projector.white_point(%d: x:%f, y:%f)\n",
          wp_set.name, wp_set.value.x, wp_set.value.y);
  if (err = projector->white_point(&wp_get)) {
    print_error(err);
  }
  fprintf(stderr, "projector.white_point():  %d: x:%f, y:%f\n",
          wp_get.name, wp_get.value.x, wp_get.value.y);
  wp_set = { hippo::Illuminant::d65, {0., 0.}};
  if (err = projector->white_point(wp_set, &wp_get)) {
    print_error(err);
  }
  fprintf(stderr, "projector.white_point(%d: x:%f, y:%f): %d: x:%f, y:%f\n",
          wp_set.name, wp_set.value.x, wp_set.value.y,
          wp_get.name, wp_get.value.x, wp_get.value.y);
  // get the keystone
  hippo::Keystone ks, ks2, ks0;
  if (err = projector->keystone(&ks)) {
    print_error(err);
  }
  fprintf(stderr, "projector.keystone()\n");
  print_keystone(ks);
  // set the keystone and get the return value back
  memset(&ks0, 0, sizeof(hippo::Keystone));
  ks0.type = ks.type;
  if (err = projector->keystone(ks0, &ks2)) {
    print_error(err);
  }
  fprintf(stderr, "projector.keystone(keystone): keystone\n");
  print_keystone(ks2);
  if (err = projector->keystone(ks)) {
    print_error(err);
  }
  if (ks.type == hippo::KeystoneType::KEYSTONE_1D) {
    ks.value_1d.pitch = 100;   // un-comment to test for error in Gen1
    if (err = projector->keystone(ks)) {
      print_error(err);
    }
    fprintf(stderr, "projector.keystone(keystone)\n");
    if (err = projector->keystone(&ks)) {
      print_error(err);
    }
    fprintf(stderr, "projector.keystone()\n");
    print_keystone(ks);
  }

  // grayscale
  if (err = projector->grayscale()) {
    print_error(err);
  }
  // state
  if (err = projector->state(&state)) {
    print_error(err);
  }
  fprintf(stderr, "projector.state(): %d\n", state);
  // structured_light_mode
  bool slm;
  if (err = projector->structured_light_mode(&slm)) {
    print_error(err);
  }
  fprintf(stderr, "projector.structured_light_mode(): %d\n", slm);
  if (err = projector->structured_light_mode(true, &slm)) {
    print_error(err);
  }
  fprintf(stderr, "projector.structured_light_mode(1): %d\n", slm);
  if (err = projector->structured_light_mode(false)) {
    print_error(err);
  }
  fprintf(stderr, "projector.structured_light_mode(0)\n");
  if (err = projector->structured_light_mode(&slm)) {
    print_error(err);
  }
  fprintf(stderr, "projector.structured_light_mode(): %d\n", slm);
  // off
  if (err = projector->off()) {
    print_error(err);
  }
  fprintf(stderr, "projector.off()\n");
  // calibration_data
  hippo::CalibrationData cal;
  if (err = projector->calibration_data(&cal)) {
    print_error(err);
  } else {
    fprintf(stderr, "projector.cam_cal:\n'%s'\n", cal.cam_cal);
    fprintf(stderr, "projector.cam_cal_hd:\n'%s'\n", cal.cam_cal_hd);
    fprintf(stderr, "projector.proj_cal:\n'%s'\n", cal.proj_cal);
    fprintf(stderr, "projector.proj_cal_hd:\n'%s'\n", cal.proj_cal_hd);
    // now free the calibration data allocated in the
    // projector->calibration_data() function
    projector->free_calibration_data(&cal);
  }
  // factory_default
  if (err = projector->factory_default()) {
    print_error(err);
  }
  fprintf(stderr, "projector.factory_default()\n");
  // info
  hippo::DeviceInfo info;
  if (err = projector->info(&info)) {
    print_error(err);
  } else {
    print_device_info(info);
    // free the device info that was allocated in the projector->info()
    // function call
    projector->free_device_info(&info);
  }
  // close
  if (err = projector->close(&open_count)) {
    return err;
  }
  fprintf(stderr, "projector.close(): open_count: %d\n", open_count);
  // unsubscribe
  if (err = projector->unsubscribe()) {
    print_error(err);
  }
  return 0LL;
}

void print_keystone(const hippo::Keystone &ks) {
  if (hippo::KeystoneType::KEYSTONE_1D == ks.type) {
    fprintf(stderr, "pitch: %f, display_area:[x:%d, y:%d, w:%d, h:%d]\n",
            ks.value_1d.pitch,
            ks.value_1d.display_area.x,
            ks.value_1d.display_area.y,
            ks.value_1d.display_area.width,
            ks.value_1d.display_area.height);
  } else  if (hippo::KeystoneType::KEYSTONE_2D == ks.type) {
    fprintf(stderr,
            "\ntop_left:{x:%d,y:%d},\t\ttop_middle:{x:%d,y:%d},\t"
            "\ttop_right:{x:%d,y:%d}"
            "\nmiddle_left:{x:%d,y:%d},\t\tcenter:{x:%d,y:%d},\t"
            "\tmiddle_right:{x:%d,y:%d}"
            "\nbottom_left:{x:%d,y:%d},\t\tbottom_middle:{x:%d,y:%d},"
            "\tbottom_right:{x:%d,y:%d}\n\n",
            ks.value_2d.top_left.x,
            ks.value_2d.top_left.y,
            ks.value_2d.top_middle.x,
            ks.value_2d.top_middle.y,
            ks.value_2d.top_right.x,
            ks.value_2d.top_right.y,
            ks.value_2d.left_middle.x,
            ks.value_2d.left_middle.y,
            ks.value_2d.center.x,
            ks.value_2d.center.y,
            ks.value_2d.right_middle.x,
            ks.value_2d.right_middle.y,
            ks.value_2d.bottom_left.x,
            ks.value_2d.bottom_left.y,
            ks.value_2d.bottom_middle.x,
            ks.value_2d.bottom_middle.y,
            ks.value_2d.bottom_right.x,
            ks.value_2d.bottom_right.y);
  }
}

const char *ProjectorState_str[
    static_cast<uint32_t>(hippo::ProjectorState::burn_in)+1] = {
  "off", "standby", "on", "overtemp", "flashing",
  "transition_to_on", "transition_to_st", "hw_fault", "initializing",
  "on_no_source", "transition_to_flash", "transition_to_grayscale",
  "grayscale", "fw_upgrade", "burn_in",
};

void projector_notification(const hippo::ProjectorNotificationParam &param,
                            void *data) {
  switch (param.type) {
    case hippo::ProjectorNotification::on_close:
      fprintf(stderr, "[SIGNAL]: projector.on_close\n");
      break;
    case hippo::ProjectorNotification::on_device_connected:
      fprintf(stderr, "[SIGNAL]: projector.on_device_connected\n");
      break;
    case hippo::ProjectorNotification::on_device_disconnected:
      fprintf(stderr, "[SIGNAL]: projector.on_device_disconnected\n");
      break;
    case hippo::ProjectorNotification::on_factory_default:
      fprintf(stderr, "[SIGNAL]: projector.on_factory_default\n");
      break;
    case hippo::ProjectorNotification::on_open:
      fprintf(stderr, "[SIGNAL]: projector.on_open\n");
      break;
    case hippo::ProjectorNotification::on_resume:
      fprintf(stderr, "[SIGNAL]: projector.on_resume\n");
      break;
    case hippo::ProjectorNotification::on_suspend:
      fprintf(stderr, "[SIGNAL]: projector.on_suspend\n");
      break;
    case hippo::ProjectorNotification::on_sohal_disconnected:
      fprintf(stderr, "[SIGNAL]: projector.on_sohal_disconnected\n");
      break;
    case hippo::ProjectorNotification::on_sohal_connected:
      fprintf(stderr, "[SIGNAL]: projector.on_sohal_connected\n");
      break;
    case hippo::ProjectorNotification::on_brightness:
      fprintf(stderr, "[SIGNAL]: projector.on_brightness: %d\n",
              param.on_brightness);
      break;
    case hippo::ProjectorNotification::on_keystone:
      fprintf(stderr, "[SIGNAL]: projector.on_keystone: ");
      print_keystone(param.on_keystone);
      break;
    case hippo::ProjectorNotification::on_solid_color:
      fprintf(stderr, "[SIGNAL]: projector.on_solid_color: %d\n",
              param.on_solid_color);
      break;
    case hippo::ProjectorNotification::on_state:
      fprintf(stderr, "[SIGNAL]: projector.on_state: %s\n",
              ProjectorState_str[static_cast<uint32_t>(param.on_state)]);
      break;
    case hippo::ProjectorNotification::on_structured_light_mode:
      fprintf(stderr, "[SIGNAL]: projector.on_structured_light_mode: %d\n",
              param.on_structured_light_mode);
      break;
    case hippo::ProjectorNotification::on_white_point:
      fprintf(stderr, "[SIGNAL]: projector.on_white_point:  %d: x:%f, y:%f\n",
              param.on_white_point.name,
              param.on_white_point.value.x, param.on_white_point.value.y);
      break;
    default:
      break;
  }
}
