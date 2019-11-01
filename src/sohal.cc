
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <mutex>   // NOLINT
#include "../include/sohal.h"
#include "../include/json.hpp"

namespace nl = nlohmann;

namespace hippo {

extern std::mutex gHippoDeviceMutex;
const char devName[] = "sohal";
extern const char *defaultHost;
extern uint32_t defaultPort;

SoHal::SoHal() :
        HippoDevice(devName, defaultHost, defaultPort, HIPPO_SOHAL, 0) {
}

SoHal::SoHal(const char *address, uint32_t port) :
        HippoDevice(devName, address, port, HIPPO_SOHAL, 0) {
}

SoHal::~SoHal(void) {
}

uint64_t SoHal::exit() {
  nl::json emptyjson;
  void *jptr = reinterpret_cast<void*>(&emptyjson);
  return SendRawMsg("exit", jptr);
}

bool SoHal::HasRegisteredCallback() {
  return (NULL != callback_);
}

uint64_t SoHal::log(LogInfo *get) {
  if (NULL == get) {
    return  MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  uint64_t err = HIPPO_OK;
  nl::json j;
  void *jptr = reinterpret_cast<void*>(&j);

  if (err = SendRawMsg("log", jptr)) {
    return err;
  }
  return logInfo_json2c(jptr, get);
}

uint64_t SoHal::log(const LogInfo &set) {
  return log(set, NULL);
}

uint64_t SoHal::log(const LogInfo &set, LogInfo *get) {
  uint64_t err = HIPPO_OK;
  nl::json jset, jget;
  void *jsetptr = reinterpret_cast<void*>(&jset);
  if (err = logInfo_c2json(set, jsetptr)) {
    return err;
  }

  void *jgetptr = reinterpret_cast<void*>(&jget);
  if (err = SendRawMsg("log", jsetptr, jgetptr)) {
    return err;
  }
  if (get != NULL) {
    err = logInfo_json2c(jgetptr, get);
  }
  return err;
}

uint64_t SoHal::subscribe(
            void(*callback)(const SoHalNotificationParam &param, void *data),
            void *data) {
  return subscribe(callback, data, NULL);
}

uint64_t SoHal::subscribe(
            void(*callback)(const SoHalNotificationParam &param, void *data),
            void *data, uint32_t *get) {
  uint64_t err = 0LL;

  if (err = HippoDevice::subscribe_raw(data, get)) {
    return err;
  }
  callback_ = callback;

  return err;
}

uint64_t SoHal::version(char **get) {
  uint64_t err = 0;

  nl::json j;
  void *jptr = reinterpret_cast<void*>(&j);
  if (err = SendRawMsg("version", jptr)) {
    *get = NULL;
    return err;
  }
  try {
    *get = strdup(j.get<std::string>().c_str());
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_,
                            HIPPO_MESSAGE_ERROR);
  }
  return err;
}

void SoHal::free_version(char *version_to_free) {
  free(version_to_free);
}

uint64_t SoHal::logInfo_c2json(const hippo::LogInfo &loginfo, void *obj) {
  uint64_t err = HIPPO_OK;
  if (obj == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  nl::json params = {
    { "level", loginfo.level },
  };
  reinterpret_cast<nl::json*>(obj)->push_back(params);
  return HIPPO_OK;
}

uint64_t SoHal::logInfo_json2c(const void *obj, hippo::LogInfo *loginfo) {
  if (obj == NULL || loginfo == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *logInfoData = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!logInfoData->is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // get the individual items from json
    auto loglevel = logInfoData->at("level");

    // check for proper type
    if (!loglevel.is_number_integer()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // put the items into the return
    loginfo->level = loglevel.get<uint32_t>();
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return HIPPO_OK;
}

const char *SoHalNotification_str[] = {
  "on_close", "on_device_connected", "on_device_disconnected",
  "on_factory_default", "on_open", "on_open_count", "on_resume", "on_suspend",
  "on_sohal_disconnected", "on_sohal_connected",
  "on_exit", "on_log",
};

void SoHal::ProcessSignal(char *method, void *obj) {
  if (NULL == callback_) {
    return;
  }
  uint64_t err = 0LL;
  int32_t idx = 0;
  idx = str_to_idx(SoHalNotification_str,
    method,
    static_cast<uint32_t>(
      SoHalNotification::on_close),
    static_cast<uint32_t>(
      SoHalNotification::on_log));
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
  SoHalNotificationParam param;
  param.type = static_cast<hippo::SoHalNotification>(idx);

  switch (param.type) {
  case SoHalNotification::on_open_count:
    param.on_open_count = v.get<uint32_t>();
    break;

  case SoHalNotification::on_exit:
    break;

  case SoHalNotification::on_log:
    logInfo_json2c(reinterpret_cast<const void*>(&v),
                   &param.on_log);
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
