
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#include <windows.h>    // for Sleep
#include <cstdio>
#include <fstream>
#include <vector>
#include "../include/adder.h"


void print_error(uint64_t err) {
  char err_msg[256];
  hippo::strerror(err, sizeof(err_msg), err_msg);
  fprintf(stderr, "[ERROR %08llx:%08llx]: %s\n",
          err>>32, err&0xffffffff, err_msg);
}

class BlackAdder : public hippo::Adder {
  uint64_t add_point_cb(const hippo::PointX &p1,
                        const hippo::PointX &p2,
                        hippo::PointX *pr) override {
    fprintf(stderr, "%s\n", __FUNCTION__);

    // add them
    pr->x = p1.x + p2.x;
    pr->y = p1.y + p2.y;

    return 0LL;
  }

  uint64_t keystone_cb(const hippo::CameraKeystoneX &k,
                       hippo::CameraKeystoneX *kr) override {
    fprintf(stderr, "%s\n", __FUNCTION__);

    // copy param into result
    memcpy(kr, &k, sizeof(hippo::CameraKeystoneX));

    return 0LL;
  }

  uint64_t version_cb(hippo::wcharptr *v) override {
    fprintf(stderr, "%s\n", __FUNCTION__);

    wchar_t *wstr = L"你好, I don't know my version"
        L" but here is a \U0001f412 with a \U0001f34c";
    v->data = _wcsdup(wstr);

    return 0LL;
  }

  uint64_t return_error_cb() {
    fprintf(stderr, "%s\n", __FUNCTION__);
    return MAKE_HIPPO_ERROR(facility_,
                            hippo::HIPPO_ERROR);
  }

  uint64_t binary_data_cb(const hippo::b64bytes &b1,
                          const hippo::b64bytes &b2,
                          hippo::b64bytes *br1) override {
    fprintf(stderr, "%s\n", __FUNCTION__);
    if (b1.len != b2.len) {
      return -1;
    }
    br1->resize(b1.len);
    for (int i=0; i < b1.len; i++) {
      br1->data[i] = b1.data[i] + b2.data[i];
    }
    return 0LL;
  }

  uint64_t hidden_array_cb(const hippo::DataWithB64Bytes &data_b64,
                           const hippo::DataWithWcharptr &data_wcharptr,
                           hippo::DataWithB64Bytes *ret) override {
    fprintf(stderr, "%s\n", __FUNCTION__);

    ret->counter = data_b64.counter;
    ret->hidden_b64bytes.resize(ret->counter);
    for (int i=0; i < ret->counter; i++) {
      ret->hidden_b64bytes.data[i] = data_b64.hidden_b64bytes.data[i] +
          data_wcharptr.hidden_wcharptr.data[i];
    }
    return 0LL;
  }
};


int main(int argc, char *argv[]) {
  uint64_t err = 0LL;
  BlackAdder badder;

  if (err = badder.connect_device()) {
    print_error(err);
    goto clean_up;
  }
  while (!badder.needs_to_disconnect()) {
    Sleep(1000);
  }
  // now disconnect as a SW device
  badder.disconnect_device();

clean_up:
  return 0;
}
