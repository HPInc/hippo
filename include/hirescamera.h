
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#ifndef INCLUDE_HIRESCAMERA_H_
#define INCLUDE_HIRESCAMERA_H_

#include "../include/hippo_camera.h"
#include "../include/common_types.h"

#if COMPILING_DLL
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif

namespace hippo {

// Enumerates the possible modes of the camera.
typedef enum class CameraMode {
  MODE_4416x3312,  // Full camera resolution
  MODE_2208x1656,  // 2-binning camera mode (1/4th resolution)
  MODE_1104x828,   // 4-binning camera mode (1/16th resolution)
} CameraMode;

typedef struct Rgb {
  uint32_t red;
  uint32_t green;
  uint32_t blue;
} Rgb;

typedef struct CameraConfig {
  uint32_t exposure;
  uint16_t fps;
  uint16_t gain;
  CameraMode mode;
  Rgb white_balance;
} CameraConfig;

typedef struct Strobe {
  uint32_t frames;    // Turn the flash on for this many camera frames
  uint32_t gain;      // Temporary gain value to use while flashing
  uint32_t exposure;  // Temporary exposure value to use while flashing
} Strobe;

typedef enum class AutoOrFixedType {
  TYPE_NONE,
  TYPE_AUTO,
  TYPE_UINT,
  TYPE_RGB,
  TYPE_MODE,
} AutoOrFixedType;

typedef struct AutoOrFixed {
  AutoOrFixedType type;
  union {
    uint32_t value;
    Rgb rgb;
    CameraMode mode;
  } value;
} AutoOrFixed;

typedef struct CameraSettings {
  AutoOrFixed exposure;
  AutoOrFixed gain;
  AutoOrFixed white_balance;
  bool flip_frame;
  bool gamma_correction;
  bool lens_color_shading;
  bool lens_shading;
  bool mirror_frame;
} CameraSettings;

// A CameraQuadrilateral includes four points which should define a convex
// quadrilateral: all interior angles are less than 180 degrees and the two
// diagonals both lie inside the quadrilateral. In other words, x in top_left
// should always be to the left of x in top_right and y in top_right should
// always be above of y in bottom_right. Equivalent restrictions apply for
// bottom_left.x with bottom_right.x and for top_left.y with bottom_left.y.
typedef struct CameraQuadrilateral {
  Point top_left;
  Point top_right;
  Point bottom_left;
  Point bottom_right;
} CameraQuadrilateral;

typedef struct CameraResolution {
  uint32_t width;   // The width of the image in pixels
  uint32_t height;  // The height of the image in pixels
  uint32_t fps;     // The camera's frame rate in frames per second
} CameraResolution;

// The CameraKeystone type defines the keystone values used for a given
// resolution. The camera's keystone values are defined by setting offset
// values from the default non-keystoned image (where all x and y values
// are set to 0). A positive x or y offset will move a point to the right
// or down, while negative values move a point to the left or up. A point
// can not be moved outside of the maximum image area (beyond the
// non-keystoned image). This means that setting the top_left x and y
// offsets to negative values or setting the bottom_right x and y offsets
// to positive values is not allowed as the pixels will fall outside of the
// image boundaries. Equivalent restrictions apply to bottom_left and
// top_right. Note: The valid range for the x and y values in each point are
// such that the points never fall outside of the edges of the image for the
// corresponding effective size (or parent resolution) of each frame output
// size. Please see table below for relationship between effective size and
// output size:

//  Sensor Size         Effective Size    Output Size          FPS
//  4416 x 3312         4352 x 3264       4352 x 3264            6
//  4416 x 3312         4352 x 3264       4352 x 2896            6
//  4416 x 3312         4352 x 3264       3840 x 2160           10
//  4416 x 3312         4352 x 3264       2176 x 1632           15
//  4416 x 3312         4352 x 3264       2176 x 1448           15
//  4416 x 3312         4352 x 3264       1920 x 1080           15
//  4416 x 3312         4352 x 3264       640 x 480             15
//  4416 x 3312         4224 x 3168       4224 x 3168            6
//  4416 x 3312         4224 x 3168       1056 x 792            15
//  4416 x 3312         4224 x 3168       1056 x 704            15
//  4416 x 3312         4224 x 3168       960 x 540             15
//  2208 x 1656         2176 x 1632       2176 x 1632           25
//  2208 x 1656         2176 x 1632       2176 x 1448           25
//  2208 x 1656         2176 x 1632       1920 x 1080           30
//  2208 x 1656         2112 x 1584       2112 x 1584           25
//  2208 x 1656         2112 x 1584       1056 x 792            30
//  2208 x 1656         2112 x 1584       1056 x 704            30
//  2208 x 1656         2112 x 1584       960 x 540             30
//  1104 x 828          1056 x 792        1056 x 792            60
//  1104 x 828          1056 x 792        1056 x 704            60
//  1104 x 828          1056 x 792        960 x 540             60
//  1104 x 828          1056 x 792        640 x 480             60
//  1104 x 828          1056 x 594        416 x 234             60

// Note: When setting the keystone value it is not necessary to include all
// points.The values will not be modified for any points that are not
// included.However, any point that is provided needs to include both the
// 'x' and 'y' values.
// Note : The keystone values are based on the default streaming conditions
// (flip = true, mirror = false for HP Z 3D Camera hirescamera).
typedef struct CameraKeystone {
  //  Indicates if this keystone value is enabled or disabled. If it is set to
  // true, the provided values will be used. If it is set to false, the default
  // non-keystoned image will be used. This provides an easy way to switch
  // between the full image and the keystone corrected image.
  bool enabled;
  // The keystone offset values.
  CameraQuadrilateral value;
} CameraKeystone;

// Enumerates the different keystone tables.
typedef enum class CameraKeystoneTable {
  // The values in the RAM table are used when the camera starts streaming
  // at a given resolution. Values written directly to the RAM table using
  // the keystone_table_entries method are not persistent across power cycles.
  // However, they will be used until the camera loses power or they are
  // overwritten (for an individual resolution using keystone_table_entries
  // or as the entire table using keystone_table). When the camera powers up,
  // the RAM table will be initialized with the values from the last table set
  // using the keystone_table method.
  RAM,
  // The default keystone table. This table is programmed into the camera's
  // firmware and can be used to return the keystone values to the default
  // state. SoHAL clients can not store values in this table.
  DEFAULT,
  // This table is stored in flash memory. Values stored to this table will
  // persist across power cycles.
  FLASH_MAX_FOV,
  // This table is stored in flash memory. Values stored to this table will
  // persist across power cycles.
  FLASH_FIT_TO_MAT,
} CameraKeystoneTable;

// The CameraKeystoneTableEntry type defines the keystone values used for a
// given resolution. Please see the description under the CameraKeystone
// type for more information on the keystone values. The resolution table
// defined under that type and also all of the same conditions and restrictions
// on corner offset values apply here as well.
typedef struct CameraKeystoneTableEntry {
  // The resolution this keystone will be applied for.
  CameraResolution resolution;
  // Indicates if this keystone value is enabled or disabled. If it is set to
  // true, the provided values will be used. If it is set to false, the
  // non-keystoned image will be used. This provides an easy way to switch
  // between the full image and the keystone corrected image.
  bool enabled;
  // The keystone values to use. Note that these values are offset values
  // defined as pixels in the hirescamera's effective size (or parent
  // resolution)
  CameraQuadrilateral value;
} CameraKeystoneTableEntry;

// The CameraKeystoneTableEntries type defines the
// return type for the keystone_table_entries function.
typedef struct CameraKeystoneTableEntries {
  // The keystone table this type is refering to.
  CameraKeystoneTable type;
  // A pointer to the list of entries with the values set
  // for the given table.
  CameraKeystoneTableEntry *entries;
} CameraKeystoneTableEntries;

typedef enum class CameraLedStateEnum {
  // The LEDs are not emitting light
  off,
  // Two of the LEDs are illuminated
  low,
  // All of the LEDs are illuminated. This state is only supported when
  // capturing a still image. It is not a supported value for the streaming
  // CameraLedState.
  high,
  // LEDs will be turned on only if deemed necessary. This state is only
  // supported when capturing a still image. It is not a supported value
  // for the streaming CameraLedState.
  // SoHAL's auto is a c++ restricted word - using "automatic" instead
  automatic,
} CameraLedStateEnum;

// Provides information on the state settings for the HiResCamera LEDs.
typedef struct CameraLedState {
  // The LED state to use when the camera is capturing a still image.
  // All values in the CameraLedStateEnum are supported.
  CameraLedStateEnum capture;
  // The LED state to use when the camera is streaming video
  // (preview mode). Only the off and low states are supported for this
  // mode. If this is set to the high or auto state, SoHal will return
  // an error.
  CameraLedStateEnum streaming;
} CameraLedState;

// Enumerates the possible status values for various aspects of the
// camera hardware and firmware functionality.
typedef enum class CameraStatus {
  ok,      // No errors or warnings have been detected and this item is
           // behaving as expected.
  busy,    // This item is currently busy.
  error,  // This item has reported an error.
} CameraStatus;

typedef struct CameraDeviceStatus {
  // The status of the most recent generic data extension get request.
  CameraStatus generic_get;
  // The status of the most recent generic data extension set request.
  CameraStatus generic_set;
  // The status of the ISP (Image Signal Processor) colorbar extension
  CameraStatus isp_colorbar;
  //  The status of the ISP function extension
  CameraStatus isp_function;
  // The status of the ISP boot up. If this item is set to "busy",
  // the device is currently held in reset.
  CameraStatus isp_fw_boot;
  //  The status of the ISP reset request extension
  CameraStatus isp_reset;
  // The status of the ISP state restore after boot up
  CameraStatus isp_restore;
  //  The status of the ISP video stream extension
  CameraStatus isp_videostream;
  // The status of loading the lens correction (LENC) calibration
  // table on boot up
  CameraStatus load_lenc_calibration;
  // The status of loading the white balance calibration table on boot up
  CameraStatus load_white_balance_calibration;
  // The status of the most recent special data extensionget request
  CameraStatus special_get;
  // The status of the most recent special data extension set request
  CameraStatus special_set;
  // The status of the thermal sensors. If this item is set to "error",
  // at least one of the thermal sensors is not functioning properly.
  CameraStatus thermal_sensor_error;
  // If this item is set to "error", that indicates the maximum temperature
  // threshold was reached and the camera is in a thermal shutdown state.
  // The camera will not be functional until it has been disconnected from
  // the USB port, allowed to cool down, and then reconnected.Simply cooling
  // the device will not restore functionality after a thermal shutdown - a
  // power reset is also required.
  CameraStatus thermal_shutdown;
} CameraDeviceStatus;

// This indicates the power line frequency control is enabled with
// the specified value. The frequency is expressed in hertz and the
// only supported values are 50 Hz and 60 Hz.
typedef enum class PowerLineFrequency {
  disabled,
  hz_50,
  hz_60,
} PowerLineFrequency;

typedef enum class HiResCameraNotification {
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

  // hirescamera specific notifications below

  // This notification occurs when a client sends a command to set the
  // brightness and the command executes successfully. It includes an
  // unsigned int parameter with the current brightness value.
  on_brightness,
  // This notification occurs when a client sends a command to set the
  // contrast and the command executes successfully. It includes an
  // unsigned int parameter with the current contrast value.
  on_contrast,
  // This notification occurs whenever the user triggers a change on the
  // camera's exposure. The parameter could be "auto" if auto_exposure has been
  // turned on or the actual exposure value as an unsigned int.
  on_exposure,
  // This notification occurs when a client sends a command to set the
  // flip_frame mode and the command executes successfully. It includes a bool
  // parameter with the current flip_frame value.
  on_flip_frame,
  // This notification occurs whenever a client triggers a change on the
  // camera's gain. The parameter could be "auto" if auto_gain has been turned
  // on or the actual gain value as an unsigned int.
  on_gain,
  // This notification occurs when a client sends a command to set the
  // gamma_correction mode and the command executes successfully. It includes a
  // bool parameter with the current gamma_correction value.
  on_gamma_correction,
  // This notification occurs when a client updates the keystone. It includes a
  // CameraKeystone parameter with the updated value.
  on_keystone,
  // This notification occurs when a client updates the keystone_table. It
  // includes a CameraKeystoneTable parameter with the updated value.
  on_keystone_table,
  // This notification occurs when a client saves an entry in the keystone
  // table using the keystone_table_entries method.It includes a
  // CameraKeystoneTableEntries parameter with the updated value.
  // Note: the memory allocated for the CameraKeystoneTableEntries
  // parameter will be atomatically freed when program control returns from
  // the callback function - calling free_keystone_table_entries with the
  // parameter is not necessary
  on_keystone_table_entries,
  // This notification occurs when a client sends a command to set the
  // led_state and the command executes successfully. It includes a
  // CameraLedState parameter with the current led_state value.
  on_led_state,
  // This notification occurs when a client sends a command to set the
  // lens_color_shading mode and the command executes successfully. It
  // includes a bool parameter with the current lens_color_shading value.
  on_lens_color_shading,
  // This notification occurs when a client sends a command to set the
  // lens_shading mode and the command executes successfully. It includes
  // a bool parameter with the current lens_shading value.
  on_lens_shading,
  // This notification occurs when a client sends a command to set the
  // mirror_frame mode and the command executes successfully. It includes
  // a bool parameter with the current mirror_frame value.
  on_mirror_frame,
  // This notification occurs when a client sends a command to set the
  // power_line_frequency and the command executes successfully. It includes
  // a PowerLineFrequency parameter with the current value.
  on_power_line_frequency,
  // This notification occurs when a client sends a command to reset the
  // hirescamera, and the command executes successfully.
  on_reset,
  // This notification occurs when a client sends a command to set the
  // saturation and the command executes successfully. It includes an
  // unsigned int parameter with the current saturation value.
  on_saturation,
  // This notification occurs when a client sends a command to set the
  // sharpness and the command executes successfully. It includes an
  // unsigned int parameter with the current sharpness value.
  on_sharpness,
  // This notification occurs when a client sends a strobe command and the
  // command executes successfully. It includes a Strobe parameter with the
  // settings that were used.
  on_strobe,
  // This notification occurs whenever a client triggers a change on the
  // camera's white_balance. The parameter could be "auto" if
  // auto_white_balance has been turned on or the actual white_balance
  // value as an Rgb.
  on_white_balance,
  // This notification occurs whenever a client successfully sets the
  // camera's white_balance_temperature.
  on_white_balance_temperature,
} HiResCameraNotification;

// The parameters that are passed when a hirescamera notification is triggered
typedef struct HiResCameraNotificationParam {
  // the type of notification that is being passed
  HiResCameraNotification type;
  union {
    // parameter passed with the on_open_count notification
    uint32_t on_open_count;
    // parameter passed with the on_brightness notification
    uint16_t on_brightness;
    // parameter passed with the on_contrast notification
    uint16_t on_contrast;
    // parameter passed with the on_exposure notification
    AutoOrFixed on_exposure;
    // parameter passed with the on_flip_frame notification
    bool on_flip_frame;
    // parameter passed with the on_gain notification
    AutoOrFixed on_gain;
    // parameter passed with the on_gamma_correction notification
    bool on_gamma_correction;
    // parameter passed with the on_lens_color_shading notification
    bool on_lens_color_shading;
    // parameter passed with the on_lens_shading notification
    bool on_lens_shading;
    // parameter passed with the on_led_state notification
    CameraLedState on_led_state;
    // parameter passed with the on_mirror_frame notification
    bool on_mirror_frame;
    // parameter passed with the on_power_line_frequency notification
    PowerLineFrequency on_power_line_frequency;
    // parameter passed with the on_strobe notification
    Strobe on_strobe;
    // parameter passed with the on_white_balance notification
    AutoOrFixed on_white_balance;
    // parameter passed with the on_keystone notification
    CameraKeystone on_keystone;
    // parameter passed with the on_keystone_table notification
    CameraKeystoneTable on_keystone_table;
    // parameter passed with the on_keystone_table_entries notification
    CameraKeystoneTableEntries on_keystone_table_entries;
    // parameter passed with the on_saturation notification
    uint16_t on_saturation;
    // parameter passed with the on_sharpness notification
    uint16_t on_sharpness;
    // parameter passed with the on_white_balance_temperature notification
    uint16_t on_white_balance_temperature;
  };
  uint32_t num_keystone_table_entries;
} HiResCameraNotificationParam;


class DLLEXPORT HiResCamera : public HippoCamera {
 public:
  HiResCamera();
  explicit HiResCamera(uint32_t device_index);
  HiResCamera(const char *address, uint32_t port);
  HiResCamera(const char *address, uint32_t port, uint32_t device_index);
  virtual ~HiResCamera(void);

  // Calling this method with a get parameters acts as a Get request
  // and returns a bool indicating if auto_exposure control mode is active
  uint64_t auto_exposure(bool *get);

  // Calling this method with a set parameter acts as a Set request and sets
  // the auto_exposure control mode on the camera
  // Calling it with a false parameter will disable auto exposure control
  // mode and will trigger an on_exposure notification with the exposure's
  // last active value.
  uint64_t auto_exposure(bool set);

  // Calling this method with a set parameter acts as a Set request and sets
  // the auto_exposure control mode on the camera it then returns a bool
  // in the get variable indicating if auto_exposure control mode is active
  // Calling it with a set=false parameter will disable auto exposure control
  // mode and will trigger an on_exposure notification with the exposure's
  // last active value.
  uint64_t auto_exposure(bool set, bool* get);

  // Calling this method with a get parameters acts as a Get request
  // and returns a bool indicating if auto_gain control mode is active
  uint64_t auto_gain(bool *get);

  // Calling this method with a set parameter acts as a Set request and sets
  // the auto_gain control mode on the camera
  // Calling it with a false parameter will disable auto gain control
  // mode and will trigger an on_gain notification with the gain's
  // last active value.
  uint64_t auto_gain(bool set);

  // Calling this method with a set parameter acts as a Set request and sets
  // the auto_gain control mode on the camera it then returns a bool
  // in the get variable indicating if auto_gain control mode is active
  // Calling it with a set=false parameter will disable auto gain control
  // mode and will trigger an on_gain notification with the gain's
  // last active value.
  uint64_t auto_gain(bool set, bool* get);

  // Calling this method with a get parameters acts as a Get request and
  // returns a bool indicating if auto_white_balance control mode is active.
  uint64_t auto_white_balance(bool *get);
  // Calling this method with a set parameter acts as a Set request. Calling
  // it with set=true will enable auto white balance control mode and will
  // trigger an on_white_balance notification with an "auto" parameter.
  // Calling it with set=false will disable auto white balance control mode
  // and will trigger an on_white_balance notification with the white balance's
  // last active value.
  uint64_t auto_white_balance(bool set);

  // Calling this method with a set parameter acts as a Set request. Calling
  // it with set=true will enable auto white balance control mode and will
  // trigger an on_white_balance notification with an "auto" parameter.
  // Calling it with set=false will disable auto white balance control mode
  // and will trigger an on_white_balance notification with the white balance's
  // last active value. After setting the auto white balance, this function
  // will then return the current state of the auto white balance in the
  // get parameter
  uint64_t auto_white_balance(bool set, bool* get);

  // This method allows clients to get or set the camera's current brightness
  // value. The valid range for brightness is: 16 <= brightness <= 255 Calling
  // this method with a get parameter acts as a Get request and returns an
  // unsigned int indicating the current setting.
  uint64_t brightness(uint16_t *get);

  // Calling this method with a set parameter will set the current brightness
  // value to the provided parameter. On success, the response will include an
  // unsigned int confirming the value that was set and SoHal will send an
  // on_brightness notification to all subscribed clients.
  uint64_t brightness(uint16_t set);

  // Calling this method with a set and get parameter will set the current
  // brightness value to the provided parameter.
  // The valid range for brightness is: 16 <= brightness <= 255
  // On success, the response will include an unsigned int confirming the value
  // that was set and SoHal will send an on_brightness notification to all
  // subscribed clients. After seting the brightness, this method will return
  // the current value in the get parameter
  uint64_t brightness(uint16_t set, uint16_t *get);

  // This method returns the 'index' of the hirescamera within the operating
  // system video input devices.
  // Note: This method does not require the camera to be open.
  uint64_t camera_index(uint32_t *get);

  // This camera_settings method allows clients to set the values for
  // multiple hirescamera settings in one call. Calling this method with a
  // set parameter acts as a Set request and will set the camera to the values
  // passed as parameters. On success will return a CameraSettings object and
  // send an individual notification for each of the values on the
  // CameraSettings object.
  // Note: There are a few differences to keep in mind when using this method
  // for HP Z 3D Camera hirescamera devices. The hirescamera in the HP Z 3D
  // Camera does not support lens_color_shading. The lens_color_shading field
  // should either be left out of the CameraSettings parameter sent to SoHal
  // or have the value set to false. If lens_color_shading is included and set
  // to true, SoHal will return an error. Additionally, the exposure, gain and
  // white_balance fields may not be set to a CameraMode parameter for HP Z 3D
  // Camera hirescamera devices.
  uint64_t camera_settings(const CameraSettings &set);

  // This camera_settings method allows clients to get or set the values for
  // multiple hirescamera settings in one call. Calling this method with a
  // get parameter acts as a Get request and returns a CameraSettings object
  // with the active camera settings.
  uint64_t camera_settings(CameraSettings *get);

  // This camera_settings method allows clients to set the values for
  // multiple hirescamera settings in one call. Calling this method with a
  // set parameter acts as a Set request and will set the camera to the values
  // passed as parameters. On success will return a CameraSettings object and
  // send an individual notification for each of the values on the
  // CameraSettings object. Once set the current settings will be returned in
  // the get parameter
  // Note: There are a few differences to keep in mind when using this method
  // for HP Z 3D Camera hirescamera devices. The hirescamera in the HP Z 3D
  // Camera does not support lens_color_shading. The lens_color_shading field
  // should either be left out of the CameraSettings parameter sent to SoHal
  // or have the value set to false. If lens_color_shading is included and set
  // to true, SoHal will return an error. Additionally, the exposure, gain and
  // white_balance fields may not be set to a CameraMode parameter for HP Z 3D
  // Camera hirescamera devices.
  uint64_t camera_settings(const CameraSettings &set, CameraSettings *get);

  // This method allows clients to get or set the camera's current contrast
  // value. The valid range for contrast is: 0 <= contrast <= 10 Calling this
  // method with a get parameter acts as a Get request and returns an unsigned
  // int indicating the current setting.
  uint64_t contrast(uint16_t *get);

  // Calling this method with a set parameter will set the current contrast
  // value to the provided parameter. On success, the response will include
  // an unsigned int confirming the value that was set and SoHal will send an
  // on_contrast notification to all subscribed clients.
  uint64_t contrast(uint16_t set);

  // Calling this method with a set parameter will set the current contrast
  // value to the provided parameter. On success, the response will include
  // an unsigned int confirming the value that was set and SoHal will send an
  // on_contrast notification to all subscribed clients. After setting the
  // contrast, the current contrast will be returned in the get parameter
  uint64_t contrast(uint16_t set, uint16_t *get);

  // This method accepts a CameraMode parameter and returns a CameraConfig
  // value with the values set at the factory for exposure, gain and
  // white_balance
  uint64_t default_config(CameraMode mode, CameraConfig *get);

  // This method returns a CameraDeviceStatus object with information on
  // the status of various aspects of the camera's hardware and firmware
  // functionality. It can be used to determine if the camera is currently
  // in a thermal shutdown state and needs to be disconnected and allowed
  // to cool down in order to restore functionality.
  uint64_t device_status(CameraDeviceStatus *get);

  // Calling this method with a get parameter acts as a Get request and returns
  // the current exposure setting. Please note that if the camera's
  // auto_exposure control mode is on, consecutive calls to this method may
  // return different values.
  // Note: Sprout, Sprout Pro, and Sprout Pro G2 hirescameras will only
  // retrieve the exposure setting if the camera is currently streaming.
  // If the camera is not streaming, this method will return a default value of
  // 65535.
  uint64_t exposure(uint16_t *get);

  // Calling this method with a set parameter will automatically set the camera
  // in manual exposure control mode and set the current exposure value to the
  // provided parameter. On success SoHal will send an on_exposure
  // notification to all subscribed clients.
  // Note: Sprout, Sprout Pro, and Sprout Pro G2 hirescameras will only apply
  // the new exposure if the camera is currently streaming.
  // The valid ranges for exposure for different hirescameras are
  // listed below:
  //   Sprout, Sprout Pro,
  //   and Sprout Pro G2 hirescameras:                   1 <= exposure <= 3385
  //   HP Z 3D Camera hirescameras :                     1 <= exposure <= 20000
  uint64_t exposure(uint16_t set);

  // Calling this method with a set parameter will automatically set the camera
  // in manual exposure control mode and set the current exposure value to the
  // provided parameter. On success, the response will include an unsigned int
  // in the set parameter confirming the value that was set and SoHal will send
  // an on_exposure notification to all subscribed clients.
  // Note: Sprout, Sprout Pro, and Sprout Pro G2 hirescameras will only apply
  // the new exposure if the camera is currently streaming.
  // The valid ranges for exposure for different hirescameras are
  // listed below:
  //   Sprout, Sprout Pro,
  //   and Sprout Pro G2 hirescameras:                   1 <= exposure <= 3385
  //   HP Z 3D Camera hirescameras :                     1 <= exposure <= 20000
  uint64_t exposure(uint16_t set, uint16_t *get);

  // Calling this method with a get parameter acts as a Get request and returns
  // a bool indicating the current flip_frame value.
  uint64_t flip_frame(bool *get);

  // Calling this method with a set parameter turns the flip_frame mode on or
  // off. On success SoHal will send an on_flip_frame notification to all
  // subscribed clients.
  uint64_t flip_frame(bool set);

  // Calling this method with a set parameter turns the flip_frame mode on or
  // off. On success, the response will include a bool in the get parameter
  // confirming the value that was set and SoHal will send an on_flip_frame
  // notification to all subscribed clients.
  uint64_t flip_frame(bool set, bool *get);

  // Calling this method with a get parameter acts as a Get request and
  // returns an |unsigned int| indicating the current gain setting. Please note
  // that if the camera's auto_gain control mode is on, consecutive calls to
  // this method may return different values.
  // Note: Sprout, Sprout Pro, and Sprout Pro G2 hirescameras will only
  // retrieve the gain setting if the camera is currently streaming.If the
  // camera is not streaming, it will return a default value of 255.
  uint64_t gain(uint16_t *get);

  // Calling this method with a set parameter will automatically set the camera
  // in manual gain control mode and set the current gain value to the provided
  // parameter. On success SoHal will send an on_gain notification to all
  // subscribed clients.
  // The valid ranges for gain for different hirescameras are listed below:
  //    Sprout, Sprout Pro, and Sprout Pro G2 hirescameras: 0 <= gain <= 127
  //    HP Z 3D Camera hirescameras:                        0 <= gain <= 236
  uint64_t gain(uint16_t set);

  // Calling this method with a set parameter will automatically set the camera
  // in manual gain control mode and set the current gain value to the provided
  // parameter. On success, the response will include an unsigned int in the
  // get parameter confirming the value that was set and SoHal will send an
  // on_gain notification to all subscribed clients.
  // The valid ranges for gain for different hirescameras are listed below:
  //    Sprout, Sprout Pro, and Sprout Pro G2 hirescameras: 0 <= gain <= 127
  //    HP Z 3D Camera hirescameras:                        0 <= gain <= 236
  uint64_t gain(uint16_t set, uint16_t *get);

  // Calling this method with a get parameter acts as a Get request and returns
  // a bool indicating the current value.
  uint64_t gamma_correction(bool *get);

  // Calling this method with a set parameter turns the gamma_correction mode
  // on or off. On success SoHal will send an on_gamma_correction notification
  // to all subscribed clients.
  uint64_t gamma_correction(bool set);

  // Calling this method with a set parameter turns the gamma_correction mode
  // on or off. On success, the response will include a bool in the get
  // parameter confirming the value that was set and SoHal will send an
  // on_gamma_correction notification to all subscribed clients.
  uint64_t gamma_correction(bool set, bool *get);

  // The keystone methods get or set the camera's keystone values using the ISP
  // (Image Signal Processor) registers. It can only be used to query or modify
  // the keystone values that are being used while the camera is currently
  // streaming frames. It does not modify the values in the keystone tables and
  // values written using this method will not be persistent. That is, the next
  // time the camera starts streaming frames, it will use the values from the
  // RAM keystone table, and any values modified using this method will be lost
  // To learn more about storing or using values in the RAM or flash keystone
  // tables, see the keystone_table_entries and keystone_table methods.
  // Calling this method with a get parameter acts as a Get request and returns
  // the current value as a CameraKeystone object
  uint64_t keystone(CameraKeystone *get);

  // The keystone methods get or set the camera's keystone values using the ISP
  // (Image Signal Processor) registers. It can only be used to query or modify
  // the keystone values that are being used while the camera is currently
  // streaming frames. It does not modify the values in the keystone tables and
  // values written using this method will not be persistent. That is, the next
  // time the camera starts streaming frames, it will use the values from the
  // RAM keystone table, and any values modified using this method will be lost
  // To learn more about storing or using values in the RAM or flash keystone
  // tables, see the keystone_table_entries and keystone_table methods.
  // Calling this method with a set parameter acts as a Set request.
  // On success it will trigger an on_keystone notification.
  // When setting a CameraKeystone value, the units for the x and y values in
  // each Point are pixels in the hirescamera's effective size (or the parent
  // resolution) of the resolution being streamed. The valid range for the x
  // and y values in each point are such that the points never fall outside of
  // the edges of the image for each parent resolution. Clients can use the
  // streaming_resolution and parent_resolution methods to query the current
  // streaming and parent resolution values. The four points in a
  // CameraKeystone must define a convex quadrilateral: all interior angles
  // are be less than 180 degrees and the two diagonals both lie inside the
  // quadrilateral.
  // Note: The value actually set by the hardware may differ from the value
  // passed in. To check the value set by the hardware use a keystone
  // function with a get parameter
  uint64_t keystone(const CameraKeystone &set);

  // The keystone methods get or set the camera's keystone values using the ISP
  // (Image Signal Processor) registers. It can only be used to query or modify
  // the keystone values that are being used while the camera is currently
  // streaming frames. It does not modify the values in the keystone tables and
  // values written using this method will not be persistent. That is, the next
  // time the camera starts streaming frames, it will use the values from the
  // RAM keystone table, and any values modified using this method will be lost
  // To learn more about storing or using values in the RAM or flash keystone
  // tables, see the keystone_table_entries and keystone_table methods.
  // Calling this method with a set parameter acts as a Set request.
  // On success it will return a CameraKeystone object that confirms the value
  // set and trigger an on_keystone notification.
  // When setting a CameraKeystone value, the units for the x and y values in
  // each Point are pixels in the hirescamera's effective size (or the parent
  // resolution) of the resolution being streamed. The valid range for the x
  // and y values in each point are such that the points never fall outside of
  // the edges of the image for each parent resolution. Clients can use the
  // streaming_resolution and parent_resolution methods to query the current
  // streaming and parent resolution values. The four points in a
  // CameraKeystone must define a convex quadrilateral: all interior angles
  // are be less than 180 degrees and the two diagonals both lie inside the
  // quadrilateral.
  // Note: The value returned is the value that was actually set by the
  // hardware (which may differ from the value passed in).
  uint64_t keystone(const CameraKeystone &set, CameraKeystone *get);

  // Calling this method with a get parameter acts as a Get request and
  // returns a CameraKeystoneTable object. If the ram table matches one
  // of the other keystone tables (for all resolutions), this method will
  // return the name of the table it matches. If the ram values do not match
  // one the other tables, this method will return ram.
  uint64_t keystone_table(CameraKeystoneTable *get);

  // The HP Z 3D Camera high resolution camera has several different keystone
  // tables which are defined in the CameraKeystoneTable. When the camera
  // begins streaming at a given resolution, it uses the settings stored in the
  // ram keystone table for that resolution. Clients can use this
  // keystone_table method to re-initialize the ram table with settings from
  // one of the other tables. The table selected using this keystone_table
  // method will also serve as the power-up table. This means that the next
  // time the camera powers up, the values from the selected table will be
  // copied into the ram table. If this method is called with ram as the
  // parameter, it will have no effect. The values in the RAM table and the
  // selected power-up table will not change and SoHal will not send the
  // on_keystone_table notification.
  // Calling this method with a set parameter acts as a Set request. On
  // success it will trigger an on_keystone_table notification.
  uint64_t keystone_table(const CameraKeystoneTable &set);

  // The HP Z 3D Camera high resolution camera has several different keystone
  // tables which are defined in the CameraKeystoneTable. When the camera
  // begins streaming at a given resolution, it uses the settings stored in the
  // ram keystone table for that resolution. Clients can use this
  // keystone_table method to re-initialize the ram table with settings from
  // one of the other tables. The table selected using this keystone_table
  // method will also serve as the power-up table. This means that the next
  // time the camera powers up, the values from the selected table will be
  // copied into the ram table. If this method is called with ram as the
  // parameter, it will have no effect. The values in the RAM table and the
  // selected power-up table will not change and SoHal will not send the
  // on_keystone_table notification.
  // Calling this method with a set parameter acts as a Set request. On
  // success it will return a CameraKeystoneTable object that confirms the
  // value set and trigger an on_keystone_table notification.
  uint64_t keystone_table(const CameraKeystoneTable &set,
                          CameraKeystoneTable *get);

  // keystone_table_entries
  // This method is used to either query or update the values in the various
  // keystone tables. Separate keystone values can be stored in each table for
  // each of the camera's supported resolutions. The values stored in the ram
  // keystone table are applied when the camera begins streaming frames at a
  // particular resolution. Clients can query or store keystone values in the
  // ram table. However, please note that any keystone values written directly
  // into the ram table will not persist across power cycles. When the firmware
  // initializes, it will fill the ram keystone table with a copy of either the
  // default table or one of the flash tables (depending on on which table was
  // last selected using the keystone_table method). Additionally, clients can
  // request that the ram table be re-initialized with one of the other tables
  // using the keystone_table method. If the camera is already streaming,
  // setting values in the ram table will not update the keystone values the
  // camera is using for the active stream. However, values set in the ram
  // table will be used the next time the camera starts streaming or changes
  // streaming resolutions. Clients can use the keystone method to change the
  // current keystone values when the camera is already streaming (without the
  // need to restart the stream or change resolutions). The default keystone
  // table is compiled into the camera firmware. Clients are allowed to query
  // the values from this table, but are not allowed to store new values in the
  // default table.Clients can store values into the flash_max_fov and
  // flash_fit_to_mat tables.Values stored to these two tables will persist
  // across power cycles and can be used to re - initialize the ram
  // table(through the keystone_table method or the camera's power up sequence)

  // Get keystone table entries
  // Calling this method with a CameraKeystoneTable parameter acts as
  // a Get request and will return a CameraKeystoneTableEntries object
  // containing all the entries in the table. Internally this function
  // allocates memory for the returned CameraKeystoneTableEntries values,
  // and free_keystone_table_entries must be called passing in the get
  // parameter to avoid a memory leak
  uint64_t keystone_table_entries(const CameraKeystoneTable &param,
                                 CameraKeystoneTableEntries *get,
                                 uint32_t *num_entries);

  // Get keystone table entries
  // Calling this method with CameraKeystoneTable and a list of
  // CameraResolution parameters acts as Get and will return a
  // CameraKeystoneTableEntries object containing only the entries for
  // the resolutions in the list. This function will return a
  // CameraKeystoneTableEntries object containing only the entries for
  // the resolutions in the list
  // Internally this function allocates memory for the returned
  // CameraKeystoneTableEntries values, and free_keystone_table_entries
  // must be called passing in the get parameter to avoid a memory leak.
  uint64_t keystone_table_entries(const CameraKeystoneTable &param,
                                 CameraResolution *resoution_list,
                                 uint32_t num_resolutions,
                                 CameraKeystoneTableEntries *get,
                                 uint32_t *num_entries);

  // Set keystone table entries
  // Calling this method with CameraKeystoneTable and a list of
  // CameraKeystoneTableEntry parameters acts as a Set request. On success it
  // will return a CameraKeystoneTableEntries object that confirms the list of
  // values changed and trigger an on_keystone_table_entries notification. When
  // setting a CameraKeystoneTableEntry value, the units for the x and y values
  // in each Point are pixels in the hirescamera's effective size (or the
  // parent resolution) of the resolution being set. The valid range for the x
  // and y values in each point are such that the points never fall outside of
  // the edges of the image for each parent resolution. Clients can use the
  // streaming_resolution and parent_resolution methods to query the current
  // streaming and parent resolution values. Additionally the four points in a
  // CameraKeystoneTableEntry must define a convex quadrilateral: all interior
  // angles are be less than 180 degrees and the two diagonals both lie inside
  // the quadrilateral.
  // Note: The value returned is the value that was actually set by the
  // hardware(which may differ from the value passed in).
  uint64_t keystone_table_entries(const CameraKeystoneTableEntries &set,
                                  const uint32_t &num_entries);

  // set then get keystone table entries
  // Calling this method with CameraKeystoneTable and a list of
  // CameraKeystoneTableEntry parameters acts as a Set request. On success it
  // will return a CameraKeystoneTableEntries object that confirms the list of
  // values changed and trigger an on_keystone_table_entries notification. When
  // setting a CameraKeystoneTableEntry value, the units for the x and y values
  // in each Point are pixels in the hirescamera's effective size (or the
  // parent resolution) of the resolution being set. The valid range for the x
  // and y values in each point are such that the points never fall outside of
  // the edges of the image for each parent resolution. Clients can use the
  // streaming_resolution and parent_resolution methods to query the current
  // streaming and parent resolution values. Additionally the four points in a
  // CameraKeystoneTableEntry must define a convex quadrilateral: all interior
  // angles are be less than 180 degrees and the two diagonals both lie inside
  // the quadrilateral.
  // Note: Internally this function allocates memory for the returned
  // CameraKeystoneTableEntries values, and free_keystone_table_entries
  // must be called passing in the get parameter to avoid a memory leak.
  // Note: The value returned is the value that was actually set by the
  // hardware(which may differ from the value passed in).
  uint64_t keystone_table_entries(const CameraKeystoneTableEntries &set,
                                  const uint32_t &num_set_entries,
                                  CameraKeystoneTableEntries *get,
                                  uint32_t *num_get_entries);

  // frees the memory allocated in the get calls of
  // keystone_table_entries - this must be called after the user
  // gets any CameraKeystoneTableEntries in any of the get
  // parameters of keystone_table_entries() functions
  void free_keystone_table_entries(CameraKeystoneTableEntries *entries,
                                   uint32_t num_entries);

  // Calling this method with a get parameter acts as a Get request and returns
  // the current settings for the camera LEDs.
  uint64_t led_state(CameraLedState *get);

  // Calling this method with a set parameter will change the LED state
  // settings and trigger an on_led_state notification.
  uint64_t led_state(const CameraLedState &set);

  // Calling this method with a set parameter will change the LED state
  // settings and trigger an on_led_state notification. It will then
  // confirm what was set and return the values in the get parameter.
  uint64_t led_state(const CameraLedState &set, CameraLedState *get);

  // Calling this method with a get parameters acts as a Get request and
  // returns a bool indicating the current value.
  uint64_t lens_color_shading(bool *get);

  // Calling this method with a set parameter turns the lens_color_shading
  // mode on or off. On success SoHal will send an on_lens_color_shading
  // notification to all subscribed clients.
  uint64_t lens_color_shading(bool set);

  // Calling this method with a set parameter turns the lens_color_shading
  // mode on or off. On success, the response will include a bool confirming
  // the value that was set and SoHal will send an on_lens_color_shading
  // notification to all subscribed clients.
  uint64_t lens_color_shading(bool set, bool *get);

  // Calling this method with a get parameters acts as a Get request and
  // returns a bool indicating the current value.
  uint64_t lens_shading(bool *get);

  // Calling this method with a set parameter turns the lens_shading
  // mode on or off. On success SoHal will send an on_lens_shading
  // notification to all subscribed clients.
  uint64_t lens_shading(bool set);

  // Calling this method with a set parameter turns the lens_shading
  // mode on or off. On success, the response will include a bool confirming
  // the value that was set and SoHal will send an on_lens_shading
  // notification to all subscribed clients.
  uint64_t lens_shading(bool set, bool *get);

  // Calling this method with a get parameters acts as a Get request and
  // returns a bool indicating the current value.
  uint64_t mirror_frame(bool *get);

  // Calling this method with a set parameter turns the mirror_frame
  // mode on or off. On success SoHal will send an on_mirror_frame
  // notification to all subscribed clients.
  uint64_t mirror_frame(bool set);

  // Calling this method with a set parameter turns the mirror_frame
  // mode on or off. On success, the response will include a bool confirming
  // the value that was set and SoHal will send an on_mirror_frame
  // notification to all subscribed clients.
  uint64_t mirror_frame(bool set, bool *get);

  // This method will return the parent resolution of either the provided
  // resolution or the camera's current streaming resolution, as a
  // CameraResolution object. Note that this method is not currently supported
  // for Sprout high resolution cameras. Calling this method for a Sprout
  // hirescamera device will return a 'Functionality not available' error,
  // which does not provide information on the current state of the camera.
  // If no parameter is provided, this method will return the parent
  // resolution of the camera's current streaming resolution. If the camera is
  // not currently streaming frames, this method will return an error such as
  // 'The camera is not streaming'.
  uint64_t parent_resolution(CameraResolution *get);

  // This method will return the parent resolution of either the provided
  // resolution or the camera's current streaming resolution, as a
  // CameraResolution object. Note that this method is not currently supported
  // for Sprout high resolution cameras. Calling this method for a Sprout
  // hirescamera device will return a 'Functionality not available' error,
  // which does not provide information on the current state of the camera.
  // If a CameraResolution parameter is included, this method will return the
  // parent resolution of the provided resolution. In this case the camera does
  // not need to be streaming frames.
  uint64_t parent_resolution(const CameraResolution &provided,
                             CameraResolution *get);

  // This method allows clients to get or set the camera's power line frequency
  // value. Clients can select if the power line frequency setting should be
  // disabled, enabled at 50 Hz, or enabled at 60 Hz.
  // Calling this method wit a get parameter acts as a Get request and returns
  // a PowerLineFrequency object indicating the current setting.
  uint64_t power_line_frequency(PowerLineFrequency *get);

  // Calling this method with a set parameter will set the current power
  // line frequency value to the provided parameter. On success SoHal will
  // send an on_power_line_frequency notification to all subscribed clients.
  uint64_t power_line_frequency(const PowerLineFrequency &set);

  // Calling this method with a set parameter will set the current power
  // line frequency value to the provided parameter. On success, the response
  // will include a PowerLineFrequency object in the get parameter confirming
  // the value that was set and SoHal will send an on_power_line_frequency
  // notification to all subscribed clients.
  uint64_t power_line_frequency(const PowerLineFrequency &set,
                                PowerLineFrequency *get);

  // Reboots the hirescamera. This is equivalent to physically disconnecting
  // and reconnecting the device's usb cable. As such, it will cause
  // hirescamera.on_device_disconnected, system.on_device_disconnected,
  // hirescamera.on_device_connected, and system.on_device_connected
  // notifications. Additionally, the behavior should be identical to when the
  // hirescamera's usb cable is physically disconnected and reconnected. The
  // device will be closed when the on_device_disconnected notifications are
  // sent. By the time the on_device_connected notifications are sent, all
  // settings will be restored to the hardware's startup values. After the
  // connected notifications, the hirescamera will need to be opened and any
  // desired settings will need to be set. On success, this method will also
  // trigger an on_reset notification. Note that the on_device_disconnected
  // and on_device_connected notifications may come before or after the
  // response to the command and hirescamera.on_reset notification.
  uint64_t reset();

  // Calling this method with a get parameter acts as a Get request and
  // returns an unsigned int indicating the current setting.
  uint64_t saturation(uint16_t *get);

  // Calling this method with an unsigned int parameter will set the current
  // saturation value to the provided parameter. On success SoHal will send
  // an on_saturation notification to all subscribed clients.
  // The valid range for saturation is : 0 <= saturation <= 63
  uint64_t saturation(uint16_t set);

  // Calling this method with an unsigned int parameter will set the current
  // saturation value to the provided parameter. On success, the response will
  // include an unsigned int in the get parameter confirming the value that was
  // set and SoHal will send an on_saturation notification to all subscribed
  // clients.
  // The valid range for saturation is : 0 <= saturation <= 63
  uint64_t saturation(uint16_t set, uint16_t *get);

  // Calling this method with a get parameter acts as a Get request and
  // returns an unsigned int indicating the current setting.
  uint64_t sharpness(uint16_t *get);

  // Calling this method with an unsigned int parameter will set the
  // current sharpness value to the provided parameter. On success,
  // SoHal will send an on_sharpness notification to all subscribed clients.
  // The valid range for sharpness is: 0 <= sharpness <= 4
  uint64_t sharpness(uint16_t set);

  // Calling this method with an unsigned int parameter will set the
  // current sharpness value to the provided parameter. On success,
  // the response will include an unsigned int in the get parameter
  // confirming the value that was set and SoHal will send an
  // on_sharpness notification to all subscribed clients.
  // The valid range for sharpness is: 0 <= sharpness <= 4
  uint64_t sharpness(uint16_t set, uint16_t *get);

  // This method will return the camera's current streaming resolution as a
  // CameraResolution object. If the camera is not currently streaming frames,
  // this method will return an error such as 'The camera is not streaming'.
  // Note that this method is currently not supported for Sprout high
  // resolution cameras.Calling this method for a Sprout hirescamera device
  // will return a 'Functionality not available' error, which does not provide
  // information on the current state of the camera.
  uint64_t streaming_resolution(CameraResolution *get);

  // This method sets a gpio from the hirescamera into the projector that
  // triggers a high intensity flash for the duration (in camera frames)
  // established in the Strobe parameter. It also temporarily changes the
  // gain and the exposure to those passed as parameter. This method triggers
  // an on_strobe notification.
  // Note: Internally the camera GPIO will perform equivalent functionality
  // as the projector.flash(no_content), and, therefore, there is a limit on
  // how many frames the flash can be on.
  // Note: Valid parameter ranges are: 1 <= frames <= 254, 0 <= gain <= 127,
  // 1 <= exposure <= 3385.
  // Note: This function should be called only when streaming frames,
  // otherwise it will return a Device is in the wrong state error.
  //******************************************************************
  // Note: The camera will stop responding if a command from any client
  // connected to SoHAL is sent to the camera while the camera is in
  // strobe mode and a system reboot will be required to reset the
  // camera and fix it.
  //******************************************************************
  uint64_t strobe(const Strobe &set);

  // subscribe to notifications
  uint64_t subscribe(void(*callback)(const HiResCameraNotificationParam &param,
                                     void *data),
                     void *data);
  uint64_t subscribe(void(*callback)(const HiResCameraNotificationParam &param,
                                     void *data),
                     void *data, uint32_t *get);

  // unsubscribe from notifications
  uint64_t unsubscribe();
  uint64_t unsubscribe(uint32_t *get);

  // Calling this method with a get parameter acts as a Get request and returns
  // the current white balance as an Rgb color. Please note that if the
  // camera's auto_white_balance mode is on, consecutive calls to this method
  // may return different values.
  uint64_t white_balance(hippo::Rgb *get);

  // Calling this method with a set parameter will automatically set the camera
  // in manual white balance mode and set the current white balance value to
  // the provided parameter.On success SoHal will send an
  // on_white_balance notification to all subscribed clients.
  // The valid ranges for white balance for different hirescameras are listed
  // below:
  //   Sprout, Sprout Pro, and
  //   Sprout Pro G2 hirescameras: 1024 <= rgb.red, rgb.green, rgb.blue <= 4095
  //   HP Z 3D Camera hirescameras:   0 <= rgb.red, rgb.green, rgb.blue <= 2047
  uint64_t white_balance(const hippo::Rgb &set);

  // Calling this method with a set parameter will automatically set the camera
  // in manual white balance mode and set the current white balance value to
  // the provided parameter.On success, the response will include an Rgb
  // confirming the value that was set and SoHal will send an
  // on_white_balance notification to all subscribed clients.
  // The valid ranges for white balance for different hirescameras are listed
  // below:
  //   Sprout, Sprout Pro, and
  //   Sprout Pro G2 hirescameras: 1024 <= rgb.red, rgb.green, rgb.blue <= 4095
  //   HP Z 3D Camera hirescameras:   0 <= rgb.red, rgb.green, rgb.blue <= 2047
  uint64_t white_balance(const hippo::Rgb &set, hippo::Rgb *get);

  // This method allows clients to get or set the camera's current white
  // balance as a color temperature. The white balance temperature is
  // represented as a value in 100's of degrees Kelvin. So a return value
  // of 40 would correspond to 4000 degrees Kelvin. The valid range for
  // white balance temperature is: 25 <= temperature <= 125. Calling this
  // method with a get parameter acts as a Get request and returns an unsigned
  // int representing the current white balance color temperature value.
  // Please note that this Get request is only supported when the camera's
  // white balance was most recently set to a color temperature. This is
  // because the Rgb white balance has a range which does not always
  // correspond to a value on the temperature curve.If the white balance
  // was manually set to an RGB value(through the hirescamera.white_balance
  // method), or if the camera's auto_white_balance mode is on, this method
  // will return a "wrong state" error. In these situations the camera's
  // white balance RGB values can still be queried using the
  // hirescamera.white_balance method.
  uint64_t white_balance_temperature(uint16_t *get);

  // Calling this method with an set parameter will automatically
  // set the camera in manual white balance mode and set the current white
  // balance temperature value to the provided parameter. On success,
  // SoHal will send an on_white_balance_temperature notification to
  // all subscribed clients. As long as the camera is streaming, clients can
  // set the white_balance_temperature and query the corresponding RGB values
  // using the hirescamera.white_balance method.
  uint64_t white_balance_temperature(uint16_t set);

  // Calling this method with an set parameter will automatically
  // set the camera in manual white balance mode and set the current white
  // balance temperature value to the provided parameter. On success, the
  // response will include an unsigned int in the get parameter
  // confirming that the value that was set.  Afer that SoHal will send an
  // on_white_balance_temperature notification to all subscribed clients.
  // As long as the camera is streaming, clients can set the
  // white_balance_temperature and query the corresponding RGB values
  // using the hirescamera.white_balance method.
  uint64_t white_balance_temperature(uint16_t set, uint16_t *get);

 protected:
  uint64_t AutoOrFixed_json2c(const void *obj, const char *key,
                              hippo::AutoOrFixed *get);
  uint64_t AutoOrFixed_json2c(const void *obj, hippo::AutoOrFixed *get);
  uint64_t AutoOrFixed_c2json(const hippo::AutoOrFixed &set, const char *key,
                              void *obj);
  uint64_t Bool_json2c(const void *obj, const char *key, bool *get);
  uint64_t CameraConfig_json2c(const void *obj, CameraConfig *cf);
  uint64_t CameraStatus_json2c(const void *obj, CameraStatus *get);
  uint64_t CameraDeviceStatus_json2c(const void* obj,
                                     CameraDeviceStatus* get);
  uint64_t CameraQuadrilateral_json2c(const void *obj,
                                      CameraQuadrilateral *get);
  uint64_t CameraSettings_c2json(const hippo::CameraSettings &set, void *obj);
  uint64_t CameraSettings_json2c(const void *obj, hippo::CameraSettings *get);
  uint64_t CameraKeystone_c2json(const hippo::CameraKeystone &set, void *obj);
  uint64_t CameraKeystone_json2c(const void *obj, hippo::CameraKeystone *get);
  uint64_t CameraKeystoneTable_c2json(const hippo::CameraKeystoneTable &set,
                                      void *obj);
  uint64_t CameraKeystoneTableAndResolution_c2json(
                               const hippo::CameraKeystoneTable &table,
                               const hippo::CameraResolution *resolution_list,
                               const uint32_t &num_resolutions,
                               void *obj);
  uint64_t CameraKeystoneTable_json2c(const void *obj,
                                      hippo::CameraKeystoneTable *get);
  uint64_t CameraKeystoneTableEntry_c2json(
                                   const hippo::CameraKeystoneTableEntry &set,
                                   void *obj);
  uint64_t CameraKeystoneTableEntry_json2c(const void *obj,
                                      hippo::CameraKeystoneTableEntry *get);
  uint64_t CameraKeystoneTableEntries_c2json(
                                        const CameraKeystoneTableEntries &set,
                                        const uint32_t num_entries,
                                        void *obj);
  uint64_t CameraKeystoneTableEntries_json2c(const void *obj,
                                             CameraKeystoneTableEntries *get,
                                             uint32_t *num_entries);
  uint64_t CameraLedState_c2json(const hippo::CameraLedState &set,
                                 void *obj);
  uint64_t CameraLedState_json2c(const void *obj,
                                 hippo::CameraLedState *get);
  uint64_t CameraLedStateEnum_json2c(const void *obj,
                                     hippo::CameraLedStateEnum *state);
  uint64_t PowerLineFrequency_c2json(const hippo::PowerLineFrequency *set,
                                     void *obj);
  uint64_t PowerLineFrequency_json2c(const void *obj,
                                     hippo::PowerLineFrequency *get);
  uint64_t ParsePointJson(void *jsonPoint, Point *cPoint);
  uint64_t Resolution_c2json(const hippo::CameraResolution &set, void *obj);
  uint64_t Resolution_json2c(const void *obj, hippo::CameraResolution *get);
  uint64_t Strobe_json2c(const  void *obj, hippo::Strobe *get);
  uint64_t Strobe_c2json(const hippo::Strobe &set, void *obj);
  uint64_t WhiteBalance_json2c(void *obj, hippo::Rgb *wb);
  uint64_t WhiteBalance_c2json(const hippo::Rgb &wb, void *obj);

  // Callback items
  void ProcessSignal(char *method, void *params) override;
  bool HasRegisteredCallback();

  void (*callback_)(const HiResCameraNotificationParam &param, void *data);
};

}   // namespace hippo

#endif   // INCLUDE_HIRESCAMERA_H_
