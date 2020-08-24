
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#ifndef INCLUDE_HIPPO_DEVICE_H_
#define INCLUDE_HIPPO_DEVICE_H_

#include "../include/hippo.h"
#include "../include/common_types.h"
#include "../include/system_types.h"  // for TemperatureInfo

#if COMPILING_DLL
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif

namespace std {
class thread;
};

namespace hippo {

class HippoWS;

const uint32_t MAX_DEV_LEN = 64;
const uint32_t MAX_ADDR_LEN = 256;

// The Vendor ID (VID) and Product ID (PID) for each device are listed in the
// table below. Note that the HP Z 3D Camera's High Resolution Camera and UVC
// Camera are the same physical device, but the Product ID and functionality
// available changes based on if the device is plugged into a USB 2.0 or
// USB 3.0 port. When the device is plugged into a USB 2.0 port, it will be
// identified as a uvccamera device instead of a hirescamera device.
//
//  Device                                    Vendor ID       Product ID
//--------------------------------------------------------------
//  Capture Stage                             0x0403          0x7838
//  Depth Camera - Sprout, Sprout Pro         0x8086          0x0A66
//  Depth Camera - Sprout Pro G2              0x2BC5          0x0405
//  Depth Camera - HP Z 3D Camera             0x2BC5          0x0406
//  Desk Lamp                                 0x03F0          0x0251
//  High Resolution Camera -
//    Sprout, Sprout Pro, Sprout Pro G2       0x05A9          0xF580
//  High Resolution Camera (USB 3.0) -
//    HP Z 3D Camera                          0x05C8          0xF582
//  Projector - Sprout, Sprout Pro            0x03F0          0x0651
//  Projector - Sprout Pro G2                 0x03F0          0x0751
//  SButtons                                  0x03F0          0x0451
//  Touchmat - Sprout, Sprout Pro             0x0596          0x0548
//  Touchmat - Sprout Pro G2                  0x04DD          0x99AC
//  UVC Camera (USB 2.0) -
//    HP Z 3D Camera                          0x05C8          0xF583
//  UVC Camera                                Various         Various


// DeviceID contains a collection of fundamental information about the device.
typedef struct DeviceID {
  // The alias of a device, meaning there is no @ sign in name
  // even if the index of the device is greater than zero
  char *name;
  // Zero-based index of an array of devices of the same class.
  uint32_t index;
  // vendor id of the device, normally used as hex on the system
  uint32_t vendor_id;
  // product id of the device, normally used as hex on the system
  uint32_t product_id;
} DeviceID;

// Provides information about a connected display.
typedef struct DisplayInfo {
  // one of rgb, ir, depth or points depending on the streams supported
  // by the camera
  char *hardware_id;
  // Indicates if this display is also the primary display
  bool primary_display;
  // The current position and resolution settings for this display.
  Rectangle coordinates;
} DisplayInfo;

// DeviceInfo contains a collection of information about the device.
// It is similar to the DeviceID, but contains additional fields.
typedef struct DeviceInfo {
  char *fw_version;
  // The alias of a device, meaning there is no @ sign in name
  // even if the index of the device is greater than zero
  char *name;
  // serial number of device
  char *serial;
  // Zero-based index of an array of devices of the same class.
  uint32_t index;
  // vendor id of the device, normally used as hex on the system
  uint32_t vendor_id;
  // product id of the device, normally used as hex on the system
  uint32_t product_id;
} DeviceInfo;

typedef struct SupportedDevice {
  char *name;
} SupportedDevice;

typedef enum class DeviceNotification {
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
} DeviceNotification;

// Describes a base device abstraction that contains functionality that is
// available on all devices that SoHal supports.
//
// Note: All examples use "deviceName" in the method because these requests
// have the name of the specific device.For example, to open the projector,
// a client should send a "projector.open" method.
// The table below lists the name used in methods and notifications for each
// device. The 'deviceName' shown in the examples should be replaced by the
// name from the second column.
//    Device                        DeviceName
//    -----------------------------------------
//    Capture Stage                 capturestage
//    Depth Camera                  depthcamera
//    Desk Lamp                     desklamp
//    High Resolution Camera        hirescamera
//    Projector                     projector
//    SButtons                      sbuttons
//    Touchmat                      touchmat
//    UVCCamera                     uvccamera
//
// Access to all devices is provided without any arbitration;
//
// When the host system is entering a sleep or hibernate state, shutting down,
// or restarting, SoHal will perform specific energy saving actions and close
// all devices.The actions that are performed for each device are listed in the
// table below.Note that individual notifications are not sent for each action,
// but each device will send an on_suspend notification after the suspend
// actions have been performed, and then an on_resume notification when the
// system wakes up again.
//
//    Device                        Suspend Actions
//    -----------------------------------------------------------------------
//    Capture Stage                 Turns off all LEDs Closes the device
//    Depth Camera                  Closes the device
//    Desk Lamp                     Turns the desk lamp off Closes the device
//    High Resolution Camera        Closes the device
//    Projector                     Turns the projector off Closes the device
//    SButtons                      Turns off all LEDs Closes the device
//    Touchmat                      Disables touch Closes the device
//    UVCCamera                     Closes the device
class DLLEXPORT HippoDevice {
 public:
  HippoDevice(const char *dev, const char *address, uint32_t port,
              HippoFacility facility, uint32_t device_index);
  virtual ~HippoDevice(void);

  // Close the device. If the calling client is the only client that has the
  // device open, this call will close the existing handle to the device
  // endpoint and trigger an on_close notification.  If the calling client
  //  previously had the device open (so this call has changed the open_count
  // value), this will also trigger an on_open_count notification.
  virtual uint64_t close();

  // same as close()
  // This method returns an integer indicating the number of clients that have
  //  the device open (equal to the value returned by open_count() ).
  virtual uint64_t close(uint32_t *open_count);


  // The factory_default method resets the device to default settings and
  // triggers an on_factory_default notification. What "factory_default"
  // means is device dependent and is detailed in the table below.
  //
  //  Device                        Default Settings
  //  ------------------------------------------------------------------------
  //  Capture Stage                 Turns off all LEDs Sets blink rate time_on
  //                                and time_off to 500 ms Moves the capture
  //                                stage to the home position
  //
  //  Depth Camera                  Disables all camera streams Turns off the
  //                                IR flood light(Sprout Pro G2 and HP Z 3D
  //                                Camera only) Enables the laser(Sprout Pro
  //                                G2 and HP Z 3D Camera only) Desk Lamp Turns
  //                                the desk lamp off
  //
  //  High Resolution Camera Sprout
  //                                Sets exposure, gain, and white_balance to
  //                                'auto' Disables all Image Signal Processing
  //                                functionality(flip_frame, gamma_correction,
  //                                lens_shading, lens_color_shading, and
  //                                mirror_frame)
  //
  //  High Resolution Camera HP Z 3D Camera
  //                                Sets exposure, gain, and white_balance to
  //                                'auto' Enables flip_frame,
  //                                gamma_correction, and lens_shading.
  //                                Disables mirror_frame Resets the Keystone
  //                                flash_fit_to_mat, flash_max_fov, and ram
  //                                tables to the default values
  //
  //  Projector                     Sets brightness to 100 % Sets the keystone
  //                                to the factory setting Sets white_point to
  //                                the "d65" Illuminant(Sprout Pro G2 only)
  //                                Turns the projector off
  //
  //  SButtons                      Turns off all LEDs Sets hold_count to 121
  //                                Sets led_on_off_rate to 2 Sets
  //                                led_pulse_rate to 4
  //
  //  Touchmat                      Disables touch and active_pen input
  //                                Disables the active area and sets the
  //                                start = (0, 0) and end = (15360, 8640)
  //                                (Sprout Pro G2 only)
  //                                Sets active_pen_range to ten_mm
  //                                (Sprout Pro G2 only)
  //                                Disables device_palm_rejection
  //                                (Sprout Pro G2 only)
  //
  //  UVC Camera                    Currently does not modify any settings,
  //                                but this may change in a future release.
  virtual uint64_t factory_default();

  // Info returns basic information about the device that uniquely identifies
  // it to SoHal and on the system. Every device has the info method. SoHal
  // also supports a generic system.devices function that returns an array
  // of DeviceInfo items, with one for each connected device.
  //
  // Note: that the the info function allocates memory internally.
  // The user must call free_device_info() passing in the same
  // DeviceInfo pointer when done with the memory
  // in order to avoid a memory leak
  virtual uint64_t info(DeviceInfo *get);

  // this function must be called with the same pointer that gets passed
  // into the info() fuction above
  // failing call this function after getting the device info will result
  // in a memory leak.
  virtual void free_device_info(DeviceInfo *dev_info_to_free);

  // This method returns True if the device is currently
  // connected and False if it is not
  virtual uint64_t is_device_connected(bool *get);

  // Open the device. If no other clients have the device open, this
  // creates a handle to the device endpoint. If open is called, and
  // close is never called, the device will remain open until the caller
  // socket is disconnected (assuming other clients have not opened the same
  // device). As many clients can be using the same device and open and close
  // are expensive methods, SoHal uses a reference counter of clients that have
  // the device open. The device will be open when the first call to
  // deviceName.open arrives and will be closed when the last client closes it
  // or disconnects.
  // This method may trigger an on_open and / or an on_open_count notification.
  // SoHal will send an on_open notification if the device was in a closed
  // state and has been opened.SoHal will send an on_open_count notification
  // if the calling client did not already have the device open(so this call
  // has changed the open_count value).
  virtual uint64_t open();

  // same as open() fuction
  // This method will return an integer indicating the number of clients that
  // have the device open(equal to the value returned by open_count).
  virtual uint64_t open(uint32_t *open_count);

  // Returns the number of clients that currently have this device open.
  // If the number is greater than 0, the device is already open.
  virtual uint64_t open_count(uint32_t *open_count);


  // This method returns a list of TemperatureInfo objects that provide
  // information on the system temperatures. Please note that there are
  // a few differences between calling this system.temperatures method
  // and calling the deviceName.temperatures method which is available
  // for each individual device:
  //    If a device is not already open
  //      system.temperatures will automatically open the device, query
  //      the temperatures, and then close the device.
  //      deviceName.temperatures will return an error such as Device
  //      is not open
  //    If there is an error communicating with one or more sensors or devices
  //      system.temperatures will ignore errors and exclude information for
  //      sensors or devices it is not able to retrieve information from. The
  //      return value will include the temperature information for all other
  //      sensors.
  //      deviceName.temperatures will return an error if it is unable to
  //      communicate with the device or retrieve information from any of the
  //      device's sensors. If one sensor fails, the client will not receive
  //      information for other sensors.
  //
  // Note: internally this function allocates and fills a TemperatureInfo array
  // using malloc. The caller is responsible for ensuring that
  // the free_temperatures function gets called in order to avoid a memory leak
  virtual uint64_t temperatures(TemperatureInfo **get, uint64_t *num_temps);

  // the free_temperatures function deallocates the memory that gets
  // allocated in the temperatures() failing call this function after
  // getting the temperature info will result in a memory leak.
  // note this function is safe to call even if temperatures()
  // returns an error
  virtual void free_temperatures(TemperatureInfo *temperature_info_to_free);

  // unsubscribe to notifications
  uint64_t unsubscribe();
  uint64_t unsubscribe(uint32_t *get);

 protected:
  virtual uint64_t Connect();
  void Disconnect();

  bool IsConnected();
  bool IsConnectedWs();
  bool IsConnectedWsSig();
  uint64_t EnsureConnected();

  virtual void ProcessSignal(char *method, void *params);

  uint64_t SendRawMsg(const char *method, void *ret_obj);
  uint64_t SendRawMsg(const char *method, const void *param, void *ret_obj);
  uint64_t SendRawMsg(const char *method, const void *param,
                      unsigned int timeout, void *ret_obj);

  uint64_t GenerateJsonRpc(const char *devName, const char *method,
                           const void *param, unsigned char **jsonrpc);
  uint64_t GenerateJsonRpc(const char *method, const void *param,
                           unsigned char **jsonrpc);
  uint64_t GenerateJsonRpcResponse(const void *id, const void *result,
                                   char **jsonrpc);
  uint64_t GenerateJsonRpcError(const void *id, uint64_t err, char **jsonrpc);

  uint64_t GetRawResultOrError(void *obj);

  void WaitForSignalRaw(void);
  virtual bool HasRegisteredCallback();

  uint64_t deviceInfo_json2c(void *obj, DeviceInfo *info);

  uint64_t bool_get(const char *fname, bool *get);
  uint64_t bool_set_get(const char *fname, bool set, bool *get);
  uint64_t uint16_get(const char *fname, uint16_t *get);
  uint64_t uint16_set_get(const char *fname, uint16_t set, uint16_t *get);
  uint64_t uint32_get(const char *fname, uint32_t *get);
  uint64_t uint32_set_get(const char *fname, uint32_t set, uint32_t *get);
  uint64_t float_get(const char *fname, float *get);
  uint64_t float_set_get(const char *fname, float set, float *get);
  int32_t str_to_idx(const char **names, const char *str,
                     uint32_t first, uint32_t last);

  virtual uint64_t tempInfo_json2c(const void *obj,
                           hippo::TemperatureInfo *temperature_info);
  virtual uint64_t tempInfos_json2c(const void *obj,
                            hippo::TemperatureInfo **temperature_info,
                            uint64_t *num_temps);

  uint64_t subscribe_raw(void *data, uint32_t *get);
  uint64_t subscribe_raw_p(uint32_t *get);
  void SendSignal(const char *method, void *param);

  uint32_t device_index_;
  HippoWS *ws_, *wsSig_;
  void *module_;
  uint32_t id_;
  char devName_[MAX_DEV_LEN];
  char host_[MAX_ADDR_LEN];
  uint32_t port_;
  HippoFacility facility_;
  std::thread *signal_th_;
  void *callback_data_;
};

}   // namespace hippo

#endif   // INCLUDE_HIPPO_DEVICE_H_
