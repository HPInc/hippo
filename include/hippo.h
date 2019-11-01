
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#ifndef INCLUDE_HIPPO_H_
#define INCLUDE_HIPPO_H_

#include <stdint.h>

#if COMPILING_DLL
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif


namespace hippo {

typedef enum HippoError {
  /// 0x000, no error, equivalent to S_OK
  HIPPO_OK = 0,
  /// 0x200, generic hiPPo error
  HIPPO_ERROR = 0x200,
  /// 0x201, a request timed out
  HIPPO_TIMEOUT,
  /// 0x202, a non-implemented function was called
  HIPPO_FUNC_NOT_AVAILABLE,
  /// 0x203, a generic non-specific error was found in the return by the device
  ///        while processing a request
  HIPPO_MESSAGE_ERROR,
  /// 0x204, a parameter out of the acceptable range was passed in
  HIPPO_PARAM_OUT_OF_RANGE,
  /// 0x205, deprecated
  HIPPO_CMD_LEN,
  /// 0x206, memory could not be allocated
  HIPPO_MEM_ALLOC,
  /// 0x207, deprecated
  HIPPO_STD_ERROR,
  /// 0x208, a request was sent to a device that is not open
  HIPPO_DEV_NOT_FOUND,
  /// 0x209, deprecated
  HIPPO_DEV_INTERFACE_NOT_FOUND,
  /// 0x20a, a connection could not be established with the device
  ///        because another connection is already open
  HIPPO_DEV_IN_USE,
  /// 0x20b, an error happened while opening a connection with the device
  HIPPO_OPEN,
  /// 0x20c, an error happened while closing a connection with the device
  HIPPO_CLOSE,
  /// 0x20d, the write pipe of the connection was not available
  HIPPO_WRITE,
  /// 0x20e, the read pipe of the connection was not available
  HIPPO_READ,
  /// 0x20f, a call to HidD_GetFeature() failed
  HIPPO_GET_FEATURE,
  /// 0x210, a call to HidD_SetFeature() failed
  HIPPO_SET_FEATURE,
  /// 0x212, an error occurred during HW flashing of the projector
  HIPPO_FLASH_ERROR,
  /// 0x212, the device was not able to complete the request
  HIPPO_MESSAGE_PROCESSING_ERROR,
  /// 0x213, the device cannot complete the request until it completes
  ///        a previous request
  HIPPO_DEVICE_BUSY,
  /// 0x214, used internally as cancel event
  HIPPO_CANCEL,
  /// 0x215, read from the device less bytes than expected
  HIPPO_READ_LEN_ERROR,
  /// 0x216, read from the device less bytes than expected
  HIPPO_WRITE_LEN_ERROR,
  /// 0x217, read from the device less bytes than expected
  HIPPO_HW_FAULT,
  /// 0x218, the device is in the wrong state to complete the request
  HIPPO_WRONG_STATE_ERROR,
  /// 0x219, invalid parameter
  HIPPO_INVALID_PARAM,
  /// 0x21a, the device is drawing too much power
  HIPPO_OVERCURRENT,
  /// 0x21b, the device is unable to move
  HIPPO_STUCK,
  /// 0x21c, the hard stop has broken off of the device
  HIPPO_BROKEN_STOP,
  /// 0x21d, the optical sensor is not responding
  HIPPO_NO_OPTO_SENSOR,
  /// 0x21e, The device is not calibrated (home command required)
  HIPPO_NOT_CALIBRATED,
  /// 0x21f, The camera is not streaming
  HIPPO_NOT_STREAMING,
} HippoError;


typedef enum HippoFacility {
  /// 0x13b, generic device
  HIPPO_DEVICE = 0x13b,
  /// 0x13c, the desklamp device
  HIPPO_DESKLAMP,
  /// 0x13d, the sButtons device
  HIPPO_SBUTTONS,
  /// 0x13e, the touchMat device
  HIPPO_TOUCHMAT,
  /// 0x13f, the projector device
  HIPPO_PROJECTOR,
  /// 0x140, the high resolution camera device
  HIPPO_HIRESCAMERA,
  /// 0x141, the depth camera device
  HIPPO_DEPTHCAMERA,
  /// 0x142, the capture stage device
  HIPPO_CAPTURESTAGE,
  /// 0x143, the uvc camera device
  HIPPO_UVCCAMERA,
  /// 0x160, the SoHal device
  HIPPO_SOHAL = 0x160,
  /// 0x161 the System device
  HIPPO_SYSTEM = 0x161,
  /// 0x162 the WS device
  HIPPO_WS = 0x162,
  /// 0x163 generic sw devices
  HIPPO_SWDEVICE = 0x163,
} HippoFacility;


DLLEXPORT uint16_t GetFileId(const char *fileName);

// we use macros to ensure the line corresponds to the caller
#define MAKE_HIPPO_ERROR(facility, code)       \
  MakeHippoError((facility), (code), (uint16_t)__LINE__, \
                 hippo::GetFileId(__FILE__))

#define ADD_FILE_TO_MAP()       \
  hippo::AddFileToFileMap(__FILE__)

DLLEXPORT uint64_t MakeHippoError(HippoFacility facility, HippoError code,
                                  uint16_t line, uint16_t fileId);

DLLEXPORT inline HippoError HippoErrorCode(uint64_t err) {
  return static_cast<HippoError>(err & 0xffff);
}

DLLEXPORT const char* HippoErrorMessage(uint64_t err);

// will return an error message string containing the error code message
DLLEXPORT const char* strerror(uint64_t err);

// will return a full error message string containing the file name,
// line number, facility (device) and error code message
DLLEXPORT void strerror(uint64_t err, size_t bufsz, char *str);

// for backwards compatibility, will print SoHal error messages but
// will not print hiPPo error messages
DLLEXPORT const char* strerror();

// will add the given filename to the hiPPo file map in order to report
// the right file in error messages if the error is generated using
// hiPPo's APIs
DLLEXPORT void AddFileToFileMap(const char *name);

}  // namespace hippo


#endif   // INCLUDE_HIPPO_H_
