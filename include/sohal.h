
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#ifndef INCLUDE_SOHAL_H_
#define INCLUDE_SOHAL_H_

#include "../include/hippo_device.h"


#if COMPILING_DLL
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif

namespace hippo {

// The logging level.
// Messages with levels less than or equal to the current level will be logged
// Valid values are in the range [0, 4].
//  0 : Low level errors only
//  1 : Data for analytics
//  2 : Informational messages
//  3 : Debugging information
//  4 : All messages
typedef struct LogInfo {
  uint32_t level;
} LogInfo;

typedef enum class SoHalNotification {
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

  // SoHal specific notifications:

  // A notification that is sent just before SoHal exits.
  // This could be due to a response to a sohal.exit command,
  // the SoHal service being stopped, or SoHal catching an interrupt
  // (Ctrl+C) signal (when running as a standalone application).
  on_exit,
  // This notification occurs when a client sends a command to update
  // the logging values, and the command executes successfully.
  // It includes a LogInfo parameter with the new log information.
  on_log,
} SoHalNotification;

// The parameters that are passed when a sohal notification is triggered
typedef struct SoHalNotificationParam {
  // the type of notification that is being passed
  SoHalNotification type;
  union {
    // parameter passed with the on_open_count notification
    uint32_t on_open_count;
    // parameter passed with the on_log notification
    LogInfo on_log;
  };
} SoHalNotificationParam;

// The sohal object provides high level methods and notifications for
// the SoHal application.
class DLLEXPORT SoHal : public HippoDevice {
 public:
  SoHal();
  SoHal(const char *address, uint32_t port);
  virtual ~SoHal(void);

  // Ensures completion of current tasks and exits SoHal.
  // Part of ensuring completion of the current tasks is
  // to ensure completion of itself before SoHal finishes,
  // so it will return a response to the caller and send the
  // corresponding on_exit notification
  uint64_t exit();

  // change the log level in SoHal
  // The log method can be used to query or set the level of
  // the SoHal log file.Calling this method with a get parameter
  // acts as a Get request and returns a LogInfo value indicating
  // the current SoHal logging information.
  uint64_t log(LogInfo *get);

  // Calling this method with a set parameter acts as a Set request
  // It will trigger an an on_log notification.
  uint64_t log(const LogInfo &set);

  // Calling this method with a set parameter acts as a Set request
  // and returns a get parameter that confirms the value set.
  // It will trigger an an on_log notification.
  uint64_t log(const LogInfo &set, LogInfo *get);

  // subscribe for notifications
  uint64_t subscribe(void(*callback)(const SoHalNotificationParam &param,
                                     void *data),
                     void *data);
  uint64_t subscribe(void(*callback)(const SoHalNotificationParam &param,
                                     void *data),
                      void *data, uint32_t *get);

  // Returns a string containing the SoHal version.
  //
  // Note: this internally allocates an array of characters
  // which must be freed using the free_version function
  uint64_t version(char **get);

  // frees the memory allocated in the version() function
  void free_version(char *version_to_free);

 protected:
  // json2c
  uint64_t logInfo_json2c(const void *obj, hippo::LogInfo *get);
  uint64_t logInfo_c2json(const hippo::LogInfo &loginfo, void *obj);

  // Callback items
  void ProcessSignal(char *method, void *params) override;
  bool HasRegisteredCallback();

  void(*callback_)(const SoHalNotificationParam &param, void *data);
};

}   // namespace hippo

#endif  // INCLUDE_SOHAL_H_
