
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#include <windows.h>    // for Sleep()
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "include/hirescamera.h"


extern const char wsAddress[];
extern const uint32_t wsPort;

extern void print_error(uint64_t err);
extern void print_error(uint64_t err, const char *msg);
extern void print_device_info(const hippo::DeviceInfo &info);
void printAutoOrFixed(const hippo::AutoOrFixed &param);
void printCameraSettings(const hippo::CameraSettings &cs);
void printCameraKeystone(const hippo::CameraKeystone &ks);
void printCameraKeystoneTable(const hippo::CameraKeystoneTable &ks);
void printKeystoneTableEntries(const hippo::CameraKeystoneTableEntries &entries,
                               const uint32_t &num_entries);
void printCameraStatus(const hippo::CameraDeviceStatus &camera_status);
void printCameraLEDState(const hippo::CameraLedState *state);
void printCameraResolution(const hippo::CameraResolution &res);
void printPowerLineFrequency(const hippo::PowerLineFrequency &param);
void hirescamera_notification(const hippo::HiResCameraNotificationParam &param,
                              void *data);

extern uint64_t TestCameraStreams(hippo::HippoCamera *cam,
                                  hippo::CameraStreams st);

uint64_t TestHiResCamera(hippo::HiResCamera *hirescamera) {
  uint64_t err;
  fprintf(stderr, "##################################\n");
  fprintf(stderr, "    Now Testing HiRes Camera\n");
  fprintf(stderr, "##################################\n");

  ADD_FILE_TO_MAP();   // will add this file to the file/error map

  // subscribe
  uint32_t num_subscribe = 0;
  if (err = hirescamera->subscribe(&hirescamera_notification,
                                   reinterpret_cast<void*>(hirescamera),
                                   &num_subscribe)) {
    print_error(err);
  } else {
    fprintf(stderr, "hirescamera.subscribe: count: %d\n", num_subscribe);
  }

  bool bConnected;
  if (err = hirescamera->is_device_connected(&bConnected)) {
    return err;
  }
  if (bConnected) {
    fprintf(stderr, "Hirescamera is connected\n");
  } else {
    fprintf(stderr, "No hirescamera is connected -- not running test\n");
    return MAKE_HIPPO_ERROR(hippo::HIPPO_DEVICE,
                            hippo::HIPPO_DEV_NOT_FOUND);
  }

  // open
  uint32_t open_count = 0;
  if (err = hirescamera->open(&open_count)) {
    return err;
  }
  fprintf(stderr, "hirescamera.open(): open_count: %d\n", open_count);

  // test all streams combinations
  hippo::CameraStreams st = { 1 };
  if (err = TestCameraStreams(hirescamera, st)) {
    print_error(err);
  }

  // camera_settings
  hippo::CameraSettings cs_1, cs_2;
  if (err = hirescamera->camera_settings(&cs_1)) {
    print_error(err, "hirescamera.camera_settings get");
  } else  {
    printCameraSettings(cs_1);
  }

  bool bCameraSupportsTest = true;
  // Camera LED State
  hippo::CameraLedState state1, state2;
  // get current state
  if (err = hirescamera->led_state(&state1)) {
    print_error(err);
    // sprout camera doesn't support this function - don't error out
    if (hippo::HippoErrorCode(err) == hippo::HIPPO_FUNC_NOT_AVAILABLE) {
      bCameraSupportsTest = false;
    } else {
      return err;
    }
  } else {
    printCameraLEDState(&state1);
  }

  if (bCameraSupportsTest) {
    // turn the states to off
    state2.capture = hippo::CameraLedStateEnum::off;
    state2.streaming = hippo::CameraLedStateEnum::off;
    if (err = hirescamera->led_state(state2)) {
      print_error(err);
    } else {
      printCameraLEDState(&state2);
    }
    // now re-set the states
    if (err = hirescamera->led_state(state1, &state2)) {
      print_error(err);
    } else {
      printCameraLEDState(&state2);
    }
  }

  hippo::CameraDeviceStatus camera_status;
  if (err = hirescamera->device_status(&camera_status)) {
    print_error(err);
  } else {
    printCameraStatus(camera_status);
  }

  bCameraSupportsTest = true;
  // Camera Keystone Table
  hippo::CameraKeystoneTable kst1, kst2;
  // get current PLF
  if (err = hirescamera->keystone_table(&kst1)) {
    print_error(err);
    // sprout camera doesn't support this function - don't error out
    if (hippo::HippoErrorCode(err) == hippo::HIPPO_FUNC_NOT_AVAILABLE) {
      bCameraSupportsTest = false;
    } else {
      return err;
    }
  } else {
    printCameraKeystoneTable(kst1);
  }

  if (bCameraSupportsTest) {
    // change the table
    if (static_cast<uint32_t>(kst1) < 4) {
      kst2 = hippo::CameraKeystoneTable::RAM;
    } else {
      kst2 = hippo::CameraKeystoneTable::DEFAULT;
    }

    if (err = hirescamera->keystone_table(kst2)) {
      print_error(err);
    } else {
      printCameraKeystoneTable(kst2);
    }
    // now re-set the states
    if (err = hirescamera->keystone_table(kst1, &kst2)) {
      print_error(err);
    } else {
      printCameraKeystoneTable(kst2);
    }
  }

  bCameraSupportsTest = true;
  // Camera Power Line Frequency
  hippo::PowerLineFrequency p1, p2;
  // get current PLF
  if (err = hirescamera->power_line_frequency(&p1)) {
    print_error(err);
    // sprout camera doesn't support this function - don't error out
    if (hippo::HippoErrorCode(err) == hippo::HIPPO_FUNC_NOT_AVAILABLE) {
      bCameraSupportsTest = false;
    } else {
      return err;
    }
  } else {
    printPowerLineFrequency(p1);
  }

  if (bCameraSupportsTest) {
    // change the frequency
    if (p1 == hippo::PowerLineFrequency::disabled) {
      p2 = hippo::PowerLineFrequency::hz_60;
    } else {
      p2 = hippo::PowerLineFrequency::disabled;
    }

    if (err = hirescamera->power_line_frequency(p2)) {
      print_error(err);
    } else {
      printPowerLineFrequency(p2);
    }
    // now re-set the states
    if (err = hirescamera->power_line_frequency(p1, &p2)) {
      print_error(err);
    } else {
      printPowerLineFrequency(p2);
    }
  }

  hippo::CameraResolution res, parent;
  if (err = hirescamera->streaming_resolution(&res)) {
    print_error(err);
  } else {
    fprintf(stderr, "Current Streaming ");
    printCameraResolution(res);
  }

  if (err = hirescamera->parent_resolution(&res)) {
    print_error(err);
  } else {
    fprintf(stderr, "Parent Resolution of Current ");
    printCameraResolution(res);
  }

  res.width = 2176;
  res.height = 1448;
  res.fps = 25;

  if (err = hirescamera->parent_resolution(res, &parent)) {
    print_error(err);
  } else {
    fprintf(stderr, "Parent Resolution of 2176 x 1448 @25hz is: ");
    printCameraResolution(parent);
  }

  bCameraSupportsTest = true;
  // keystone table entries
  hippo::CameraKeystoneTable kstable;
  hippo::CameraKeystoneTableEntries ksTableEntries, ksTableEntries2;
  uint32_t num_entries1 = 0, num_entries2 = 0;

  memset(&ksTableEntries, 0, sizeof(hippo::CameraKeystoneTableEntries));
  memset(&ksTableEntries2, 0, sizeof(hippo::CameraKeystoneTableEntries));

  // lets get the items in the ram keystone table
  kstable = hippo::CameraKeystoneTable::RAM;

  if (err = hirescamera->keystone_table_entries(kstable, &ksTableEntries,
                                                &num_entries1)) {
    print_error(err);
    // sprout camera doesn't support this function - don't error out
    if (hippo::HippoErrorCode(err) == hippo::HIPPO_FUNC_NOT_AVAILABLE) {
      bCameraSupportsTest = false;
    } else {
      return err;
    }
  } else {
    printKeystoneTableEntries(ksTableEntries, num_entries1);
  }
  if (bCameraSupportsTest) {
    hippo::CameraResolution resolutions[2];
    resolutions[0].width = 640;
    resolutions[0].height = 480;
    resolutions[0].fps = 60;
    resolutions[1].width = 4352;
    resolutions[1].height = 3264;
    resolutions[1].fps = 6;

    fprintf(stderr, "Keystone_table_entries with resolution parameters:\n");
    if (err = hirescamera->keystone_table_entries(kstable,
                                                  resolutions,
                                                  2,
                                                  &ksTableEntries2,
                                                  &num_entries2)) {
      print_error(err);
    } else {
      printKeystoneTableEntries(ksTableEntries2, num_entries2);
    }

    // get a backup
    if (err = hirescamera->keystone_table_entries(kstable, &ksTableEntries2,
                                                  &num_entries2)) {
      print_error(err);
    } else {
      printKeystoneTableEntries(ksTableEntries, num_entries1);
    }

    // now change the values of the first copy and set
    if (num_entries1 > 0) {
      for (uint32_t i = 0; i < num_entries1; i++) {
        ksTableEntries.entries[i].value.bottom_left.x = 101;
        ksTableEntries.entries[i].value.bottom_right.y = 101;
        ksTableEntries.entries[i].value.top_left.x = 415;
        ksTableEntries.entries[i].value.top_left.y = 415;
        ksTableEntries.entries[i].value.bottom_right.x = -101;
        ksTableEntries.entries[i].value.bottom_right.y = -101;
        ksTableEntries.entries[i].value.top_right.x = -415;
        ksTableEntries.entries[i].value.top_right.y = 415;
      }
    }

    // set the new values
    if (err = hirescamera->keystone_table_entries(ksTableEntries,
                                                  num_entries1)) {
      print_error(err);
    } else {
      // free the allocated memory from the get above
      hirescamera->free_keystone_table_entries(&ksTableEntries, num_entries1);

      // get the current (hopefully new) values
      if (err = hirescamera->keystone_table_entries(kstable, &ksTableEntries,
                                                    &num_entries1)) {
        print_error(err);
      } else {
        // wait for notification to print first
        Sleep(1000);
        // print the new values
        fprintf(stderr, "Got keystone entries:\n");
        printKeystoneTableEntries(ksTableEntries, num_entries1);
      }
    }
    // free the items gotten above
    hirescamera->free_keystone_table_entries(&ksTableEntries, num_entries1);

    // set the old values back, and get the current values at the same time
    if (err = hirescamera->keystone_table_entries(ksTableEntries2, num_entries2,
                                                  &ksTableEntries,
                                                  &num_entries1)) {
      print_error(err);
    } else {
      // wait for notification to print first
      Sleep(1000);
      // print the newly gotten values
      printKeystoneTableEntries(ksTableEntries, num_entries1);
    }
    // and finally free all the memory
    hirescamera->free_keystone_table_entries(&ksTableEntries2, num_entries2);
  }
  hirescamera->free_keystone_table_entries(&ksTableEntries, num_entries1);


  // we can avoid sending exposure, gain and wb by setting them to TYPE_NONE
  // but so far we can not do the same with the bool types.
  // SR: Is this even useful???
  cs_1.exposure.type = hippo::AutoOrFixedType::TYPE_NONE;
  if (err = hirescamera->camera_settings(cs_1)) {
    print_error(err, "hirescamera.camera_settings set");
  } else  {
    fprintf(stderr, "hirescamera.camera_settings set\n");
  }
  cs_1.exposure.type = hippo::AutoOrFixedType::TYPE_AUTO;
  if (err = hirescamera->camera_settings(cs_1, &cs_2)) {
    print_error(err, "hirescamera.camera_settings set get");
  } else  {
    printCameraSettings(cs_2);
  }
  bool bool_set, bool_get;
  // auto_exposure
  if (err = hirescamera->auto_exposure(&bool_get)) {
    print_error(err, "hirescamera.auto_exposure");
  } else  {
    fprintf(stderr, "hirescamera.auto_exposure(): %d\n", bool_get);
  }
  bool_set = bool_get ? false : true;
  if (err = hirescamera->auto_exposure(bool_set)) {
    print_error(err, "hirescamera.auto_exposure");
  } else {
    fprintf(stderr, "hirescamera.auto_exposure(%d)\n", bool_set);
  }
  if (err = hirescamera->auto_exposure(&bool_get)) {
    print_error(err, "hirescamera.auto_exposure");
  } else {
    fprintf(stderr, "hirescamera.auto_exposure(): %d\n", bool_get);
  }
  bool_set = bool_get ? false : true;
  if (err = hirescamera->auto_exposure(bool_set, &bool_get)) {
    print_error(err, "hirescamera.auto_exposure");
  } else {
    fprintf(stderr, "hirescamera.auto_exposure(%d): %d\n",
            bool_set, bool_get);
  }
  // auto_gain
  if (err = hirescamera->auto_gain(&bool_get)) {
    print_error(err, "hirescamera.auto_gain");
  } else {
    fprintf(stderr, "hirescamera.auto_gain(): %d\n", bool_get);
  }
  bool_set = bool_get ? false : true;
  if (err = hirescamera->auto_gain(bool_set)) {
    print_error(err, "hirescamera.auto_gain");
  } else {
    fprintf(stderr, "hirescamera.auto_gain(%d)\n", bool_set);
  }
  if (err = hirescamera->auto_gain(&bool_get)) {
    print_error(err, "hirescamera.auto_gain");
  } else {
    fprintf(stderr, "hirescamera.auto_gain(): %d\n", bool_get);
  }
  bool_set = bool_get ? false : true;
  if (err = hirescamera->auto_gain(bool_set, &bool_get)) {
    print_error(err, "hirescamera.auto_gain");
  } else {
    fprintf(stderr, "hirescamera.auto_gain(%d): %d\n", bool_set, bool_get);
  }
  // auto_white_balance
  if (err = hirescamera->auto_white_balance(&bool_get)) {
    print_error(err, "hirescamera.auto_white_balance");
  } else {
    fprintf(stderr, "hirescamera.auto_white_balance(): %d\n", bool_get);
  }
  bool_set = bool_get ? false : true;
  if (err = hirescamera->auto_white_balance(bool_set)) {
    print_error(err, "hirescamera.auto_white_balance");
  } else {
    fprintf(stderr, "hirescamera.auto_white_balance(%d)\n", bool_set);
  }
  if (err = hirescamera->auto_white_balance(&bool_get)) {
    print_error(err, "hirescamera.auto_white_balance");
  } else {
    fprintf(stderr, "hirescamera.auto_white_balance(): %d\n", bool_get);
  }
  bool_set = bool_get ? false : true;
  if (err = hirescamera->auto_white_balance(bool_set, &bool_get)) {
    print_error(err, "hirescamera.auto_white_balance");
  } else {
    fprintf(stderr, "hirescamera.auto_white_balance(%d): %d\n",
            bool_set, bool_get);
  }
  // camera_index
  uint32_t index;
  if (err = hirescamera->camera_index(&index)) {
    print_error(err, "hirescamera.camera_index");
  } else {
    fprintf(stderr, "hirescamera.camera_index(): %d\n", index);
  }
  // defaultConfig
  hippo::CameraConfig cf;
  hippo::CameraMode mode = hippo::CameraMode::MODE_4416x3312;
  if (err = hirescamera->default_config(mode, &cf)) {
    print_error(err, "default_config");
  } else {
    fprintf(stderr,
            "hirescamera.default_config(): exposure: %d, fps: %d, gain: %d, "
            "mode: %d, RGB: [%d, %d, %d]\n",
            cf.exposure, cf.fps, cf.gain, static_cast<uint32_t>(cf.mode),
            cf.white_balance.red, cf.white_balance.green,
            cf.white_balance.blue);
  }
  // exposure
  uint16_t set_exp, get_exp;
  if (err = hirescamera->exposure(&get_exp)) {
    print_error(err, "hirescamera.exposure");
  } else {
    fprintf(stderr, "hirescamera.exposure(): %d\n", get_exp);
  }
  set_exp = get_exp + 1;
  if (err = hirescamera->exposure(set_exp)) {
    print_error(err, "hirescamera.exposure");
  } else {
    fprintf(stderr, "hirescamera.exposure(%d)\n", set_exp);
  }
  if (err = hirescamera->exposure(&get_exp)) {
    print_error(err, "hirescamera.exposure");
  } else {
    fprintf(stderr, "hirescamera.exposure(): %d\n", get_exp);
  }
  set_exp = get_exp - 1;
  if (err = hirescamera->exposure(set_exp, &get_exp)) {
    print_error(err, "hirescamera.exposure");
  } else {
    fprintf(stderr, "hirescamera.exposure(%d): %d\n", set_exp, get_exp);
  }
  if (err = hirescamera->auto_exposure(true)) {
    print_error(err, "hirescamera.auto_exposure");
  } else {
    fprintf(stderr, "hirescamera.auto_exposure(%d)\n", true);
  }
  // factory_default
  if (err = hirescamera->factory_default()) {
    print_error(err, "hirescamera.factory_default");
  }
  // flip_frame
  if (err = hirescamera->flip_frame(&bool_get)) {
    print_error(err, "hirescamera.flip_frame");
  } else {
    fprintf(stderr, "hirescamera.flip_frame(): %d\n", bool_get);
  }
  bool_set = bool_get ? false : true;
  if (err = hirescamera->flip_frame(bool_set)) {
    print_error(err, "hirescamera.flip_frame");
  } else {
    fprintf(stderr, "hirescamera.flip_frame(%d)\n", bool_set);
  }
  if (err = hirescamera->flip_frame(&bool_get)) {
    print_error(err, "hirescamera.flip_frame");
  } else {
    fprintf(stderr, "hirescamera.flip_frame(): %d\n", bool_get);
  }
  bool_set = bool_get ? false : true;
  if (err = hirescamera->flip_frame(bool_set, &bool_get)) {
    print_error(err, "hirescamera.flip_frame");
  } else {
    fprintf(stderr, "hirescamera.flip_frame(%d): %d\n", bool_set, bool_get);
  }
  // gain
  uint16_t set_gain, get_gain;
  if (err = hirescamera->gain(&get_gain)) {
    print_error(err, "hirescamera.gain");
  } else {
    fprintf(stderr, "hirescamera.gain():  %d\n", get_gain);
  }
  set_gain = get_gain - 1;
  if (err = hirescamera->gain(set_gain)) {
    print_error(err, "hirescamera.gain");
  } else {
    fprintf(stderr, "hirescamera.gain(%d)\n", set_gain);
  }
  if (err = hirescamera->gain(&get_gain)) {
    print_error(err, "hirescamera.gain");
  } else {
    fprintf(stderr, "hirescamera.gain():  %d\n", get_gain);
  }
  set_gain = get_gain + 1;
  if (err = hirescamera->gain(set_gain, &get_gain)) {
    print_error(err, "hirescamera.gain");
  } else {
    fprintf(stderr, "hirescamera.gain(%d): %d\n", set_gain, get_gain);
  }
  if (err = hirescamera->auto_gain(true)) {
    print_error(err, "hirescamera.auto_gain");
  } else {
    fprintf(stderr, "hirescamera.auto_gain(%d)\n", true);
  }
  // gamma_correction
  if (err = hirescamera->gamma_correction(&bool_get)) {
    print_error(err, "hirescamera.gamma_correction");
  } else {
    fprintf(stderr, "hirescamera.gamma_correction(): %d\n", bool_get);
  }
  bool_set = bool_get ? false : true;
  if (err = hirescamera->gamma_correction(bool_set)) {
    print_error(err, "hirescamera.gamma_correction");
  } else {
    fprintf(stderr, "hirescamera.gamma_correction(%d)\n", bool_set);
  }
  if (err = hirescamera->gamma_correction(&bool_get)) {
    print_error(err, "hirescamera.gamma_correction");
  } else {
    fprintf(stderr, "hirescamera.gamma_correction(): %d\n", bool_get);
  }
  bool_set = bool_get ? false : true;
  if (err = hirescamera->gamma_correction(bool_set, &bool_get)) {
    print_error(err, "hirescamera.gamma_correction");
  } else {
    fprintf(stderr, "hirescamera.gamma_correction(%d): %d\n", bool_set,
            bool_get);
  }
  // info
  hippo::DeviceInfo info;
  if (err = hirescamera->info(&info)) {
    print_error(err, "hirescamera.info");
  } else {
    print_device_info(info);
  }
  // lens_color_shading
  if (err = hirescamera->lens_color_shading(&bool_get)) {
    print_error(err, "hirescamera.lens_color_shading");
  } else {
    fprintf(stderr, "hirescamera.lens_color_shading(): %d\n", bool_get);
  }
  bool_set = bool_get ? false : true;
  if (err = hirescamera->lens_color_shading(bool_set)) {
    print_error(err, "hirescamera.lens_color_shading");
  } else {
    fprintf(stderr, "hirescamera.lens_color_shading(%d)\n", bool_set);
  }
  if (err = hirescamera->lens_color_shading(&bool_get)) {
    print_error(err, "hirescamera.lens_color_shading");
  } else {
    fprintf(stderr, "hirescamera.lens_color_shading(): %d\n", bool_get);
  }
  bool_set = bool_get ? false : true;
  if (err = hirescamera->lens_color_shading(bool_set, &bool_get)) {
    print_error(err, "hirescamera.lens_color_shading");
  } else {
    fprintf(stderr, "hirescamera.lens_color_shading(%d): %d\n", bool_set,
            bool_get);
  }
  // lens_shading
  if (err = hirescamera->lens_shading(&bool_get)) {
    print_error(err, "hirescamera.lens_shading");
  } else {
    fprintf(stderr, "hirescamera.lens_shading(): %d\n", bool_get);
  }
  bool_set = bool_get ? false : true;
  if (err = hirescamera->lens_shading(bool_set)) {
    print_error(err, "hirescamera.lens_shading");
  } else {
    fprintf(stderr, "hirescamera.lens_shading(%d)\n", bool_set);
  }
  if (err = hirescamera->lens_shading(&bool_get)) {
    print_error(err, "hirescamera.lens_shading");
  } else {
    fprintf(stderr, "hirescamera.lens_shading(): %d\n", bool_get);
  }
  bool_set = bool_get ? false : true;
  if (err = hirescamera->lens_shading(bool_set, &bool_get)) {
    print_error(err, "hirescamera.lens_shading");
  } else {
    fprintf(stderr, "hirescamera.lens_shading(%d): %d\n", bool_set,
            bool_get);
  }
  // mirror_frame
  if (err = hirescamera->mirror_frame(&bool_get)) {
    print_error(err, "hirescamera.mirror_frame");
  } else {
    fprintf(stderr, "hirescamera.mirror_frame(): %d\n", bool_get);
  }
  bool_set = bool_get ? false : true;
  if (err = hirescamera->mirror_frame(bool_set)) {
    print_error(err, "hirescamera.mirror_frame");
  } else {
    fprintf(stderr, "hirescamera.mirror_frame(%d)\n", bool_set);
  }
  if (err = hirescamera->mirror_frame(&bool_get)) {
    print_error(err, "hirescamera.mirror_frame");
  } else {
    fprintf(stderr, "hirescamera.mirror_frame(): %d\n", bool_get);
  }
  bool_set = bool_get ? false : true;
  if (err = hirescamera->mirror_frame(bool_set, &bool_get)) {
    print_error(err, "hirescamera.mirror_frame");
  } else {
    fprintf(stderr, "hirescamera.mirror_frame(%d): %d\n", bool_set, bool_get);
  }
  //// strobe
  // hippo::Strobe str;
  // str.exposure = 166;
  // str.frames = 10;
  // str.gain = 0;
  // if (err = hirescamera->strobe(str)) {
  //   print_error(err);
  // }
  // fprintf(stderr, "hirescamera.strobe(166,1,0)\n");
  // white_balance
  hippo::Rgb wb_get, wb_set;
  if (err = hirescamera->white_balance(&wb_get)) {
    print_error(err, "hirescamera.mirror_frame");
  } else {
    fprintf(stderr, "hirescamera.white_balance(): r:%d, g:%d, b:%d\n",
            wb_get.red, wb_get.green, wb_get.blue);
  }
  wb_set.red = 2020;
  wb_set.green = 2020;
  wb_set.blue = 2020;
  if (err = hirescamera->white_balance(wb_set)) {
    print_error(err, "hirescamera.white_balance");
  } else {
    fprintf(stderr, "hirescamera.white_balance(r:%d, g:%d, b:%d)\n",
            wb_set.red, wb_set.green, wb_set.blue);
  }
  if (err = hirescamera->white_balance(&wb_get)) {
    print_error(err, "hirescamera.white_balance");
  } else {
    fprintf(stderr, "hirescamera.white_balance(): r:%d, g:%d, b:%d\n",
            wb_get.red, wb_get.green, wb_get.blue);
  }
  wb_set.red = 1024;
  wb_set.green = 1025;
  wb_set.blue = 1026;
  if (err = hirescamera->white_balance(wb_set, &wb_get)) {
    print_error(err, "hirescamera.white_balance");
  } else {
    fprintf(stderr, "hirescamera.white_balance(r:%d, g:%d, b:%d): "
            "r:%d, g:%d, b:%d\n",
            wb_set.red, wb_set.green, wb_set.blue,
            wb_get.red, wb_get.green, wb_get.blue);
  }
  if (err = hirescamera->auto_white_balance(true)) {
    print_error(err, "hirescamera.auto_white_balance");
  } else {
    fprintf(stderr, "hirescamera.auto_white_balance(true)\n");
  }
  // keystone (flick only)
  hippo::CameraKeystone ks_1, ks_2;
  if (err = hirescamera->keystone(&ks_1)) {
    print_error(err, "hirescamera.keystone get");
  } else {
    printCameraKeystone(ks_1);
  }
  ks_1.value.top_left.x += 300;
  ks_1.value.top_left.y += 300;
  ks_1.enabled = true;
  if (err = hirescamera->keystone(ks_1)) {
    print_error(err, "hirescamera.keystone set");
  }
  Sleep(500);
  // now get keystone to validate
  if (err = hirescamera->keystone(&ks_2)) {
    print_error(err, "hirescamera.keystone get");
  } else {
    printCameraKeystone(ks_2);
  }
  Sleep(500);
  ks_1.value.top_left.x -= 300;
  ks_1.value.top_left.y -= 300;
  if (err = hirescamera->keystone(ks_1, &ks_2)) {
    print_error(err, "hirescamera.keystone set");
  } else {
    printCameraKeystone(ks_2);
  }

  // brightness
  uint16_t set_brightness, get_brightness;
  if (err = hirescamera->brightness(&get_brightness)) {
    print_error(err, "hirescamera.brightness");
  } else {
    fprintf(stderr, "hirescamera.brightness():  %d\n", get_brightness);
  }
  set_brightness = get_brightness - 1;
  if (err = hirescamera->brightness(set_brightness)) {
    print_error(err, "hirescamera.brightness");
  } else {
    fprintf(stderr, "hirescamera.brightness(%d)\n", set_brightness);
  }
  if (err = hirescamera->brightness(&get_brightness)) {
    print_error(err, "hirescamera.brightness");
  } else {
    fprintf(stderr, "hirescamera.brightness():  %d\n", get_brightness);
  }
  set_brightness = get_brightness + 1;
  if (err = hirescamera->brightness(set_brightness, &get_brightness)) {
    print_error(err, "hirescamera.brightness");
  } else {
    fprintf(stderr, "hirescamera.brightness(%d): %d\n",
            set_brightness, get_brightness);
  }

  // contrast
  uint16_t set_contrast, get_contrast;
  if (err = hirescamera->contrast(&get_contrast)) {
    print_error(err, "hirescamera.contrast");
  } else {
    fprintf(stderr, "hirescamera.contrast():  %d\n", get_contrast);
  }
  set_contrast = get_contrast - 1;
  if (err = hirescamera->contrast(set_contrast)) {
    print_error(err, "hirescamera.contrast");
  } else {
    fprintf(stderr, "hirescamera.contrast(%d)\n", set_contrast);
  }
  if (err = hirescamera->contrast(&get_contrast)) {
    print_error(err, "hirescamera.contrast");
  } else {
    fprintf(stderr, "hirescamera.contrast():  %d\n", get_contrast);
  }
  set_contrast = get_contrast + 1;
  if (err = hirescamera->contrast(set_contrast, &get_contrast)) {
    print_error(err, "hirescamera.contrast");
  } else {
    fprintf(stderr, "hirescamera.contrast(%d): %d\n",
            set_contrast, get_contrast);
  }
  // saturation
  uint16_t set_saturation, get_saturation;
  if (err = hirescamera->saturation(&get_saturation)) {
    print_error(err, "hirescamera.saturation");
  } else {
    fprintf(stderr, "hirescamera.saturation():  %d\n", get_saturation);
  }
  set_saturation = get_saturation - 1;
  if (err = hirescamera->saturation(set_saturation)) {
    print_error(err, "hirescamera.saturation");
  } else {
    fprintf(stderr, "hirescamera.saturation(%d)\n", set_saturation);
  }
  if (err = hirescamera->saturation(&get_saturation)) {
    print_error(err, "hirescamera.saturation");
  } else {
    fprintf(stderr, "hirescamera.saturation():  %d\n", get_saturation);
  }
  set_saturation = get_saturation + 1;
  if (err = hirescamera->saturation(set_saturation, &get_saturation)) {
    print_error(err, "hirescamera.saturation");
  } else {
    fprintf(stderr, "hirescamera.saturation(%d): %d\n",
            set_saturation, get_saturation);
  }

  // sharpness
  uint16_t set_sharpness, get_sharpness;
  if (err = hirescamera->sharpness(&get_sharpness)) {
    print_error(err, "hirescamera.sharpness");
  } else {
    fprintf(stderr, "hirescamera.sharpness():  %d\n", get_sharpness);
  }
  set_sharpness = get_sharpness - 1;
  if (err = hirescamera->sharpness(set_sharpness)) {
    print_error(err, "hirescamera.sharpness");
  } else {
    fprintf(stderr, "hirescamera.sharpness(%d)\n", set_sharpness);
  }
  if (err = hirescamera->sharpness(&get_sharpness)) {
    print_error(err, "hirescamera.sharpness");
  } else {
    fprintf(stderr, "hirescamera.sharpness():  %d\n", get_sharpness);
  }
  set_sharpness = get_sharpness + 1;
  if (err = hirescamera->sharpness(set_sharpness, &get_sharpness)) {
    print_error(err, "hirescamera.sharpness");
  } else {
    fprintf(stderr, "hirescamera.sharpness(%d): %d\n",
            set_sharpness, get_sharpness);
  }
  // white_balance_temperature
  uint16_t set_white_balance_temperature, get_white_balance_temperature;
  if (err =
      hirescamera->white_balance_temperature(&get_white_balance_temperature)) {
    print_error(err, "hirescamera.white_balance_temperature");
  } else {
    fprintf(stderr, "hirescamera.white_balance_temperature():  %d\n",
            get_white_balance_temperature);
  }
  set_white_balance_temperature = get_white_balance_temperature - 1;
  if (err =
      hirescamera->white_balance_temperature(set_white_balance_temperature)) {
    print_error(err, "hirescamera.white_balance_temperature");
  } else {
    fprintf(stderr, "hirescamera.white_balance_temperature(%d)\n",
            set_white_balance_temperature);
  }
  if (err =
      hirescamera->white_balance_temperature(&get_white_balance_temperature)) {
    print_error(err, "hirescamera.white_balance_temperature");
  } else {
    fprintf(stderr, "hirescamera.white_balance_temperature():  %d\n",
            get_white_balance_temperature);
  }
  set_white_balance_temperature = get_white_balance_temperature + 1;
  if (err =
      hirescamera->white_balance_temperature(set_white_balance_temperature,
                                             &get_white_balance_temperature)) {
    print_error(err, "hirescamera.white_balance_temperature");
  } else {
    fprintf(stderr, "hirescamera.white_balance_temperature(%d): %d\n",
            set_white_balance_temperature, get_white_balance_temperature);
  }
  // reset
  if (err = hirescamera->reset()) {
    print_error(err, "hirescamera.reset");
  }
  Sleep(5000);

  // close
  if (err = hirescamera->close(&open_count)) {
    return err;
  } else {
    fprintf(stderr, "hirescamera.close(): open_count: %d\n", open_count);
  }
  // unsubscribe
  if (err = hirescamera->unsubscribe(&num_subscribe)) {
    print_error(err);
  } else {
    fprintf(stderr, "hirescamera.unsubscribe(): count: %d\n", num_subscribe);
  }
  return 0LL;
}

void printAutoOrFixed(const hippo::AutoOrFixed &param) {
  switch (param.type) {
    case hippo::AutoOrFixedType::TYPE_AUTO:
      fprintf(stderr, "auto\n");
      break;
    case hippo::AutoOrFixedType::TYPE_UINT:
      fprintf(stderr, "%d\n", param.value.value);
      break;
    case hippo::AutoOrFixedType::TYPE_RGB:
      fprintf(stderr, "r:%d, g:%d, b:%d\n",
              param.value.rgb.red, param.value.rgb.green, param.value.rgb.blue);
      break;
    case hippo::AutoOrFixedType::TYPE_MODE:
      fprintf(stderr, "OPS! we should not get notifications with TYPE_MODE\n");
      break;
    default:
      fprintf(stderr, "OPS! Unknown type %d\n", param.type);
  }
}

void printPowerLineFrequency(const hippo::PowerLineFrequency &param) {
  switch (param) {
    case hippo::PowerLineFrequency::disabled:
      fprintf(stderr, "PowerLineFrequency: disabled\n");
      break;
    case hippo::PowerLineFrequency::hz_50:
      fprintf(stderr, "PowerLineFrequency: 50 Hz\n");
      break;
    case hippo::PowerLineFrequency::hz_60:
      fprintf(stderr, "PowerLineFrequency: 60 Hz\n");
      break;
    default:
      fprintf(stderr, "OPS! Unknown PowerLineFrequency\n");
  }
}

void printCameraSettings(const hippo::CameraSettings &cs) {
  fprintf(stderr, "-> CameraSettings\n");
  fprintf(stderr, " \\-> exposure ");
  printAutoOrFixed(cs.exposure);
  fprintf(stderr, " \\-> gain ");
  printAutoOrFixed(cs.gain);
  fprintf(stderr, " \\-> white_balance ");
  printAutoOrFixed(cs.white_balance);
  fprintf(stderr, " \\-> flip_frame %d\n", cs.flip_frame);
  fprintf(stderr, " \\-> gamma_correction %d\n", cs.gamma_correction);
  fprintf(stderr, " \\-> lens_color_shading %d\n", cs.lens_color_shading);
  fprintf(stderr, " \\-> lens_shading %d\n", cs.lens_shading);
  fprintf(stderr, " \\-> mirror_frame %d\n", cs.mirror_frame);
}

void printCameraStatus(const hippo::CameraDeviceStatus &camera_status) {
  const char *CameraStatusEnum_str[
      static_cast<uint32_t>(hippo::CameraStatus::error) + 1] = {
    "ok", "busy", "error",
  };

  fprintf(stderr, "Camera Device Status:\n");
  fprintf(stderr, "  generic_get : %s\n", CameraStatusEnum_str[
      static_cast<uint32_t>(camera_status.generic_get)]);
  fprintf(stderr, "  generic_set : %s\n", CameraStatusEnum_str[
      static_cast<uint32_t>(camera_status.generic_set)]);
  fprintf(stderr, "  isp_colorbar : %s\n", CameraStatusEnum_str[
      static_cast<uint32_t>(camera_status.isp_colorbar)]);
  fprintf(stderr, "  isp_function : %s\n", CameraStatusEnum_str[
      static_cast<uint32_t>(camera_status.isp_function)]);
  fprintf(stderr, "  isp_fw_boot : %s\n", CameraStatusEnum_str[
      static_cast<uint32_t>(camera_status.isp_fw_boot)]);
  fprintf(stderr, "  isp_reset : %s\n", CameraStatusEnum_str[
      static_cast<uint32_t>(camera_status.isp_reset)]);
  fprintf(stderr, "  isp_restore : %s\n", CameraStatusEnum_str[
      static_cast<uint32_t>(camera_status.isp_restore)]);
  fprintf(stderr, "  isp_videostream : %s\n", CameraStatusEnum_str[
      static_cast<uint32_t>(camera_status.isp_videostream)]);
  fprintf(stderr, "  load_lenc_calibration : %s\n", CameraStatusEnum_str[
      static_cast<uint32_t>(camera_status.load_lenc_calibration)]);
  fprintf(stderr, "  load_white_balance_calibration : %s\n",
          CameraStatusEnum_str[static_cast<uint32_t>(
              camera_status.load_white_balance_calibration)]);
  fprintf(stderr, "  special_get : %s\n", CameraStatusEnum_str[
      static_cast<uint32_t>(camera_status.special_get)]);
  fprintf(stderr, "  special_set : %s\n", CameraStatusEnum_str[
      static_cast<uint32_t>(camera_status.special_set)]);
  fprintf(stderr, "  thermal_sensor_error : %s\n", CameraStatusEnum_str[
      static_cast<uint32_t>(camera_status.thermal_sensor_error)]);
  fprintf(stderr, "  thermal_shutdown : %s\n", CameraStatusEnum_str[
      static_cast<uint32_t>(camera_status.thermal_shutdown)]);
}

void printCameraLEDState(const hippo::CameraLedState *state) {
  const char *CameraLedStateEnum_str[
      static_cast<uint32_t>(hippo::CameraLedStateEnum::automatic) + 1] = {
    "off", "low", "high", "auto",
  };
  fprintf(stderr, "Camera LED State:\n");
  fprintf(stderr, "  Capture: %s\n",
          CameraLedStateEnum_str[static_cast<uint32_t>(state->capture)]);
  fprintf(stderr, "  Streaming: %s\n",
          CameraLedStateEnum_str[static_cast<uint32_t>(state->streaming)]);
}

void printCameraKeystone(const hippo::CameraKeystone &ks) {
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

const char *keystoneTable_str[] = {
  "ram", "default", "flash_max_fov", "flash_fit_to_mat",
};

void printCameraKeystoneTable(const hippo::CameraKeystoneTable &ks) {
  fprintf(stderr, "-> Camera Keystone Table\n");
  fprintf(stderr, " \\ -> type: %s\n",
          keystoneTable_str[static_cast<uint32_t>(ks)]);
}

void printCameraKeystoneTableEntry(const hippo::CameraKeystoneTableEntry &ks) {
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
  fprintf(stderr, " \\ -> Resolution\n");
  fprintf(stderr, " \\    -> (Width, Height, Fps):  (%i, %i, %i)\n",
          ks.resolution.width, ks.resolution.height, ks.resolution.fps);
}

void printKeystoneTableEntries(const hippo::CameraKeystoneTableEntries &entries,
                               const uint32_t &num_entries) {
  fprintf(stderr, "-> Camera Keystone Table: %s\n",
          keystoneTable_str[static_cast<uint32_t>(entries.type)]);
  for (uint32_t i = 0; i < num_entries; i++) {
    printCameraKeystoneTableEntry(entries.entries[i]);
  }
}

void printCameraResolution(const hippo::CameraResolution &res) {
  fprintf(stderr, "Resolution: (Width, Height, Fps):  (%i, %i, %i)\n",
          res.width, res.height, res.fps);
}

void hirescamera_notification(const hippo::HiResCameraNotificationParam &param,
                              void *data) {
  // and print the notification
  switch (param.type) {
    case hippo::HiResCameraNotification::on_close:
      fprintf(stderr, "[SIGNAL]: hirescamera.on_close\n");
      break;
    case hippo::HiResCameraNotification::on_device_connected:
      fprintf(stderr, "[SIGNAL]: hirescamera.on_device_connected\n");
      break;
    case hippo::HiResCameraNotification::on_device_disconnected:
      fprintf(stderr, "[SIGNAL]: hirescamera.on_device_disconnected\n");
      break;
    case hippo::HiResCameraNotification::on_factory_default:
      fprintf(stderr, "[SIGNAL]: hirescamera.on_factory_default\n");
      break;
    case hippo::HiResCameraNotification::on_open:
      fprintf(stderr, "[SIGNAL]: hirescamera.on_open\n");
      break;
    case hippo::HiResCameraNotification::on_open_count:
      fprintf(stderr, "[SIGNAL]: hirescamera.on_open_count: %d\n",
              param.on_open_count);
      break;
    case hippo::HiResCameraNotification::on_resume:
      fprintf(stderr, "[SIGNAL]: hirescamera.on_resume\n");
      break;
    case hippo::HiResCameraNotification::on_suspend:
      fprintf(stderr, "[SIGNAL]: hirescamera.on_suspend\n");
      break;
    case hippo::HiResCameraNotification::on_sohal_disconnected:
      fprintf(stderr, "[SIGNAL]: hirescamera.on_sohal_disconnected\n");
      break;
    case hippo::HiResCameraNotification::on_sohal_connected:
      fprintf(stderr, "[SIGNAL]: hirescamera.on_sohal_connected\n");
      break;

    case hippo::HiResCameraNotification::on_exposure:
      fprintf(stderr, "[SIGNAL]: hirescamera.on_exposure: ");
      printAutoOrFixed(param.on_exposure);
      break;
    case hippo::HiResCameraNotification::on_brightness:
      fprintf(stderr, "[SIGNAL]: hirescamera.on_brightness: %d\n",
              param.on_brightness);
      break;
    case hippo::HiResCameraNotification::on_contrast:
      fprintf(stderr, "[SIGNAL]: hirescamera.on_contrast: %d\n",
              param.on_contrast);
      break;
    case hippo::HiResCameraNotification::on_flip_frame:
      fprintf(stderr, "[SIGNAL]: hirescamera.on_flip_frame: %d\n",
              param.on_flip_frame);
      break;
    case hippo::HiResCameraNotification::on_gain:
      fprintf(stderr, "[SIGNAL]: hirescamera.on_gain: ");
      printAutoOrFixed(param.on_gain);
      break;
    case hippo::HiResCameraNotification::on_gamma_correction:
      fprintf(stderr, "[SIGNAL]: hirescamera.on_gamma_correction: %d\n",
              param.on_gamma_correction);
      break;
    case hippo::HiResCameraNotification::on_keystone:
      fprintf(stderr, "[SIGNAL]: hirescamera.on_keystone\n");
      printCameraKeystone(param.on_keystone);
      break;
    case hippo::HiResCameraNotification::on_keystone_table:
      fprintf(stderr, "[SIGNAL]: hirescamera.on_keystone_table\n");
      printCameraKeystoneTable(param.on_keystone_table);
      break;
    case hippo::HiResCameraNotification::on_keystone_table_entries:
      fprintf(stderr, "[SIGNAL]: hirescamera.on_keystone_table_entries\n");
      printKeystoneTableEntries(param.on_keystone_table_entries,
                                param.num_keystone_table_entries);
      break;
    case hippo::HiResCameraNotification::on_led_state:
      fprintf(stderr, "[SIGNAL]: hirescamera.on_led_state:\n");
      printCameraLEDState(&param.on_led_state);
      break;
    case hippo::HiResCameraNotification::on_lens_color_shading:
      fprintf(stderr, "[SIGNAL]: hirescamera.on_lens_color_shading: %d\n",
              param.on_lens_color_shading);
      break;
    case hippo::HiResCameraNotification::on_lens_shading:
      fprintf(stderr, "[SIGNAL]: hirescamera.on_lens_shading: %d\n",
              param.on_lens_shading);
      break;
    case hippo::HiResCameraNotification::on_mirror_frame:
      fprintf(stderr, "[SIGNAL]: hirescamera.on_mirror_frame: %d\n",
              param.on_mirror_frame);
      break;
    case hippo::HiResCameraNotification::on_power_line_frequency:
      fprintf(stderr, "[SIGNAL]: hirescamera.on_power_line_frequency\n");
      printPowerLineFrequency(param.on_power_line_frequency);
      break;
    case hippo::HiResCameraNotification::on_strobe:
      fprintf(stderr, "[SIGNAL]: hirescamera.on_strobe\n");
      break;
    case hippo::HiResCameraNotification::on_white_balance:
      fprintf(stderr, "[SIGNAL]: hirescamera.on_white_balance: ");
      printAutoOrFixed(param.on_white_balance);
      break;
    case hippo::HiResCameraNotification::on_saturation:
      fprintf(stderr, "[SIGNAL]: hirescamera.on_saturation: %d\n",
              param.on_saturation);
      break;
    case hippo::HiResCameraNotification::on_white_balance_temperature:
      fprintf(stderr,
              "[SIGNAL]: hirescamera.on_white_balance_temperature: %d\n",
              param.on_white_balance_temperature);
      break;
    case hippo::HiResCameraNotification::on_reset:
      fprintf(stderr, "[SIGNAL]: hirescamera.on_reset\n");
      break;
    case hippo::HiResCameraNotification::on_sharpness:
      fprintf(stderr, "[SIGNAL]: hirescamera.on_sharpness: %d\n",
              param.on_sharpness);
      break;
    default:
      break;
  }
#if 0
  // sample on how to use the void *data parameter to pass objects (the
  // this pointer in this case) to the callback function
  uint64_t err = 0LL;
  hippo::HiResCamera *hirescamera = (hippo::HiResCamera*)data;
  bool bool_get;
  // in this case we just get auto_exposure
  if (err = hirescamera->auto_exposure(&bool_get)) {
    print_error(err);
  }
  fprintf(stderr, "[SIGNAL+] hirescamera.auto_exposure(): %d\n", bool_get);
#endif
}
