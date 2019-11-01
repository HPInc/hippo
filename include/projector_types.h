
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#ifndef INCLUDE_PROJECTOR_TYPES_H_
#define INCLUDE_PROJECTOR_TYPES_H_

#include <stdint.h>
#include "../include/common_types.h"

namespace hippo {

// Enumerates the possible states of the Projector device.
typedef enum class ProjectorState {
  // The projector is not powered on. Note that the projector will not be
  // communicating when it is in this state.
  off = 0x00,
  // The projector is initialized and waiting to receive commands. The LEDs are
  // off and the projector is not displaying anything.
  standby = 0x01,
  // The projector is turned on, receiving an input signal, and displaying
  // content.
  on = 0x02,
  //  The projector is above the maximum operating temperature. The display is
  // turned off and the fans are blowing at full speed.
  overtemp = 0x03,
  // The projector is displaying at maximum brightness. If the projector is
  // flashing with content, the content will be displayed in grayscale and the
  // keystone will be applied. If the projector is flashing without content,
  // the projector will display a full white image with no keystone applied.
  // Note that the projector's flash is time-limited and will automatically
  // turn off after 10 seconds.
  flashing = 0x04,
  // The projector is completing a request to transition into the on state.
  transition_to_on = 0x05,
  // The projector is completing a request to transition into the standby state
  transition_to_st = 0x06,
  // The projector has an internal hardware failure.
  hw_fault = 0x07,
  // The projector is powering up. This state is only supported for
  // Sprout Pro G2 hardware.
  initializing = 0x08,
  // The projector is on but has not detected an input video source so it is
  // not displaying content. When the projector receives an input video signal
  // it will automatically transition to the on state. This state is only
  // supported for Sprout Pro G2 hardware.
  on_no_source = 0x09,
  // The projector is completing a request to transition into the flash state.
  // This state is only supported for Sprout Pro G2 hardware.
  transition_to_flash = 0x0a,
  // The projector is completing a request to transition into the grayscale
  // state. This state is only supported for Sprout Pro G2 hardware.
  transition_to_grayscale = 0x0b,
  // The projector is displaying at the maximum sustainable brightness. Content
  // will be displayed in grayscale and the keystone will be maintained. This
  // state is similar to the flash state when content is on, but the brightness
  // is decreased so it does not need to be time-limited.
  grayscale = 0x0c,
  // The projector is in the middle of a firmware upgrade. This state is only
  // supported for Sprout Pro G2 hardware.
  fw_upgrade = 0x0d,
  // The projector is in burn in mode. This state is only supported for
  // Sprout Pro G2 hardware.
  burn_in = 0x0e,
} ProjectorState;

// enumerates the type of keystone being used in the Keystone type.
// For further details see the Keystone type description, below.
typedef enum class KeystoneType {
  KEYSTONE_1D = 1,
  KEYSTONE_2D = 2,
} KeystoneType;

// The Keystone2d is defined by setting offset values from the default
// non-keystoned display (where all x and y values are set to 0). A positive
// x or y offset will move a point to the right or down, while negative values
// move a point to the left or up. If a point is moved outside of the maximum
// projection area (beyond the non-keystoned display), the projector will not
// be able to display that point, but the image will still be warped
// accordingly. Giving the corners offsets that bring them inside of the
// maximum area will decrease the projected area. In other words, if you start
// with the maximum display (the x and y values for all points are 0), then
// setting the top_left x and y offsets to negative values (or setting the
// bottom_right x and y offsets to positive values) will not increase the
// projected area (since the projector is already at the max), but it will warp
// the image as if the corner is being displayed at the requested location
// However, setting the top_left x and y offsets to positive values (or setting
// the bottom_right x and y offsets to negative values) will make the projected
// area smaller.
typedef struct Keystone_2d {
  Point top_left;
  Point top_right;
  Point bottom_left;
  Point bottom_right;
  // Note that the top_middle and bottom_middle x values must be equal
  Point top_middle;
  Point bottom_middle;
  // Note that the left_middle and right_middle y values must be equal
  Point left_middle;
  Point right_middle;
  Point center;
} Keystone_2d;

typedef struct Keystone_1d {
  float pitch;              // range from [-20.,0.]
  // display_area where
  //   display_area.x >= 0,
  //   display_area.y >= 0,
  //   (display_area.x + display_area.width) <= 1500, and
  //   (display_area.y + display_area.height) <= 1000
  Rectangle display_area;
} Keystone_1d;

// // There are two types of keystone algorithms:
//   one dimensional (1d) and
//   two dimensional (2d).
// Sprout and Sprout Pro projectors use the 1d keystone algorithm,and
// Sprout Pro G2 projectors use the 2d keystone algorithm. The
// projector.keystone method accepts \ returns a higher level Keystone object
// which includes a KeystoneType detailing the keystone type and the actual
// value.
typedef struct Keystone {
  KeystoneType type;
  union {
    Keystone_1d value_1d;
    Keystone_2d value_2d;
  };
} Keystone;

typedef struct CalibrationData {
  char *cam_cal;
  char *cam_cal_hd;
  char *proj_cal;
  char *proj_cal_hd;
} CalibrationData;

typedef struct Corners {
  PointFloats top_left;
  PointFloats top_right;
  PointFloats bottom_left;
  PointFloats bottom_right;
} Corners;

typedef struct DPPVersion {
  uint32_t major;
  uint32_t minor;
  uint32_t patch_lsb;
  uint32_t patch_msb;
} DPPVersion;

// Enumerates the possible solid colors for the solid_color method.
typedef enum class SolidColor {
  off,      // This indicates that the projector is not in solid_color mode.
  black,
  red,
  green,
  blue,
  cyan,
  magenta,
  yellow,
  white,
} SolidColor;

typedef struct GeoFWVersion {
  uint32_t major;
  uint32_t minor;
  char package;
  uint32_t test_release;
} GeoFWVersion;

typedef struct HardwareInfo {
  Resolution input_resolution;
  Resolution output_resolution;
  uint32_t refresh_rate;
  uint32_t pixel_density;
} HardwareInfo;

// Enumerates the names of the target white points.
typedef enum class Illuminant {
  d50,      // Horizon light
  d65,      // Noon daylight
  d75,      // North sky daylight
  custom,   // Custom user writable setting
} Illuminant;

typedef struct ManufacturingData {
  uint32_t gain;
  uint32_t exposure;
  uint32_t red;
  uint32_t green;
  uint32_t blue;
  Keystone keystone;
  Corners hires_corners;
  Corners ir_corners;
} ManufacturingData;

typedef struct ProjectorLedTimes {
  // The total number of minutes the projector has been in the grayscale
  // ProjectorState
  float grayscale;
  // The total number of minutes the projector has been in the on
  // ProjectorState.
  float on;
  // An estimate of the total number of minutes the projector has been in the
  // flash ProjectorState. This is a worst case estimate, as the projector
  // firmware currently only provides the total number of times that the flash
  // method has been called. The count is converted to minutes with the
  // assumption that each call to flash has left the projector flashing for the
  // full 10 seconds.
  float flash;
} ProjectorLedTimes;

typedef struct ProjectorSpecificInfo {
  DPPVersion asic_version;
  char* column_serial;
  uint32_t eeprom_version;
  DPPVersion flash_version;
  GeoFWVersion geo_fw_version;
  uint32_t hw_version;
  char* manufacturing_time;
} ProjectorSpecificInfo;

typedef struct WhitePoint {
  // The name of the illuminant
  Illuminant name;
  // The CIE chromaticity coordinate in the 2 degree field of view (1931 color
  // space). When setting the white point, this field is optional and only
  // needs to be included to selected new values for the "custom" white point.
  PointFloats value;
} WhitePoint;

typedef enum class ProjectorNotification {
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

  // projector specific notifications below

  // This notification occurs when a client updates the brightness. It
  // includes an int parameter with the updated value.
  on_brightness,
  // This notification occurs when a client updates the keystone. It
  // includes a Keystone parameter with the updated value.
  on_keystone,
  // This notification occurs when a client updates the solid_color.
  // It includes a SolidColor parameter with the updated value.
  on_solid_color,
  // This notification occurs when the projector's state changes. It includes
  // a ProjectorState parameter which specifies the new state.
  on_state,
  // This notification occurs when the projector is put into or taken out of
  // structured_light_mode. It includes a bool parameter which is set to true
  // when the projector is in structured light mode, and false when it is not.
  on_structured_light_mode,
  // This notification occurs when a client updates the white_point. It
  // includes a WhitePoint parameter with the updated value.
  on_white_point,
} ProjectorNotification;

// The parameters that are passed when a projector notification is triggered
typedef struct ProjectorNotificationParam {
  // the type of notification that is being passed
  ProjectorNotification type;
  union {
    // parameter passed with the on_open_count notification
    uint32_t on_open_count;
    // parameter passed with the on_brightness notification
    uint32_t on_brightness;
    // parameter passed with the on_keystone notification
    Keystone on_keystone;
    // parameter passed with the on_state notification
    ProjectorState on_state;
    // parameter passed with the on_solid_color notification
    SolidColor on_solid_color;
    // parameter passed with the on_structured_light_mode notification
    bool on_structured_light_mode;
    // parameter passed with the on_white_point notification
    WhitePoint on_white_point;
  };
} ProjectorNotificationParam;

}   // namespace hippo

#endif  // INCLUDE_PROJECTOR_TYPES_H_
