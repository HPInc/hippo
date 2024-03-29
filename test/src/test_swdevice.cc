﻿
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#include <windows.h>    // for Sleep()
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <thread>   // NOLINT
#include <mutex>     // NOLINT

#include "../include/adder.h"
#include "include/sohal.h"


extern void print_error(uint64_t err);

std::mutex badder_mutex;
std::condition_variable badder_condition;

//
// 'adder' sw device
//
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
    const wchar_t *msg = L"你好, I don't know my version"
        L" but here is a \U0001f412 with a \U0001f34c";
    size_t len = wcslen(msg)+1;
    v->resize(len);
    memcpy(v->data, msg, len*sizeof(wchar_t));
    return 0LL;
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

  uint64_t return_error_cb() {
    fprintf(stderr, "%s\n", __FUNCTION__);
    return MAKE_HIPPO_ERROR(facility_,
                            hippo::HIPPO_ERROR);
  }

  // this funtion has a long timeout, so we'll just sleep for 15 seconds
  uint64_t slow_call_cb(const int32_t &i, int32_t *j) {
    fprintf(stderr, "%s\n", __FUNCTION__);

    for (int i=15; i > 0; i--) {
      fprintf(stderr, "%s ** %d\n", __FUNCTION__, i);
      Sleep(1000);

      // create a wchardata outside the switch statement
      // to keep the compiler happy
      hippo::wcharptr wchardata(9);

      // create a notification every second
      switch (i%7){
        default:
        case 0:
          // no parameter
          SendNotification("slow_call_noparam");
          break;
        case 1:
          // int parameter
          SendNotification("slow_call_int", i);
          break;
        case 2:
          // example with float parameter
          SendNotification("slow_call_float",(float)i);
          break;
        case 3:
          // example with bool parameter
          SendNotification("slow_call_bool",true);
          break;
        case 4:
          // example with char* parameter
          SendNotification("slow_call_charptr","tick");
          break;
        case 5:
          // example with wcharptr parameter
          memcpy(wchardata.data,L"你好=hello",wcslen(L"你好=hello")*sizeof(wchar_t));
          SendNotification("slow_call_wcharptr", &wchardata);
          break;
        case 6:
          // example with b64bytes parameter (in this case ascii string "012345")
          hippo::b64bytes b64data(6);
          for (int i = 0; i < b64data.len-1; i++) {
            //48 is ASCII for "0", so this should create an array with ASCII "012345"
            b64data.data[i] = 48+i;
          }
          // null terminate this string
          b64data.data[b64data.len - 1] = 0;
          SendNotification("slow_call_b64", &b64data);
          break;
      }
    }
    fprintf(stderr, "%s Finished\n", __FUNCTION__);

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

  uint64_t infinite_timeout_cb() override {
	  for (int i = 17; i > 0; i--) {
		  fprintf(stderr, "%s will return in %d seconds\n", __FUNCTION__, i);
		  Sleep(1000);
	  }
	  fprintf(stderr, "%s Finished\n", __FUNCTION__);
	  return 0LL;
  }

  // will disconnect the sw device
  uint64_t disconnect_device_cb() {
    fprintf(stderr, "%s\n", __FUNCTION__);

    // tell the main sw device loop to exit
    needs_to_disconnect(true);

    return 0LL;
  }
};

int RunBlackAdder() {
  uint64_t err = 0LL;
  BlackAdder badder;

  // Note that we can run normal SoHal commands from here:
  hippo::SoHal sohal;
  char *version = NULL;
  if (err = sohal.version(&version)) {
    print_error(err);
    sohal.free_version(version);
  } else {
    fprintf(stderr, "sohal.version: '%s'\n", version);
  }
  // grab the mutex
  std::unique_lock<std::mutex> lock(badder_mutex, std::defer_lock);
  try {
    lock.lock();
  } catch (std::system_error e) {
    return -1;
  }
  // now connect as a SW device
  if (err = badder.connect_device()) {
    print_error(err);
    goto clean_up;
  }
  // and notify the caller
  lock.unlock();
  badder_condition.notify_all();

  // and wait for commands
  while (!badder.needs_to_disconnect()) {
    Sleep(1000);
  }
  // now disconnect as a SW device
  badder.disconnect_device();

clean_up:
  fprintf(stderr, "%s EXITING\n", __FUNCTION__);
  return 0;
}

//
// functions to connect and test the 'adder' sw device
//
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

// notification function
void swdevice_notification(const hippo::SWDeviceNotificationParam &param,
                           void *data) {
  fprintf(stderr, "**************************************\n");
  fprintf(stderr, "Received Notification for method %s\n", param.methodName);
  fprintf(stderr, "The notification data consists of:  ");

  // each notification can pass back some data, and each notification might
  // have different data.  In this example the BlackAdder server passes
  // notifications named slow_call_<type>, so let's examine the method name
  // to parse the correct type out of the param
  if (strcmp(param.methodName, "on_slow_call_noparam") == 0) {
    fprintf(stderr, "\t no parameter\n");
  }
  else if (strcmp(param.methodName, "on_slow_call_int") == 0) {
    fprintf(stderr, "\t uint32: %i\n", param.uint32Data);
  }
  else if (strcmp(param.methodName, "on_slow_call_float") == 0) {
    fprintf(stderr, "\t float: %f\n", param.floatData);
  }
  else if (strcmp(param.methodName, "on_slow_call_bool") == 0) {
    fprintf(stderr, "\t bool: %s\n", param.boolData ? "true" : "false");
  }
  else if (strcmp(param.methodName, "on_slow_call_charptr") == 0) {
    if (param.charData && strlen(param.charData) > 0) {
      fprintf(stderr, "\t string: %s\n", param.charData);
    }
  }
  else if (strcmp(param.methodName, "on_slow_call_wcharptr") == 0) {
    if (param.wcharData.data && wcslen(param.wcharData.data) > 0) {
      fprintf(stderr, "\t wchardata: %ws\n", param.wcharData.data);
    }
  }
  else if (strcmp(param.methodName, "on_slow_call_b64") == 0) {
    if (param.b64bytesData.len > 0) {
      // in this example we expect an ascii string "012345" so cast the data as char*
      fprintf(stderr, "B64bytes Data is %s\n", (char*)param.b64bytesData.data);
    }
  }
  else {
    fprintf(stderr, "Error - unknown notification received\n");
  }
  fprintf(stderr, "**************************************\n");
}

uint64_t TestSWDevice(void) {
  uint64_t err = 0LL;
  fprintf(stderr, "######################################\n");
  fprintf(stderr, "  Now Testing Adder SW device Commands\n");
  fprintf(stderr, "######################################\n");

  ADD_FILE_TO_MAP();   // will add this file to the file/error map

  std::unique_lock<std::mutex> lock(badder_mutex, std::defer_lock);
  try {
    lock.lock();
  } catch (std::system_error e) {
    return MAKE_HIPPO_ERROR(hippo::HIPPO_SWDEVICE,
                            hippo::HIPPO_ERROR);
  }
  std::thread badder_th(RunBlackAdder);

  // and wait for the SW device (server) to start
  int timeout = 1;
  if (std::cv_status::timeout ==
      badder_condition.wait_for(lock,
                                std::chrono::seconds(timeout))) {
    fprintf(stderr, "** TIMEOUT!!\n");
    return MAKE_HIPPO_ERROR(hippo::HIPPO_SWDEVICE,
                            hippo::HIPPO_TIMEOUT);
  }

  // now make the client
  hippo::Adder adder;

  // and subscribe to notifications
  uint32_t num_subscribed = 0;
  adder.subscribe(&swdevice_notification, reinterpret_cast<void*>(&adder),
                  &num_subscribed);

  // PointX from ./py/adder.json
  hippo::PointX p1, p2, pr;
  p1.x = 1;
  p1.y = 2;
  p2.x = 3;
  p2.y = 4;

  if (err = adder.add_point(p1, p2, &pr)) {
    print_error(err);
  } else {
    fprintf(stderr, "adder.add_point((%d,%d),(%d,%d)) = (%d, %d)\n",
            p1.x, p1.y, p2.x, p2.y, pr.x, pr.y);
  }
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
  } else {
    printCameraKeystone(kr);
  }
  hippo::wcharptr v;
  if (err=adder.version(&v)) {
    print_error(err);
  } else {
    fwprintf(stderr, L"version: '%s'\n", v.data);
  }
  if (err = adder.return_error()) {
    print_error(err);
  }

  hippo::b64bytes b1(127), b2(127), br1;
  for (int i=0; i < b1.len; i++) {
    b1.data[i] = i;
    b2.data[i] = i;
  }
  if (err = adder.binary_data(b1, b2, &br1)) {
    print_error(err);
  } else {
    for (int i=0; i < b1.len; i++) {
      if (b1.data[i] + b2.data[i] != br1.data[i]) {
        fprintf(stderr, "data[%d] %02x + %02x != %02x\n",
                i, b1.data[i], b2.data[i], br1.data[i]);
      }
    }
  }

  int32_t i, j;
  if (err = adder.slow_call(i, &j)) {
    print_error(err);
  }

  if (err = adder.infinite_timeout()) {
	  print_error(err);
  }

  hippo::DataWithB64Bytes h1, hr1;
  hippo::DataWithWcharptr h2;
  h1.counter = 10;
  h1.hidden_b64bytes.resize(h1.counter);
  h2.counter = h1.counter + 1;   // need space for '\0'
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

  if (err = adder.disconnect_device()) {
    print_error(err);
  }
  badder_th.join();

  return 0LL;
}
