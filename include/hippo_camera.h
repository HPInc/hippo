
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#ifndef INCLUDE_HIPPO_CAMERA_H_
#define INCLUDE_HIPPO_CAMERA_H_

#include "../include/hippo_device.h"
#include "../include/common_types.h"

#if COMPILING_DLL
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif

namespace hippo {

const uint32_t kMaxNumStreams = 4;

typedef union CameraStreams {
  uint8_t value;
  struct {
    // Represents the depth camera color (RGB_888) stream
    uint8_t color : 1;
    // Represents the depth camera depth (DEPTH_MM) stream
    uint8_t depth : 1;
    // Represents the depth camera infrared (GRAY_16 or GRAY8) stream
    uint8_t ir : 1;
    // Represents the depth camera XYZ points in world coordinates
    // (float32 in millimeters ) stream
    // Note: In order to have the points stream enabled, the
    // depth stream must also be enabled.
    uint8_t points : 1;
  };
} CameraStreams;

typedef struct EnableStream {
  //  The port where the frame streaming server is listening.
  // If no streams are currently enabled, the streaming server
  // will not be open and this port value will be set to -1.
  uint32_t port;
  //  A list of streams currently enabled
  CameraStreams streams;
} EnableStream;

// This block is to allow sending per-frame parameters to enabled filters.
typedef struct FilterParameters {
  // Length of the block in bytes.
  uint16_t len;
  // currently 1
  uint8_t version;
  // Bit representing the filter mask returned by enable_filter
  uint8_t command;
} FilterParameters;

typedef enum class PixelFormat : uint8_t {
  PIXEL_UNKNOWN = 0,
  PIXEL_GRAY16 = 1,           // 2 bytes
  PIXEL_RGB888 = 2,           // 3 bytes
  PIXEL_YUV422 = 3,
  PIXEL_YUYV = 4,
  PIXEL_GRAY8 = 5,            // 1 byte
  PIXEL_DEPTH_MM16 = 6,       // 2 bytes
  PIXEL_BGRA8888 = 7,         // 4 bytes
  PIXEL_POINTS_MM32F = 8,     // 12 bytes
  PIXEL_YUY2 = 9,     // it's a 4:2:2 format @ 16 bpp (used in Flick)
  PIXEL_UYVY = 10,    // it's a 4:2:2 format @ 16 bpp
  PIXEL_NV12 = 11,    // it's a 4:0:0 format @ 12 bpp
} PixelFormat;

// The binary structure that needs to be sent to request a camera frame.
typedef struct FrameCommand {
  // 0x50, 0xa1
  uint8_t magic[2];
  // 0xde, 0xca
  uint8_t device[2];
  // currently 1
  uint8_t version;
  // Bit-mask:
  //    FRAME_SYNC: 0x00: The streaming server will wait for the next available
  //    frame before returning
  //    FRAME_ASYNC: 0x01: The streaming server will return immediately with the
  //    latest frame available.
  //    Other bits enabling filters received as return values from enable_filter
  uint8_t command;
  // The requested stream(s).
  CameraStreams stream;
  // Number of FilterParameters blocks after this FrameCommand.
  // Note that we currently don't have any filter that requires parameters,
  // so this should be set to 0 (zero).
  uint8_t num_params;
} FrameCommand;

// Header of each image frame returned by the depth camera. SoHal does support
// grabbing multiple streams at the same time, so we have this 'overall' frame
// header (FrameHeader) and a 'per stream' header (StreamHeader). If an error
// occurs while grabbing frames, the error field in the FrameHeader will be
// non-zero and the following StreamHeader will be replaced by an ErrorCode
// (note that both are 128 bits, so a frame will always be at least 128 bits
// long).
typedef struct FrameHeader {
  // 0x50, 0xa1
  uint8_t magic[2];
  // 0xde, 0xca
  uint8_t device[2];
  // currently 1
  uint8_t version;
  // the provided stream(s)
  CameraStreams stream;
  // A value of 0 means no error, otherwise an error occurred.
  uint8_t error;
  // reserved
  uint8_t reserved;
} FrameHeader;

typedef struct StreamHeader {
  // Frame width in pixels
  uint16_t width;
  //  Frame height in pixels
  uint16_t height;
  // Frame index
  uint16_t index;
  // describes which type of frame this is.  Will be
  // either color, depth, ir, or points
  CameraStreams stream;
  // format of the current frame
  PixelFormat format;
  // Timestamp when the frame was actually captured with microsecond resolution.
  uint64_t timestamp;
} StreamHeader;

// This is a 128-bit value encoding the actual error code
// sthat occurred while grabbing frames
typedef struct ErrorCode {
  uint64_t code;      // Error code
  char id[7];         // SoHal's version ID.
  uint8_t reserved;   // Reserved
} ErrorCode;

// Header of each image frame returned by the depth camera.
// SoHal does support grabbing multiple streams at the same time,
// so we have this 'overall' frame header (FrameHeader) and a
// 'per stream' header (StreamHeader). If an error occurs while
// grabbing frames, the error field in the FrameHeader will be non-zero
// and the following StreamHeader will be replaced by an ErrorCode (note
// that both are 128 bits, so a frame will always be at least 128 bits long).
typedef struct FrameHeaderData {
  union {
    StreamHeader *header;
    ErrorCode *error;
  };
  uint8_t *data;
} FrameHeaderData;

typedef struct CameraFrame {
  FrameHeader *header;
  FrameHeaderData streams[kMaxNumStreams];
  // raw_data_ is where all members of this type point to.
  // this is the only array that needs to be malloc'd/free'd
  uint8_t *raw_data_;
  // raw_length is the total length of the raw_data pointer
  size_t raw_length_;
} CameraFrame;


// Implements functionality that is available for all cameras
// (UVC) cameras.
class DLLEXPORT HippoCamera : public HippoDevice {
 public:
  /* HippoCamera(); */
  /* explicit HippoCamera(uint32_t device_index); */
  /* HippoCamera(const char *address, uint32_t port); */
  /* HippoCamera(const char *address, uint32_t port, uint32_t device_index); */

  HippoCamera(const char *dev, const char *address, uint32_t port,
              HippoFacility facility, uint32_t device_index);
  virtual ~HippoCamera(void);

  // This method returns the 'index' of the uvccamera within the operating
  // system video input devices
  uint64_t camera_index(uint32_t *get);

  // Calling this method with a get parameter acts as a Get request and returns
  // an EnableStream object with the port number for the frame streaming
  // server and the list of streams currently enabled.
  // If there are not any streams enabled when this method is called, the frame
  // streaming server will not be running and the EnableStream object will
  // list the port number as -1.
  uint64_t enable_streams(EnableStream *get);

  // Calling this method with a set parameter will enable the
  // specified depthcam streams. On success, SoHAL will create a new WebSocket
  // server in the same host but using a different port number. In order to
  // grab frames from the camera, the client should connect to the new port and
  // send binary commands as specified in the FrameCommand type.
  // On success it will trigger an on_enable_streams notification.
  //
  // Note: Due to hardware limitations, when using Sprout Pro G2 and
  // HP Z 3D Camera depth cameras the color and ir streams cannot be enabled at
  // the same time. An error will be returned if both are enabled at once. This
  // is not an issue on Sprout and Sprout Pro depth cameras.
  //
  // Note: The points stream needs the depth stream to be enabled. Enabling the
  // points stream without having depth enabled will return a Device is in the
  // wrong state error.
  //
  // Note: Due to the technology used in this camera, the laser has to be on in
  // order to get the depth data. This means that when streaming ir and depth
  // at the same time, the ir frame will have a dot pattern projected by the
  // laser.
  //
  // Note: For a description of the hardware behavior for the laser see the
  // SoHAL depthcamera documentation.
  uint64_t enable_streams(const CameraStreams &set);

  // Calling this method with a set parameter will enable the specified depth
  // camera streams
  // The return value in the get variable of this method is an EnableStream
  // object with the frame streaming server port number and the list of streams
  // currently enabled.
  // On success it will trigger an on_enable_streams notification.
  //
  // Note: The points stream needs the depth stream to be enabled. Enabling the
  // points stream without having depth enabled will return a Device is in the
  // wrong state error.
  //
  // Note: Due to the technology used in this camera, the laser has to be on in
  // order to get the depth data. This means that when streaming ir and depth
  // at the same time, the ir frame will have a dot pattern projected by the
  // laser.
  //
  // Note: For a description of the hardware behavior for the laser see the
  // SoHAL depthcamera documentation.
  uint64_t enable_streams(const CameraStreams &set, EnableStream *get);

  // Calling this method with a get parameter acts as a Get request and
  // returns a CameraStreams object with the list of streams currently
  // enabled
  uint64_t disable_streams(CameraStreams *get);

  // Calling this method with a set parameter will disable
  // the specified depth camera streams. If all open streams have been
  // disabled, this will close the image streaming server. On success
  // SoHal will send an on_disable_streams notification to all subscribed
  // clients.
  uint64_t disable_streams(const CameraStreams &set);

  // Calling this method with a set parameter will disable
  // the specified depth camera streams. If all open streams have been
  // disabled, this will close the image streaming server. On success
  // SoHal will send an on_disable_streams notification to all subscribed
  // clients. The get parameter will contain the response, and will include
  // an object with a list of the streams that remained enabled
  uint64_t disable_streams(const CameraStreams &set, CameraStreams *get);

  //**************************************************************************
  // TODO(HP Developers): implement the enable_filter function
  //**************************************************************************
  // enable_filter currently returns HIPPO_FUNC_NOT_AVAILABLE
  uint64_t enable_filter();

  // Grab a frame from the specified stream and place it in the memory
  // passed in the frame variable
  uint64_t grab_frame(const CameraStreams &streams, CameraFrame *frame);
  uint64_t grab_frame(const FrameCommand &cmd, CameraFrame *frame);
  uint64_t grab_frame(const FrameCommand &cmd,
                      const FilterParameters *param,
                      CameraFrame *frame);

  uint64_t grab_frame_async(const CameraStreams &streams, CameraFrame *frame);

  // returns the bytes per pixel for the passed in pixel format
  uint32_t BitsPerPixel(PixelFormat format);

 protected:
  HippoWS *wsFrames_;

  bool IsConnectedFrames();
  uint64_t EnsureConnectedFrames(uint32_t port);
  uint64_t ConnectFrames(uint32_t port);
  void DisconnectFrames();

  uint64_t CameraStreams_c2json(const CameraStreams &set, void *obj);
  uint64_t CameraStreams_json2c(const void *obj, CameraStreams *get);

  uint64_t EnableStream_json2c(const void *obj, EnableStream *get);

  size_t GetDataLen(const StreamHeader *header);
};

}  // namespace hippo

#endif   // INCLUDE_HIPPO_CAMERA_H_
