
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#ifndef INCLUDE_TOUCHMAT_H_
#define INCLUDE_TOUCHMAT_H_

#include "../include/hippo_device.h"


#if COMPILING_DLL
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif

namespace hippo {

// Indicates the current state of the touchmat. A value of true indicates
// that the touchmat is accepting input from the given source. A value of
// false indicates that the touchmat is ignoring input from that source.
typedef struct TouchMatState {
  bool touch;
  // Active pen input is only supported for Sprout Pro G2 touchmats. Setting
  // active_pen to True for Sprout and Sprout Pro touchmats will return an
  // error.
  bool active_pen;
} TouchMatState;

typedef struct TouchMatPoint {
  uint32_t x;     // in the range [0, 15360]
  uint32_t y;     // in the range [0, 8640]
} TouchMatPoint;

typedef struct ActiveArea {
  // If this value is true, input will only be accepted from the rectangular
  // region between the top_left and bottom_right points. If it is false,
  // input will be accepted from the entire touchmat.
  bool enabled;
  // top_left.x < bottom_right.x and top_left.y < bottom_right.y
  TouchMatPoint top_left;
  TouchMatPoint bottom_right;
} ActiveArea;

// Enumerates the possible states for the height where the active pen is
// detected.
typedef enum class ActivePenRange {
  five_mm,
  ten_mm,
  fifteen_mm,
  twenty_mm,
} ActivePenRange;

// The physical dimensions of the touch-sensitive area of the touchmat
// (the white portion of the mat, not including the darker border), in inches.
typedef struct Size {
  float width;
  float height;
} Size;

typedef struct TouchmatHardwareInfo {
  Size size;
} TouchmatHardwareInfo;

typedef enum class TouchMatNotification {
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

  // touchmat specific notifications below

  // This notification occurs when a client sends a command to update the
  // touchmat's active_area, and the command executes successfully. It
  // includes an ActiveArea parameter with the new active area.
  on_active_area,
  // This notification occurs when a client sends a command to update the
  // touchmat's active_pen_range, and the command executes successfully. It
  // includes an ActivePenRange parameter with the new range.
  on_active_pen_range,
  // This notification occurs when a client sends a command to calibrate the
  // touchmat, and the command executes successfully.
  on_calibrate,
  // This notification occurs when a client sends a command to update the
  // touchmat's device_palm_rejection, and the command executes successfully.
  // It includes a bool parameter with the new value.
  on_device_palm_rejection,
  // This notification occurs when a client sends a command to update the
  // touchmat's palm_rejection_timeout, and the command executes successfully.
  // It includes an int parameter with the new value.
  on_palm_rejection_timeout,
  // This notification occurs when a client sends a command to reset the
  // touchmat, and the command executes successfully.
  on_reset,
  // This notification occurs when a client sends a command to update the
  // touchmat state, and the command executes successfully. It includes a
  // TouchmatState parameter with the new state of the touchmat.
  on_state,
} TouchMatNotification;

// The parameters that are passed when a touchmat notification is triggered
typedef struct TouchMatNotificationParam {
  // the type of notification that is being passed
  TouchMatNotification type;
  union {
    // parameter passed with the on_open_count notification
    uint32_t on_open_count;
    // parameter passed with the on_active_area notification
    ActiveArea  on_active_area;
    // parameter passed with the on_active_pen_range notification
    ActivePenRange on_active_pen_range;
    // parameter passed with the on_device_palm_rejection notification
    bool on_device_palm_rejection;
    // parameter passed with the on_palm_rejection_timeout notification
    uint32_t on_palm_rejection_timeout;
    TouchMatState on_state;
  };
} TouchMatNotificationParam;

// The touchmat Sprout device
class DLLEXPORT TouchMat : public HippoDevice {
 public:
  TouchMat();
  explicit TouchMat(uint32_t device_index);
  TouchMat(const char *address, uint32_t port);
  TouchMat(const char *address, uint32_t port, uint32_t device_index);
  virtual ~TouchMat(void);


  // The active_area method can be used to accept input from only a portion
  // of the touchmat. When the active area is enabled, touch or active pen
  // events outside of the active area will be ignored. When the
  // ActiveArea.enabled value is set to false, the touchmat will accept input
  // from the entire mat, regardless of the top_left and bottom_right point
  // values. When the enabled value is set to true, the touchmat will only
  // accept input in the rectangular region defined by the top_left and the
  // bottom_right points. Changing the touchmat's active_area does not affect
  // the touchmat's state. The active_area settings are only used when the
  // state is set to accept either touch or active_pen input (or both). If the
  // active_area is set while the touchmat is not accepting touch or active_pen
  // input, the settings will be stored and used when the state is changed to
  // enable input. Note that the active_area will return to the default values
  // whenever the touchmat loses power (when the touchmat is disconnected or
  // when the system restarts), or when the touchmat.factory_default method is
  // called. By default the active_area is not enabled, and the region is set
  // to include the entire touchmat
  //    enabled= false,
  //    top_left.(x,y)=(0, 0),
  //    bottom_right.(x,y)=(15360, 8640)

  // Calling this method with a get parameter acts as a Get request and returns
  // the current active area as an ActiveArea.
  uint64_t active_area(ActiveArea *get);

  // Calling this method with a set parameter acts as a Set request. On success
  // it will trigger an on_active_area notification.
  uint64_t active_area(const ActiveArea &set);

  // Calling this method with a set parameter acts as a Set request. On success
  // it will return an ActiveArea in the get parameter that confirms the value
  // set and trigger an on_active_area notification.
  uint64_t active_area(const ActiveArea &set, ActiveArea *get);

  // The active_pen_range method can be used to control the approximate height
  // above the touchmat where the active pen is detected.
  // Note that the active_pen_range will return to the default value whenever
  // the touchmat loses power (when the touchmat is disconnected or when the
  // system restarts), or when the touchmat.factory_default method is called.
  // This default active_pen_range is ten_mm.

  // Calling this method with a get parameter acts as a Get request and returns
  // the current setting as an ActivePenRange.
  uint64_t active_pen_range(ActivePenRange *get);

  // Calling this method with a set parameter acts as a Set request. On success
  // it will trigger an on_active_pen_range notification.
  uint64_t active_pen_range(const ActivePenRange &set);

  // Calling this method with a set parameter acts as a Set request. On success
  // it will return an ActivePenRange in the get parameter that confirms the
  // value set and trigger an on_active_pen_range notification.
  uint64_t active_pen_range(const ActivePenRange &set, ActivePenRange *get);

  // Runs the touchmat's calibration algorithm. This calibration method takes
  // the raw touch value for every pixel and determines the offset value
  // necessary to zero it. This creates a calibration table which is stored in
  // the touchmat's controller memory.
  // This can be thought of as similar to zeroing out a scale, but instead for
  // the touch response of every pixel.Note: Nothing should be touching the mat
  // when this calibrate method is called.If the touchmat is calibrated with
  // objects touching it, this can lead to ghost touches when those objects are
  // removed.Additionally, the mat should be calibrated in the same location
  // and/or position as it will be used in normal operation.
  // Note : This method is not related to xy position calibration.On success,
  // this method will trigger an on_calibrate notification.
  uint64_t calibrate();

  // The device_palm_rejection variable is used to control if the touchmat's
  // internal palm rejection is enabled. Note that the touchmat's palm
  // rejection is only active when both touch and active pen are enabled
  // (i.e. the state is active_pen=true, touch=true). If the device's palm
  // rejection is enabled, the touchmat will only send either the active pen
  // signal or the touch signal to the operating system at a given time. If
  // the device palm rejection is not enabled, the touchmat will send both the
  // touch and active pen signals to the operating system simultaneously, and
  // the operating system will determine which signal to accept input from at
  // a given time. Note that the device_palm_rejection will return to the
  // default value whenever the touchmat loses power (when the touchmat is
  // disconnected or when the system restarts), or when the
  // touchmat.factory_default method is called. By default the
  // device_palm_rejection is set to false.

  // Calling this method with a get parameter acts as a Get request and returns
  // the current setting as a bool value.
  uint64_t device_palm_rejection(bool *get);

  // Calling this method with a set parameter acts as a Set request. On success
  // it will trigger an on_device_palm_rejection notification.
  uint64_t device_palm_rejection(bool set);

  // Calling this method with a set parameter acts as a Set request. On success
  // it will return a bool in the get parameter that confirms the value set and
  // trigger an on_device_palm_rejection notification.
  uint64_t device_palm_rejection(bool set, bool *get);

  // Returns the touchmat's TouchmatHardwareInfo data.
  uint64_t hardware_info(TouchmatHardwareInfo *get);

  // When the touchmat's internal palm rejection is active, so the touchmat
  // is ignoring touch input in favor of active pen input, all input must stop
  // for palm_rejection_timeout milliseconds before the mat will begin
  // accepting touch input again. If the touchmat receives any input
  // (touch or pen) before the full timeout has elapsed, the clock is reset and
  // the timeout starts over.
  // Note: This setting is only used when the device is handling palm rejection
  // That is, when the touchmat.state is set to active_pen=true, touch=true
  // and the touchmat.device_palm_rejection is set to true. The
  // palm_rejection_timeout will return to the default value whenever the
  // touchmat loses power (when the touchmat is disconnected or when the
  // system restarts).

  // Calling this method without parameters acts as a Get request and returns
  // the current setting in the get parameter as an int value.
  uint64_t palm_rejection_timeout(uint32_t *get);

  // Calling this method with a set parameter acts as a Set request. On success
  // it will trigger an on_palm_rejection_timeout notification.
  // Valid range is 150 <= set <= 2000
  uint64_t palm_rejection_timeout(uint32_t set);

  // Calling this method with a set parameter acts as a Set request. On success
  // it will return an int in the get parameter that confirms the value set and
  // trigger an on_palm_rejection_timeout notification.
  // Valid range is 150 <= set <= 2000
  uint64_t palm_rejection_timeout(uint32_t set, uint32_t *get);

  // Reboots the touchmat. This is equivalent to physically undocking and
  // redocking the device. As such, it will cause
  // touchmat.on_device_disconnected, system.on_device_disconnected,
  // touchmat.on_device_connected, and system.on_device_connected notifications
  // Additionally, the behavior should be identical to when the touchmat is
  // physically undocked and redocked. The device will be closed when the the
  // on_device_disconnected notifications are sent. By the time the
  // on_device_connected notifications are sent, all settings will be restored
  // to the hardware's default values. After the connected notifications, the
  // touchmat will need to be opened and any desired settings will need to be
  // set. On success, this method will also trigger an on_reset notification.
  // Note that the on_device_disconnected and on_device_connected notifications
  // may come before or after the response to the command and touchmat.on_reset
  // notification.
  uint64_t reset();

  // The touchmat's state indicates which sources the touchmat is accepting
  // input from. Note that the state will return to the firmware's default
  // values whenever the touchmat loses power (when the touchmat is
  // disconnected or when the system restarts). The default state for Sprout
  // and Sprout Pro touchmats is active_pen=false, touch=false. The default
  // state for Sprout Pro G2 touchmats is active_pen=true, touch=true. Also
  // note that the touchmat.factory_default method will disable touch and
  // active pen input for all versions of the touchmat.

  // Calling this method without parameters acts as a Get request and returns
  // the current state as a TouchmatState object.
  uint64_t state(hippo::TouchMatState *get);

  // Calling this method with a set parameter will change the state of the
  // touchmat and will trigger an on_state notification.
  uint64_t state(const hippo::TouchMatState &set);

  // Calling this method with a set parameter will change the state of the
  // touchmat and will trigger an on_state notification.  It will also
  // confirm the current state of the touchmat and return the value in the
  // get parameter
  uint64_t state(const hippo::TouchMatState &set, hippo::TouchMatState *get);

  // Subscribe to touchmat notificationss
  uint64_t subscribe(void(*callback)(const TouchMatNotificationParam &param,
                                     void *data),
                     void *data);
  uint64_t subscribe(void(*callback)(const TouchMatNotificationParam &param,
                                     void *data),
                     void *data, uint32_t *get);

  // unsubscribe from notifications
  uint64_t unsubscribe();
  uint64_t unsubscribe(uint32_t *get);

 protected:
  uint64_t touchMatState_json2c(void *obj, TouchMatState *state);
  uint64_t touchMatState_c2json(const hippo::TouchMatState &state, void *obj);
  uint64_t active_area_json2c(void *obj, ActiveArea *area);
  uint64_t active_area_c2json(const hippo::ActiveArea &area, void *obj);
  uint64_t active_pen_range_json2c(void *obj, hippo::ActivePenRange *range);
  uint64_t active_pen_range_c2json(const hippo::ActivePenRange &area,
                                   void *obj);
  uint64_t hardware_info_json2c(void *obj, hippo::TouchmatHardwareInfo *info);
  // Callback items
  void ProcessSignal(char *method, void *obj) override;
  bool HasRegisteredCallback();

  void (*callback_)(const TouchMatNotificationParam &param, void *data);
};

}   // namespace hippo

#endif  // INCLUDE_TOUCHMAT_H_
