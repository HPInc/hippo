
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#ifndef INCLUDE_SYSTEM_TYPES_H_
#define INCLUDE_SYSTEM_TYPES_H_

#include <stdint.h>

namespace hippo {

// Enumerates the names of the different temperature sensors that
// system.temperatures() may return data for.
typedef enum class TempInfoSensors {
  // The LED Printed Circuit Board Assembly (PCBA) temperature sensor for a
  // Sprout, Sprout Pro, or Sprout Pro G2 projector.
  led = 0x00,
  // The red LED temperature sensor for a Sprout Pro G2 projector.
  red = 0x01,
  // The green LED temperature sensor for a Sprout Pro G2 projector.
  green = 0x02,
  // The formatter PCA temperature sensor for a Sprout or Sprout Pro projector.
  formatter = 0x03,
  // The LED heat sink temperature sensor for a Sprout or Sprout Pro projector.
  heatsink = 0x04,
  // The temperature sensor for a Sprout high resolution camera. Note that for
  // Sprout and Sprout Pro units, this sensor is read through the desklamp
  // device. For Sprout Pro G2 units, this temperature sensor is read through
  // the projector device.
  hirescamera = 0x05,
  // The temperature sensor for a Sprout depthcamera. For Sprout and Sprout Pro
  // units, this sensor is read through the desklamp device. For Sprout Pro G2
  // units, this temperature is read through the depthcamera device.
  depthcamera = 0x06,
  // The temperature sensor that controls the thermoelectric cooler (TEC)
  // located within the laser projector package of a Sprout Pro G2
  // depthcamera.
  depthcamera_tec = 0x07,
  // The temperature sensor for an HP Z 3D Camera high resolution camera.
  hirescamera_z_3d = 0x08,
  // The temperature sensor for the main board of an HP Z 3D Camera high
  // resolution camera.
  hirescamera_z_3d_system = 0x09,
  // The temperature sensor that controls the thermoelectric cooler (TEC)
  // located within the laser projector package of an HP Z 3D Camera
  // depthcamera.
  depthcamera_z_3d_tec = 0x0A,
} TempInfoSensors;

// the device through which the temperature sensor is connected
typedef enum class TemperatureConnectionDevices {
  // the temperature device is connected through the depth camera
  through_depthcamera,
  // the temperature device is connected through the desklamp
  through_desklamp,
  // the temperature device is connected through the high resolution camera
  through_hirescamera,
  // the temperature device is connected through the projector
  through_projector,
} TemperatureConnectionDevices;

typedef struct TemperatureInfoDevice {
  // the device through which this temperature sensor is connected
  // For information about this see the descriptions in the
  // TempInfoSensors items. For example, if the temperature sensor
  // is the hirescamera on a Sprout G2, then the connectedDevice
  // will be the projector
  TemperatureConnectionDevices connectedDevice;
  // the index of the device
  uint32_t index;
} TemperatureInfoDevice;

// The TemperatureInfo type includes the important information for a given
// temperature sensor. All temperatures are in degrees Celsius
typedef struct TemperatureInfo {
  // The device through which SoHal accesses this temperature sensor.
  // This includes the device name and index (i.e. depthcamera@1) to
  // allow clients to differentiate between sensors when multiple
  // instances of the same device type are connected
  TemperatureInfoDevice device;
  // The current temperature
  float current;
  // The temperature at which the device goes into an over-temperature mode.
  // At this point the firmware may take corrective action (such as disabling
  // functionality or cutting power) in order to prevent damage to the
  // hardware. Additionally, for some devices it may be necessary to
  // disconnect and reconnect the usb cable after the unit has cooled down in
  // order to reestablish a connection.
  float max;
  // When a device temperature passes this safe value, it is starting to get
  // hot. The firmware will not limit functionality at this point, but it is
  // advisable for the client or user to take corrective action. If the device
  // continues to heat up and passes the max temperature, it will remain in
  // over-temperature mode until it has cooled back down below this safe
  // temperature.
  float safe;
  // The name identifying the temperature sensor
  TempInfoSensors sensor;
} TemperatureInfo;

}   // namespace hippo

#endif  // INCLUDE_SYSTEM_TYPES_H_
