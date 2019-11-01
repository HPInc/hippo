
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <mutex>   // NOLINT
#include "../include/uvccamera.h"
#include "../include/hippo_ws.h"
#include "../include/json.hpp"

namespace nl = nlohmann;

namespace hippo {

extern std::mutex gHippoDeviceMutex;
const char devName[] = "uvccamera";
extern const char *defaultHost;
extern uint32_t defaultPort;

UVCCamera::UVCCamera() :
        HippoCamera(devName, defaultHost, defaultPort, HIPPO_UVCCAMERA, 0),
        callback_(NULL) {
}

UVCCamera::UVCCamera(uint32_t device_index) :
        HippoCamera(devName, defaultHost, defaultPort, HIPPO_UVCCAMERA,
                    device_index),
        callback_(NULL) {
}

UVCCamera::UVCCamera(const char *address, uint32_t port) :
        HippoCamera(devName, address, port, HIPPO_UVCCAMERA, 0),
        callback_(NULL) {
}

UVCCamera::UVCCamera(const char *address, uint32_t port,
                     uint32_t device_index) :
        HippoCamera(devName, address, port, HIPPO_UVCCAMERA, device_index),
        callback_(NULL) {
}

UVCCamera::~UVCCamera(void) {
}

uint64_t UVCCamera::camera_index(uint32_t *get) {
  return uint32_get("camera_index", get);
}

uint64_t UVCCamera::subscribe(
  void(*callback)(const UVCCameraNotificationParam &param, void *data),
  void *data) {
  return subscribe(callback, data, NULL);
}

uint64_t UVCCamera::subscribe(
  void(*callback)(const UVCCameraNotificationParam &param, void *data),
  void *data, uint32_t *get) {
  uint64_t err = 0LL;

  if (err = HippoDevice::subscribe_raw(data, get)) {
    return err;
  }
  callback_ = callback;

  return err;
}

uint64_t UVCCamera::unsubscribe() {
  callback_ = NULL;
  return HippoDevice::unsubscribe();
}

uint64_t UVCCamera::unsubscribe(uint32_t *get) {
  callback_ = NULL;
  return HippoDevice::unsubscribe(get);
}

bool UVCCamera::HasRegisteredCallback() {
  return (NULL != callback_);
}

const char *UVCCameraNotification_str[] = {
  "on_close", "on_device_connected", "on_device_disconnected",
  "on_factory_default", "on_open", "on_open_count", "on_resume", "on_suspend",
  "on_sohal_disconnected", "on_sohal_connected",
};

void UVCCamera::ProcessSignal(char *method, void *obj) {
  if (NULL == callback_) {
    return;
  }
  uint64_t err = 0LL;
  int32_t idx = 0;
  idx = str_to_idx(UVCCameraNotification_str,
    method,
    static_cast<uint32_t>(
      UVCCameraNotification::on_close),
    static_cast<uint32_t>(
      UVCCameraNotification::on_sohal_connected));
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
  UVCCameraNotificationParam param;
  param.type = static_cast<hippo::UVCCameraNotification>(idx);

  switch (param.type) {
  case UVCCameraNotification::on_open_count:
    param.on_open_count = v.get<uint32_t>();
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
