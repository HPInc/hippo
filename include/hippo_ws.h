
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#ifndef INCLUDE_HIPPO_WS_H_
#define INCLUDE_HIPPO_WS_H_

#include "../include/hippo.h"

#if COMPILING_DLL
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif


namespace hippo {

class HippoLWS;

typedef enum class WsConnectionType {
  TEXT = 0,
  BINARY = 1,
} WsConnectionType;

class DLLEXPORT HippoWS {
 public:
  explicit HippoWS(HippoFacility facility);
  ~HippoWS(void);

  uint64_t Connect(const char *host, uint32_t port, WsConnectionType type,
                   int timeout);
  uint64_t Connect(const char *host, uint32_t port, WsConnectionType type,
                   uint32_t rx_buffer_size, int timeout);
  uint64_t Disconnect();
  bool Connected();

  uint64_t SendRequest(const unsigned char *request, WsConnectionType type);
  uint64_t SendRequest(const unsigned char *request, WsConnectionType type,
                       int timeout, unsigned char **response);
  uint64_t SendRequest(const unsigned char *request, size_t req_len,
                       WsConnectionType type, int timeout,
                       unsigned char **response, size_t *res_len);

  uint64_t StopSignalLoop();
  uint64_t WaitForSignal(unsigned char **response);
  uint64_t ReadResponse(unsigned char **response);

 protected:
  HippoFacility facility_;

  HippoLWS *hlws_;
};

}   // namespace hippo

#endif   // INCLUDE_HIPPO_WS_H_
