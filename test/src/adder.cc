
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#include <include/json.hpp>
#include "../include/adder.h"

namespace nl = nlohmann;

namespace hippo {

const char devName[] = "adder";

Adder::Adder() :
    HippoSwDevice(devName) {
  ADD_FILE_TO_MAP();
}

Adder::Adder(uint32_t device_index) :
    HippoSwDevice(devName, device_index) {
}

Adder::Adder(const char *address, uint32_t port) :
    HippoSwDevice(devName, address, port) {
}

Adder::Adder(const char *address, uint32_t port,
                         uint32_t device_index) :
    HippoSwDevice(devName, address, port, device_index) {
}

Adder::~Adder(void) {
}

// member functions implementing the client side of the sw device

uint64_t Adder::add_point(const PointX &p1,
                          const PointX &p2,
                          PointX *pr) {
  uint64_t err = 0LL;

  // parse parameters
  nl::json jp1;
  void *jp1_ptr = reinterpret_cast<void*>(&jp1);
  if (err = PointX_c2json(p1, jp1_ptr)) {
    return err;
  }
  nl::json jp2;
  void *jp2_ptr = reinterpret_cast<void*>(&jp2);
  if (err = PointX_c2json(p2, jp2_ptr)) {
    return err;
  }
  // create a list with all parameters
  nl::json jset;
  jset.push_back(jp1);
  jset.push_back(jp2);
  void *jset_ptr = reinterpret_cast<void*>(&jset);

  // send command
  nl::json jget;
  void *jget_ptr = reinterpret_cast<void*>(&jget);

  // std::string s = reinterpret_cast<nl::json*>(jset_ptr)->dump();
  // fprintf(stderr, "SendRawMsg('%s')\n", s.c_str());
  int timeout = 1;
  if (err = SendRawMsg("add_point", jset_ptr, timeout, jget_ptr)) {
    return err;
  }
  // get the results
  // s = reinterpret_cast<nl::json*>(jget_ptr)->dump();
  // fprintf(stderr, "  Response('%s')\n", s.c_str());
  if (pr != NULL) {
    err = PointX_json2c(jget_ptr, pr);
  }
  return err;
}

uint64_t Adder::keystone(const CameraKeystoneX &k,
                         CameraKeystoneX *kr) {
  uint64_t err = 0LL;

  // parse parameters
  nl::json jk;
  void *jk_ptr = reinterpret_cast<void*>(&jk);
  if (err = CameraKeystoneX_c2json(k, jk_ptr)) {
    return err;
  }
  // create a list with all parameters
  nl::json jset;
  jset.push_back(jk);
  void *jset_ptr = reinterpret_cast<void*>(&jset);

  // send command
  nl::json jget;
  void *jget_ptr = reinterpret_cast<void*>(&jget);

  // std::string s = reinterpret_cast<nl::json*>(jset_ptr)->dump();
  // fprintf(stderr, "SendRawMsg('%s')\n", s.c_str());
  int timeout = 1;
  if (err = SendRawMsg("keystone", jset_ptr, timeout, jget_ptr)) {
    return err;
  }
  // get the results
  // s = reinterpret_cast<nl::json*>(jget_ptr)->dump();
  // fprintf(stderr, "  Response('%s')\n", s.c_str());
  if (kr != NULL) {
    err = CameraKeystoneX_json2c(jget_ptr, kr);
  }
  return err;
}

uint64_t Adder::version(wcharptr *v) {
  uint64_t err = 0LL;

  // parse parameters
  // create a list with all parameters
  nl::json jset;
  void *jset_ptr = reinterpret_cast<void*>(&jset);

  // send command
  nl::json jget;
  void *jget_ptr = reinterpret_cast<void*>(&jget);

  // std::string s = reinterpret_cast<nl::json*>(jset_ptr)->dump();
  // fprintf(stderr, "SendRawMsg('%s')\n", s.c_str());
  int timeout = 1;
  if (err = SendRawMsg("version", jset_ptr, timeout, jget_ptr)) {
    return err;
  }
  // get the results
  // s = reinterpret_cast<nl::json*>(jget_ptr)->dump();
  // fprintf(stderr, "  Response('%s')\n", s.c_str());
  if (v != NULL) {
    err = wcharptr_json2c(jget_ptr, v);
  }
  return err;
}

uint64_t Adder::disconnect_device() {
  uint64_t err = 0LL;

  // parse parameters
  // create a list with all parameters
  nl::json jset;
  void *jset_ptr = reinterpret_cast<void*>(&jset);

  // send command
  nl::json jget;
  void *jget_ptr = reinterpret_cast<void*>(&jget);

  // std::string s = reinterpret_cast<nl::json*>(jset_ptr)->dump();
  // fprintf(stderr, "SendRawMsg('%s')\n", s.c_str());
  int timeout = 1;
  if (err = SendRawMsg("disconnect_device", jset_ptr, timeout, jget_ptr)) {
    return err;
  }
  // get the results
  // s = reinterpret_cast<nl::json*>(jget_ptr)->dump();
  // fprintf(stderr, "  Response('%s')\n", s.c_str());

  return err;
}

uint64_t Adder::binary_data(const b64bytes &b1,
                            const b64bytes &b2,
                            b64bytes *br1) {
  uint64_t err = 0LL;

  // parse parameters
  nl::json jb1;
  void *jb1_ptr = reinterpret_cast<void*>(&jb1);
  if (err = b64bytes_c2json(b1, jb1_ptr)) {
    return err;
  }
  nl::json jb2;
  void *jb2_ptr = reinterpret_cast<void*>(&jb2);
  if (err = b64bytes_c2json(b2, jb2_ptr)) {
    return err;
  }
  // create a list with all parameters
  nl::json jset;
  jset.push_back(jb1);
  jset.push_back(jb2);
  void *jset_ptr = reinterpret_cast<void*>(&jset);

  // send command
  nl::json jget;
  void *jget_ptr = reinterpret_cast<void*>(&jget);

  // std::string s = reinterpret_cast<nl::json*>(jset_ptr)->dump();
  // fprintf(stderr, "SendRawMsg('%s')\n", s.c_str());
  int timeout = 10;
  if (err = SendRawMsg("binary_data", jset_ptr, timeout, jget_ptr)) {
    return err;
  }
  // get the results
  // s = reinterpret_cast<nl::json*>(jget_ptr)->dump();
  // fprintf(stderr, "  Response('%s')\n", s.c_str());
  if (br1 != NULL) {
    err = b64bytes_json2c(jget_ptr, br1);
  }
  return err;
}

uint64_t Adder::return_error() {
  uint64_t err = 0LL;

  // parse parameters
  // create a list with all parameters
  nl::json jset;
  void *jset_ptr = reinterpret_cast<void*>(&jset);

  // send command
  nl::json jget;
  void *jget_ptr = reinterpret_cast<void*>(&jget);

  // std::string s = reinterpret_cast<nl::json*>(jset_ptr)->dump();
  // fprintf(stderr, "SendRawMsg('%s')\n", s.c_str());
  int timeout = 2;
  if (err = SendRawMsg("return_error", jset_ptr, timeout, jget_ptr)) {
    return err;
  }
  // get the results
  // s = reinterpret_cast<nl::json*>(jget_ptr)->dump();
  // fprintf(stderr, "  Response('%s')\n", s.c_str());

  return err;
}

uint64_t Adder::slow_call(const int32_t &f1,
                          int32_t *f2) {
  uint64_t err = 0LL;

  // parse parameters
  nl::json jf1;
  void *jf1_ptr = reinterpret_cast<void*>(&jf1);
  if (err = int32_t_c2json(f1, jf1_ptr)) {
    return err;
  }
  // create a list with all parameters
  nl::json jset;
  jset.push_back(jf1);
  void *jset_ptr = reinterpret_cast<void*>(&jset);

  // send command
  nl::json jget;
  void *jget_ptr = reinterpret_cast<void*>(&jget);

  // std::string s = reinterpret_cast<nl::json*>(jset_ptr)->dump();
  // fprintf(stderr, "SendRawMsg('%s')\n", s.c_str());
  int timeout = 30;
  if (err = SendRawMsg("slow_call", jset_ptr, timeout, jget_ptr)) {
    return err;
  }
  // get the results
  // s = reinterpret_cast<nl::json*>(jget_ptr)->dump();
  // fprintf(stderr, "  Response('%s')\n", s.c_str());
  if (f2 != NULL) {
    err = int32_t_json2c(jget_ptr, f2);
  }
  return err;
}

uint64_t Adder::hidden_array(const DataWithB64Bytes &data_b64,
                             const DataWithWcharptr &data_wcharptr,
                             DataWithB64Bytes *ret) {
  uint64_t err = 0LL;

  // parse parameters
  nl::json jdata_b64;
  void *jdata_b64_ptr = reinterpret_cast<void*>(&jdata_b64);
  if (err = DataWithB64Bytes_c2json(data_b64, jdata_b64_ptr)) {
    return err;
  }
  nl::json jdata_wcharptr;
  void *jdata_wcharptr_ptr = reinterpret_cast<void*>(&jdata_wcharptr);
  if (err = DataWithWcharptr_c2json(data_wcharptr, jdata_wcharptr_ptr)) {
    return err;
  }
  // create a list with all parameters
  nl::json jset;
  jset.push_back(jdata_b64);
  jset.push_back(jdata_wcharptr);
  void *jset_ptr = reinterpret_cast<void*>(&jset);

  // send command
  nl::json jget;
  void *jget_ptr = reinterpret_cast<void*>(&jget);

  // std::string s = reinterpret_cast<nl::json*>(jset_ptr)->dump();
  // fprintf(stderr, "SendRawMsg('%s')\n", s.c_str());
  int timeout = 1;
  if (err = SendRawMsg("hidden_array", jset_ptr, timeout, jget_ptr)) {
    return err;
  }
  // get the results
  // s = reinterpret_cast<nl::json*>(jget_ptr)->dump();
  // fprintf(stderr, "  Response('%s')\n", s.c_str());
  if (ret != NULL) {
    err = DataWithB64Bytes_json2c(jget_ptr, ret);
  }
  return err;
}

// member functions implementing the server side of the sw device
const char Adder_device_json[] =
"[{"
" \"device_name\": \"adder\","
" \"api\": ["
"  {"
"   \"doc\": ["
"    \"function that adds two points\","
"    \"p1 and p2 are two input parameters\","
"    \"pr is an oputput parameter\""
"   ],"
"   \"method\": \"add_point\","
"   \"params\": ["
"    {"
"     \"name\": \"p1\","
"     \"type\": \"PointX\""
"    },"
"    {"
"     \"name\": \"p2\","
"     \"type\": \"PointX\""
"    }"
"   ],"
"   \"result\": ["
"    {"
"     \"name\": \"pr\","
"     \"type\": \"PointX\""
"    }"
"   ],"
"   \"timeout\": 1"
"  },"
"  {"
"   \"doc\": ["
"    \"function that copies keystone parameters\""
"   ],"
"   \"method\": \"keystone\","
"   \"params\": ["
"    {"
"     \"name\": \"k\","
"     \"type\": \"CameraKeystoneX\""
"    }"
"   ],"
"   \"result\": ["
"    {"
"     \"name\": \"kr\","
"     \"type\": \"CameraKeystoneX\""
"    }"
"   ],"
"   \"timeout\": 1"
"  },"
"  {"
"   \"doc\": ["
"    \"function returns sw device version\""
"   ],"
"   \"method\": \"version\","
"   \"params\": [],"
"   \"result\": ["
"    {"
"     \"name\": \"v\","
"     \"type\": \"string\""
"    }"
"   ],"
"   \"timeout\": 1"
"  },"
"  {"
"   \"doc\": ["
"    \"will disconnect the sw device\""
"   ],"
"   \"method\": \"disconnect_device\","
"   \"params\": [],"
"   \"result\": [],"
"   \"timeout\": 1"
"  },"
"  {"
"   \"doc\": ["
"    \"function using binary data\""
"   ],"
"   \"method\": \"binary_data\","
"   \"params\": ["
"    {"
"     \"name\": \"b1\","
"     \"type\": \"b64bytes\""
"    },"
"    {"
"     \"name\": \"b2\","
"     \"type\": \"b64bytes\""
"    }"
"   ],"
"   \"result\": ["
"    {"
"     \"name\": \"br1\","
"     \"type\": \"b64bytes\""
"    }"
"   ],"
"   \"timeout\": 10"
"  },"
"  {"
"   \"doc\": ["
"    \"function that will return an error\","
"    \"please note that does not have any parameter nor returns any value\""
"   ],"
"   \"method\": \"return_error\","
"   \"params\": [],"
"   \"result\": [],"
"   \"timeout\": 2"
"  },"
"  {"
"   \"doc\": ["
"    \"slow function call to test timeouts\""
"   ],"
"   \"method\": \"slow_call\","
"   \"params\": ["
"    {"
"     \"name\": \"f1\","
"     \"type\": \"integer\""
"    }"
"   ],"
"   \"result\": ["
"    {"
"     \"name\": \"f2\","
"     \"type\": \"integer\""
"    }"
"   ],"
"   \"timeout\": 30"
"  },"
"  {"
"   \"doc\": ["
"    \"function to test b64 and arrays inside schemas to test leaks\""
"   ],"
"   \"method\": \"hidden_array\","
"   \"params\": ["
"    {"
"     \"name\": \"data_b64\","
"     \"type\": \"DataWithB64Bytes\""
"    },"
"    {"
"     \"name\": \"data_wcharptr\","
"     \"type\": \"DataWithWcharptr\""
"    }"
"   ],"
"   \"result\": ["
"    {"
"     \"name\": \"ret\","
"     \"type\": \"DataWithB64Bytes\""
"    }"
"   ],"
"   \"timeout\": 1"
"  }"
" ]"
"}]";

uint64_t Adder::connect_device(void) {
  uint64_t err = 0LL;
  if (err = HippoSwDevice::connect_device(Adder_device_json)) {
    return err;
  }
  return 0LL;
}

uint64_t Adder::ProcessCommand(const char *method, void *param, void *result) {
  typedef uint64_t (Adder::*cb)(void*, void*);

  std::map<std::string, cb> map = {
    {"add_point", &Adder::add_point_cb_p},
    {"keystone", &Adder::keystone_cb_p},
    {"version", &Adder::version_cb_p},
    {"disconnect_device", &Adder::disconnect_device_cb_p},
    {"binary_data", &Adder::binary_data_cb_p},
    {"return_error", &Adder::return_error_cb_p},
    {"slow_call", &Adder::slow_call_cb_p},
    {"hidden_array", &Adder::hidden_array_cb_p},
  };
  return (this->*(map[method]))(param, result);
}

// callbacks
uint64_t Adder::add_point_cb(const PointX &,
                             const PointX &,
                             PointX *) {
  return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_FUNC_NOT_AVAILABLE);
}

uint64_t Adder::keystone_cb(const CameraKeystoneX &,
                            CameraKeystoneX *) {
  return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_FUNC_NOT_AVAILABLE);
}

uint64_t Adder::version_cb(wcharptr *) {
  return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_FUNC_NOT_AVAILABLE);
}

uint64_t Adder::disconnect_device_cb() {
  return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_FUNC_NOT_AVAILABLE);
}

uint64_t Adder::binary_data_cb(const b64bytes &,
                               const b64bytes &,
                               b64bytes *) {
  return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_FUNC_NOT_AVAILABLE);
}

uint64_t Adder::return_error_cb() {
  return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_FUNC_NOT_AVAILABLE);
}

uint64_t Adder::slow_call_cb(const int32_t &,
                             int32_t *) {
  return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_FUNC_NOT_AVAILABLE);
}

uint64_t Adder::hidden_array_cb(const DataWithB64Bytes &,
                                const DataWithWcharptr &,
                                DataWithB64Bytes *) {
  return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_FUNC_NOT_AVAILABLE);
}

// callbacks private
uint64_t Adder::add_point_cb_p(void *param, void *result) {
  uint64_t err = 0LL;
  nl::json *params = reinterpret_cast<nl::json*>(param);

  PointX p1;
  try {
    if (err = PointX_json2c(&params->at(0), &p1)) {
      return err;
    }
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  PointX p2;
  try {
    if (err = PointX_json2c(&params->at(1), &p2)) {
      return err;
    }
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  PointX pr;
  if (err = add_point_cb(p1, p2, &pr)) {
    return err;
  }
  if (err = PointX_c2json(pr, result)) {
    return err;
  }
  return err;
}

uint64_t Adder::keystone_cb_p(void *param, void *result) {
  uint64_t err = 0LL;
  nl::json *params = reinterpret_cast<nl::json*>(param);

  CameraKeystoneX k;
  try {
    if (err = CameraKeystoneX_json2c(&params->at(0), &k)) {
      return err;
    }
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  CameraKeystoneX kr;
  if (err = keystone_cb(k, &kr)) {
    return err;
  }
  if (err = CameraKeystoneX_c2json(kr, result)) {
    return err;
  }
  return err;
}

uint64_t Adder::version_cb_p(void *param, void *result) {
  uint64_t err = 0LL;
  nl::json *params = reinterpret_cast<nl::json*>(param);

  wcharptr v;
  if (err = version_cb(&v)) {
    return err;
  }
  if (err = wcharptr_c2json(v, result)) {
    return err;
  }
  return err;
}

uint64_t Adder::disconnect_device_cb_p(void *param, void *result) {
  uint64_t err = 0LL;
  nl::json *params = reinterpret_cast<nl::json*>(param);


  if (err = disconnect_device_cb()) {
    return err;
  }  return err;
}

uint64_t Adder::binary_data_cb_p(void *param, void *result) {
  uint64_t err = 0LL;
  nl::json *params = reinterpret_cast<nl::json*>(param);

  b64bytes b1;
  try {
    if (err = b64bytes_json2c(&params->at(0), &b1)) {
      return err;
    }
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  b64bytes b2;
  try {
    if (err = b64bytes_json2c(&params->at(1), &b2)) {
      return err;
    }
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  b64bytes br1;
  if (err = binary_data_cb(b1, b2, &br1)) {
    return err;
  }
  if (err = b64bytes_c2json(br1, result)) {
    return err;
  }
  return err;
}

uint64_t Adder::return_error_cb_p(void *param, void *result) {
  uint64_t err = 0LL;
  nl::json *params = reinterpret_cast<nl::json*>(param);


  if (err = return_error_cb()) {
    return err;
  }  return err;
}

uint64_t Adder::slow_call_cb_p(void *param, void *result) {
  uint64_t err = 0LL;
  nl::json *params = reinterpret_cast<nl::json*>(param);

  int32_t f1;
  try {
    if (err = int32_t_json2c(&params->at(0), &f1)) {
      return err;
    }
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  int32_t f2;
  if (err = slow_call_cb(f1, &f2)) {
    return err;
  }
  if (err = int32_t_c2json(f2, result)) {
    return err;
  }
  return err;
}

uint64_t Adder::hidden_array_cb_p(void *param, void *result) {
  uint64_t err = 0LL;
  nl::json *params = reinterpret_cast<nl::json*>(param);

  DataWithB64Bytes data_b64;
  try {
    if (err = DataWithB64Bytes_json2c(&params->at(0), &data_b64)) {
      return err;
    }
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  DataWithWcharptr data_wcharptr;
  try {
    if (err = DataWithWcharptr_json2c(&params->at(1), &data_wcharptr)) {
      return err;
    }
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  DataWithB64Bytes ret;
  if (err = hidden_array_cb(data_b64, data_wcharptr, &ret)) {
    return err;
  }
  if (err = DataWithB64Bytes_c2json(ret, result)) {
    return err;
  }
  return err;
}

// Adder's types parser/generators
uint64_t Adder::PointX_c2json(const PointX &set, void *obj) {
  if (obj == NULL) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  nl::json params;
  nl::json x;
  if (int32_t_c2json(set.x, &x)) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  params["x"] = x;

  nl::json y;
  if (int32_t_c2json(set.y, &y)) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  params["y"] = y;

  *(reinterpret_cast<nl::json*>(obj)) = params;
  return HIPPO_OK;
}

uint64_t Adder::PointX_json2c(const void *obj, PointX *get) {
  uint64_t err = HIPPO_OK;
  if (obj == NULL || get == NULL) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  const nl::json *j = reinterpret_cast<const nl::json*>(obj);
  if (!j->is_object()) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_MESSAGE_ERROR);
  }
  const void *j_obj = NULL;
  try {
    j_obj = reinterpret_cast<const void*>(&j->at("x"));
    if (int32_t_json2c(j_obj, &(get->x))) {
      return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
    }
    j_obj = reinterpret_cast<const void*>(&j->at("y"));
    if (int32_t_json2c(j_obj, &(get->y))) {
      return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
    }
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_ERROR);
  }
  return err;
}

uint64_t Adder::CameraQuadrilateralX_c2json(const CameraQuadrilateralX &set,
                                            void *obj) {
  if (obj == NULL) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  nl::json params;
  nl::json top_left;
  if (PointX_c2json(set.top_left, &top_left)) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  params["top_left"] = top_left;

  nl::json top_right;
  if (PointX_c2json(set.top_right, &top_right)) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  params["top_right"] = top_right;

  nl::json bottom_left;
  if (PointX_c2json(set.bottom_left, &bottom_left)) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  params["bottom_left"] = bottom_left;

  nl::json bottom_right;
  if (PointX_c2json(set.bottom_right, &bottom_right)) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  params["bottom_right"] = bottom_right;

  *(reinterpret_cast<nl::json*>(obj)) = params;
  return HIPPO_OK;
}

uint64_t Adder::CameraQuadrilateralX_json2c(const void *obj,
                                            CameraQuadrilateralX *get) {
  uint64_t err = HIPPO_OK;
  if (obj == NULL || get == NULL) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  const nl::json *j = reinterpret_cast<const nl::json*>(obj);
  if (!j->is_object()) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_MESSAGE_ERROR);
  }
  const void *j_obj = NULL;
  try {
    j_obj = reinterpret_cast<const void*>(&j->at("top_left"));
    if (PointX_json2c(j_obj, &(get->top_left))) {
      return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
    }
    j_obj = reinterpret_cast<const void*>(&j->at("top_right"));
    if (PointX_json2c(j_obj, &(get->top_right))) {
      return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
    }
    j_obj = reinterpret_cast<const void*>(&j->at("bottom_left"));
    if (PointX_json2c(j_obj, &(get->bottom_left))) {
      return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
    }
    j_obj = reinterpret_cast<const void*>(&j->at("bottom_right"));
    if (PointX_json2c(j_obj, &(get->bottom_right))) {
      return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
    }
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_ERROR);
  }
  return err;
}

uint64_t Adder::CameraKeystoneX_c2json(const CameraKeystoneX &set, void *obj) {
  if (obj == NULL) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  nl::json params;
  nl::json enabled;
  if (bool_c2json(set.enabled, &enabled)) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  params["enabled"] = enabled;

  nl::json value;
  if (CameraQuadrilateralX_c2json(set.value, &value)) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  params["value"] = value;

  *(reinterpret_cast<nl::json*>(obj)) = params;
  return HIPPO_OK;
}

uint64_t Adder::CameraKeystoneX_json2c(const void *obj, CameraKeystoneX *get) {
  uint64_t err = HIPPO_OK;
  if (obj == NULL || get == NULL) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  const nl::json *j = reinterpret_cast<const nl::json*>(obj);
  if (!j->is_object()) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_MESSAGE_ERROR);
  }
  const void *j_obj = NULL;
  try {
    j_obj = reinterpret_cast<const void*>(&j->at("enabled"));
    if (bool_json2c(j_obj, &(get->enabled))) {
      return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
    }
    j_obj = reinterpret_cast<const void*>(&j->at("value"));
    if (CameraQuadrilateralX_json2c(j_obj, &(get->value))) {
      return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
    }
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_ERROR);
  }
  return err;
}

uint64_t Adder::DataWithB64Bytes_c2json(const DataWithB64Bytes &set,
                                        void *obj) {
  if (obj == NULL) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  nl::json params;
  nl::json counter;
  if (int32_t_c2json(set.counter, &counter)) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  params["counter"] = counter;

  nl::json hidden_b64bytes;
  if (b64bytes_c2json(set.hidden_b64bytes, &hidden_b64bytes)) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  params["hidden_b64bytes"] = hidden_b64bytes;

  *(reinterpret_cast<nl::json*>(obj)) = params;
  return HIPPO_OK;
}

uint64_t Adder::DataWithB64Bytes_json2c(const void *obj,
                                        DataWithB64Bytes *get) {
  uint64_t err = HIPPO_OK;
  if (obj == NULL || get == NULL) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  const nl::json *j = reinterpret_cast<const nl::json*>(obj);
  if (!j->is_object()) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_MESSAGE_ERROR);
  }
  const void *j_obj = NULL;
  try {
    j_obj = reinterpret_cast<const void*>(&j->at("counter"));
    if (int32_t_json2c(j_obj, &(get->counter))) {
      return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
    }
    j_obj = reinterpret_cast<const void*>(&j->at("hidden_b64bytes"));
    if (b64bytes_json2c(j_obj, &(get->hidden_b64bytes))) {
      return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
    }
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_ERROR);
  }
  return err;
}

uint64_t Adder::DataWithWcharptr_c2json(const DataWithWcharptr &set,
                                        void *obj) {
  if (obj == NULL) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  nl::json params;
  nl::json counter;
  if (int32_t_c2json(set.counter, &counter)) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  params["counter"] = counter;

  nl::json hidden_wcharptr;
  if (wcharptr_c2json(set.hidden_wcharptr, &hidden_wcharptr)) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  params["hidden_wcharptr"] = hidden_wcharptr;

  *(reinterpret_cast<nl::json*>(obj)) = params;
  return HIPPO_OK;
}

uint64_t Adder::DataWithWcharptr_json2c(const void *obj,
                                        DataWithWcharptr *get) {
  uint64_t err = HIPPO_OK;
  if (obj == NULL || get == NULL) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
  }
  const nl::json *j = reinterpret_cast<const nl::json*>(obj);
  if (!j->is_object()) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_MESSAGE_ERROR);
  }
  const void *j_obj = NULL;
  try {
    j_obj = reinterpret_cast<const void*>(&j->at("counter"));
    if (int32_t_json2c(j_obj, &(get->counter))) {
      return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
    }
    j_obj = reinterpret_cast<const void*>(&j->at("hidden_wcharptr"));
    if (wcharptr_json2c(j_obj, &(get->hidden_wcharptr))) {
      return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_INVALID_PARAM);
    }
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_ERROR);
  }
  return err;
}


}    // namespace hippo
