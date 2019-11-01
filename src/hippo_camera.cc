
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <mutex>   // NOLINT
#include "../include/hippo_camera.h"
#include "../include/hippo_ws.h"
#include "../include/json.hpp"

namespace nl = nlohmann;

namespace hippo {

extern std::mutex gHippoDeviceMutex;
const char devName[] = "camera";
extern const char *defaultHost;
extern uint32_t defaultPort;


HippoCamera::HippoCamera(const char *dev, const char *address, uint32_t port,
                         HippoFacility facility, uint32_t device_index) :
    HippoDevice(dev, address, port, facility, device_index),
    wsFrames_(NULL) {
}

HippoCamera::~HippoCamera(void) {
  if (wsFrames_) {
    DisconnectFrames();
  }
}

bool HippoCamera::IsConnectedFrames() {
  return static_cast<bool>(NULL != wsFrames_);
}

uint64_t HippoCamera::EnsureConnectedFrames(uint32_t port) {
  uint64_t err = 0LL;
  if (!IsConnectedFrames()) {
    err = ConnectFrames(port);
  }
  return err;
}

uint64_t HippoCamera::ConnectFrames(uint32_t port) {
  if (NULL == (wsFrames_ = new (std::nothrow)HippoWS(facility_))) {
    return MAKE_HIPPO_ERROR(facility_,
                            HIPPO_MEM_ALLOC);
  }
  return wsFrames_->Connect(host_, port, WsConnectionType::BINARY, 1024);
}

void HippoCamera::DisconnectFrames() {
  if (IsConnectedFrames()) {
    if (wsFrames_) {
      (void)wsFrames_->Disconnect();
      delete wsFrames_;
      wsFrames_ = NULL;
    }
  }
}

uint64_t HippoCamera::camera_index(uint32_t *get) {
  return uint32_get("camera_index", get);
}

uint64_t HippoCamera::enable_streams(const CameraStreams &set) {
  return enable_streams(set, NULL);
}

uint64_t HippoCamera::enable_streams(EnableStream *get) {
  uint64_t err = 0LL;
  nl::json j;
  void *jptr = reinterpret_cast<void*>(&j);

  if (NULL == get) {
    MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  if (err = SendRawMsg("enable_streams", jptr)) {
    return err;
  }
  if (err = EnableStream_json2c(jptr, get)) {
    return err;
  }
  return 0LL;
}

uint64_t HippoCamera::enable_streams(const CameraStreams &set,
                                     EnableStream *get) {
  uint64_t err = 0LL;
  nl::json jset, jget;
  void *jget_ptr = reinterpret_cast<void*>(&jget);
  void *jset_ptr = reinterpret_cast<void*>(&jset);

  if (err = CameraStreams_c2json(set, jset_ptr)) {
    return err;
  }
  if (err = SendRawMsg("enable_streams", jset_ptr, jget_ptr)) {
    return err;
  }
  EnableStream st;
  if (err = EnableStream_json2c(jget_ptr, &st)) {
    return err;
  }
  if (NULL != get) {
    memcpy(get, &st, sizeof(st));
  }
  if (err = EnsureConnectedFrames(st.port)) {
    return err;
  }
  return 0LL;
}

uint64_t HippoCamera::disable_streams(const CameraStreams &set) {
  return disable_streams(set, NULL);
}

uint64_t HippoCamera::disable_streams(CameraStreams *get) {
  uint64_t err = 0LL;
  nl::json j;
  void *jptr = reinterpret_cast<void*>(&j);

  if (NULL == get) {
    MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  if (err = SendRawMsg("disable_streams", jptr)) {
    return err;
  }
  if (err = CameraStreams_json2c(jptr, get)) {
    return err;
  }
  return 0LL;
}

uint64_t HippoCamera::disable_streams(const CameraStreams &set,
                                      CameraStreams *get) {
  uint64_t err = 0LL;
  nl::json jset, jget;
  void *jget_ptr = reinterpret_cast<void*>(&jget);
  void *jset_ptr = reinterpret_cast<void*>(&jset);

  if (err = CameraStreams_c2json(set, jset_ptr)) {
    return err;
  }
  if (err = SendRawMsg("disable_streams", jset_ptr, jget_ptr)) {
    return err;
  }
  CameraStreams st;
  if (err = CameraStreams_json2c(jget_ptr, &st)) {
    return err;
  }
  if (NULL != get) {
    memcpy(get, &st, sizeof(st));
  }
  if (wsFrames_ && (0 == st.value)) {
    DisconnectFrames();
  }
  return 0LL;
}

uint64_t HippoCamera::enable_filter() {
  return MAKE_HIPPO_ERROR(facility_, HIPPO_FUNC_NOT_AVAILABLE);
}

uint64_t HippoCamera::CameraStreams_c2json(const CameraStreams &set,
                                           void *obj) {
  // test inputs to ensure non-null pointers
  if (obj == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  uint64_t err = 0LL;
  const int KEYS_LEN = 4;
  char *keys[KEYS_LEN] = { "color", "depth", "ir", "points" };
  nl::json params;

  for (int j = 1, k = 0; k <= KEYS_LEN; j<<=1, k++) {
    if (set.value & j) {
      params.push_back(keys[k]);
    }
  }
  // json['params'] is sent as a list containing the object
  reinterpret_cast<nl::json*>(obj)->push_back(params);

  return err;
}

uint64_t HippoCamera::CameraStreams_json2c(const void *obj,
                                           CameraStreams *get) {
  // test inputs to ensure non-null pointers
  if (obj == NULL || get == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  uint64_t err = 0LL;
  const int KEYS_LEN = 4;
  char *keys[KEYS_LEN] = { "color", "depth", "ir", "points" };
  const nl::json *streams = reinterpret_cast<const nl::json*>(obj);

  if (!streams->is_array()) {
    return MAKE_HIPPO_ERROR(facility_,
                            HIPPO_MESSAGE_ERROR);
  }
  get->value = 0;
  for (std::size_t i = 0; i < streams->size(); i++) {
    const nl::json *tmp = &streams->at(i);
    if (tmp->is_string()) {
      bool found = false;
      std::string name = tmp->get<std::string>();
      for (int j = 1, k = 0; k < KEYS_LEN; j<<=1, k++) {
        if (!strncmp(name.c_str(), keys[k], strlen(keys[k]))) {
          get->value |= j;
          found = true;
          break;
        }
      }
      if (!found) {
        return MAKE_HIPPO_ERROR(facility_,  HIPPO_MESSAGE_ERROR);
      }
    } else {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
    }
  }
  return err;
}

uint64_t HippoCamera::EnableStream_json2c(const void *obj, EnableStream *get) {
  // test inputs to ensure non-null pointers
  if (obj == NULL || get == NULL) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  // const int KEYS_LEN = 4;
  // char *keys[KEYS_LEN] = { "color", "depth", "ir", "points" };
  const nl::json *jptr = reinterpret_cast<const nl::json*>(obj);
  const nl::json *streams;

  try {
    get->port = jptr->at("port").get<uint32_t>();
    streams = &(jptr->at("streams"));
  } catch (nl::json::exception) {     // out_of_range or type_error
    return MAKE_HIPPO_ERROR(facility_, HIPPO_INVALID_PARAM);
  }
  return CameraStreams_json2c(reinterpret_cast<const void*>(streams),
                              &get->streams);
}

uint64_t HippoCamera::grab_frame_async(const CameraStreams &streams,
                                       CameraFrame *frame) {
  FrameCommand cmd = { { 0x50, 0xa1 }, { 0xde, 0xca }, 1, 0, 0, 0 };
  cmd.command = 1;
  cmd.stream.value = streams.value;

  return grab_frame(cmd, frame);
}

uint64_t HippoCamera::grab_frame(const CameraStreams &streams,
                                 CameraFrame *frame) {
  FrameCommand cmd = { { 0x50, 0xa1 }, { 0xde, 0xca }, 1, 0, 0, 0 };
  cmd.stream.value = streams.value;

  return grab_frame(cmd, frame);
}

uint64_t HippoCamera::grab_frame(const FrameCommand &cmd, CameraFrame *frame) {
  if (0 != cmd.num_params) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_PARAM_OUT_OF_RANGE);
  }
  return grab_frame(cmd, NULL, frame);
}

uint64_t HippoCamera::grab_frame(const FrameCommand &cmd,
                                 const FilterParameters *param,
                                 CameraFrame *frame) {
  uint64_t err = 0;
  size_t res_len = 0;
  unsigned char *response = NULL;

  int timeout = 10;
  if (err = wsFrames_->SendRequest(reinterpret_cast<const unsigned char*>(&cmd),
                                   sizeof(FrameCommand),
                                   WsConnectionType::BINARY, timeout,
                                   &response, &res_len)) {
    return err;
  }
  if (res_len < sizeof(FrameHeader) + sizeof(StreamHeader)) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_MESSAGE_ERROR);
  }
  if (NULL == frame->raw_data_) {
    frame->raw_data_ = reinterpret_cast<uint8_t*>(response);
    frame->raw_length_ = res_len;
  } else {
    memcpy(frame->raw_data_, response, res_len);
  }
  frame->header = reinterpret_cast<FrameHeader*>(frame->raw_data_);

  size_t idx = sizeof(FrameHeader);
  if (frame->header->error) {
    frame->streams[0].error =
        reinterpret_cast<ErrorCode*>(frame->raw_data_ + idx);
  } else {
    for (uint32_t i = frame->header->stream.value, ii = 0;
         i > 0;
         i >>= 1, ii++) {
      if (i & 0x01) {
        // asign the Stream header
        frame->streams[ii].header =
            reinterpret_cast<StreamHeader*>(frame->raw_data_ + idx);
        idx += sizeof(StreamHeader);
        size_t size = GetDataLen(frame->streams[ii].header);
        // size_t size = (frame->streams[ii].header->width *
        //                frame->streams[ii].header->height *
        //                BytesPerPixel(static_cast<PixelFormat>(
        //                    frame->streams[ii].header->format)));
        // asign the data
        frame->streams[ii].data = reinterpret_cast<uint8_t*>(response + idx);
        idx += size;
      }
    }
  }
  if (response != reinterpret_cast<unsigned char*>(frame->raw_data_)) {
    free(response);
  }
  return 0LL;
}

size_t HippoCamera::GetDataLen(const StreamHeader *header) {
  const uint32_t BITS_PER_BYTE = 8;

  size_t bits = (header->width *
                 header->height *
                 BitsPerPixel(static_cast<PixelFormat>(header->format)));

  return static_cast<size_t>(bits/BITS_PER_BYTE);
}

uint32_t HippoCamera::BitsPerPixel(PixelFormat format) {
  switch (format) {
    case PixelFormat::PIXEL_UNKNOWN:
      return 0;
    case PixelFormat::PIXEL_GRAY16:
      return 16;
    case PixelFormat::PIXEL_RGB888:
      return 24;
      // case PIXEL_YUV422:
      //   return ;
    // case PIXEL_YUYV:       // it's a 4:2:2 format @ 16 bpp
    //   return 2;
    case PixelFormat::PIXEL_GRAY8:
      return 8;
    case PixelFormat::PIXEL_DEPTH_MM16:
      return 16;
    case PixelFormat::PIXEL_BGRA8888:
      return 32;
    case PixelFormat::PIXEL_POINTS_MM32F:
      return 12*8;
    case PixelFormat::PIXEL_YUY2:
      return 16;
    case PixelFormat::PIXEL_UYVY:
      return 16;
    case PixelFormat::PIXEL_NV12:
      return 12;
  }
  return 0;
}

}  // namespace hippo
