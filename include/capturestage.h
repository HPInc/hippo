
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#ifndef INCLUDE_CAPTURESTAGE_H_
#define INCLUDE_CAPTURESTAGE_H_

#include "../include/hippo_device.h"

#if COMPILING_DLL
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif

namespace hippo {

// Enumerates the possible states of the capture stage LEDs.
typedef enum class LedStateEnum {
  // The LED is not emitting light
  led_off = 0x00,
  // The LED is illuminated
  led_on = 0x01,
  //  The LED is continuously blinking on and off at a rate defined
  // by the led_on_off_rate. The LED will be illuminated for time_on
  // seconds and off for time_off seconds. If multiple LEDs are in this
  // state, their blinking will be synchronized.
  led_blink_in_phase = 0x02,
  // LEDs in this state will be blinking at opposite times from LEDs
  // in the blink_in_phase state: off for time_on seconds and illuminated
  // for time_off seconds.
  led_blink_off_phase = 0x03
} LedStateEnum;

// Provides information on the state of each of the capture stage LEDs.
typedef struct LedState {
  LedStateEnum amber;
  LedStateEnum red;
  LedStateEnum white;
}LedState;

typedef struct LedOnOffRate {
  uint32_t time_on;   // in milliseconds, with a range of [10, 65535]
  uint32_t time_off;  // in milliseconds, with a range of [10, 65535]
}LedOnOffRate;

typedef enum class CaptureStageNotification {
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

  // capture stage specific notifications

  // This notification occurs when a client sends a home command and the
  // capture stage is moved to the home position.
  on_home,
  // This notification occurs when a client updates the led_on_off_rate.
  // It includes an LedOnOffRate parameter with the new values.
  on_led_on_off_rate,
  // This notification occurs when a client updates the led_state.
  // It includes a CaptureStageLedState parameter with the new state
  // for each LED.
  on_led_state,
  // This notification occurs when a client sends a rotate command and
  // the command executes successfully. It includes a float parameter
  // set to the number of degrees the capture stage rotated.
  on_rotate,
  // This notification occurs when a client sends a tilt command and
  // the command executes successfully. It includes a float parameter
  // set to the tilt rotation angle the capture stage moved to.
  on_tilt,
} CaptureStageNotification;

// The parameters that are passed when a capturestage notification is triggered
typedef struct CaptureStageNotificationParam {
  // the type of notification that is being passed
  CaptureStageNotification type;
  union {
    // parameter passed with the on_open_count notification
    uint32_t on_open_count;
    // parameter passed with the on_off_rate notification
    LedOnOffRate on_off_rate;
    // parameter passed with the on_led_state notification
    LedState on_led_state;
    // parameter passed with the on_rotate notification
    float on_rotate;
    // parameter passed with the on_tilt notification
    float on_tilt;
  };
} CaptureStageNotificationParam;

// name of port that capture stage is connected to i.e. "COM3"
typedef struct CaptureStageSpecificInfo {
  char *port;
} CaptureStageSpecificInfo;

class DLLEXPORT CaptureStage : public HippoDevice {
 public:
  CaptureStage();
  explicit CaptureStage(uint32_t device_index);
  CaptureStage(const char *address, uint32_t port);
  CaptureStage(const char *address, uint32_t port, uint32_t device_index);
  virtual ~CaptureStage(void);

  // Returns information specific to the capture stage,
  // as a CaptureStageSpecificInfo value.
  // Specifically: gets the com port which the capturestage is connected to
  //
  // Note: This function internally allocates memory and the
  // free_capturestage_info() function must be called afterwards in order to
  // avoid a memory leak
  uint64_t device_specific_info(CaptureStageSpecificInfo *get);

  // frees the memory allocated inside device_specific_info()
  void free_capturestage_info(CaptureStageSpecificInfo *info_to_free);

  // This home method needs to be called to calibrate the capture stage when it
  // is first connected to the system. Calls to capturestage.tilt will return
  // errors if home has not been called since the stage was plugged in.
  // Calling home on a capture stage that has already been calibrated will
  // cause it to return to the untilted position (equivalent to sending a
  // capturestage.tilt command with a parameter of 0).
  // On success it will trigger an on_home notification.
  uint64_t home();

  // The led_on_off_rate method is used to control the number of milliseconds
  // the LEDs stay on and off for in the blink_in_phase and blink_off_phase
  // states. Passing a get parameter will return the current state of the LEDs
  uint64_t led_on_off_rate(hippo::LedOnOffRate *get);

  // The led_on_off_rate method is used to control the number of milliseconds
  // the LEDs stay on and off for in the blink_in_phase and blink_off_phase
  // states. Passing a set parameter will set the current state of the LEDs
  uint64_t led_on_off_rate(const hippo::LedOnOffRate &set);

  // The led_on_off_rate method is used to control the number of milliseconds
  // the LEDs stay on and off for in the blink_in_phase and blink_off_phase
  // states. Passing a set and get parameter will set the current state of
  // the LEDs and subsequently return the state of the LEDs after the set
  uint64_t led_on_off_rate(const hippo::LedOnOffRate &set,
                           hippo::LedOnOffRate *get);

  // Calling this method acts as a Get request and returns
  // the current state of the capture stage LEDs.
  uint64_t led_state(hippo::LedState *get);

  // Calling this method with a set parameter will change the state
  // of the LEDs and trigger an on_led_state notification.
  uint64_t led_state(const hippo::LedState &set);

  // Calling this method with a set parameter will change the state
  // of the LEDs and return the current state in the get parameter.
  // it will also trigger an on_led_state notification.
  uint64_t led_state(const hippo::LedState &set,
                     hippo::LedState *get);

  // The rotate method is used to turn the top surface of the capture stage.
  // Positive angles rotate the unit clockwise, and negative values rotate
  // the unit counterclockwise.
  // Calling this method with a set parameter will cause the capture stage
  // to rotate that many degrees from its current position and will trigger
  // an on_rotate notification.
  uint64_t rotate(float set);

  // The get function returns a float indicating the number of degrees the
  // capture stage is has rotated. Note that there is some variation when
  // rotating the unit, so always check the return value.
  uint64_t rotate(float *get);

  // Calling this method with a set parameter will cause the capture stage
  // to rotate that many degrees from its current position and will trigger
  // an on_rotate notification. After rotation the current rotation
  // angle will be returned in the get parameter
  uint64_t rotate(float set, float *get);

  // This method returns the capture stage's current rotation angle in degrees.
  // The rotation angle is the sum of all rotate commands since the capture
  // stage was connected.
  uint64_t rotation_angle(float *get);

  // subscribe to notifications
  uint64_t subscribe(void(*callback)(const CaptureStageNotificationParam &param,
                                     void *data),
                     void *data);
  uint64_t subscribe(void(*callback)(const CaptureStageNotificationParam &param,
                                     void *data),
                     void *data, uint32_t *get);

  // unsubscribe from notifications
  uint64_t unsubscribe();
  uint64_t unsubscribe(uint32_t *get);

  // The capture stage tilts by rotating the base of the unit. When the tilt
  // rotation angle is at 0, the top of the unit will be parallel to the bottom.
  // When the tilt rotation angle is set to 180 degrees, the top of the unit
  // will be at a 15 degree angle relative to the bottom.
  // Calling this method with a set parameter will set the tilt rotation
  // angle of the capture stage to the provided value and will trigger an
  // on_tilt notification.
  // The value passed into the set method is the absolute tilt rotation angle
  // that will be used.The value passed in will not be added to the current
  // value, as is the case with the rotate method.
  uint64_t tilt(float set);

  // Calling this method with a get parameter will return a float
  // indicating the final tilt rotation angle of the capture stage.
  // Note that there is some variation
  // when rotating the unit, so always check the return value.
  uint64_t tilt(float *get);

  // The value passed into the set method is the absolute tilt rotation angle
  // that will be used.The value passed in will not be added to the current
  // value, as is the case with the rotate method. After the angle has been set
  // it will return a float  indicating the final tilt rotation angle of the
  // capture stage. Note that there is some variation
  // when rotating the unit, so always check the return value.
  uint64_t tilt(float set, float *get);

 protected:
  uint64_t captureStageInfo_json2c(void *obj, CaptureStageSpecificInfo *info);
  uint64_t ledOnOffRate_c2json(const hippo::LedOnOffRate &ledRate, void *obj);
  uint64_t ledOnOffRate_json2c(void *obj, hippo::LedOnOffRate *ledRate);
  uint64_t ledState_c2json(const hippo::LedState &ledSt, void *obj);
  uint64_t ledState_json2c(void *obj, hippo::LedState *state);

  // Callback items
  void ProcessSignal(char *method, void *obj) override;
  bool HasRegisteredCallback();

  void(*callback_)(const CaptureStageNotificationParam &param, void *data);
};

}   // namespace hippo

#endif  // INCLUDE_CAPTURESTAGE_H_
