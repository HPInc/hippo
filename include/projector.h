
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#ifndef INCLUDE_PROJECTOR_H_
#define INCLUDE_PROJECTOR_H_

#include "../include/hippo_device.h"
#include "../include/projector_types.h"


#if COMPILING_DLL
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif

namespace hippo {

class DLLEXPORT Projector : public HippoDevice {
 public:
  Projector();
  explicit Projector(uint32_t device_index);
  Projector(const char *address, uint32_t port);
  Projector(const char *address, uint32_t port, uint32_t device_index);
  virtual ~Projector(void);

  // A method to control the amount of light the projector outputs when it's
  // in the on ProjectorState. Calling this method with a get parameter acts
  // as a Get request and returns the current brightness as a int value.
  uint64_t brightness(uint32_t *get);

  // Calling this method with a set parameter acts as a Set request. On
  // success it will trigger an on_brightness notification.
  // Valid range is 30 <= set <= 100
  uint64_t brightness(uint32_t set);

  // Calling this method with a set parameter acts as a Set request. On
  // success it will return an int in the get parameter that confirms the
  // value set and trigger an on_brightness notification.
  // Valid range is 30 <= set <= 100
  // Note: For Sprout Pro G2 projectors, the value returned is the value
  // that was actually set by the hardware (which could potentially differ
  // from the value passed in).
  // Note that Sprout and Sprout Pro projectors simply return the value that
  // was passed in.
  uint64_t brightness(uint32_t set, uint32_t *get);

  // Returns the projector's 3d calibration data as a CalibrationData value.
  // note that the the calibration_data function allocates memory,
  // and the user must call free_calibration_data() passing in the same
  // CalibrationData pointer when done with the memory in order to avoid a
  // memory leak
  uint64_t calibration_data(hippo::CalibrationData *get);

  // free_calibration_data frees the memory allocated in the calibration_data()
  // function
  // failing call this function after getting the calibration data will
  // result in a memory leak.
  void free_calibration_data(hippo::CalibrationData *cal_data_to_free);

  // Returns a collection of information specific to the projector,
  // such as additional version numbers, as a ProjectorSpecificInfo value.
  // This function allocates memory internally and the user is responsible for
  // calling free_projector_specific_info() afterwards to avoid a memory leak
  uint64_t device_specific_info(hippo::ProjectorSpecificInfo *get);

  // frees the memory allocated in device_specific_info
  // failing call this function after getting the projector specific info will
  // result in a memory leak.
  void free_projector_specific_info(
                  hippo::ProjectorSpecificInfo *info_to_free);

  // Puts the projector in flash mode: maximum brightness, and if content is
  // turned off, maximum keystone. This mode is time limited with a maximum
  // duration of 10 seconds. If the limit is reached the projector will return
  // to the previous ProjectorState. Calling flash again before the limit is
  // reached will not increment the timeout; it will simply return the number
  // of seconds remaining.
  // This mode accepts a content bool parameter. If the parameter is true, the
  // actual display content will be shown in grayscale (useful if you need to
  // display black and white patterns at maximum brightness) and the current
  // keystone will be maintained during the flash. If the parameter is false
  // the projector will display a full white image with no keystone applied.
  // On success, if the projector changes states it will trigger one or more
  // on_state notifications.
  // Note: When the flash is on, it can be turned off by changing the state of
  // the projector using a projector.on command.
  uint64_t flash(bool set);

  // Puts the projector in flash mode: maximum brightness, and if content is
  // turned off, maximum keystone. This mode is time limited with a maximum
  // duration of 10 seconds. If the limit is reached the projector will return
  // to the previous ProjectorState. Calling flash again before the limit is
  // reached will not increment the timeout; it will simply return the number
  // of seconds remaining.
  // This mode accepts a content bool parameter. If the parameter is true, the
  // actual display content will be shown in grayscale (useful if you need to
  // display black and white patterns at maximum brightness) and the current
  // keystone will be maintained during the flash. If the parameter is false
  // the projector will display a full white image with no keystone applied.
  // On success, this method returns an int containing the number of seconds
  // remaining before the flash times out and turns off. If the projector
  // changes states it will trigger one or more on_state notifications.
  // Note: When the flash is on, it can be turned off by changing the state of
  // the projector using a projector.on command.
  uint64_t flash(bool set, uint32_t *get);

  // Puts the projector in grayscale mode: maximum sustainable brightness and
  // normal keystone applied. The projected content will be displayed in
  // grayscale. This mode is not time limited.
  // If the projector changes states it will trigger one or more on_state
  // notifications.
  // Note: When grayscale mode is on, it can be turned off by changing the
  // state of the projector using a projector.on command.
  uint64_t grayscale();

  // Returns the projector's HardwareInfo data.
  uint64_t hardware_info(HardwareInfo *get);

  // Calling this method with a get parameter acts as a Get request and returns
  // the current value as a Keystone object.
  uint64_t keystone(hippo::Keystone *get);

  // Calling this method with a set parameter acts as a Set request. On success
  // it will trigger an on_keystone notification.
  // Note: The value returned is the value that was actually set by the hardware
  // (which may differ from the value passed in).
  uint64_t keystone(const hippo::Keystone &set);

  // Calling this method with a set parameter acts as a Set request. On success
  // it will return a Keystone object in the get parameter that confirms the
  // value set and trigger an on_keystone notification.
  // Note: The value returned is the value that was actually set by the hardware
  // (which may differ from the value passed in).
  uint64_t keystone(const hippo::Keystone &set, hippo::Keystone *get);

  // Returns a ProjectorLedTimes object detailing the number of minutes that
  // the projector has been in the on, grayscale, and flash ProjectorStates.
  // Note that the time listed for the flash state is a worst case scenario.
  // The projector firmware currently only provides the total number of times
  // that flash has been called. This method converts that count into minutes,
  // with the assumption that each call to flash has left the projector
  // flashing for the full 10 seconds, so it is likely a significant over
  // estimate. For further details see the SoHAL documentation.
  uint64_t led_times(hippo::ProjectorLedTimes *get);

  // Returns the projector's ManufacturingData
  uint64_t manufacturing_data(hippo::ManufacturingData *get);

  // Returns information obtained from the operating system on the current
  // display settings for position and resolution of the projector as a
  // Rectangle object.
  uint64_t monitor_coordinates(hippo::Rectangle *get);

  // Turns the projector's display off. Note that this will put the projector
  // in the standby ProjectorState, where the display is off but the projector
  // is initialized and waiting for commands. If the projector changes states
  // it will trigger one or more on_state notifications.
  uint64_t off(void);

  // Turns the projector on. If the projector changes states it will trigger
  // one or more on_state notifications.
  uint64_t on(void);


  // Calling this method without parameters acts as a Get request and returns
  // a SolidColor value indicating the status of solid_color in the projector.
  uint64_t solid_color(hippo::SolidColor *get);

  // This method can be used to put the projector in solid color mode:
  // displaying the specified solid color across the entire projected area.
  // Note that when the projector is in solid color mode the keystone setting
  // is ignored and the maximum area is projected.
  // Calling this method with a set parameter acts as a Set request and will
  // then trigger an on_solid_color notification.
  uint64_t solid_color(hippo::SolidColor set);

  // This method can be used to put the projector in solid color mode:
  // displaying the specified solid color across the entire projected area.
  // Note that when the projector is in solid color mode the keystone setting
  // is ignored and the maximum area is projected.
  // Calling this method with a set parameter acts as a Set request and returns
  // a value as a SolidColor in the get parameter that confirms the value set.
  // It will then trigger an on_solid_color notification.
  uint64_t solid_color(hippo::SolidColor set, hippo::SolidColor *get);

  // Returns a ProjectorState object indicating the current state of the
  // projector.
  uint64_t state(hippo::ProjectorState *get);

  // Calling this method without parameters acts as a Get request and returns
  // a bool value in the get parameter indicating if the projector is currently
  // in structured light mode.
  uint64_t structured_light_mode(bool *get);

  // When the projector is in structured light mode it will map the top
  // 1920x1080 pixels from the input image to the projected image 1:1 with no
  // scaling. The remainder of the input frame will be cropped, so the bottom
  // 200 pixels of a 1920x1280 input image will not be included in the
  // projected image.
  // Note that structured light mode is not maintained when turning the
  // projector display off and then on again.When the projector is turned on
  // from the standby ProjectorState, structured light mode will be false.
  // Calling this method with a set parameter acts as a Set request and will
  // trigger an on_structured_light_mode notification.
  uint64_t structured_light_mode(const bool set);

  // When the projector is in structured light mode it will map the top
  // 1920x1080 pixels from the input image to the projected image 1:1 with no
  // scaling. The remainder of the input frame will be cropped, so the bottom
  // 200 pixels of a 1920x1280 input image will not be included in the
  // projected image.
  // Note that structured light mode is not maintained when turning the
  // projector display off and then on again.When the projector is turned on
  // from the standby ProjectorState, structured light mode will be false.
  // Calling this method with a set parameter acts as a Set request and returns
  // a value as a bool in the get parameter that confirms the value set. It will
  // trigger an on_structured_light_mode notification.
  uint64_t structured_light_mode(const bool set, bool *get);

  // Subscribe to projector notifications
  uint64_t subscribe(void(*callback)(const ProjectorNotificationParam &param,
                                     void *data),
                     void *data);
  uint64_t subscribe(void(*callback)(const ProjectorNotificationParam &param,
                                     void *data),
                     void *data, uint32_t *get);

  // unsubscribe from notifications
  uint64_t unsubscribe();
  uint64_t unsubscribe(uint32_t *get);

  // A method to control the projector's target white point. The projector
  // supports several CIE Standard Illuminant values which can be selected by
  // name. It also supports a custom setting which can be modified by including
  // a value object in the WhitePoint parameter with the desired chromaticity
  // coordinates in the CIE 1931 color space. Details on the supported options
  // are included in the table below:
  //
  // Name         CIEx          CIEy        Color       Temp Description
  // d50          0.34567       0.35850     5003        D50 Horizon light
  // d65          0.31271       0.32902     6504        D65 Noon daylight
  // d75          0.29902       0.31485     7504        D75 North sky daylight
  // custom       0.31271*      0.32902*    N/A         Custom user writable
  //                                                    setting
  // (*) The coordinates in the custom section are populated by the firmware as
  // defaults, but the values can be overwritten by passing in a WhitePoint
  // type with the name set to custom and the desired chromaticity coordinates
  // in the value field.

  // Calling this method with a get parameter acts as a Get request and returns
  // the projector's current white point target as a WhitePoint object.
  uint64_t white_point(hippo::WhitePoint *get);

  // Calling this method with a set parameter acts as a Set request. On success
  // it will trigger an on_white_point notification. To specify a custom set of
  // chromaticity coordinates, provide a WhitePoint object with the name set
  // to custom and the value field set to the desired target white point. Note
  // that when setting the white point, the value field is optional. If only
  // the name is included, the specified white point target will still be
  // selected (using the most recently specified custom coordinate if the name
  // is set to custom). Also note that if the name is set to one of the
  // standard illuminant values (rather than custom), any coordinate values
  // provided in the value field are ignored, since the defined coordinate for
  // that illuminant is always used.
  uint64_t white_point(const hippo::WhitePoint &set);

  // Calling this method with a set parameter acts as a Set request. On success
  // it will return a WhitePoint in the get parameter that confirms the value
  // set and trigger an on_white_point notification. To specify a custom set of
  // chromaticity coordinates, provide a WhitePoint object with the name set
  // to custom and the value field set to the desired target white point. Note
  // that when setting the white point, the value field is optional. If only
  // the name is included, the specified white point target will still be
  // selected (using the most recently specified custom coordinate if the name
  // is set to custom). Also note that if the name is set to one of the
  // standard illuminant values (rather than custom), any coordinate values
  // provided in the value field are ignored, since the defined coordinate for
  // that illuminant is always used.
  uint64_t white_point(const hippo::WhitePoint &set, hippo::WhitePoint *get);

 protected:
  uint64_t calibrationData_json2c(void *obj, hippo::CalibrationData *cal);
  uint64_t dppversion_json2c(void *obj, hippo::DPPVersion *dppversion);
  uint64_t geoversion_json2c(void *obj, hippo::GeoFWVersion *geoversion);
  uint64_t hardwareInfo_json2c(void * obj, hippo::HardwareInfo *info);
  uint64_t keystone_json2c(void *obj, hippo::Keystone *get);
  uint64_t keystone_c2json(const hippo::Keystone &ks, void *obj);
  uint64_t ledtimes_json2c(void *obj, hippo::ProjectorLedTimes *ledtimes);
  uint64_t mfgData_json2c(void *obj, hippo::ManufacturingData *mfgdata);
  uint64_t projector_specific_info_json2c(void *obj,
                                      hippo::ProjectorSpecificInfo *info);
  uint64_t rectangle_json2c(void *obj, hippo::Rectangle *rect);
  uint64_t state_json2c(void *obj, hippo::ProjectorState *state);
  uint64_t solid_color_c2json(const hippo::SolidColor &color, void *obj);
  uint64_t solid_color_json2c(void *obj, hippo::SolidColor *color);
  uint64_t white_point_json2c(void *obj, hippo::WhitePoint *wp);
  uint64_t white_point_c2json(const hippo::WhitePoint &wp, void *obj);

  // helper functions for json parsing
  uint64_t parseCornersJson(void *jsonCorners, Corners *cCorners);
  uint64_t parseCornerJson(void *jsonCorner, PointFloats *cCorner);
  uint64_t parsePointJson(void *jsonPoint, Point *cPoint);

  // Callback items
  void ProcessSignal(char *method, void *obj) override;
  bool HasRegisteredCallback();

  void (*callback_)(const ProjectorNotificationParam &param, void *data);
};

}   // namespace hippo

#endif  // INCLUDE_PROJECTOR_H_
