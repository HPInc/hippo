
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#include <windows.h>    // for Sleep()
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "include/projector.h"
#include "include/capturestage.h"
#include "include/depthcamera.h"
#include "include/hirescamera.h"
#include "include/sbuttons.h"
#include "include/touchmat.h"
#include "include/desklamp.h"
#include "include/system.h"
#include "include/sohal.h"
#include "include/uvccamera.h"


const char wsAddress[] = "localhost";
const uint32_t wsPort = 20641;

extern uint64_t TestProjector(hippo::Projector *projector);
extern uint64_t TestSButtons(hippo::SButtons *sbuttons);
extern uint64_t TestTouchMat(hippo::TouchMat *touchmat);
extern uint64_t TestSystem(hippo::System *system);
extern uint64_t TestDepthCamera(hippo::DepthCamera *depthcamera);
extern uint64_t TestHiResCamera(hippo::HiResCamera *hirescamera);
extern uint64_t TestCaptureStage(hippo::CaptureStage *capturestage);
extern uint64_t TestSoHal(hippo::SoHal *sohal);
extern uint64_t TestUVCCamera(hippo::UVCCamera *uvccamera);
extern uint64_t TestDeskLamp(hippo::DeskLamp *desklamp);
extern uint64_t TestSWDevice();

void print_error(uint64_t err) {
  char err_msg[256];
  hippo::strerror(err, sizeof(err_msg), err_msg);
  fprintf(stderr, "[ERROR %08llx:%08llx]: %s\n",
          err>>32, err&0xffffffff, err_msg);
}

void print_error(uint64_t err, const char *msg) {
  char err_msg[256];
  hippo::strerror(err, sizeof(err_msg), err_msg);
  fprintf(stderr, "[ERROR %08llx:%08llx]: %s '%s'\n",
          err>>32, err&0xffffffff, msg, err_msg);
}

void print_device_info(const hippo::DeviceInfo &info) {
  fprintf(stderr, "info: fw_version: '%s', name: '%s', serial: '%s', "
          "index: %d, vendor_id: 0x%x, product_id: 0x%x\n",
          info.fw_version, info.name, info.serial, info.index,
          info.vendor_id, info.product_id);
}

const char *TempSensor_str[
    static_cast<uint32_t>(hippo::TempInfoSensors::depthcamera_z_3d_tec) + 1] = {
  "led driver", "red led", "green led", "formatter", "heatsink",
  "high res camera", "depth camera", "depth camera tec",
  "Hirescamera Z 3D main", "Hirescamera Z 3d system", "Depthcamera Z 3D tec",
};

const char *Connection_str[
    static_cast<uint32_t>(
        hippo::TemperatureConnectionDevices::through_projector) + 1] = {
  "depth camera", "desklamp", "high res camera", "projector"
};

void print_temperatures(const hippo::TemperatureInfo * temps,
                        uint64_t num_temperatures) {
  for (uint64_t i = 0; i < num_temperatures; i++) {
    fprintf(stderr, "%s:\n  Connected through the %s\n",
            TempSensor_str[static_cast<uint32_t>(temps[i].sensor)],
            Connection_str[
                static_cast<uint32_t>(temps[i].device.connectedDevice)]);
    fprintf(stderr, "  Device Index:\t%i\n", temps[i].device.index);
    fprintf(stderr, "  Current:\t%f\n  Max : \t%f\n  Safe\t\t%f\n",
            temps[i].current, temps[i].max, temps[i].safe);
  }
}

int main(int argc, char *argv[]) {
  uint64_t err;
  char *host = NULL;
  uint32_t port = 0;
  hippo::Projector *projector = NULL;
  hippo::HiResCamera *hirescamera = NULL;
  hippo::CaptureStage *capturestage = NULL;
  hippo::SButtons *sbuttons = NULL;
  hippo::DeskLamp *desklamp = NULL;
  hippo::System *system = NULL;
  hippo::DepthCamera *depthcamera = NULL;
  hippo::TouchMat *touchmat = NULL;
  hippo::SoHal *sohal = NULL;
  hippo::UVCCamera *uvccamera = NULL;

  if (argc == 3) {
    host = argv[1];
    port = atoi(argv[2]);
  }
  if (host) {
    projector = new hippo::Projector(host, port);
    capturestage = new hippo::CaptureStage(host, port);
    depthcamera = new hippo::DepthCamera(host, port);
    hirescamera = new hippo::HiResCamera(host, port);
    sbuttons = new hippo::SButtons(host, port);
    touchmat = new hippo::TouchMat(host, port);
    desklamp = new hippo::DeskLamp(host, port);
    sohal = new hippo::SoHal(host, port);
    system = new hippo::System(host, port);
    uvccamera = new hippo::UVCCamera(host, port);
  } else {
    projector = new hippo::Projector();
    capturestage = new hippo::CaptureStage();
    depthcamera = new hippo::DepthCamera();
    hirescamera = new hippo::HiResCamera();
    sbuttons = new hippo::SButtons();
    touchmat = new hippo::TouchMat();
    desklamp = new hippo::DeskLamp();
    sohal = new hippo::SoHal();
    system = new hippo::System();
    uvccamera = new hippo::UVCCamera();
  }
  ADD_FILE_TO_MAP();   // will add this file to the file/error map

  // test system to get initial temps
  if (err = TestSystem(system)) {
    print_error(err);
  }
  // test projector, which should warm it up a bit
  if (err = TestProjector(projector)) {
    print_error(err);
  }
  // test system again to see if temps changed
  if (err = TestSystem(system)) {
      print_error(err);
  }
  // test the uvc camera
  if (err = TestUVCCamera(uvccamera)) {
    print_error(err);
  }
  // test depth camera
  if (err = TestDepthCamera(depthcamera)) {
    print_error(err);
  }
  // test turntable
  if (err = TestCaptureStage(capturestage)) {
    print_error(err);
  }
  // test high resolution camera
  if (err = TestHiResCamera(hirescamera)) {
    print_error(err);
  }
  // test capsense buttons
  if (err = TestSButtons(sbuttons)) {
    print_error(err);
  }
  // test touchmat
  if (err = TestTouchMat(touchmat)) {
    print_error(err);
  }
  // test desklamp (on Sprout 1.0/1.55)
  if (err = TestDeskLamp(desklamp)) {
    print_error(err);
  }
  // test sohal
  if (err = TestSoHal(sohal)) {
    print_error(err);
  }

  fprintf(stderr, "***\n*** Stop SoHal and start it again please\n***\n");
  for (int i = 10; i > 0; i--) {
    fprintf(stderr, "*** %d\n", i);
    Sleep(1000);
  }
  fprintf(stderr, "*** Let's continue!!\n");

  if (err = TestSystem(system)) {
    print_error(err);
  }
  if (err = TestSoHal(sohal)) {
    print_error(err);
  }

  delete projector;
  delete capturestage;
  delete hirescamera;
  delete depthcamera;
  delete sbuttons;
  delete system;
  delete desklamp;
  delete touchmat;
  delete sohal;
  delete uvccamera;

  // test that after deleting the lws context we can connect again
  sohal = new hippo::SoHal();
  if (err = TestSoHal(sohal)) {
    print_error(err);
  }
  delete sohal;

  if (err = TestSWDevice()) {
    print_error(err);
  }

  return 0;
}
