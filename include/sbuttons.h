
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#ifndef INCLUDE_SBUTTONS_H_
#define INCLUDE_SBUTTONS_H_

#include "../include/hippo_device.h"


#if COMPILING_DLL
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif

namespace hippo {

// The ButtonLedColor enumerates the colors for the sbuttons LEDs.
typedef enum class ButtonLedColor {
  orange,
  white,
  white_orange,
} ButtonLedColor;

// The ButtonLedMode enumerates the different modes for the sbuttons LEDs.
typedef enum class ButtonLedMode {
  // The LED is continuously ramping on and then off again
  breath,
  // The LED ramps on and then stays on
  controlled_on,
  // The LED ramps off and then stays off
  controlled_off,
  // The LED is off. Setting the mode to off will immediately turn off the
  // light, as opposed to the controlled_off which dims the light until the
  // LED is off.
  off,
  // The LED is on. Setting the mode to on will immediately set the LED full
  // brightness, as opposed to the controlled_on which slowly increases the
  // illumination until the LED is at full brightness.
  on,
  // The LED is flashing on and off.
  pulse
} ButtonLedMode;

// The ButtonId enumerates the sbuttons.
typedef enum class ButtonId {
  left,
  center,
  right,
} ButtonId;

// The ButtonLedState details the current or desired state for an sbuttons LED.
typedef struct ButtonLedState {
  ButtonLedColor color;
  ButtonLedMode mode;
} ButtonLedState;

// used with the on_led_state notification
typedef struct ButtonLedStateNotification {
  ButtonLedState state;
  ButtonId id;
} ButtonLedStateNotification;

// The ButtonPressType enumerates the different types of button press events.
typedef enum class ButtonPressType {
  tap,
  hold,
} ButtonPressType;

// The ButtonPress type provides details for the on_button_press notification,
// which is sent when the user taps or holds one of the sbuttons.
typedef struct ButtonPress {
  ButtonId id;
  ButtonPressType type;
} ButtonPress;

typedef enum class SButtonsNotification {
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

  // sbuttons specific notifications below

  // This notification occurs when a client sets the hold_threshold. It
  // includes an int parameter with the new value.
  on_hold_threshold,
  // This notification occurs when a client sets the led_on_off_rate. It
  // includes an int parameter with the new value.
  on_led_on_off_rate,
  // This notification occurs when a client sets the led_pulse_rate. It
  // includes an int parameter with the new value.
  on_led_pulse_rate,
  // This notification occurs when a client updates the led_state. It
  // includes a ButtonLedStateNotification parameter indicating which
  // button's state changed,what the new state is.
  on_led_state,
  // This notification occurs when the user presses a button. It includes
  // a ButtonPress parameter with the specific details of the press event.
  on_button_press,
} SButtonsNotification;

// The parameters that are passed when a sbuttons notification is triggered
typedef struct SButtonsNotificationParam {
  // the type of notification that is being passed
  SButtonsNotification type;
  union {
    // parameter passed with the on_open_count notification
    uint32_t on_open_count;
    // parameter passed with the on_hold_threshold notification
    uint32_t on_hold_threshold;
    // parameter passed with the on_led_on_off_rate notification
    uint32_t on_led_on_off_rate;
    // parameter passed with the on_led_pulse_rate notification
    uint32_t on_led_pulse_rate;
    // parameter passed with the on_led_state notification
    ButtonLedStateNotification on_led_state;
    // parameter passed with the on_button_press notification
    ButtonPress on_button_press;
  };
} SButtonsNotificationParam;


class DLLEXPORT SButtons : public HippoDevice {
 public:
  SButtons();
  explicit SButtons(uint32_t device_index);
  SButtons(const char *address, uint32_t port);
  SButtons(const char *address, uint32_t port, uint32_t device_index);
  virtual ~SButtons(void);

  // The hold_threshold is the threshold time that differentiates
  // between a short button press (tap) and a long button press (hold).
  // Calling this method with a get parameter acts as a Get request and
  // returns an int in the get parameter with the current value.
  uint64_t hold_threshold(uint32_t *get);

  // The hold_threshold is the threshold time that differentiates
  // between a short button press (tap) and a long button press (hold).
  // Calling this method with an |int| parameter acts as a Set request.
  // On success it will trigger an on_hold_threshold notification.
  uint64_t hold_threshold(uint32_t set);

  // The hold_threshold is the threshold time that differentiates
  // between a short button press (tap) and a long button press (hold).
  // Calling this method with an |int| parameter acts as a Set request and
  // returns an int in the get parameter that confirms the value set. On
  // success it will trigger an on_hold_threshold notification.
  uint64_t hold_threshold(uint32_t set, uint32_t *get);

  // The led_on_off_rate controls the rate at which the LEDs ramp on and
  // off in breath mode. This rate is also used by the controlled_on and
  // controlled_off modes.
  // Calling this method with a get parameters acts as a Get request and
  // returns an int with the current value.
  uint64_t led_on_off_rate(uint32_t *get);

  // The led_on_off_rate controls the rate at which the LEDs ramp on and
  // off in breath mode. This rate is also used by the controlled_on and
  // controlled_off modes.
  // Calling this method with an set parameter acts as a Set request and
  // upon success it will trigger an on_led_on_off_rate notification.
  // Valid range is 1 <= set <= 20.
  uint64_t led_on_off_rate(uint32_t set);

  // The led_on_off_rate controls the rate at which the LEDs ramp on and
  // off in breath mode. This rate is also used by the controlled_on and
  // controlled_off modes.
  // Calling this method with an set parameter acts as a Set request and
  // returns an int in the get parameter that confirms the value set.
  // On success it will trigger an on_led_on_off_rate notification.
  // Valid range is 1 <= set <= 20.
  uint64_t led_on_off_rate(uint32_t set, uint32_t *get);

  // The led_pulse_rate controls the rate at which the LEDs flash on and
  // off in pulse mode. Calling this method with a get parameter acts as
  // a Get request and returns an int in the get parameter with the current
  // value.
  uint64_t led_pulse_rate(uint32_t *get);

  // The led_pulse_rate controls the rate at which the LEDs flash on and
  // off in pulse mode.
  // Calling this method with a set parameter acts as a Set request and
  // upon success it will trigger an on_led_pulse_rate notification.
  // Valid range is 1 <= set <= 20.
  uint64_t led_pulse_rate(uint32_t set);

  // The led_pulse_rate controls the rate at which the LEDs flash on and
  // off in pulse mode.
  // Calling this method with a set parameter acts as a Set request and
  // returns an int in the get parameter that confirms the value set.
  // On success it will trigger an on_led_pulse_rate notification.
  // Valid range is 1 <= set <= 20.
  uint64_t led_pulse_rate(uint32_t set, uint32_t *get);

  // Calling this method with a get parameter acts as a Get request and
  // returns the current state of the specified LED.
  uint64_t led_state(ButtonId id, ButtonLedState *get);

  // Calling this method with ButtonId and ButtonLedState parameters acts as
  // a Set request and upon success it will trigger an
  // on_led_state notification.
  uint64_t led_state(ButtonId id, const ButtonLedState &set);

  // Calling this method with ButtonId and ButtonLedState parameters acts as
  // a Set request and returns a ButtonLedState in the get parameter that
  // confirms the value set. On success it will trigger an
  // on_led_state notification.
  uint64_t led_state(ButtonId id, const ButtonLedState &set,
                     ButtonLedState *get);

  // subscribe to notifications
  uint64_t subscribe(void(*callback)(const SButtonsNotificationParam &param,
                                     void *data),
                     void *data);
  uint64_t subscribe(void(*callback)(const SButtonsNotificationParam &param,
                                     void *data),
                     void *data, uint32_t *get);

  // unsubscribe from notifications
  uint64_t unsubscribe();
  uint64_t unsubscribe(uint32_t *get);

 protected:
  // json2c
  uint64_t button_led_state_json2c(void *obj, hippo::ButtonLedState *state);
  uint64_t button_led_state_c2json(hippo::ButtonId id,
                                   const hippo::ButtonLedState &state,
                                   void *obj);
  uint64_t button_led_state_notification_json2c(
      void *obj, hippo::ButtonLedStateNotification *state);
  uint64_t button_press_notification_json2c(
      void *obj, hippo::ButtonPress *touch);
  // Callback items
  void ProcessSignal(char *method, void *obj) override;
  bool HasRegisteredCallback();

  void (*callback_)(const SButtonsNotificationParam &param, void *data);
};

}   // namespace hippo

#endif  // INCLUDE_SBUTTONS_H_
