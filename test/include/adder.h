
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#ifndef TEST_INCLUDE_ADDER_H_
#define TEST_INCLUDE_ADDER_H_

#include <include/hippo_swdevice.h>

namespace hippo {

typedef struct PointX {
  int32_t x;
  int32_t y;
} PointX;

typedef struct CameraQuadrilateralX {
  PointX top_left;
  PointX top_right;
  PointX bottom_left;
  PointX bottom_right;
} CameraQuadrilateralX;

typedef struct CameraKeystoneX {
  bool enabled;
  CameraQuadrilateralX value;
} CameraKeystoneX;

typedef struct DataWithB64Bytes {
  int32_t counter;
  b64bytes hidden_b64bytes;
} DataWithB64Bytes;

typedef struct DataWithWcharptr {
  int32_t counter;
  wcharptr hidden_wcharptr;
} DataWithWcharptr;


class Adder : public HippoSwDevice {
 public:
  using HippoSwDevice::disconnect_device;

  Adder();
  explicit Adder(uint32_t device_index);
  Adder(const char *address, uint32_t port);
  Adder(const char *address, uint32_t port, uint32_t device_index);
  virtual ~Adder(void);

  uint64_t connect_device(void);

  // to use the sw device as client

  // function that adds two points
  // p1 and p2 are two input parameters
  // pr is an oputput parameter
  uint64_t add_point(const PointX &p1,
                     const PointX &p2,
                     PointX *pr);
  // function that copies keystone parameters
  uint64_t keystone(const CameraKeystoneX &k,
                    CameraKeystoneX *kr);
  // function returns sw device version
  uint64_t version(wcharptr *v);
  // will disconnect the sw device
  uint64_t disconnect_device();
  // function using binary data
  uint64_t binary_data(const b64bytes &b1,
                       const b64bytes &b2,
                       b64bytes *br1);
  // function that will return an error
  // please note that does not have any parameter nor returns any value
  uint64_t return_error();
  // slow function call to test timeouts
  uint64_t slow_call(const int32_t &f1,
                     int32_t *f2);
  // function to test b64 and arrays inside schemas to test leaks
  uint64_t hidden_array(const DataWithB64Bytes &data_b64,
                        const DataWithWcharptr &data_wcharptr,
                        DataWithB64Bytes *ret);

  // override to implement sw device server callbacks

  // function that adds two points
  // p1 and p2 are two input parameters
  // pr is an oputput parameter
  virtual uint64_t add_point_cb(const PointX &p1,
                                const PointX &p2,
                                PointX *pr);
  // function that copies keystone parameters
  virtual uint64_t keystone_cb(const CameraKeystoneX &k,
                               CameraKeystoneX *kr);
  // function returns sw device version
  virtual uint64_t version_cb(wcharptr *v);
  // will disconnect the sw device
  virtual uint64_t disconnect_device_cb();
  // function using binary data
  virtual uint64_t binary_data_cb(const b64bytes &b1,
                                  const b64bytes &b2,
                                  b64bytes *br1);
  // function that will return an error
  // please note that does not have any parameter nor returns any value
  virtual uint64_t return_error_cb();
  // slow function call to test timeouts
  virtual uint64_t slow_call_cb(const int32_t &f1,
                                int32_t *f2);
  // function to test b64 and arrays inside schemas to test leaks
  virtual uint64_t hidden_array_cb(const DataWithB64Bytes &data_b64,
                                   const DataWithWcharptr &data_wcharptr,
                                   DataWithB64Bytes *ret);

 protected:
  uint64_t ProcessCommand(const char *method, void *params,
                          void *result) override;
  uint64_t add_point_cb_p(void *param, void *result);
  uint64_t keystone_cb_p(void *param, void *result);
  uint64_t version_cb_p(void *param, void *result);
  uint64_t disconnect_device_cb_p(void *param, void *result);
  uint64_t binary_data_cb_p(void *param, void *result);
  uint64_t return_error_cb_p(void *param, void *result);
  uint64_t slow_call_cb_p(void *param, void *result);
  uint64_t hidden_array_cb_p(void *param, void *result);

  // Adder's types parsers/generators
  uint64_t PointX_c2json(const PointX &set,
                         void *obj);
  uint64_t PointX_json2c(const void *obj,
                         PointX *get);
  uint64_t CameraQuadrilateralX_c2json(const CameraQuadrilateralX &set,
                                       void *obj);
  uint64_t CameraQuadrilateralX_json2c(const void *obj,
                                       CameraQuadrilateralX *get);
  uint64_t CameraKeystoneX_c2json(const CameraKeystoneX &set,
                                  void *obj);
  uint64_t CameraKeystoneX_json2c(const void *obj,
                                  CameraKeystoneX *get);
  uint64_t DataWithB64Bytes_c2json(const DataWithB64Bytes &set,
                                   void *obj);
  uint64_t DataWithB64Bytes_json2c(const void *obj,
                                   DataWithB64Bytes *get);
  uint64_t DataWithWcharptr_c2json(const DataWithWcharptr &set,
                                   void *obj);
  uint64_t DataWithWcharptr_json2c(const void *obj,
                                   DataWithWcharptr *get);
};

}   // namespace hippo

#endif   // TEST_INCLUDE_ADDER_H_
