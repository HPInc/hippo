
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#ifndef INCLUDE_DESKLAMP_H_
#define INCLUDE_DESKLAMP_H_

#include "../include/hippo_device.h"


#if COMPILING_DLL
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif

namespace hippo {

// The DesklampState enumerates the possible states of the desklamp device.
typedef enum class DeskLampState {
  off,    // the desklamp is turned off and is emitting no light
  low,    // the desklamp is emitting some light
  high    // the desklamp is emitting the maximum amount of light supported
} DeskLampState;

typedef enum class DeskLampNotification {
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

  // desklamp specific notifications below

  // This notification occurs when a client successfully sets the state of the
  // desklamp (using the high, low, or off method). This notification also
  // occurs when the user changes the state of the desklamp by touching the
  // device, but only if the device is open when the change occurs
  on_state,
} DeskLampNotification;

// The parameters that are passed when a desklamp notification is triggered
typedef struct DeskLampNotificationParam {
  // the type of notification that is being passed
  DeskLampNotification type;
  union {
    // parameter passed with the on_open_count notification
    uint32_t on_open_count;
    // parameter passed with the on_state notification
    DeskLampState on_state;
  };
} DeskLampNotificationParam;

class DLLEXPORT DeskLamp : public HippoDevice {
 public:
  DeskLamp();
  explicit DeskLamp(uint32_t device_index);
  DeskLamp(const char *address, uint32_t port);
  DeskLamp(const char *address, uint32_t port, uint32_t device_index);
  virtual ~DeskLamp(void);

  //  the desklamp is emitting the maximum amount of light supported
  uint64_t high(void);

  // Turns the desklamp LEDs to low intensity and triggers an
  // on_state notification.
  uint64_t low(void);

  // Turns the desklamp LEDs off and triggers an on_state notification.
  uint64_t off(void);

  // Returns the state as a DesklampState value.
  uint64_t state(DeskLampState *get);

  // subscribe to notifications
  uint64_t subscribe(void(*callback)(const DeskLampNotificationParam &param,
                                     void *data),
                     void *data);
  uint64_t subscribe(void(*callback)(const DeskLampNotificationParam &param,
                                     void *data),
                     void *data, uint32_t *get);

  // unsubscribe from notifications
  uint64_t unsubscribe();
  uint64_t unsubscribe(uint32_t *get);

 protected:
  uint64_t desklamp_state_json2c(void *obj, hippo::DeskLampState *state);

  // Callback items
  void ProcessSignal(char *method, void *obj) override;
  bool HasRegisteredCallback();

  void(*callback_)(const DeskLampNotificationParam &param, void *data);
};

}   // namespace hippo

#endif  // INCLUDE_DESKLAMP_H_
