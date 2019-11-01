
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#ifndef INCLUDE_DEPTHCAMERA_H_
#define INCLUDE_DEPTHCAMERA_H_

#include "../include/hippo_camera.h"

#if COMPILING_DLL
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif

namespace hippo {

typedef struct CalibrationIntrinsics {
  float fx;
  float fy;
  float cx;
  float cy;
} CalibrationIntrinsics;

typedef struct CalibrationDistortion {
  float k1;
  float k2;
  float k3;
  float p1;
  float p2;
} CalibrationDistortion;

typedef struct IrRgbCalibration {
  //  a 4 element vector [fx,fy,cx,cy]
  CalibrationIntrinsics ir_intrinsics;
  //  a 4 element vector [fx,fy,cx,cy]
  CalibrationIntrinsics rgb_intrinsics;
  // a 5 element vector [k1,k2,k3,p1,p2]
  CalibrationDistortion ir_distortion;
  // a 5 element vector[k1, k2, k3, p1, p2]
  CalibrationDistortion rgb_distortion;
  // a 4x4 3D matrix transformation in column major order
  float matrix_transformation[4][4];
  // Is the image mirrored?
  bool mirror;
} IrRgbCalibration;

typedef enum class DepthCameraNotification {
  // This notification occurs when the device is really closed. In other words,
  // when the number of clients who have the device open goes from 1 to 0.
  // However SoHal does not send a notification when the number of clients that
  // have the device open decrease but the device is not required to close, for
  // example, when the number of clients goes from 2 to 1. In that case, SoHal
  // will only send an device_on_open_count notification.
  on_close,
  // This notification occurs whenever the device is connected.
  // Note: system also sends on_device_connected notifications for all devices.
  // Details can be found in the system.on_device_connected section of the
  // SoHAL documentation.
  on_device_connected,
  // This notification occurs whenever the device is disconnected.
  // Note: system also sends on_device_disconnected notifications for all
  // devices. Details can be found in the system.on_device_disconnected
  // section of the SoHAL documentation.
  on_device_disconnected,
  // This notification occurs whenever a client calls the factory_default
  // method to reset the device to the factory settings.
  on_factory_default,
  // This notification occurs when the device is really opened. For example,
  // when the number of clients that have opened the device goes from 0 to 1.
  // However, SoHal does not send this notification when the number of clients
  // that have opened the device increases but the device is not required to
  // open, for example when the number of clients goes from 1 to 2. In that
  // case, SoHal will only send an device_on_open_count notification.
  on_open,
  // This notification occurs when the number of clients that have a device
  // open changes. This may occur when a client calls deviceName.open or
  // deviceName.close and this causes the device's open_count to change.
  // However, SoHal does not send this notification if a client already has a
  // device opened and then calls open again, or if a client that does not
  // have a device open calls close.
  on_open_count,
  // This notification occurs whenever the system has woken up from the
  // suspend state, and the device can be opened again.
  on_resume,
  // This notification occurs when the host system is entering a sleep
  // or hibernate state, shutting down, or restarting. When this notification
  // is sent, any energy saving actions for this device have been performed
  // (as detailed in the table at the top of this Device documentation) and the
  // device has been closed. A device cannot be opened until the system is
  // resumed and the device's on_resume notification has been sent out.
  on_suspend,
  // This notification occurs when the connection
  // to SoHal is lost at websocket level
  on_sohal_disconnected,
  // This notification occurs when, after losing connection to SoHal,
  // the connection is reestablished
  on_sohal_connected,

  // depthcamera specific notifications below

  // This notification occurs when a client sends an enable_streams command
  // and the command executes successfully. It includes the list of currently
  // enabled streams as a parameter.
  on_enable_streams,
  // This notification occurs when a client sends a disable_streams command
  // and the command executes successfully. It includes the list of currently
  // enabled streams as a parameter
  on_disable_streams,
  // This notification occurs when a client sends a command to set the
  // ir_flood_on value and the command executes successfully. It includes a
  // bool parameter with the current ir_flood_on status.
  on_ir_flood_on,
  // This notification occurs when a client sends a command to set the
  // laser_on value and the command executes successfully. It includes a
  // bool parameter with the current laser_on status.
  on_laser_on,
} DepthCameraNotification;

// The parameters that are passed when a depthcamera notification is triggered
typedef struct DepthCameraNotificationParam {
  // the type of notification that is being passed
  DepthCameraNotification type;
  union {
    // parameter passed with the on_open_count notification
    uint32_t on_open_count;
    // parameter passed with the on_enable_streams notification
    CameraStreams on_enable_streams;
    // parameter passed with the on_disable_streams notification
    CameraStreams on_disable_streams;
    // parameter passed with the on_ir_flood_on notification
    bool on_ir_flood_on;
    // parameter passed with the on_laser_on notification
    bool on_laser_on;
  };
} DepthCameraNotificationParam;

// The depthcamera device provides access to the depth camera hardware
// components of the HP Z 3D Camera and all versions of Sprout
class DLLEXPORT DepthCamera : public HippoCamera {
 public:
  DepthCamera();
  explicit DepthCamera(uint32_t device_index);
  DepthCamera(const char *address, uint32_t port);
  DepthCamera(const char *address, uint32_t port, uint32_t device_index);
  virtual ~DepthCamera(void);

  // Calling this method with a set parameters acts as a Set request and
  // sets the the infrared flood light of the depthcamera to on (true)
  // or off (false).
  uint64_t ir_flood_on(bool set);

  // Calling this method with a get parameters acts as a Get request and
  // returns a bool indicating if the infrared flood light is on (true)
  //  or off (false).
  uint64_t ir_flood_on(bool *get);

  // Calling this method with a set parameters acts as a Set request and
  // sets the the infrared flood light of the depthcamera to on (true)
  // or off (false). The get parameter returns a bool indicating if
  // the infrared flood light is on (true) or off (false).
  uint64_t ir_flood_on(bool set, bool* get);

  // This method returns the calibration data between the depth camera's
  // IR and RGB cameras as an IrRgbCalibration object.
  uint64_t ir_to_rgb_calibration(IrRgbCalibration *get);

  // Calling this method with a set parameters acts as a Set request and
  // sets the the laser of the depthcamera to on (true) or off (false).
  uint64_t laser_on(bool set);

  // Calling this method with a get parameters acts as a Get request and
  // returns a bool indicating if the laser of the depthcamera is on (true)
  // or off (false).
  uint64_t laser_on(bool *get);

  // Calling this method with a set parameters acts as a Set request and
  // sets the the laser of the depthcamera to on (true) or off (false), it
  // then returns the curren state of the laser in the get paramerter
  uint64_t laser_on(bool set, bool* get);

  // subscribe to notifications
  uint64_t subscribe(void(*callback)(const DepthCameraNotificationParam &param,
                                     void *data),
                     void *data);
  uint64_t subscribe(void(*callback)(const DepthCameraNotificationParam &param,
                                     void *data),
                     void *data, uint32_t *get);

  // unsubscribe from notifications
  uint64_t unsubscribe();
  uint64_t unsubscribe(uint32_t *get);

 protected:
  uint64_t irRGBcalibration_json2c(void *obj, hippo::IrRgbCalibration *cal);
  uint64_t parseIntrinsics(void *jsonIntrinsics,
                           hippo::CalibrationIntrinsics* cIntrinsics);
  uint64_t parseDistortion(void *jsonDistortion,
                           hippo::CalibrationDistortion *cDistortion);

  // Callback items
  void ProcessSignal(char *method, void *params) override;
  bool HasRegisteredCallback();

  void(*callback_)(const DepthCameraNotificationParam &param, void *data);
};

}   // namespace hippo

#endif   // INCLUDE_DEPTHCAMERA_H_
