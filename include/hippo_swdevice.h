
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#ifndef INCLUDE_HIPPO_SWDEVICE_H_
#define INCLUDE_HIPPO_SWDEVICE_H_

#include "../include/hippo_device.h"

#if COMPILING_DLL
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif

namespace std {
class mutex;
};

namespace hippo {

class DLLEXPORT wcharptr {
 public:
  wchar_t *data;

  wcharptr();
  explicit wcharptr(size_t len);
  ~wcharptr();

  uint64_t resize(size_t len);
};

class DLLEXPORT b64bytes {
 public:
  uint8_t *data;
  size_t len;

  b64bytes();
  explicit b64bytes(size_t size);
  ~b64bytes();

  uint64_t resize(size_t len);
};

class DLLEXPORT SWDeviceNotificationParam {
 public:
  SWDeviceNotificationParam();
  ~SWDeviceNotificationParam();

  // the name of the notification (up to 128 chars)
  char *methodName;

  // The various types of data that the parameter could be
  // Note: if the JSON library indicates that the
  // parameter is not valid then that particular field is not filled

  // the data as a uint32 type
  uint32_t uint32Data;

  // the data as a float type
  float floatData;

  // the data as boolean type
  bool boolData;

  // the data as a char type
  char* charData;

  // the data as a wcharptr type
  wcharptr wcharData;

  // the data as a b64bytes type
  b64bytes b64bytesData;
};

class DLLEXPORT HippoSwDevice : public HippoDevice {
 public:
  explicit HippoSwDevice(const char *devName);
  HippoSwDevice(const char *devName, uint32_t device_index);
  HippoSwDevice(const char *devName, const char *address, uint32_t port);
  HippoSwDevice(const char *devName, const char *address, uint32_t port,
                uint32_t device_index);
  virtual ~HippoSwDevice(void);

  uint64_t needs_to_disconnect(bool set);
  uint64_t needs_to_disconnect(bool *get);
  bool needs_to_disconnect(void);

  uint64_t disconnect_device(void);

  // subscribe to notifications
  uint64_t subscribe(void(*callback)(const SWDeviceNotificationParam &param,
                                     void *data),
                     void *data);
  uint64_t subscribe(void(*callback)(const SWDeviceNotificationParam &param,
                                     void *data),
                     void *data, uint32_t *get);

  // unsubscribe from notifications
  uint64_t unsubscribe();
  uint64_t unsubscribe(uint32_t *get);

 protected:
  uint64_t connect_device(const char *json);

  void WaitForCommand(void);
  virtual uint64_t ProcessCommand(const char *method, void *params,
                                  void *result);

  uint64_t SendNotification(const char *notificationName);
  uint64_t SendNotification(const char *notificationName,
                            int32_t param);
  uint64_t SendNotification(const char *notificationName,
                            float param);
  uint64_t SendNotification(const char *notificationName,
                            bool param);
  uint64_t SendNotification(const char *notificationName,
                            char* param);
  uint64_t SendNotification(const char *notificationName,
                            wcharptr* param);
  uint64_t SendNotification(const char *notificationName,
                            b64bytes* param);

  uint64_t int32_t_c2json(const int32_t &set, void *obj);
  uint64_t int32_t_json2c(const void *obj, int32_t *get);
  uint64_t float_c2json(const float &set, void *obj);
  uint64_t float_json2c(const void *obj, float *get);
  uint64_t bool_c2json(const bool &set, void *obj);
  uint64_t bool_json2c(const void *obj, bool *get);

  uint64_t wcharptr_c2json(const wcharptr &set, void *obj);
  uint64_t wcharptr_json2c(const void *obj, wcharptr *get);
  uint64_t b64bytes_c2json(const b64bytes &set, void *obj);
  uint64_t b64bytes_json2c(const void *obj, b64bytes *get);

  // Callback items
  void ProcessSignal(char *method, void *obj) override;
  bool HasRegisteredCallback();

  void(*callback_)(const SWDeviceNotificationParam &param, void *data);


 private:
  // private function that actually sends notifications
  // this is called by the protected functions that take specific types in
  uint64_t SendNotification(const char *method, const void *param);

 private:
  HippoWS *wsCmd_;
  std::thread *cmd_th_;
  std::mutex *needs_to_disconnect_mutex_;
  bool needs_to_disconnect_;
};

}   // namespace hippo

#endif   // INCLUDE_HIPPO_SWDEVICE_H_
