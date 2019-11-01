
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#include <windows.h>   // for GetCurrentThreadId

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>

#include <string>
#include <unordered_map>
#include <mutex>     // NOLINT

#include "../include/hippo.h"


namespace hippo {

class HippoRand {
 public:
  static HippoRand& GetInstance(void) {
    static HippoRand instance;
    return instance;
  }
  uint16_t Rand() {
    return std::rand() % 0xffff + 1;
  }
 private:
  HippoRand() {
    // use current time as seed for random generator
    std::srand(static_cast<uint32_t>(std::time(nullptr)));
  }
  ~HippoRand() {
  }
};

std::unordered_map<int, std::string> errorMap;
const char* GetFileName(uint16_t file_id);

uint64_t MakeHippoError(HippoFacility facility, HippoError code,
                               uint16_t line, uint16_t fileId) {
  // we need to convert to an hex representation of the line
  // to be consistent with SoHal
  char line_str[6];
  (void)itoa(line, line_str, 10);
  uint32_t line_hex = strtol(line_str, NULL, 16);
  uint64_t data = ((int32_t)fileId << 16) | (line_hex & 0xffff);
  uint32_t err = ((0x20000000) |
                  (facility << 16) |
                  ((uint32_t)(code)));
  return (data << 32) | err;
}

const char* strerror() {
  std::unordered_map<int, std::string>::const_iterator t;
  t = errorMap.find(GetCurrentThreadId());
  if (t == errorMap.end()) {
    return "";
  }
  return t->second.c_str();
}

const char* strerror(uint64_t err) {
  std::unordered_map<int, std::string>::const_iterator t;
  t = errorMap.find(GetCurrentThreadId());
  if (t == errorMap.end()) {
    return HippoErrorMessage(err);
  }
  return t->second.c_str();
}

void strerror(uint64_t err, size_t bufsz, char *str) {
  uint16_t file_id = (err >> 48) & 0xffff;
  uint16_t line_no = (err >> 32) & 0xffff;
  uint16_t facility = (err >> 16) & 0x7ff;
  const char *file_name = NULL, *facility_name = NULL;
  // get the facility name
  switch (facility) {
    case HIPPO_DEVICE:
      facility_name = "hippo_device";
      break;
    case HIPPO_DESKLAMP:
      facility_name = "desklamp";
      break;
    case HIPPO_SBUTTONS:
      facility_name = "sbuttons";
      break;
    case HIPPO_TOUCHMAT:
      facility_name = "touchmat";
      break;
    case HIPPO_PROJECTOR:
      facility_name = "projector";
      break;
    case HIPPO_HIRESCAMERA:
      facility_name = "hirescamera";
      break;
    case HIPPO_DEPTHCAMERA:
      facility_name = "depthcamera";
      break;
    case HIPPO_CAPTURESTAGE:
      facility_name = "capturestage";
      break;
    case HIPPO_UVCCAMERA:
      facility_name = "uvccamera";
      break;
    case HIPPO_SOHAL:
      facility_name = "sohal";
      break;
    case HIPPO_SYSTEM:
      facility_name = "system";
      break;
    case HIPPO_WS:
      facility_name = "hippo_ws";
      break;
    case HIPPO_SWDEVICE:
      facility_name = "swdevice";
      break;
    default:
      facility_name = "unknown device";
      break;
  }
  // get the file name and message
  const char *msg = NULL;
  if (NULL == (file_name = GetFileName(file_id))) {
    // it's an internal SoHal file
    std::unordered_map<int, std::string>::const_iterator t;
    t = errorMap.find(GetCurrentThreadId());
    if (t == errorMap.end()) {
      msg = "";
    } else {
      msg = t->second.c_str();
    }
    char line_str[6];
    (void)itoa(line_no, line_str, 16);
    snprintf(str, bufsz, "<unknown_file>:%s (%s) '%s'",
             line_str, facility_name, msg);
  } else {
    // is hiPPo error message
    msg = HippoErrorMessage(err);
    snprintf(str, bufsz, "%s:%x (%s) '%s'", file_name, line_no,
             facility_name, msg);
  }
}

uint64_t setError(const char *errStr) {
  uint64_t err = 0LL;
  try {
    hippo::errorMap[GetCurrentThreadId()] = std::string(errStr);
  } catch (...) {
    err = -1;
  }
  return err;
}

uint64_t clearError() {
  return setError("");
}

static std::unordered_map <std::string, uint16_t> srcFileMap = {
  { "base64.cc", 0xbb64 },
  { "capturestage.cc", 0xbbc5 },
  { "depthcamera.cc", 0xbbdc },
  { "desklamp.cc", 0xbbd1 },
  { "hippo.cc", 0xbb00 },
  { "hippo_camera.cc", 0xbb01 },
  { "hippo_device.cc", 0xbb0d },
  { "hippo_swdevice.cc", 0xbb5d },
  { "hippo_ws.cc", 0xbb55 },
  { "hirescamera.cc", 0xbbea },
  { "projector.cc", 0xbb08 },
  { "projector_types.cc", 0xbb09 },
  { "sbuttons.cc", 0xbbb0 },
  { "sohal.cc", 0xbb0a },
  { "system.cc", 0xbb5e },
  { "system_types.cc", 0xbb5d },
  { "touchmat.cc", 0xbba1 },
  { "uvccamera.cc", 0xbbcc },
};

const char* GetFileName(uint16_t file_id) {
  std::unordered_map<std::string, uint16_t>::const_iterator file_it;
  for (file_it = srcFileMap.begin(); file_it != srcFileMap.end(); ++file_it) {
    if (file_it->second == file_id) {
      return file_it->first.c_str();
    }
  }
  return NULL;
}

void AddFileToFileMap(const char *path) {
  if (!path) {
    return;
  }
  const char *file_name = strrchr(path, '\\');
  if (!file_name) {
    file_name = path;
  } else {
    file_name++;
  }
  std::unordered_map<std::string, uint16_t>::const_iterator t;
  t = srcFileMap.find(file_name);
  if (t != srcFileMap.end()) {
    return;   // already registered
  }
  uint16_t id;
  do {
    id = HippoRand::GetInstance().Rand();
  } while (GetFileName(id));

  // fprintf(stderr, "*** %s %x size: %zd\n", file_name, id, srcFileMap.size());

  srcFileMap.insert(std::pair<std::string, uint16_t>(file_name, id));
}

uint16_t GetFileId(const char *path) {
  uint16_t file_not_found = 0xffff;
  if (!path) {
    return file_not_found;
  }
  const char *file_name = strrchr(path, '\\');
  if (!file_name) {
    file_name = path;
  } else {
    file_name++;
  }
  std::unordered_map<std::string, uint16_t>::const_iterator t;
  t = srcFileMap.find(file_name);
  if (t == srcFileMap.end()) {
    return file_not_found;
  }
  return t->second;
}

uint64_t CaptureLock(std::unique_lock<std::mutex> *lock,
                     HippoFacility facility) {
  try {
    lock->lock();
  } catch (std::system_error e) {
    return MAKE_HIPPO_ERROR(facility, HIPPO_ERROR);
  }
  return 0LL;
}

const char* HippoErrorMessage(uint64_t err) {
  const char *str = NULL;

  switch (HippoErrorCode(err)) {
    case HIPPO_ERROR:
      str = "Generic Error";
      break;
    case HIPPO_TIMEOUT:
      str = "Timeout error";
      break;
    case HIPPO_FUNC_NOT_AVAILABLE:
      str = "Functionality not available";
      break;
    case HIPPO_MESSAGE_ERROR:
      str = "Message error";
      break;
    case HIPPO_PARAM_OUT_OF_RANGE:
      str = "Out of Range Parameter";
      break;
    case HIPPO_CMD_LEN:
      str = "Command with unexpected length";
      break;
    case HIPPO_MEM_ALLOC:
      str = "Memory error";
      break;
    case HIPPO_STD_ERROR:
      str = "Generic std::exception";
      break;
    case HIPPO_DEV_NOT_FOUND:
      str = "Device not found";
      break;
    case HIPPO_DEV_INTERFACE_NOT_FOUND:
      str = "Device interface not found";
      break;
    case HIPPO_DEV_IN_USE:
      str = "Device in use";
      break;
    case HIPPO_OPEN:
      str = "Error opening device";
      break;
    case HIPPO_CLOSE:
      str = "Error closing device";
      break;
    case HIPPO_WRITE:
      str = "Error writing to device";
      break;
    case HIPPO_READ:
      str = "Error reading from device";
      break;
    case HIPPO_GET_FEATURE:
      str = "Error getting feature from device";
      break;
    case HIPPO_SET_FEATURE:
      str = "Error setting feature from device";
      break;
    case HIPPO_FLASH_ERROR:
      str = "Error when triggering flash";
      break;
    case HIPPO_MESSAGE_PROCESSING_ERROR:
      str = "The device was not able to complete the request";
      break;
    case HIPPO_DEVICE_BUSY:
      str = "Device is busy";
      break;
    case HIPPO_CANCEL:
      str = "Operation cancelled";
      break;
    case HIPPO_READ_LEN_ERROR:
      str = "Received incorrect number of bytes from the device";
      break;
    case HIPPO_WRITE_LEN_ERROR:
      str = "Wrote incorrect number of bytes to the device";
      break;
    case HIPPO_HW_FAULT:
      str = "hardware fault";
      break;
    case HIPPO_WRONG_STATE_ERROR:
      str = "Device is in the wrong state";
      break;
    case HIPPO_INVALID_PARAM:
      str = "Invalid parameter";
      break;
    case HIPPO_OVERCURRENT:
      str = "Device is using too much power";
      break;
    case HIPPO_STUCK:
      str = "Device is unable to move";
      break;
    case HIPPO_BROKEN_STOP:
      str = "Device's hard stop has broken off";
      break;
    case HIPPO_NO_OPTO_SENSOR:
      str = "Optical sensor is not responding";
      break;
    case HIPPO_NOT_CALIBRATED:
      str = "Device is not calibrated. Please home the device";
      break;
    case HIPPO_NOT_STREAMING:
      str = "The camera is not streaming";
      break;
    default:
      str = "Unknown hiPPo error code";
      break;
  }
  return str;
}

}  // namespace hippo
