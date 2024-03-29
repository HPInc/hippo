
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#include <mutex>   // NOLINT
#include <thread>   // NOLINT

#include "../include/hippo_swdevice.h"
#include "../include/hippo_ws.h"
#include "../include/json.hpp"
#include "../include/base64.h"

namespace nl = nlohmann;

namespace hippo {

extern const char *defaultHost;
extern uint32_t defaultPort;

extern uint64_t CaptureLock(std::unique_lock<std::mutex> *lock,
                            HippoFacility facility);

HippoSwDevice::HippoSwDevice(const char *devName) :
    HippoDevice(devName, defaultHost, defaultPort, HIPPO_SWDEVICE, 0),
    cmd_th_(NULL), wsCmd_(NULL), needs_to_disconnect_(false),
    needs_to_disconnect_mutex_(NULL) {
}

HippoSwDevice::HippoSwDevice(const char *devName, uint32_t device_index) :
    HippoDevice(devName, defaultHost, defaultPort, HIPPO_SWDEVICE,
                device_index),
    cmd_th_(NULL), wsCmd_(NULL), needs_to_disconnect_(false),
    needs_to_disconnect_mutex_(NULL) {
}

HippoSwDevice::HippoSwDevice(const char *devName,
                             const char *address, uint32_t port) :
    HippoDevice(devName, address, port, HIPPO_SWDEVICE, 0),
    cmd_th_(NULL), wsCmd_(NULL), needs_to_disconnect_(false),
    needs_to_disconnect_mutex_(NULL) {
}

HippoSwDevice::HippoSwDevice(const char *devName,
                             const char *address, uint32_t port,
                             uint32_t device_index) :
    HippoDevice(devName, address, port, HIPPO_SWDEVICE, device_index),
    cmd_th_(NULL), wsCmd_(NULL), needs_to_disconnect_(false),
    needs_to_disconnect_mutex_(NULL) {
}

HippoSwDevice::~HippoSwDevice(void) {
  if (HasRegisteredCallback()) {
    unsubscribe();
  }
  if (NULL != cmd_th_) {
    disconnect_device();
  }
}

uint64_t HippoSwDevice::connect_device(const char *json) {
  uint64_t err = 0LL;

  if (NULL != cmd_th_) {
    return 0LL;
  }
  // fprintf(stderr, "%s 1 %p\n", __FUNCTION__, this);

  if (NULL == (wsCmd_ = new (std::nothrow)HippoWS(facility_))) {
    return MAKE_HIPPO_ERROR(facility_,
                            HIPPO_MEM_ALLOC);
  }
  int timeout = 5;   // in seconds
  wsCmd_->Connect(host_, port_, WsConnectionType::TEXT, timeout);

  nl::json ret_obj;
  unsigned char *request = NULL, *response = NULL;
  nl::json j = nl::json::parse(json);
  if (err = GenerateJsonRpc("system", "device_connected", &j, &request)) {
    goto clean_up;
  }
  timeout = 10;
  if (err = wsCmd_->SendRequest(request, WsConnectionType::TEXT, timeout,
                                &response)) {
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
  // everything went OK
  needs_to_disconnect(false);
  needs_to_disconnect_mutex_ = new std::mutex();
  cmd_th_ = new std::thread(&HippoSwDevice::WaitForCommand, this);

  // fprintf(stderr, "%s n %p %p\n", __FUNCTION__, this, cmd_th_);

clean_up:
  free(request);
  free(response);

  return err;
}

uint64_t HippoSwDevice::disconnect_device() {
  uint64_t err = 0LL;
  if (NULL == cmd_th_) {
    return 0LL;
  }
  if (!wsCmd_->StopSignalLoop()) {
    // and wait for the signal thread to finish
    cmd_th_->join();
    // then delete the allocated memory
    delete cmd_th_;
    cmd_th_ = NULL;
  }
  wsCmd_->Disconnect();
  delete wsCmd_;
  wsCmd_ = NULL;
  needs_to_disconnect(true);
  delete needs_to_disconnect_mutex_;

  return 0LL;
}

// SR: we need to revisit this command and see if can simulate
// void HippoDevice::WaitForSignalRaw(void) better
void HippoSwDevice::WaitForCommand(void) {
  uint64_t err = 0LL;
  unsigned char *signal = NULL;
  nl::json *params = NULL;

  while (true) {
    free(signal);
    if (err = wsCmd_->WaitForSignal(&signal)) {
      // fprintf(stderr, "*** %s Error: %llx\n", __FUNCTION__, err);
    }
    // fprintf(stderr, "*** %s SIGNAL! %s\n", __FUNCTION__, signal);
    if (NULL == signal) {
      // got a NULL signal, meaning we need to exit the thread
      break;
    } else {    // if (HasRegisteredCallback()) {
      try {
        nl::json jsonRet = nl::json::parse(signal);
        auto method_j = jsonRet.at("method");
        auto id_j = jsonRet.at("id");
        if (method_j.is_string()) {
          std::string method_s = method_j.get<std::string>();
          // no need to new anymore!
          const char *method = method_s.c_str() + method_s.find('.') + 1;
          params = new (std::nothrow) nl::json(jsonRet.value("params",
                                                             nl::json()));
          nl::json result;
          char *jsonrpc = NULL;
          if (err = ProcessCommand(method,
                                   reinterpret_cast<void*>(params),
                                   reinterpret_cast<void*>(&result))) {
            (void)GenerateJsonRpcError(reinterpret_cast<void*>(&id_j),
                                       err, &jsonrpc);
          } else {
            (void)GenerateJsonRpcResponse(reinterpret_cast<void*>(&id_j),
                                          reinterpret_cast<void*>(&result),
                                          &jsonrpc);
          }
          // fprintf(stderr, "*** response: '%s'\n", jsonrpc);
          // send message without waiting for response
          if (err = wsCmd_->SendRequest(
                  reinterpret_cast<unsigned char*>(jsonrpc),
                  WsConnectionType::TEXT)) {
            // SR: who do we send error back to?
          }
        } else {
          fprintf(stderr, "** signal.method is not a string\n");
        }
      } catch (nl::json::exception) {     // out_of_range or type_error
        fprintf(stderr, "** Ooops! No 'method' in notification! '%s' \n",
                signal);
      }
    }
  }
  free(signal);
  // fprintf(stderr, "*** %s Exiting\n", __FUNCTION__);
}

uint64_t HippoSwDevice::needs_to_disconnect(bool set) {
  uint64_t err;

  if (!needs_to_disconnect_mutex_) {
      return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_OPEN);
  }
  std::unique_lock<std::mutex> lock(*needs_to_disconnect_mutex_,
                                    std::defer_lock);
  if (err = CaptureLock(&lock, HIPPO_SWDEVICE)) {
    return err;
  }
  needs_to_disconnect_ = set;
  lock.unlock();

  return 0LL;
}

uint64_t HippoSwDevice::needs_to_disconnect(bool *get) {
  uint64_t err;

  if (!needs_to_disconnect_mutex_) {
      return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_OPEN);
  }

  std::unique_lock<std::mutex> lock(*needs_to_disconnect_mutex_,
                                    std::defer_lock);
  if (err = CaptureLock(&lock, HIPPO_SWDEVICE)) {
    return err;
  }
  *get = needs_to_disconnect_;
  lock.unlock();

  return 0LL;
}

bool HippoSwDevice::needs_to_disconnect(void) {
  bool ntd = false;
  uint64_t err;
  err = needs_to_disconnect(&ntd);
  if (err) {
      return true;
  }
  return ntd;
}

uint64_t HippoSwDevice::ProcessCommand(const char *, void *, void *) {
  return 0LL;
}

// private function that does the actual sending of the notification
uint64_t HippoSwDevice::SendNotification(const char *method,
                                         const void *params) {
  uint64_t err = HIPPO_OK;
  try {
    std::string device = devName_;
    std::string devName = device.substr(0, device.find("@"));
    std::string methodString = devName + ".on_" + method;

    nl::json msg = { { "jsonrpc", "2.0" }, { "method", methodString } };
    if (NULL !=
      params && !reinterpret_cast<const nl::json*>(params)->empty()) {
      msg["params"] = *(reinterpret_cast<const nl::json*>(params));
    }
    std::string msg_s = msg.dump();

    unsigned char *jsonrpc =
      reinterpret_cast<unsigned char*>(strdup(msg_s.c_str()));
    // send notification
    if (err = wsCmd_->SendRequest(
      reinterpret_cast<unsigned char*>(jsonrpc),
      WsConnectionType::TEXT)) {
      // Who do we send error back to?
    }
  }
  catch (nl::json::exception) {     // out_of_range or type_error
    fprintf(stderr, "** Ooops! No 'method' in notification! '%s' \n",
      "SendNotification() function");
  }
  return err;
}

uint64_t HippoSwDevice::subscribe(void(*callback)(
                                        const SWDeviceNotificationParam &param,
                                        void *data),
                                  void *data) {
  return subscribe(callback, data, NULL);
}
uint64_t HippoSwDevice::subscribe(void(*callback)(
                                        const SWDeviceNotificationParam &param,
                                        void *data),
                                  void *data, uint32_t *get) {
  uint64_t err = 0LL;

  if (err = HippoDevice::subscribe_raw(data, get)) {
    return err;
  }
  callback_ = callback;

  return err;
}

uint64_t HippoSwDevice::unsubscribe() {
  callback_ = NULL;
  return HippoDevice::unsubscribe();
}

uint64_t HippoSwDevice::unsubscribe(uint32_t *get) {
  callback_ = NULL;
  return HippoDevice::unsubscribe(get);
}

// protected member functions

// send notification with no parameter
uint64_t HippoSwDevice::SendNotification(const char *notificationName) {
  return SendNotification(notificationName, static_cast<void*>(nullptr));
}

// send notification with integer parameter
uint64_t HippoSwDevice::SendNotification(const char *notificationName,
                                         int32_t param) {
  if (notificationName == nullptr) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  nl::json jparam;
  uint64_t err = int32_t_c2json(param, &jparam);
  if (err != HIPPO_OK) {
    return err;
  }
  return SendNotification(notificationName, reinterpret_cast<void*>(&jparam));
}

// send notification with float param
uint64_t HippoSwDevice::SendNotification(const char *notificationName,
                                         float param) {
  if (notificationName == nullptr) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  nl::json jparam;
  uint64_t err = float_c2json(param, &jparam);
  if (err != HIPPO_OK) {
    return err;
  }
  return SendNotification(notificationName, reinterpret_cast<void*>(&jparam));
}

// send notification with bool param
uint64_t HippoSwDevice::SendNotification(const char *notificationName,
                                         bool param) {
  if (notificationName == nullptr) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  nl::json jparam;
  uint64_t err = bool_c2json(param, &jparam);
  if (err != HIPPO_OK) {
    return err;
  }
  return SendNotification(notificationName, reinterpret_cast<void*>(&jparam));
}

// send notification with char* param
uint64_t HippoSwDevice::SendNotification(const char *notificationName,
                                         char* param) {
  if (notificationName == nullptr || param == nullptr) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }

  nl::json jparam = param;
  if (!jparam.is_string()) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }

  return SendNotification(notificationName, reinterpret_cast<void*>(&jparam));
}

// send notification with wcharptr param
uint64_t HippoSwDevice::SendNotification(const char *notificationName,
                                         wcharptr* param) {
  if (notificationName == nullptr || param->data == nullptr
) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  nl::json jparam;
  uint64_t err = wcharptr_c2json(*param, &jparam);
  if (err != HIPPO_OK) {
    return err;
  }
  return SendNotification(notificationName, reinterpret_cast<void*>(&jparam));
}

// send notification with b64bytes* param
uint64_t HippoSwDevice::SendNotification(const char *notificationName,
                                         b64bytes* param) {
  if (notificationName == nullptr || param == nullptr) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  nl::json jparam;
  uint64_t err = b64bytes_c2json(*param, &jparam);
  if (err != HIPPO_OK) {
    return err;
  }
  return SendNotification(notificationName, reinterpret_cast<void*>(&jparam));
}

uint64_t HippoSwDevice::int32_t_c2json(const int32_t &set, void *obj) {
  if (obj == NULL) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  *(reinterpret_cast<nl::json*>(obj)) = set;

  return HIPPO_OK;
}

uint64_t HippoSwDevice::int32_t_json2c(const void *obj, int32_t *get) {
  if (obj == NULL || get == NULL) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  const nl::json *j = reinterpret_cast<const nl::json*>(obj);
  if (!j->is_number_integer()) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_MESSAGE_ERROR);
  }
  *get = j->get<int32_t>();

  return HIPPO_OK;
}

uint64_t HippoSwDevice::float_c2json(const float &set, void *obj) {
  if (obj == NULL) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  *(reinterpret_cast<nl::json*>(obj)) = set;

  return HIPPO_OK;
}

uint64_t HippoSwDevice::float_json2c(const void *obj, float *get) {
  if (obj == NULL || get == NULL) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  const nl::json *j = reinterpret_cast<const nl::json*>(obj);
  if (!j->is_number()) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_MESSAGE_ERROR);
  }
  *get = j->get<float>();

  return HIPPO_OK;
}

uint64_t HippoSwDevice::bool_c2json(const bool &set, void *obj) {
  if (obj == NULL) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  *(reinterpret_cast<nl::json*>(obj)) = set;

  return HIPPO_OK;
}

uint64_t HippoSwDevice::bool_json2c(const void *obj, bool *get) {
  if (obj == NULL || get == NULL) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  const nl::json *j = reinterpret_cast<const nl::json*>(obj);
  if (!j->is_boolean()) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_MESSAGE_ERROR);
  }
  *get = j->get<bool>();

  return HIPPO_OK;
}

uint64_t HippoSwDevice::wcharptr_c2json(const wcharptr &set, void *obj) {
  if (obj == NULL || set.data == NULL) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  std::setlocale(LC_ALL, "en_US.utf8");
  std::mbstate_t state = std::mbstate_t();
  const wchar_t *data = set.data;
  size_t size = 1 + std::wcsrtombs(NULL, &data, 0, &state);
  // fwprintf(stderr, L"%s  size: %zd\n", set.data, size);
  b64bytes b(size);
  if (b.len == 0 || b.data ==NULL) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_MEM_ALLOC);
  }
  (void)wcsrtombs(reinterpret_cast<char*>(b.data), &data, b.len, &state);
  // fprintf(stderr, "%s (str): '%s'\n", __FUNCTION__, b.data);

  return b64bytes_c2json(b, obj);
}

uint64_t HippoSwDevice::wcharptr_json2c(const void *obj, wcharptr *get) {
  if (obj == NULL || get == NULL) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  const nl::json *j = reinterpret_cast<const nl::json*>(obj);
  if (!j->is_string()) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_MESSAGE_ERROR);
  }
  uint64_t err;
  b64bytes b;
  if (err = b64bytes_json2c(obj, &b)) {
    return err;
  }
  std::setlocale(LC_ALL, "en_US.utf8");
  std::mbstate_t state = std::mbstate_t();
  const char *data = reinterpret_cast<const char*>(b.data);
  size_t size = 1 + mbsrtowcs(NULL, &data, b.len, &state);
  // fprintf(stderr, "%s  size: %zd %zd\n", __FUNCTION__, size, b.len);
  get->resize(size);
  (void)mbsrtowcs(get->data, &data, size, &state);

  return HIPPO_OK;
}

uint64_t HippoSwDevice::b64bytes_c2json(const b64bytes &set, void *obj) {
  if (obj == NULL || set.data == NULL) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
#ifdef _ORG_
  // convert vector<char> to base64 encoded string
  std::string encoded;
  encoded = base64_encode(reinterpret_cast<const unsigned char*>(set.data),
                          set.len);

  *(reinterpret_cast<nl::json*>(obj)) = _strdup(encoded.c_str());
#else
  uint64_t err;
  char *data = NULL;
  if (err = base64_encode(set, &data)) {
    return err;
  }
  *(reinterpret_cast<nl::json*>(obj)) = data;
#endif
  return HIPPO_OK;
}

uint64_t HippoSwDevice::b64bytes_json2c(const void *obj, b64bytes *get) {
  if (obj == NULL || get == NULL) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  const nl::json *j = reinterpret_cast<const nl::json*>(obj);
  if (!j->is_string()) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_MESSAGE_ERROR);
  }
#ifdef _ORG_
  std::string b64string, decoded_string;
  b64string = j->get<std::string>();
  decoded_string = base64_decode(b64string);

  get->len = decoded_string.length();
  get->data = reinterpret_cast<uint8_t*>(malloc(get->len));
  if (get->data == NULL) {
    get->len = 0;
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_MEM_ALLOC);
  }
  memcpy(get->data, decoded_string.data(), get->len);
#else
  uint64_t err;
  std::string b64string = j->get<std::string>();

  if (err = base64_decode(b64string.c_str(), get)) {
    return err;
  }
#endif
  return HIPPO_OK;
}

bool HippoSwDevice::HasRegisteredCallback() {
  return (NULL != callback_);
}

void HippoSwDevice::ProcessSignal(char *method, void *obj) {
  // check to ensure that there is a callback
  if (NULL == callback_) {
    return;
  }

  // make the parameter
  SWDeviceNotificationParam param;

  // copy the method name over.  This mallocs data which
  // gets freed in the SWDeviceNotificationParam destructor
  if (strlen(method) > 0) {
    size_t len = strlen(method);
    param.methodName = reinterpret_cast<char*>(malloc(len+1));
    memcpy(param.methodName, method, len);
    param.methodName[len] = 0;
  }

  // now try to cast the json as the various types
  nl::json v, *params = reinterpret_cast<nl::json*>(obj);
  try {
    v = params->at(0);
    if (v.is_number_integer()) {
      param.uint32Data = v.get<uint32_t>();
    }
    if (v.is_boolean()) {
      param.boolData = v.get<bool>();
    }
    if (v.is_number_float()) {
      param.floatData = v.get<float>();
    }

    if (v.is_string()) {
      // copy over the char* interpretation of the parameter
      // This mallocs data which gets freed in the
      // SWDeviceNotificationParam destructor
      std::string strDat = v.get<std::string>();
      param.charData = reinterpret_cast<char*>(malloc(strDat.length() + 1));
      memcpy(param.charData, strDat.c_str(), strDat.length());
      param.charData[strDat.length()] = 0;

      // convert the b64bytes interpretation
      b64bytes_json2c(&v, &param.b64bytesData);

      // convert the wcharptr interpretation
      wcharptr_json2c(&v, &param.wcharData);
    }
  }
  catch (nl::json::exception) {     // out_of_range or type_error
                                    // do nothing
  }

  // now pass the data back to the callback
  (*callback_)(param, callback_data_);
}

//
// data types
//
wcharptr::wcharptr() :
    data(NULL) {
}

wcharptr::wcharptr(size_t size) :
    data(NULL) {
  resize(size);
}

wcharptr::~wcharptr() {
  free(data);
}

uint64_t wcharptr::resize(size_t size) {
  wchar_t *d = reinterpret_cast<wchar_t*>(realloc(data, size*sizeof(wchar_t)));
  if (d) {
    data = d;
    if (size > 0) {
      data[size-1] = '\0';
    }
  }
  return NULL == d;
}

b64bytes::b64bytes() :
    data(NULL), len(0) {
}

b64bytes::b64bytes(size_t size):
    len(0), data(NULL) {
  resize(size);
}

b64bytes::~b64bytes() {
  free(data);
}

uint64_t b64bytes::resize(size_t size) {
  uint8_t *d = reinterpret_cast<uint8_t*>(realloc(data, size));
  if (d) {
    data = d;
    len = size;
  }
  return NULL == d;
}

// initialize the parameters
SWDeviceNotificationParam::SWDeviceNotificationParam():
  methodName(nullptr),
  uint32Data(0),
  floatData(0),
  charData(0) {
}

// free any allocated data
SWDeviceNotificationParam::~SWDeviceNotificationParam() {
  if (methodName) {
    free(methodName);
    methodName = nullptr;
  }
  if (charData) {
    free(charData);
    charData = nullptr;
  }
}

}    // namespace hippo
