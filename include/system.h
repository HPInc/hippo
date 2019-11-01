
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#ifndef INCLUDE_SYSTEM_H_
#define INCLUDE_SYSTEM_H_

#include "../include/hippo_device.h"
#include "../include/system_types.h"
#include "../include/common_types.h"

#if COMPILING_DLL
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif

namespace hippo {

typedef struct LensDistortion {
  // The center of the lens distortion
  PointFloats center;
  // A list with the radial distortion coefficient initially
  // initialized to zeros
  float kappa[6];
  // A list with the tangential distortion coefficients initially
  // initialized to zeros
  float p[2];
} LensDistortion;

typedef enum class CameraNameType {
  depthcamera,
  hirescamera,
} CameraNameType;

typedef enum class CameraStreamType {
  rgb,
  depth,
  ir,
  points,
}CameraStreamType;

// Enumerates the power states that are sent out as notifications
typedef enum class PowerStateType {
  // Sent when the display has been turned on.
  display_on,
  // Sent when the display has been turned off.
  display_off,
  // Sent when the display has been dimmed, usually from
  // the computer being idle for a specified amount of time
  display_dimmed,
  // Sent when the system is suspending operation
  // (such as going to sleep)
  suspend,
  // Sent when the system is resuming from a low-power
  // state (such as waking up from sleep)
  resume,
  // Sent when the user is logging off.
  // This item is deprecated and will be removed in a
  // future release. Please use the on_session_change
  // notification to receive log_off events.
  log_off,
  // Sent when the system is shutting down or restarting
  shut_down,
}PowerStateType;

// Enumerates the session change events that are sent out as notifications.
typedef enum class SessionChangeEvent {
  // The session was connected to the console terminal
  // or RemoteFX session
  console_connect,
  // The session was disconnected from the console
  // terminal or RemoteFX session
  console_disconnect,
  // A user has logged on to the session
  session_logon,
  // A user has logged off of the session
  session_logoff,
  // The session has been locked
  session_lock,
  // The session has been unlocked
  session_unlock,
}SessionChangeEvent;

typedef struct CameraStream {
  // index corresponding to the SoHal index in the
  // device name (ie hirescamera@index).
  uint32_t index;
  // camera device name, currently only 'depthcamera' or 'hirescamera'
  // are supported
  CameraNameType name;
  // one of rgb, ir, depth or points depending on the streams supported by
  // the camera
  CameraStreamType stream;
} CameraStream;

typedef struct Camera3DMappingParameter {
  CameraStream from;        // The origin device of the transformation
  CameraStream to;          // The destination device of the transformation
} Camera3DMappingParameter;

typedef struct CameraParameters {
  // The camera resolution used to perform the calibration
  Resolution calibration_resolution;
  // Information on the particular camera and data stream used.
  CameraStream camera;
  // The focal length used in camera calibration.
  PointFloats focal_length;
  // The radial and tangential lens distortion coefficients
  LensDistortion lens_distortion;
} CameraParameters;

typedef struct Camera3DMapping {
  // The origin device of the  transformation
  CameraParameters from;
  // a 4x4 3D matrix transformation in column major order
  float matrix_transformation[4][4];
  // The destination device of the transformation
  CameraParameters to;
} Camera3DMapping;

// Dictionary containing the ID information of certain hardware components.
typedef struct HardwareIDs {
  char **sprout_projector;    // A list containing the IDs for the projector(s)
  char **sprout_touchscreen;  // A list containing the IDs for the touchscreens
} HardwareIDs;

// The SessionChange type includes information about a session change event
typedef struct SessionChange {
  // The type of session change event
  SessionChangeEvent change_event;
  // The operating system's session identifier for the session that
  // triggered the event
  uint32_t session_id;
} SessionChange;

// The SessionState type enumerates the possible states of the console session.
typedef enum class SessionState {
  locked,                 // The session is locked.
  unlocked,               // The session is unlocked.
  unknown,                // The session state is not known.
} SessionState;

typedef enum class SystemNotification {
  // This notification occurs whenever a supported
  // device has been connected. The parameter is
  // the DeviceID. The memory allocated in the
  // DeviceID parameter is automatically freed when
  // program control returns from the callback.  There
  // is no need to call free_device_id() passing in the
  // parameter
  on_device_connected,
  // This notification occurs whenever a supported
  // device has been disconnected. The parameter
  // is the DeviceID. The memory allocated in the
  // DeviceID parameter is automatically freed when
  // program control returns from the callback.  There
  // is no need to call free_device_id() passing in the
  // parameter
  on_device_disconnected,
  // This notification occurs whenever a display is
  // connected, disconnected, or there is a change
  // in the display settings (such as changes in
  // display positioning, ordering, resolution, and
  // orientation). The parameter will contain a
  // list of DisplayInfo objects with the updated
  // information for each display that is connected
  // to the system. The memory allocated for the
  // DisplayInfo objects will be automatically freed
  // when program control returns from the callback
  // function. There is no need for the user to call
  // free_display_list and pass in the display list
  // parameter
  on_display_change,
  // This notification occurs whenever a power
  // state change happens at the operating system
  // level.
  on_power_state,
  // This notification occurs when a session change
  // event happens at the operating system level.
  on_session_change,
  // This notification occurs when the current
  // temperature for a TemperatureSensor goes
  // above that sensor's safe temperature limit.
  // It includes a TemperatureInfo parameter with
  // information on the affected sensor.
  // Note: For Sprout Pro G2 these temperature
  // notifications originate from the projector.
  // For HP Z 3D Camera they originate from the
  // hirescamera. As such, notifications will only
  // be sent out when one of these devices is
  // connected and open.
  on_temperature_high,
  // This notification occurs when the current
  // temperature for a TemperatureSensor goes
  // above that sensor's max temperature limit.
  // It includes a TemperatureInfo parameter with
  // information on the affected sensor.
  // Note: For Sprout Pro G2 these temperature
  // notifications originate from the projector.
  // For HP Z 3D Camera they originate from the
  // hirescamera. As such, notifications will only
  // be sent out when one of these devices is
  // connected and open.
  on_temperature_overtemp,
  // This notification occurs when a
  // TemperatureSensor that is above its safe
  // temperature cools back down below the safe
  // value. It includes a TemperatureInfo
  // parameter with information on the affected
  // sensor.
  // Note: For Sprout Pro G2 these temperature
  // notifications originate from the projector.
  // For HP Z 3D Camera they originate from the
  // hirescamera. As such, notifications will only
  // be sent out when one of these devices is
  // connected and open.
  on_temperature_safe,
  // This notification occurs when the connection
  // to SoHal is lost at websocket level
  on_sohal_disconnected,
  // This notification occurs when, after losing connection to SoHal,
  // the connection is reestablished
  on_sohal_connected,
} SystemNotification;

// The parameters that are passed when a system notification is triggered
typedef struct SystemNotificationParam {
  // the type of notification that is being passed
  SystemNotification type;
  union {
    // parameter passed with the on_device_connected notification
    DeviceID on_device_connected;
    // parameter passed with the on_device_disconnected notification
    DeviceID on_device_disconnected;
    // parameter passed with the on_display_change notification
    // This will be a pointer to an array of DisplayInfos
    // the number of DisplayInfos in the array will be contained
    // in the num_displays value below
    DisplayInfo *on_display_change;
    // parameter passed with the on_power_state notification
    PowerStateType on_power_state;
    // parameter passed with the on_temperature_high notification
    TemperatureInfo on_temperature_high;
    // parameter passed with the on_temperature_overtemp notification
    TemperatureInfo on_temperature_overtemp;
    // parameter passed with the on_temperature_safe notification
    TemperatureInfo on_temperature_safe;
    // parameter passed with the on_session_change notification
    SessionChange on_session_change;
  };
  // the number of DisplayInfo items in the array when the
  // on_display_change notification is the notification stored in
  // the type variable
  uint64_t num_displays;
} SystemNotificationParam;

// The system object provides methods and notifications that include
// information on several devices or apply to the system as a whole.
class DLLEXPORT System : public HippoDevice {
 public:
  System();
  System(const char *address, uint32_t port);
  virtual ~System(void);

  // Returns a  Camera3DMapping  object with the 3D transformation
  // requested using the  Camera3DMappingParameter  parameter.The
  // following transformations are currently supported :
  //    Sprout Pro G2 :
  //      depthcamera.rgb to hirescamera.rgb
  //      depthcamera.ir to depthcamera.rgb
  //      hirescamera.rgb to depthcamera.rgb
  //    HP Z 3D Camera :
  //      depthcamera.rgb to hirescamera.rgb
  //      depthcamera.ir to hirescamera.rgb
  //      depthcamera.ir to depthcamera.rgb
  uint64_t camera_3d_mapping(const Camera3DMappingParameter &set,
                             Camera3DMapping *get);

  // Returns a list with a DeviceInfo object for each supported device
  // that is connected to the system. Internally this function allocates
  // memory.  The caller is responsible for ensuring that free_devices
  // is called in order to avoid a memory leak
  uint64_t devices(DeviceInfo **get, uint64_t *num_devices);

  // frees the memory allocated in the devices() function
  // failing to call this function after calling devices()
  // will result in a memory leak
  //
  // Note this function is safe to call even if devices()
  // returns an error
  void free_devices(DeviceInfo *info, uint64_t num_devices);

  // Returns a list with a DeviceID object for each supported device
  // that is connected to the system. Similar to devices but without
  // the firmware version and serial number for each connected device.
  // Inernally this funcon allocates and fills a DeviceID array using malloc
  // The caller is responsible for ensuring that free_device_ids is called
  //
  // Note: This method is much faster than calling devices because it
  // doesn't open a connection to each individual device to get its
  // firmware version and serial number. Use this method if these two
  // parameters are not needed.
  uint64_t device_ids(DeviceID **get, uint64_t *num_devices);

  // frees the memory allocated in the device_ids() function
  // failing to call this function after calling device_ids()
  // will result in a memory leak
  // Note: this function is safe to call even if device_ids()
  // returns an error
  void free_device_ids(DeviceID *ids, uint64_t num_devices);

  // A method which simply echoes the parameter back to the caller.
  // This method is provided for debugging purposes, as it provides
  // a simple way to test a connection or JSON-RPC message
  //
  // Note: This function allocates memory internally
  // to return the string back in the get variable
  // The free_echo_string must be called passing in the
  // returned get char* or else a memory leak will ocurr
  uint64_t echo(const char *set, char **get);

  // free_echo_string() must be called after calling echo
  // to avoid a memory leak
  void free_echo_string(char *echo_string_to_free);

  // Returns a dictionary of type HardwareIDs containing hardware ID
  // information on some hardware components such as the sprout projector
  // and sprout touchscreens.
  //
  // Note: this function allocates memory internally, the
  // free_hardware_ids function must be called after calling this to
  // avoid a memory leak
  uint64_t hardware_ids(HardwareIDs *get,
                        uint64_t *num_projectors,
                        uint64_t *num_touchscreens);

  // frees the memory allocated inside hardware_ids()
  void free_hardware_ids(HardwareIDs *ids_to_free,
                         uint64_t num_projectors,
                         uint64_t num_touchscreens);

  // Returns the state of the current console session as a SessionState object.
  uint64_t is_locked(SessionState * get);

  // Returns a list with a DisplayInfo object for each display that
  // is connected to the system.
  //
  // Note:  This function allocates memory internally, and the user
  // is responsible for calling free_display_list afterwards or a
  // memory leak will ocurr
  uint64_t list_displays(DisplayInfo **get, uint64_t *num_displays);

  // Frees the memory allocated by the list_displays command
  // this is safe to call even if list_displays returns with an error
  void free_display_list(DisplayInfo *list_to_free, uint64_t num_to_free);

  // Returns the current session ID (i.e. the active user session).
  // If no active session is attached to the console,
  // then this will return an error.
  uint64_t session_id(uint32_t *get);

  // Returns a list with the names of all devices that SoHal supports.
  // This does not indicate that all supported devices are connected or
  // available on the current hardware. For information on devices that
  // are currently connected, use the system.devices method.
  uint64_t supported_devices(SupportedDevice **get, uint64_t *num_devices);

  // frees the memory allocated in the supported_devices() function
  // failing to call this function after calling supported_devices()
  // will result in a memory leak
  // note this function is safe to call even if supported_devices()
  // returns an error
  void free_supported_devices(SupportedDevice *devices, uint64_t num_devices);

  //**************************************************************************
  // TODO(HP Developers): Temperatures with device name input parameter
  //                      to get subset of temps
  //**************************************************************************

  uint64_t subscribe(void(*callback)(const SystemNotificationParam &param,
                                     void *data),
                     void *data);
  uint64_t subscribe(void(*callback)(const SystemNotificationParam &param,
                                     void *data),
                      void *data, uint32_t *get);

  // unsubscribe from notifications
  uint64_t unsubscribe();
  uint64_t unsubscribe(uint32_t *get);

 protected:
  uint64_t cam_3d_map_json2c(const void *obj,
                             hippo::Camera3DMapping *cameraMapping);
  uint64_t camera_parameters_json2c(const void *obj,
                                    hippo::CameraParameters *cameraParams);
  uint64_t camera_stream_json2c(const void *obj,
                                hippo::CameraStream *cameraStream);
  uint64_t cam_3d_mapping_c2json(
                const hippo::Camera3DMappingParameter &camMapParam, void *obj);
  uint64_t camera_stream_c2json(
                const hippo::CameraStream &cameraStream, void *obj);
  uint64_t devices_json2c(const void *obj, DeviceInfo **device_info,
                          uint64_t *num_devices);
  uint64_t device_id_json2c(const void *obj, DeviceID *id_info);
  uint64_t device_ids_json2c(const void *obj, DeviceID **id_info,
                             uint64_t *num_devices);
  uint64_t echo_json2c(const void *obj, char **echo_return_str);
  uint64_t hardware_ids_json2c(const void *obj, HardwareIDs *get,
                               uint64_t *num_projectors,
                               uint64_t *num_touchscreens);
  uint64_t is_locked_json2c(const void *obj, SessionState *session_state);
  uint64_t list_displays_json2c(const void *obj, DisplayInfo **display_info,
                                uint64_t *num_displays);
  uint64_t powerstate_json2c(const void *obj, PowerStateType *powerState);
  uint64_t sessionchange_json2c(const void *obj,
                                hippo::SessionChange *session_change);
  uint64_t supported_devices_json2c(const void *obj,
                                    SupportedDevice **device_names,
                                    uint64_t *num_devices);

  // frees an individual device id, such as those generated
  // by notifications.  This function is called internally by
  // the free_device_ids() function
  void free_device_id(DeviceID *id_to_free);

  // frees the memory allocated inside an individual DisplayInfo item
  // this is called internally by the free_display_list() function
  void free_display_info(DisplayInfo *display_info);

  // Callback items
  void ProcessSignal(char *method, void *obj) override;
  bool HasRegisteredCallback();

  void(*callback_)(const SystemNotificationParam &param, void *data);
};

}   // namespace hippo

#endif  // INCLUDE_SYSTEM_H_
