
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <mutex>   // NOLINT
#include "../include/json.hpp"
#include "../include/system.h"

namespace nl = nlohmann;

namespace hippo {

extern std::mutex gHippoDeviceMutex;
const char devName[] = "system";
extern const char *defaultHost;
extern uint32_t defaultPort;

System::System() :
    HippoDevice(devName, defaultHost, defaultPort, HIPPO_SYSTEM, 0) {
}

System::System(const char *address, uint32_t port) :
    HippoDevice(devName, address, port, HIPPO_SYSTEM, 0) {
}

System::~System(void) {
}

uint64_t System::camera_3d_mapping(const Camera3DMappingParameter &set,
                                   Camera3DMapping *get) {
  if (NULL == get) {
    return  MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  uint64_t err = HIPPO_OK;
  nl::json jset, jget;
  void *jsetptr = reinterpret_cast<void*>(&jset);
  void *jgetptr = reinterpret_cast<void*>(&jget);

  if (err = cam_3d_mapping_c2json(set, jsetptr)) {
    return err;
  }
  if (err = SendRawMsg("camera_3d_mapping", jsetptr, jgetptr)) {
    return err;
  }
  return cam_3d_map_json2c(jgetptr, get);
}

uint64_t System::devices(DeviceInfo **get, uint64_t *num_devices) {
  *num_devices = 0;
  if (NULL == get) {
    return  MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  uint64_t err = HIPPO_OK;
  nl::json j;
  void *jptr = reinterpret_cast<void *>(&j);

  if (err = SendRawMsg("devices", jptr)) {
    *get = NULL;
    *num_devices = 0;
    return err;
  }
  return devices_json2c(jptr, get, num_devices);
}

uint64_t System::device_ids(DeviceID **get, uint64_t *num_devices) {
  *num_devices = 0;
  if (NULL == get) {
    return  MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  uint64_t err = HIPPO_OK;
  nl::json j;
  void *jptr = reinterpret_cast<void *>(&j);

  if (err = SendRawMsg("device_ids", jptr)) {
    *get = NULL;
    *num_devices = 0;
    return err;
  }
  return device_ids_json2c(jptr, get, num_devices);
}

uint64_t System::echo(const char *set, char **get) {
  if (NULL == set || NULL == get) {
    return  MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  uint64_t err = HIPPO_OK;
  *get = NULL;

  nl::json jset, jget;
  jset = { std::string(set) };
  void *jsetptr = reinterpret_cast<void*>(&jset);
  void *jgetptr = reinterpret_cast<void*>(&jget);

  if (err = SendRawMsg("echo", jsetptr, jgetptr)) {
    return err;
  }

  return echo_json2c(jgetptr, get);
}

uint64_t System::hardware_ids(HardwareIDs *get,
                      uint64_t *num_projectors,
                      uint64_t *num_touchscreens) {
  if (NULL == get) {
    return  MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  uint64_t err = HIPPO_OK;

  nl::json j;
  void *jptr = reinterpret_cast<void *>(&j);

  if (err = SendRawMsg("hardware_ids", jptr)) {
    return err;
  }
  return hardware_ids_json2c(jptr, get, num_projectors, num_touchscreens);
}


void System::free_hardware_ids(HardwareIDs *ids_to_free,
                                   uint64_t num_projectors,
                                   uint64_t num_touchscreens) {
  for (int i = 0; i < num_projectors; i++) {
    free(ids_to_free->sprout_projector[i]);
    ids_to_free->sprout_projector[i] = nullptr;
  }
  for (int i = 0; i < num_touchscreens; i++) {
    free(ids_to_free->sprout_touchscreen[i]);
    ids_to_free->sprout_touchscreen[i] = nullptr;
  }

  free(ids_to_free->sprout_projector);
  ids_to_free->sprout_projector = nullptr;
  free(ids_to_free->sprout_touchscreen);
  ids_to_free->sprout_touchscreen = nullptr;
}

uint64_t System::is_locked(SessionState * get) {
  if (NULL == get) {
    return  MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  uint64_t err = HIPPO_OK;

  nl::json j;
  void *jptr = reinterpret_cast<void *>(&j);

  if (err = SendRawMsg("is_locked", jptr)) {
    return err;
  }
  return is_locked_json2c(jptr, get);
}

uint64_t System::list_displays(DisplayInfo **get, uint64_t *num_displays) {
  if (NULL == get || NULL == num_displays) {
    return  MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  uint64_t err = HIPPO_OK;
  *get = NULL;

  nl::json j;
  void *jptr = reinterpret_cast<void *>(&j);

  if (err = SendRawMsg("list_displays", jptr)) {
    *get = NULL;
    *num_displays = 0;
    return err;
  }
  return list_displays_json2c(jptr, get, num_displays);
}

uint64_t System::supported_devices(SupportedDevice **get,
                                   uint64_t *num_devices) {
  *num_devices = 0;
  if (NULL == get) {
    return  MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  uint64_t err = HIPPO_OK;
  nl::json j;
  void *jptr = reinterpret_cast<void*>(&j);

  if (err = SendRawMsg("supported_devices", jptr)) {
    *get = NULL;
    *num_devices = 0;
    return err;
  }
  return supported_devices_json2c(jptr, get, num_devices);
}

uint64_t System::session_id(uint32_t *get) {
  return uint32_get("session_id", get);
}

uint64_t System::subscribe(
  void(*callback)(const SystemNotificationParam &param, void *data),
  void *data) {
  return subscribe(callback, data, NULL);
}

uint64_t System::subscribe(
  void(*callback)(const SystemNotificationParam &param, void *data),
  void *data, uint32_t *get) {
  uint64_t err = 0LL;

  if (err = HippoDevice::subscribe_raw(data, get)) {
    return err;
  }
  callback_ = callback;

  return err;
}

uint64_t System::unsubscribe() {
  callback_ = NULL;
  return HippoDevice::unsubscribe();
}

uint64_t System::unsubscribe(uint32_t *get) {
  callback_ = NULL;
  return HippoDevice::unsubscribe(get);
}

bool System::HasRegisteredCallback() {
  return (NULL != callback_);
}

const char *SystemNotification_str[] = {
  "on_device_connected", "on_device_disconnected", "on_display_change",
  "on_power_state", "on_session_change", "on_temperature_high",
  "on_temperature_overtemp", "on_temperature_safe",
  "on_sohal_disconnected", "on_sohal_connected",
};

void System::ProcessSignal(char *method, void *obj) {
  if (NULL == callback_) {
    return;
  }
  uint64_t err = 0LL;
  int32_t idx = 0;
  idx = str_to_idx(SystemNotification_str,
    method,
    static_cast<uint32_t>(
      SystemNotification::on_device_connected),
    static_cast<uint32_t>(
      SystemNotification::on_sohal_connected));
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
  SystemNotificationParam param;
  param.num_displays = 0;
  param.type = static_cast<hippo::SystemNotification>(idx);
  switch (param.type) {
  case SystemNotification::on_device_connected:
    err = device_id_json2c(reinterpret_cast<const void*>(&v),
                           &param.on_device_connected);
    break;
  case SystemNotification::on_device_disconnected:
    err = device_id_json2c(reinterpret_cast<const void*>(&v),
                           &param.on_device_disconnected);
    break;
  case SystemNotification::on_display_change:
    err = list_displays_json2c(reinterpret_cast<const void*>(&v),
                              &param.on_display_change,
                              &param.num_displays);
    break;
  case SystemNotification::on_power_state:
    err = powerstate_json2c(reinterpret_cast<const void*>(&v),
                            &param.on_power_state);
    break;
  case SystemNotification::on_session_change:
    err = sessionchange_json2c(reinterpret_cast<const void*>(&v),
                                &param.on_session_change);
    break;
  case SystemNotification::on_temperature_high:
    err = tempInfo_json2c(reinterpret_cast<const void*>(&v),
                          &param.on_temperature_high);
    break;
  case SystemNotification::on_temperature_overtemp:
    err = tempInfo_json2c(reinterpret_cast<const void*>(&v),
                          &param.on_temperature_overtemp);
    break;
  case SystemNotification::on_temperature_safe:
    err = tempInfo_json2c(reinterpret_cast<const void*>(&v),
                          &param.on_temperature_safe);
    break;
  default:
    break;
  }
  // call the user supplied callback function
  if (!err) {
    (*callback_)(param, callback_data_);
  }

  // now free the memory that was allocated in the notifications
  switch (param.type) {
  case SystemNotification::on_device_connected:
    free_device_id(&param.on_device_connected);
    break;
  case SystemNotification::on_device_disconnected:
    free_device_id(&param.on_device_disconnected);
    break;
  case SystemNotification::on_display_change:
    free_display_list(param.on_display_change, param.num_displays);
    break;
  default:
    break;
  }

  delete params;
}

}  // namespace hippo
