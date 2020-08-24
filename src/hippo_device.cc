
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#define NOMINMAX
#include <windows.h>   // for GetCurrentThreadId
#include <tchar.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <mutex>   // NOLINT
#include <thread>   // NOLINT
#include <algorithm>    // std::min

#include "../include/hippo_device.h"
#include "../include/hippo_ws.h"
#include "../include/json.hpp"

namespace nl = nlohmann;

namespace hippo {

// global mutex to block access to devices
std::mutex gHippoDeviceMutex;

const uint32_t MAX_METHOD_LEN = 128;

const char *defaultHost = "localhost";
uint32_t defaultPort = 20641;

extern uint64_t clearError();
extern uint64_t setError(const char *errStr);


HippoDevice::HippoDevice(const char *dev, const char *host, uint32_t port,
                         HippoFacility facility, uint32_t device_index) :
    device_index_(device_index), ws_(NULL), wsSig_(NULL), module_(NULL), id_(0),
    port_(port), facility_(facility), signal_th_(NULL) {
  snprintf(devName_, sizeof(devName_), "%s@%d", dev, device_index_);
  if (host) {
    snprintf(host_, sizeof(host_), "%s", host);
  } else {
    host_[0] = '\0';
  }
}

HippoDevice::~HippoDevice(void) {
  Disconnect();
}

bool HippoDevice::IsConnected() {
  return IsConnectedWs() || IsConnectedWsSig();
}

bool HippoDevice::IsConnectedWs() {
  return (NULL == ws_) ? false : ws_->Connected();
}

bool HippoDevice::IsConnectedWsSig() {
  return (NULL == wsSig_) ? false : wsSig_->Connected();
}

uint64_t HippoDevice::EnsureConnected() {
  uint64_t err = 0LL;
  if (!IsConnectedWs()) {
    err = Connect();
  }
  return err;
}

uint64_t HippoDevice::Connect() {
  if (NULL == (ws_ = new (std::nothrow)HippoWS(facility_))) {
    return MAKE_HIPPO_ERROR(facility_,
                            HIPPO_MEM_ALLOC);
  }
  int timeout = 5;   // in seconds
  uint64_t err = ws_->Connect(host_, port_, WsConnectionType::TEXT,
                              timeout);
  // if an error ocurred, delete the pointer
  if (err) {
    delete ws_;
    ws_ = NULL;
  }
  return err;
}

void HippoDevice::Disconnect() {
  if (IsConnectedWs()) {
    ws_->Disconnect();
    delete ws_;
    ws_ = NULL;
  }
  if (IsConnectedWsSig()) {
    // stop waiting for signals
    if (!wsSig_->StopSignalLoop()) {
      // and wait for the signal thread to finish
      signal_th_->join();
      // then delete the allocated memory
      delete signal_th_;
      signal_th_ = NULL;
    }
    wsSig_->Disconnect();
    delete wsSig_;
    wsSig_ = NULL;
  }
}

uint64_t HippoDevice::factory_default() {
  nl::json emptyjson;
  void *jptr = reinterpret_cast<void*>(&emptyjson);
  return SendRawMsg("factory_default", jptr);
}

uint64_t HippoDevice::info(DeviceInfo *get) {
  if (NULL == get) {
    return  MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  uint64_t err = HIPPO_OK;
  nl::json j;
  void *jptr = reinterpret_cast<void*>(&j);

  if (err = SendRawMsg("info", jptr)) {
    return err;
  }
  return deviceInfo_json2c(jptr, get);
}

uint64_t HippoDevice::is_device_connected(bool *get) {
  return bool_get("is_device_connected", get);
}

void HippoDevice::free_device_info(DeviceInfo *dev_info_to_free) {\
  free(dev_info_to_free->fw_version);
  free(dev_info_to_free->name);
  free(dev_info_to_free->serial);

  // zero out the pointers
  dev_info_to_free->fw_version = NULL;
  dev_info_to_free->name = NULL;
  dev_info_to_free->serial = NULL;
}

uint64_t HippoDevice::open() {
  uint32_t open_count = 0;
  return open(&open_count);
}

uint64_t HippoDevice::open(uint32_t *open_count) {
  return uint32_get("open", open_count);
}

uint64_t HippoDevice::open_count(uint32_t *open_count) {
  return uint32_get("open_count", open_count);
}

uint64_t HippoDevice::close() {
  uint32_t open_count = 0;
  return close(&open_count);
}

uint64_t HippoDevice::close(uint32_t *open_count) {
  return uint32_get("close", open_count);
}

uint64_t HippoDevice::subscribe_raw(void *data, uint32_t *get) {
  uint64_t err = 0LL;

  std::lock_guard<std::mutex> lock(gHippoDeviceMutex);

  if (err = subscribe_raw_p(get)) {
    delete wsSig_;
    wsSig_ = NULL;
    return err;
  }
  // everything went OK
  callback_data_ = data;
  if (NULL == signal_th_) {
    signal_th_ = new std::thread(&HippoDevice::WaitForSignalRaw, this);
  }
  return err;
}

uint64_t HippoDevice::subscribe_raw_p(uint32_t *get) {
  uint64_t err = 0LL;
  if (!IsConnectedWsSig()) {
    if (NULL == wsSig_) {
      if (NULL == (wsSig_ = new (std::nothrow)HippoWS(facility_))) {
        return MAKE_HIPPO_ERROR(facility_,
                                HIPPO_MEM_ALLOC);
      }
    }
    int timeout = 5;   // in seconds
    if (err = wsSig_->Connect(host_, port_, WsConnectionType::TEXT,
                              timeout)) {
      return err;
    }
  }
  nl::json ret_obj;
  unsigned char *request = NULL, *response = NULL;
  if (err = GenerateJsonRpc("subscribe", NULL, &request)) {
    goto clean_up;
  }
  int timeout = 10;
  if (err = wsSig_->SendRequest(request, WsConnectionType::TEXT,
                                timeout, &response)) {
    goto clean_up;
  }
  // check the return from subscribe is OK
  try {
    ret_obj = nl::json::parse(response);
  } catch (nl::json::exception) {     // out_of_range or type_error
    err = MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
    goto clean_up;
  }
  if (err = GetRawResultOrError(&ret_obj)) {
    goto clean_up;
  }
  if (NULL != get) {
    // parse out the response
    uint32_t get_int = 0;
    try {
      get_int = ret_obj.get<uint32_t>();
    } catch (nl::json::exception e) {     // out_of_range or type_error
      err = MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
      goto clean_up;
    }
    *get = get_int;
  }
clean_up:
  free(request);
  free(response);
  return err;
}

uint64_t HippoDevice::temperatures(TemperatureInfo **get, uint64_t *num_temps) {
  if (NULL == get) {
    return  MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  uint64_t err = HIPPO_OK;
  nl::json j;
  void *jptr = reinterpret_cast<void*>(&j);

  if (err = SendRawMsg("temperatures", jptr)) {
    *get = NULL;
    *num_temps = 0;
    return err;
  }
  return tempInfos_json2c(jptr, get, num_temps);
}

void HippoDevice::free_temperatures(TemperatureInfo *temperature_info_to_free) {
  free(temperature_info_to_free);
  temperature_info_to_free = nullptr;
}

uint64_t HippoDevice::unsubscribe() {
  return unsubscribe(NULL);
}

uint64_t HippoDevice::unsubscribe(uint32_t *get) {
  uint64_t err = 0LL;

  std::lock_guard<std::mutex> lock(gHippoDeviceMutex);

  if (NULL == signal_th_) {
    return 0LL;
  }
  // stop waiting for signals
  if (err = wsSig_->StopSignalLoop()) {
    return err;
  }
  // and wait for the signal thread to finish
  signal_th_->join();
  // then delete the allocated memory
  delete signal_th_;
  // and assign it to NULL
  signal_th_ = NULL;
  callback_data_ = NULL;

  // then send the unsubscribe command so we can get the *get param
  nl::json ret_obj, id;
  unsigned char *request = NULL, *response = NULL;
  if (err = GenerateJsonRpc("unsubscribe", NULL, &request)) {
    goto clean_up;
  }
  int timeout = 10;
  if (err = wsSig_->SendRequest(request, WsConnectionType::TEXT, timeout,
                                &response)) {
    goto clean_up;
  }
  // make sure we got a response, not a timeout
  if (response == NULL) {
    err = MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    goto clean_up;
  }
  // check the return from unsubscribe is OK
  try {
    ret_obj = nl::json::parse(response);
  } catch (nl::json::exception) {     // out_of_range or type_error
    err = MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
    goto clean_up;
  }
  if (err = GetRawResultOrError(&ret_obj)) {
    bool found = false;
    do {
      if (err = wsSig_->ReadResponse(&response)) {
        goto clean_up;
      }
      // make sure we got a response, not a timeout
      if (response == NULL) {
        err = MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
        goto clean_up;
      }
      // check the return from unsubscribe is OK
      try {
        ret_obj = nl::json::parse(response);
      } catch (nl::json::exception) {     // out_of_range or type_error
        err = MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
        goto clean_up;
      }
      try {
        id = ret_obj.at("id");
        found = true;
      } catch (nl::json::exception) {     // out_of_range or type_error
        found = false;
        free(response);
      }
    } while (!found);

    if (err = GetRawResultOrError(&ret_obj)) {
      goto clean_up;
    }
  }
  // parse out the response
  uint32_t get_int;
  try {
    get_int = ret_obj.get<uint32_t>();
  } catch (nl::json::exception e) {     // out_of_range or type_error
    err = MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
    goto clean_up;
  }
  if (get) {
    *get = get_int;
  }
  // everything went OK, so we disconnect
  if (err = wsSig_->Disconnect()) {
    goto clean_up;
  }
  delete wsSig_;
  wsSig_ = NULL;

clean_up:
  free(request);
  free(response);

  // if we got an error, relaunch the thread since this thread needs to exist
  // in order to properly unsubscribe
  if (err != HIPPO_OK) {
    signal_th_ = new std::thread(&HippoDevice::WaitForSignalRaw, this);
  }
  return err;
}

void HippoDevice::SendSignal(const char *method, void *param) {
  char *m = strdup(method);
  void *p = (NULL == param) ? new (std::nothrow) nl::json() : param;

  std::thread th(&HippoDevice::ProcessSignal, this, m, p);

  th.detach();
}

void HippoDevice::WaitForSignalRaw(void) {
  uint64_t err = 0LL;
  unsigned char *signal = NULL;
  nl::json *params = NULL;

  while (true) {
    free(signal);
    err = wsSig_->WaitForSignal(&signal);
    // if we got a disconnect signal, then send it out
    if (HippoErrorCode(err) == HIPPO_WRONG_STATE_ERROR) {
      if (HasRegisteredCallback()) {
        SendSignal("on_sohal_disconnected", NULL);
      }
      do {
        Sleep(1000);
      } while (subscribe_raw_p(NULL));
      SendSignal("on_sohal_connected", NULL);
    } else {
      if (NULL == signal) {
        // got a NULL signal, meaning we need to exit the thread
        break;
      } else if (HasRegisteredCallback()) {
        try {
          nl::json jsonRet = nl::json::parse(signal);
          auto method_j = jsonRet.at("method");
          if (method_j.is_string()) {
            std::string method_s = method_j.get<std::string>();
            params = new (std::nothrow) nl::json(jsonRet.value("params",
                                                               nl::json()));
            SendSignal(method_s.c_str() + method_s.find('.') + 1,
                       params);
          } else {
            fprintf(stderr, "** signal.method is not a string\n");
          }
        } catch (nl::json::exception) {    // out_of_range or type_error
          // there is no 'method' key in the notification, so this may
          // be a response to a 'subscribe' command on the main user thread
        }
      }
    }
  }
  free(signal);
}

bool HippoDevice::HasRegisteredCallback() {
  return false;
}

void HippoDevice::ProcessSignal(char *method, void *params) {
  fprintf(stderr, "[SIGNAL]: Sorry not implemented for this device yet\n");
}

uint64_t HippoDevice::SendRawMsg(const char *method, void *ret_obj) {
  return SendRawMsg(method, NULL, ret_obj);
}

// sends the command via the c++ backend and converts the string response
// to a nl::json containing the "result" value of the response in case of
// success or returns the SoHal error back to the caller
uint64_t HippoDevice::SendRawMsg(const char *method, const void *param,
                                 void *ret_obj) {
  int timeout = 10;
  return SendRawMsg(method, param, timeout, ret_obj);
}

uint64_t HippoDevice::SendRawMsg(const char *method, const void *param,
                                 unsigned int timeout, void *ret_obj) {
  uint64_t err = 0LL;

  std::lock_guard<std::mutex> lock(gHippoDeviceMutex);      // lock the mutex

  if (err = EnsureConnected()) {
    return err;
  }
  if (err = hippo::clearError()) {
    return err;
  }
  if (NULL == ret_obj) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  unsigned char *request = NULL, *response = NULL;
  if (err = GenerateJsonRpc(method, param, &request)) {
    goto clean_up;
  }
  if (err = ws_->SendRequest(request, WsConnectionType::TEXT, timeout,
                             &response)) {
    goto clean_up;
  }
  try {
    *(reinterpret_cast<nl::json*>(ret_obj)) = nl::json::parse(response);
  } catch (nl::json::exception) {     // out_of_range or type_error
    err = MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
    goto clean_up;
  }
  err = GetRawResultOrError(ret_obj);

clean_up:
  free(request);
  free(response);

  return err;
}

uint64_t HippoDevice::GenerateJsonRpc(const char *method, const void *param,
                                      unsigned char **jsonrpc) {
  return GenerateJsonRpc(devName_, method, param, jsonrpc);
}

uint64_t HippoDevice::GenerateJsonRpc(const char *devName,
                                      const char *method, const void *param,
                                      unsigned char **jsonrpc) {
  char devMethod[MAX_METHOD_LEN] = { 0 };
  snprintf(devMethod, MAX_METHOD_LEN, "%s.%s", devName, method);
  uint32_t thId = GetCurrentThreadId();
  const uint32_t kIdLen = 128;
  char id[kIdLen];
  uint32_t idOffset = 6;
  snprintf(id, kIdLen, "%p:%04x:%d", this, thId, id_++);

  nl::json msg = {{"jsonrpc", "2.0"}, {"id", id}, {"method", devMethod}};
  if (NULL != param && !reinterpret_cast<const nl::json*>(param)->empty()) {
    msg["params"] = *(reinterpret_cast<const nl::json*>(param));
  }
  std::string msg_s = msg.dump();

  *jsonrpc = reinterpret_cast<unsigned char*>(strdup(msg_s.c_str()));

  return 0LL;
}

uint64_t HippoDevice::GenerateJsonRpcResponse(const void *id,
                                              const void *result,
                                              char **jsonrpc) {
  nl::json msg = {{"jsonrpc", "2.0"},
                  {"id", *(reinterpret_cast<const nl::json*>(id))},
                  {"result", *(reinterpret_cast<const nl::json*>(result))}};

  std::string msg_s = msg.dump();

  *jsonrpc = strdup(msg_s.c_str());

  return 0LL;
}

uint64_t HippoDevice::GenerateJsonRpcError(const void *id,
                                           uint64_t err,
                                           char **jsonrpc) {
  char data[26] = { 0 };
  snprintf(data, sizeof(data), "%s:%04llx%04llx:%08llx",
           "unknown", (err>>48) & 0xffff, (err>>32) & 0xffff, err & 0xffffffff);

  nl::json msg = {{"jsonrpc", "2.0"},
                  {"id", *(reinterpret_cast<const nl::json*>(id))},
                  {"error", { {"code", HippoErrorCode(err)},
                              {"data", data},
                              {"message", HippoErrorMessage(err) }}}};

  std::string msg_s = msg.dump();

  *jsonrpc = strdup(msg_s.c_str());

  return 0LL;
}

uint64_t HippoDevice::GetRawResultOrError(void *obj) {
  // check if has a result or an error
  uint64_t err = 0LL;

  try {
    *(reinterpret_cast<nl::json*>(obj)) =
        reinterpret_cast<nl::json*>(obj)->at("result");
  } catch (nl::json::exception) {     // out_of_range or type_error
    // no 'result' key, check for 'error'
    try {
      nl::json error = reinterpret_cast<nl::json*>(obj)->at("error");
      // have 'error' key, parse the error and return it
      nl::json data = error.at("data");
      if (!data.is_string()) {
        return MAKE_HIPPO_ERROR(facility_,
                                HIPPO_MESSAGE_ERROR);
      }
      std::string data_str = data.get<std::string>();
      size_t pos = data_str.rfind(":");
      std::string e_code = data_str.substr(data_str.rfind(":")+1, 8);
      std::string l_number = data_str.substr(data_str.rfind(":", pos-1)+1, 8);
      err = (((uint64_t)(strtoul(l_number.c_str(), NULL, 16)) << 32) |
             (uint64_t)(strtoul(e_code.c_str(), NULL, 16)));
      data = error.at("message");
      if (!data.is_string()) {
        return MAKE_HIPPO_ERROR(facility_,
                                HIPPO_MESSAGE_ERROR);
      }
      std::string message_str = data.get<std::string>();
      setError(message_str.c_str());
    } catch (nl::json::exception) {     // out_of_range or type_error
      return MAKE_HIPPO_ERROR(facility_,
                              HIPPO_MESSAGE_ERROR);
    }
  }
  return err;
}

// json2c
uint64_t HippoDevice::deviceInfo_json2c(void *obj,
                                        DeviceInfo *info) {
  if (obj == NULL || info == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *jsonDevInfo = reinterpret_cast<const nl::json*>(obj);
  try {
    auto fw_version = jsonDevInfo->at("fw_version");
    auto name = jsonDevInfo->at("name");
    auto serial = jsonDevInfo->at("serial");
    auto index = jsonDevInfo->at("index");
    auto vendor_id = jsonDevInfo->at("vendor_id");
    auto product_id = jsonDevInfo->at("product_id");

    if (!fw_version.is_string() || !name.is_string() ||
        !serial.is_string() || !index.is_number_integer() ||
        !vendor_id.is_number_integer() || !product_id.is_number_integer()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    info->fw_version = strdup(fw_version.get<std::string>().c_str());
    info->name = strdup(name.get<std::string>().c_str());
    info->serial = strdup(serial.get<std::string>().c_str());
    info->index = index.get<uint32_t>();
    info->vendor_id = vendor_id.get<uint32_t>();
    info->product_id = product_id.get<uint32_t>();
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  return HIPPO_OK;
}

uint64_t HippoDevice::tempInfo_json2c(
    const void *obj,
    hippo::TemperatureInfo *temperature_info) {
  if (obj == NULL || temperature_info == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *jsonTempInfo = reinterpret_cast<const nl::json*>(obj);
  try {
    auto curr_temperature_item = *jsonTempInfo;
    if (!curr_temperature_item.is_object()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // get temperature items
    auto device = curr_temperature_item.at("device");
    auto curr_temp = curr_temperature_item.at("current");
    auto max_temp = curr_temperature_item.at("max");
    auto safe_temp = curr_temperature_item.at("safe");
    auto sensor_name = curr_temperature_item.at("sensor_name");
    // check to make sure they are valid floats
    if (!curr_temp.is_number_float() || !max_temp.is_number_float() ||
        !safe_temp.is_number_float() || !sensor_name.is_string() ||
        !device.is_string()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // put them int the array
    (*temperature_info).current = curr_temp.get<float>();
    (*temperature_info).max = max_temp.get<float>();
    (*temperature_info).safe = safe_temp.get<float>();
    std::string sens_name = sensor_name.get<std::string>();
    std::string dev_name = device.get<std::string>();

    const char *TempSensorId_str[] = { "led", "red", "green", "formatter",
                                       "heatsink", "hirescamera",
                                       "depthcamera", "depthcamera_tec",
                                       "hirescamera_z_3d",
                                       "hirescamera_z_3d_system",
                                       "depthcamera_z_3d_tec",
    };

    // get the proper enum value for the sensor name
    int32_t idx = str_to_idx(TempSensorId_str, sens_name.c_str(),
                             static_cast<uint32_t>(
                                 TempInfoSensors::led),
                             static_cast<uint32_t>(
                                 TempInfoSensors::depthcamera_z_3d_tec));
    if (idx < 0) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    (*temperature_info).sensor = static_cast<hippo::TempInfoSensors>(idx);


    // parse the device name
    // if we don't find the @ in devname@index return an error
    std::size_t found = dev_name.find("@");
    if (found == std::string::npos) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    std::string device_str = dev_name.substr(0, found);
    std::string devidx = dev_name.substr(found + 1);
    // get the index
    (*temperature_info).device.index = stoi(devidx);
    // get the device
    const char *DeviceConnected_str[] = { "depthcamera",
                                          "desklamp",
                                          "hirescamera",
                                          "projector" };
    idx = str_to_idx(DeviceConnected_str, device_str.c_str(),
                     static_cast<uint32_t>(
                         TemperatureConnectionDevices::through_depthcamera),
                     static_cast<uint32_t>(
                         TemperatureConnectionDevices::through_projector));
    if (idx < 0) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    (*temperature_info).device.connectedDevice =
        static_cast<hippo::TemperatureConnectionDevices>(idx);
  }
  catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }

  return HIPPO_OK;
}

uint64_t HippoDevice::tempInfos_json2c(
    const void *obj,
    hippo::TemperatureInfo **temperature_info,
    uint64_t *num_temps) {
  *num_temps = 0;
  if (temperature_info == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  *temperature_info = nullptr;
  if (obj == NULL) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  const nl::json *jsonTempInfo = reinterpret_cast<const nl::json*>(obj);
  try {
    if (!jsonTempInfo->is_array()) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
    // get the number of items in the list
    uint64_t num_items = jsonTempInfo->size();
    if (num_items < 0) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    } else if (num_items == 0) {
      return 0LL;
    }

    // allocate the memory to store the temperature info
    *temperature_info = reinterpret_cast<TemperatureInfo*>(
        malloc(num_items * sizeof(TemperatureInfo)));
    if (!temperature_info) {
      fprintf(stderr, "** Error allocating temperature array\n");
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MEM_ALLOC);
    }
    uint64_t err;
    // iterate over the list to parse the individual items
    for (int i = 0; i < num_items; i++) {
      err = tempInfo_json2c(reinterpret_cast<const void*>(&jsonTempInfo->at(i)),
                            &(*temperature_info)[i]);
      if (err != HIPPO_OK) {
        return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
      }
    }
    *num_temps = num_items;
  }
  catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }

  return HIPPO_OK;
}

///////////////
// generic API calls
///////////////

uint64_t HippoDevice::bool_get(const char *fname, bool *get) {
  uint64_t err = 0LL;
  if (NULL == get) {
    MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  nl::json jget;
  // send the request to SoHAL
  void *jptr = reinterpret_cast<void*>(&jget);
  if (err = SendRawMsg(fname, jptr)) {
    return err;
  }
  // and parse out the response
  try {
    *get = reinterpret_cast<const nl::json*>(jptr)->get<bool>();
  } catch (nl::json::exception e) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  return err;
}

uint64_t HippoDevice::bool_set_get(const char *fname, bool set, bool *get) {
  uint64_t err = 0LL;
  nl::json jset;
  nl::json jget;
  // set the parameter to true or false
  jset.push_back(set);
  void *jset_ptr = reinterpret_cast<void*>(&jset);
  void *jget_ptr = reinterpret_cast<void*>(&jget);
  // send the request to SoHAL
  if (err = SendRawMsg(fname, jset_ptr, jget_ptr)) {
    return err;
  }
  // if the get value is requested, then parse the response
  if (get != NULL) {
    try {
      *get = reinterpret_cast<const nl::json*>(jget_ptr)->get<bool>();
    } catch (nl::json::exception e) {     // out_of_range or type_error
      return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
    }
  }
  return 0LL;
}

uint64_t HippoDevice::uint16_get(const char *fname, uint16_t *get) {
  if (NULL == get) {
    MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  uint32_t get32 = 0;
  uint64_t err = uint32_get(fname, &get32);
  *get = static_cast<uint16_t>(get32);
  return err;
}

uint64_t HippoDevice::uint16_set_get(const char *fname,
                                     uint16_t set, uint16_t *get) {
  uint64_t err = 0;
  if (NULL == get) {
    err = uint32_set_get(fname, static_cast<uint32_t>(set), NULL);
  } else {
    uint32_t get32 = 0;
    err = uint32_set_get(fname, static_cast<uint32_t>(set), &get32);
    *get = static_cast<uint16_t>(get32);
  }
  return err;
}

uint64_t HippoDevice::uint32_get(const char *fname, uint32_t *get) {
  uint64_t err = 0LL;
  if (NULL == get) {
    MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  nl::json jget;
  // send the request to SoHAL
  void *jptr = reinterpret_cast<void*>(&jget);
  if (err = SendRawMsg(fname, jptr)) {
    return err;
  }
  // and parse out the response
  try {
    *get = reinterpret_cast<const nl::json*>(jptr)->get<uint32_t>();
  } catch (nl::json::exception e) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  return err;
}

uint64_t HippoDevice::uint32_set_get(const char *fname,
                                     uint32_t set, uint32_t *get) {
  uint64_t err = 0LL;
  nl::json jset;
  nl::json jget;
  // set the parameter to true or false
  jset.push_back(set);
  void *jset_ptr = reinterpret_cast<void*>(&jset);
  void *jget_ptr = reinterpret_cast<void*>(&jget);
  // send the request to SoHAL
  if (err = SendRawMsg(fname, jset_ptr, jget_ptr)) {
    return err;
  }
  // if the get value is requested, then parse the response
  if (get != NULL) {
    try {
      *get = reinterpret_cast<const nl::json*>(jget_ptr)->get<uint32_t>();
    } catch (nl::json::exception e) {     // out_of_range or type_error
      return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
    }
  }
  return 0LL;
}

uint64_t HippoDevice::float_get(const char *fname, float *get) {
  uint64_t err = 0LL;
  if (NULL == get) {
    MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  nl::json jget;
  // send the request to SoHAL
  void *jptr = reinterpret_cast<void*>(&jget);
  if (err = SendRawMsg(fname, jptr)) {
    return err;
  }
  // and parse out the response
  try {
    *get = reinterpret_cast<const nl::json*>(jptr)->get<float>();
  } catch (nl::json::exception e) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  return err;
}

uint64_t HippoDevice::float_set_get(const char *fname,
                                    float set, float *get) {
  uint64_t err = 0LL;
  nl::json jset;
  nl::json jget;
  // set the parameter to true or false
  jset.push_back(set);
  void *jset_ptr = reinterpret_cast<void*>(&jset);
  void *jget_ptr = reinterpret_cast<void*>(&jget);
  // send the request to SoHAL
  if (err = SendRawMsg(fname, jset_ptr, jget_ptr)) {
    return err;
  }
  // if the get value is requested, then parse the response
  if (get != NULL) {
    try {
      *get = reinterpret_cast<const nl::json*>(jget_ptr)->get<float>();
    } catch (nl::json::exception e) {     // out_of_range or type_error
      return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
    }
  }
  return 0LL;
}

int32_t HippoDevice::str_to_idx(const char **names,
                                const char *str,
                                uint32_t first, uint32_t last) {
  size_t str_len = strlen(str);
  for (uint32_t i = first; i <= last; i++) {
    if (str_len == strlen(names[i])) {
      if (!strncmp(str, names[i], std::min(str_len, strlen(names[i])))) {
        return static_cast<int32_t>(i);
      }
    }
  }
  return -1;
}

}  // namespace hippo
