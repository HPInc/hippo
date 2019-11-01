
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#include <cstdio>
#include <cstdlib>
#include "../include/adder.h"

void print_error(uint64_t err) {
  char err_msg[256];
  hippo::strerror(err, sizeof(err_msg), err_msg);
  fprintf(stderr, "[ERROR %08llx:%08llx]: %s\n",
          err>>32, err&0xffffffff, err_msg);
}

void printCameraKeystone(const hippo::CameraKeystoneX &ks) {
  fprintf(stderr, "-> Camera Keystone\n");
  fprintf(stderr, " \\ -> Enabled: %i\n", ks.enabled);
  fprintf(stderr, " \\ -> Value\n");
  fprintf(stderr, " \\    -> Bottom Left:  (%i, %i)\n",
          ks.value.bottom_left.x, ks.value.bottom_left.y);
  fprintf(stderr, " \\    -> Bottom Right: (%i, %i)\n",
          ks.value.bottom_right.x, ks.value.bottom_right.y);
  fprintf(stderr, " \\    -> Top Left:    (%i, %i)\n",
          ks.value.top_left.x, ks.value.top_left.y);
  fprintf(stderr, " \\    -> Top Right:   (%i, %i)\n",
          ks.value.top_right.x, ks.value.top_right.y);
}


int main(int argc, char *argv[]) {
  uint64_t err = 0LL;
  hippo::Adder adder;

  // PointX from ./py/adder.json
  hippo::PointX p1, p2, pr;
  p1.x = 1;
  p1.y = 2;
  p2.x = 3;
  p2.y = 4;

  if (err = adder.add_point(p1, p2, &pr)) {
    print_error(err);
  }
  fprintf(stderr, "adder.add_point((%d,%d),(%d,%d)) = (%d, %d)\n",
          p1.x, p1.y, p2.x, p2.y, pr.x, pr.y);

  // CameraKeystoneX from ./py/adder.json
  hippo::CameraKeystoneX k, kr;

  k.enabled = true;
  k.value.bottom_left.x = 10;
  k.value.bottom_left.y = 11;
  k.value.bottom_right.x = 12;
  k.value.bottom_right.y = 13;
  k.value.top_left.x = 14;
  k.value.top_left.y = 15;
  k.value.top_right.x = 16;
  k.value.top_right.y = 17;

  if (err=adder.keystone(k, &kr)) {
    print_error(err);
  }
  printCameraKeystone(kr);

  hippo::wcharptr v;
  if (err=adder.version(&v)) {
    print_error(err);
  }
  fwprintf(stderr, L"version: '%s'\n", v.data);

  if (err = adder.return_error()) {
    print_error(err);
  }

  hippo::b64bytes b1(10), b2(10), br1;
  for (int i=0; i < b1.len; i++) {
    b1.data[i] = i;
    b2.data[i] = i;
  }
  if (err = adder.binary_data(b1, b2, &br1)) {
    print_error(err);
  } else {
    fprintf(stderr, "<b1,b2,br1>:\n  ");
    for (int i=0; i < b1.len; i++) {
      fprintf(stderr, "<%02x,%02x,%02x>,", b1.data[i], b2.data[i], br1.data[i]);
    }
    fprintf(stderr, "\n");
  }

  hippo::DataWithB64Bytes h1, hr1;
  hippo::DataWithWcharptr h2;
  h1.counter = 10;
  h1.hidden_b64bytes.resize(h1.counter);
  h2.counter = 10;
  h2.hidden_wcharptr.resize(h2.counter);
  for (int i=0; i < h1.counter; i++) {
    h1.hidden_b64bytes.data[i] = i;
    h2.hidden_wcharptr.data[i] = i+1;
  }
  if (err = adder.hidden_array(h1, h2, &hr1)) {
    print_error(err);
  } else {
    fprintf(stderr, "<h1,h2,hr1>:\n  ");
    for (int i=0; i < h1.counter; i++) {
      fprintf(stderr, "<%02x,%02x,%02x>,",
              h1.hidden_b64bytes.data[i], h2.hidden_wcharptr.data[i],
              hr1.hidden_b64bytes.data[i]);
    }
    fprintf(stderr, "\n");
  }
  return 0;
}
