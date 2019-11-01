
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

 protected:
  uint64_t connect_device(const char *json);

  void WaitForCommand(void);
  virtual uint64_t ProcessCommand(const char *method, void *params,
                                  void *result);

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

 private:
  HippoWS *wsCmd_;
  std::thread *cmd_th_;
  std::mutex *needs_to_disconnect_mutex_;
  bool needs_to_disconnect_;
};

}   // namespace hippo

#endif   // INCLUDE_HIPPO_SWDEVICE_H_
