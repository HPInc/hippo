
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#include <cstdio>
#include <cstring>
#include "include/system.h"

const char *PowerState_str[] = {
  "display on", "display off", "display dimmed", "suspend",
  "resume", "logoff", "shutdown",
};

const char *SessionChangeEvent_str[] = {
  "console connect", "console disconnect",
  "session logon", "session logoff",
  "session lock", "session unlock",
};

const char *SessionState_str[] = {
  "locked", "unlocked", "unknown",
};

extern void print_error(uint64_t err);
extern void print_temperatures(const hippo::TemperatureInfo * temps,
                               uint64_t num_temperatures);

void system_notification(const hippo::SystemNotificationParam &param,
                         void *data);


void print_supported_devs(hippo::SupportedDevice * devices,
                          uint64_t num_devices) {
  for (uint64_t i = 0; i < num_devices; i++) {
    fprintf(stderr, "%s\n", devices[i].name);
  }
}

void print_camera_stream(hippo::CameraStream *stream) {
  switch (stream->name) {
    case hippo::CameraNameType::depthcamera:
      fprintf(stderr, "Depthcamera at Index %i ", stream->index);
      break;
    case hippo::CameraNameType::hirescamera:
      fprintf(stderr, "High Resolution Camera at Index %i ", stream->index);
      break;
  }

  switch (stream->stream) {
    case hippo::CameraStreamType::depth:
      fprintf(stderr, "Depth Stream\n");
      break;
    case hippo::CameraStreamType::rgb:
      fprintf(stderr, "Color Stream\n");
      break;
    case hippo::CameraStreamType::ir:
      fprintf(stderr, "IR Stream\n");
      break;
    case hippo::CameraStreamType::points:
      fprintf(stderr, "Points Stream\n");
      break;
  }
}

void print_camera_params(hippo::CameraParameters *params) {
  print_camera_stream(&params->camera);
  fprintf(stderr, "\tCalibration Resolution (%i,%i)\n",
          static_cast<int>(params->calibration_resolution.width),
          static_cast<int>(params->calibration_resolution.height));
  fprintf(stderr, "\tLens Distortion (%f, %f),\n"
          "kappa = (%.5e,\n\t %.5e,\n\t %.5e,\n\t %.5e,\n\t %.5e,\n\t %.5e)\n"
          "p = \t(%.5e,\n\t %.5e)\n",
          params->lens_distortion.center.x, params->lens_distortion.center.y,
          params->lens_distortion.kappa[0], params->lens_distortion.kappa[1],
          params->lens_distortion.kappa[2], params->lens_distortion.kappa[3],
          params->lens_distortion.kappa[4], params->lens_distortion.kappa[5],
          params->lens_distortion.p[0], params->lens_distortion.p[1]);
  fprintf(stderr, "Focal Length (%f,%f)\n", params->focal_length.x,
          params->focal_length.y);
}

void print_3d_mapping(hippo::Camera3DMapping * map) {
  fprintf(stderr, "Mapping from :\n");
  print_camera_params(&map->from);
  fprintf(stderr, "\nMapping to :\n");
  print_camera_params(&map->to);
  fprintf(stderr,
          "\nTransformation Matrix between the two streams listed above:\n\t"
          "[[%f,\t%f,\t%f,\t%f]\n\t [%f,\t%f,\t%f,\t%f]\n\t "
          "[%f,\t%f,\t%f,\t%f]\n\t [%f,\t%f,\t%f,\t%f]]\n",
          map->matrix_transformation[0][0], map->matrix_transformation[0][1],
          map->matrix_transformation[0][2], map->matrix_transformation[0][3],
          map->matrix_transformation[1][0], map->matrix_transformation[1][1],
          map->matrix_transformation[1][2], map->matrix_transformation[1][3],
          map->matrix_transformation[2][0], map->matrix_transformation[2][1],
          map->matrix_transformation[2][2], map->matrix_transformation[2][3],
          map->matrix_transformation[3][0], map->matrix_transformation[3][1],
          map->matrix_transformation[3][2], map->matrix_transformation[3][3]);
}

void print_device_id(const hippo::DeviceID *id) {
  fprintf(stderr, "%s:\n", id->name);
  fprintf(stderr, "  index:\t%d\n", id->index);
  fprintf(stderr, "  vid:  \t%d\n", id->vendor_id);
  fprintf(stderr, "  pid:  \t%d\n", id->product_id);
}

void print_device_ids(const hippo::DeviceID *ids, uint64_t num_devices) {
  fprintf(stderr, "Connected Devices' IDs:\n");
  for (uint64_t i = 0; i < num_devices; i++) {
    print_device_id(&ids[i]);
  }
}

void print_devices(hippo::DeviceInfo *info, uint64_t num_devices) {
  fprintf(stderr, "Connected Devices' Info:\n");
  for (uint64_t i = 0; i < num_devices; i++) {
    fprintf(stderr, "%s:\n", info[i].name);
    fprintf(stderr, "  fw version:\t%s\n", info[i].fw_version);
    fprintf(stderr, "  serial:\t%s\n", info[i].serial);
    fprintf(stderr, "  index:\t%d\n", info[i].index);
    fprintf(stderr, "  vid:  \t%d\n", info[i].vendor_id);
    fprintf(stderr, "  pid:  \t%d\n", info[i].product_id);
  }
}

void print_display_list(hippo::DisplayInfo *info, uint64_t num_displays) {
  for (uint64_t i = 0; i < num_displays; i++) {
    fprintf(stderr, "Hardware ID: %s\n", info[i].hardware_id);
    fprintf(stderr, "  Is Primary: %s\n",
            info[i].primary_display ? "True":"False");
    fprintf(stderr, "  Coordinates: (%d,%d,%d,%d)\n", info[i].coordinates.width,
            info[i].coordinates.height, info[i].coordinates.x,
            info[i].coordinates.y);
  }
}

void print_hardware_ids(hippo::HardwareIDs *hwIDs,
                        uint64_t numProjectors,
                        uint64_t numTouchscreens) {
  fprintf(stderr, "There are %lld projectors and %lld touchscreens\n",
          numProjectors, numTouchscreens);
  fprintf(stderr, "  Projectors:\n");
  for (int i = 0; i < numProjectors; i++) {
    fprintf(stderr, "    %s\n", hwIDs->sprout_projector[i]);
  }
  fprintf(stderr, "  Touchscreens:\n");
  for (int i = 0; i < numTouchscreens; i++) {
    fprintf(stderr, "    %s\n", hwIDs->sprout_touchscreen[i]);
  }
}

uint64_t TestSystem(hippo::System *system) {
  uint64_t err;
  fprintf(stderr, "#################################\n");
  fprintf(stderr, "  Now Testing System Commands\n");
  fprintf(stderr, "#################################\n");

  ADD_FILE_TO_MAP();   // will add this file to the file/error map

  // subscribe
  uint32_t get = 0;
  if (err = system->subscribe(&system_notification,
                              reinterpret_cast<void*>(system),
                              &get)) {
    print_error(err);
  } else {
    fprintf(stderr, "system.subscribe(): %d\n", get);
  }
  char *echo_return;
  if (err = system->echo("HiPPo is the best!", &echo_return)) {
    system->free_echo_string(echo_return);
    return err;
  }
  fprintf(stderr, "Echo(`HiPPo is the best!`) returned: %s\n", echo_return);
  system->free_echo_string(echo_return);

  uint32_t sessionID;
  if (err = system->session_id(&sessionID)) {
    return err;
  }
  fprintf(stderr, "Current Session ID is: %d\n", sessionID);

  hippo::SessionState sessionState;
  if (err = system->is_locked(&sessionState)) {
    return err;
  }
  fprintf(stderr, "Current Session State is: %s\n",
          SessionState_str[static_cast<uint32_t>(sessionID)]);

  // get the supported device list
  hippo::SupportedDevice *supported_devices;
  uint64_t num_supported_devs;
  if (err = system->supported_devices(&supported_devices,
                                      &num_supported_devs)) {
    system->free_supported_devices(supported_devices, num_supported_devs);
    return err;
  }
  fprintf(stderr, "There are %u supported devices\n",
          static_cast<int>(num_supported_devs));

  // print out the supported devices
  print_supported_devs(supported_devices, num_supported_devs);
  // then free the allocated memory
  system->free_supported_devices(supported_devices, num_supported_devs);

  hippo::HardwareIDs hwIDs;
  uint64_t numProjectors;
  uint64_t numTouchscreens;
  if (err = system->hardware_ids(&hwIDs, &numProjectors, &numTouchscreens)) {
    return err;
  }
  print_hardware_ids(&hwIDs, numProjectors, numTouchscreens);
  system->free_hardware_ids(&hwIDs, numProjectors, numTouchscreens);

  hippo::DisplayInfo *display_list;
  uint64_t num_displays;
  if (err = system->list_displays(&display_list, &num_displays)) {
    system->free_display_list(display_list, num_displays);
    return err;
  }
  fprintf(stderr, "There are %u attached displays\n",
          static_cast<int>(num_displays));
  print_display_list(display_list, num_displays);
  system->free_display_list(display_list, num_displays);

  hippo::TemperatureInfo *temps;
  uint64_t num_temperatures = 0;

  // get the temperatures
  if (err = system->temperatures(&temps, &num_temperatures)) {
    system->free_temperatures(temps);
    return err;
  }
  fprintf(stderr, "There are %u items with temperatures\n",
          static_cast<int>(num_temperatures));

  // print out the temperatures
  print_temperatures(temps, num_temperatures);

  // free the memory that was allocated in the system->temperatures call
  system->free_temperatures(temps);

  // get devices info
  hippo::DeviceInfo *info;
  uint64_t num_devices = 0;
  if (err = system->devices(&info, &num_devices)) {
    system->free_devices(info, num_devices);
    return err;
  }
  print_devices(info, num_devices);
  system->free_devices(info, num_devices);

  // get device IDs
  hippo::DeviceID *ids;
  uint64_t num_device_ids = 0;
  if (err = system->device_ids(&ids, &num_device_ids)) {
    system->free_device_ids(ids, num_device_ids);
    return err;
  }
  print_device_ids(ids, num_device_ids);
  system->free_device_ids(ids, num_device_ids);

  // get the depthcamera rgb to highres rgb xform
  hippo::Camera3DMapping cam3dmap;
  hippo::Camera3DMappingParameter camMapParamD2HR;
  camMapParamD2HR.from.index = 0;
  camMapParamD2HR.from.name = hippo::CameraNameType::depthcamera;
  camMapParamD2HR.from.stream = hippo::CameraStreamType::rgb;

  camMapParamD2HR.to.index = 0;
  camMapParamD2HR.to.name = hippo::CameraNameType::hirescamera;
  camMapParamD2HR.to.stream = hippo::CameraStreamType::rgb;

  if (err = system->camera_3d_mapping(camMapParamD2HR, &cam3dmap)) {
    return err;
  }
  print_3d_mapping(&cam3dmap);

  // get the highres rgb to depthcamera rgb xform
  memset(&cam3dmap, 0, sizeof(hippo::Camera3DMapping));
  hippo::Camera3DMappingParameter camMapParamHR2D;
  camMapParamHR2D.from.index = 0;
  camMapParamHR2D.from.name = hippo::CameraNameType::hirescamera;
  camMapParamHR2D.from.stream = hippo::CameraStreamType::rgb;

  camMapParamHR2D.to.index = 0;
  camMapParamHR2D.to.name = hippo::CameraNameType::depthcamera;
  camMapParamHR2D.to.stream = hippo::CameraStreamType::rgb;

  if (err = system->camera_3d_mapping(camMapParamHR2D, &cam3dmap)) {
    return err;
  }
  print_3d_mapping(&cam3dmap);

  // get the depthcamera ir to depthcamera rgb xform
  memset(&cam3dmap, 0, sizeof(hippo::Camera3DMapping));
  hippo::Camera3DMappingParameter camMapParamIR2DRGB;
  camMapParamIR2DRGB.from.index = 0;
  camMapParamIR2DRGB.from.name = hippo::CameraNameType::depthcamera;
  camMapParamIR2DRGB.from.stream = hippo::CameraStreamType::ir;
  camMapParamIR2DRGB.to.index = 0;
  camMapParamIR2DRGB.to.name = hippo::CameraNameType::depthcamera;
  camMapParamIR2DRGB.to.stream = hippo::CameraStreamType::rgb;

  if (err = system->camera_3d_mapping(camMapParamIR2DRGB, &cam3dmap)) {
    return err;
  }
  print_3d_mapping(&cam3dmap);

  uint32_t num_subscribe;
  // unsubscribe
  if (err = system->unsubscribe(&num_subscribe)) {
    print_error(err);
  } else {
    fprintf(stderr, "system.unsubscribe(): count: %d\n", num_subscribe);
  }
  return 0LL;
}

void system_notification(const hippo::SystemNotificationParam &param,
                         void *data) {
  hippo::System *sys = (hippo::System*)data;

  switch (param.type) {
    case hippo::SystemNotification::on_device_connected:
      fprintf(stderr, "[SIGNAL]: system.on_device_connected\n");
      print_device_id(&param.on_device_connected);
      break;
    case hippo::SystemNotification::on_device_disconnected:
      fprintf(stderr, "[SIGNAL]: system.on_device_disconnected\n");
      print_device_id(&param.on_device_disconnected);
      break;
    case hippo::SystemNotification::on_display_change:
      fprintf(stderr, "[SIGNAL]: system.on_display_change\n");
      print_display_list(param.on_display_change, param.num_displays);
      break;
    case hippo::SystemNotification::on_power_state:
      fprintf(stderr, "[SIGNAL]: system.on_power_state: %s\n",
              PowerState_str[static_cast<uint32_t>(param.on_power_state)]);
      break;
    case hippo::SystemNotification::on_session_change:
      fprintf(stderr, "[SIGNAL]: system.on_session_change %d:%s\n",
              param.on_session_change.session_id,
              SessionChangeEvent_str[
                  static_cast<uint32_t>(param.on_session_change.change_event)]);
      break;
    case hippo::SystemNotification::on_temperature_high:
      fprintf(stderr, "[SIGNAL]: system.on_temperature_high\n");
      print_temperatures(&param.on_temperature_high, 1);
      break;
    case hippo::SystemNotification::on_temperature_overtemp:
      fprintf(stderr, "[SIGNAL]: system.on_temperature_overtemp\n");
      print_temperatures(&param.on_temperature_overtemp, 1);
      break;
    case hippo::SystemNotification::on_temperature_safe:
      fprintf(stderr, "[SIGNAL]: system.on_temperature_safe\n");
      print_temperatures(&param.on_temperature_safe, 1);
      break;
    case hippo::SystemNotification::on_sohal_disconnected:
      fprintf(stderr, "[SIGNAL]: system.on_sohal_disconnected\n");
      break;
    case hippo::SystemNotification::on_sohal_connected:
      fprintf(stderr, "[SIGNAL]: system.on_sohal_connected\n");
      break;
    default:
      break;
  }
}
