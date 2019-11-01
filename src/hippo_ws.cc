
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <libwebsockets.h>

#include <mutex>     // NOLINT
#include <thread>    // NOLINT
#include <atomic>    // NOLINT
#include <set>

#include "../include/hippo_ws.h"

#undef VERBOSE
#undef VERBOSE_MSG

namespace hippo {

const char *kCloseConnectionStr = "close_connection";

extern uint64_t CaptureLock(std::unique_lock<std::mutex> *lock,
                            HippoFacility facility);

static int ws_callback(struct lws *wsi, enum lws_callback_reasons reason,
                       void *user, void *in, size_t len);

class HippoLWS;
class WsContext;

static struct lws_protocols protocols_[] = { {
    // name: protocol name - very important that matches SoHal
    "SoHal-jsonrpc",
    // callback
    ws_callback,
    // per_session_data_size
    0,
    // rx_buffer_size: it must be >= permessage-deflate rx size
    // dumb-increment only sends very small packets, so we set
    // this accordingly.  If your protocol will send bigger
    // things, adjust this to match
    512 + 640*480*3,
    // id
    static_cast<uint32_t>(LWS_WRITE_TEXT),
    // user
    NULL,   // reinterpret_cast<void*>(&UserData0),
    // tx_packet_size
    0,
  }, {
    // name: protocol name - very important that matches SoHal
    "SoHal-binary",
    // callback
    ws_callback,
    // per_session_data_size
    0,
    // rx_buffer_size: it must be >= permessage-deflate rx size
    // dumb-increment only sends very small packets, so we set
    // this accordingly.  If your protocol will send bigger
    // things, adjust this to match
    512 + 4352*3264*2,
    // id
    static_cast<uint32_t>(LWS_WRITE_BINARY),
    // user
    NULL,   // reinterpret_cast<void*>(&UserData1),
    // tx_packet_size
    0,
  }, {
    NULL, NULL, 0, 0, 0, NULL, 0
  },   /* End of list */
};

// returns the next multiple of 'mult' of the input 'value'
size_t NextMultiple(size_t mult, size_t value) {
  return (mult + value - 1) & ~(mult-1);
}

//
// class to store the 'request' command to be sent over websockets
//
class WsRequest {
 public:
  WsRequest() :
      data_len_(0), ptr_len_(0), data_(NULL), type_(LWS_WRITE_TEXT) {
  }

  ~WsRequest() {
    delete(data_);
  }

  HippoError SetData(const unsigned char *request, size_t len,
                     WsConnectionType type) {
    type_ = (type == WsConnectionType::TEXT) ?
        LWS_WRITE_TEXT : LWS_WRITE_BINARY;
    if (len > ptr_len_) {
      // We need to reserve LWS_PRE space for the libws message header
      // rounded up to the next multiple of 128 bytes
      ptr_len_ = NextMultiple(128, len);
      if (NULL == (data_ = reinterpret_cast<unsigned char*>(
              realloc(data_, LWS_PRE + ptr_len_)))) {
        return HIPPO_MEM_ALLOC;
      }
    }
    // leave LWS_PRE bytes for the lws library to fill up with protocol stuff
    (void)memcpy(data_+LWS_PRE, request, len);
    data_len_ = len;

    return HIPPO_OK;
  }

  // returns the data and sets the len to 0
  void GetData(unsigned char **request, size_t *len,
               lws_write_protocol *type) {
    *request = data_;
    *len = data_len_;
    *type = type_;
    data_len_ = 0;
  }

 private:
  size_t data_len_, ptr_len_;
  unsigned char *data_;
  lws_write_protocol type_;
};

//
// class to store the ws response (fragmented messages if needed)
//
class WsResponse {
 public:
  WsResponse() :
      data_len_(0), ptr_len_(0), data_(NULL), received_(false) {
  }

  ~WsResponse() {
    delete(data_);
  }

  void Init() {
    received_ = false;
    data_len_ = 0;
  }

  int SetData(const char *in, size_t len, bool finalFragment) {
    if (data_len_+len > ptr_len_) {
      ptr_len_ = NextMultiple(128, data_len_+len);
      if (NULL == (data_ = (unsigned char*)realloc(data_, ptr_len_))) {
        return HIPPO_MEM_ALLOC;
      }
    }
    memcpy(data_+data_len_, in, len);
    data_len_ += len;
    if (finalFragment) {
      received_ = true;
    }
    return 0;
  }

  HippoError GetData(unsigned char **data, size_t *len) {
    if (!received_) {
      return HIPPO_READ;
    }
    // always reallow to size+1 so in text mode we can add a \0
    *data = (unsigned char*)realloc(NULL, data_len_+1);
    memcpy(*data, data_, data_len_);
    *len = data_len_;
    return HIPPO_OK;
  }

  bool Received() {
    return received_;
  }

 private:
  size_t data_len_, ptr_len_;
  unsigned char *data_;
  bool received_;
};


//
// struct containing per socket connection information to be able to
// link the lws socket to the actual client sending the command
//
struct ClientData {
  HippoLWS *hlws_;
  WsRequest request_;
  WsResponse response_;
};

//
// class containing the low level logic/members for LWS implementation
// the class member functions are defined below
//
class HippoLWS {
 public:
  explicit HippoLWS(HippoFacility facility);
  ~HippoLWS();

  int ClientClosed();
  int Established();
  int ClientConnectionError();
  int Writable();
  int Receive(const char *in, size_t len);

  uint64_t Connect(const char *host, int port,
                   const char *protocol_name, int timeout);
  uint64_t Disconnect();
  uint64_t StopSignalLoop(void);
  uint64_t SendRequest(const unsigned char *request, size_t req_len,
                       WsConnectionType type,
                       unsigned char **response, size_t *res_len);
  uint64_t SendRequest(const unsigned char *request, size_t req_len,
                       WsConnectionType type, int timeout,
                       unsigned char **response, size_t *res_len);

  uint64_t Read(unsigned char **response, size_t *len, int timeout);
  uint64_t Read_p(std::unique_lock<std::mutex> *lock,
                  unsigned char **response, size_t *len, int timeout);

  bool Connected(void);

 private:
  HippoFacility facility_;
  ClientData client_data_;
  struct lws *lws_;

  std::mutex ws_mutex_;
  std::condition_variable ws_condition_;

  std::atomic<bool> connected_;
  std::atomic<bool> cancel_read_;
};

//
// Singleton to manage the single lws context across all connections
//
class WsContext {
 public:
  static WsContext& GetInstance(void) {
    static WsContext instance;
    return instance;
  }

  // called from a user thread
  uint64_t Connect(const char *host, int port, const char *protocol_name,
                   ClientData *data, struct lws **lws) {
    uint64_t err = 0LL;
    std::unique_lock<std::mutex> lock(ctx_mutex_, std::defer_lock);
    if (CaptureLock(&lock, facility_)) {
      return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
    }
    if (NULL == context_) {
      struct lws_context_creation_info ctx_info;
      memset(&ctx_info, 0, sizeof ctx_info);
      ctx_info.port = CONTEXT_PORT_NO_LISTEN;
      ctx_info.iface = NULL;
      ctx_info.protocols = protocols_;
      ctx_info.ssl_cert_filepath = NULL;
      ctx_info.ssl_private_key_filepath = NULL;
      ctx_info.gid = -1;
      ctx_info.uid = -1;
      ctx_info.options = 0;
      ctx_info.ws_ping_pong_interval = 30;   // will send ping if iddle for sec

      if (NULL == (context_ = lws_create_context(&ctx_info))) {
        err = MAKE_HIPPO_ERROR(facility_, HIPPO_OPEN);
        goto clean_up;
      }
      socket_thread_ = new std::thread(&WsContext::socket_loop, this);
      socket_thread_->detach();
    }
    struct lws_client_connect_info conn_info;
    memset(&conn_info, 0, sizeof(conn_info));
    conn_info.context = context_;
    conn_info.address = host;
    conn_info.port = port;
    conn_info.path = "/";
    conn_info.host = host;
    conn_info.pwsi = lws;    // will store the lws_ before returning
    conn_info.userdata = data;     // 'user' field in callback
    conn_info.protocol = protocol_name;
    conn_info.local_protocol_name = protocol_name;

    if (NULL == lws_client_connect_via_info(&conn_info)) {
      err = MAKE_HIPPO_ERROR(facility_, HIPPO_OPEN);
      goto clean_up;
    }
    // we'll be waiting for the Established callback
    connections_ += connections_pending_increment_;
 clean_up:
    lock.unlock();
    return err;
  }

  // called from the LWS thread's callback function when a connection is
  // closed
  int ClientClosed(HippoLWS *priv) {
    connections_ -= connections_increment_;
    return 0;
  }

  // called from the LWS thread's callback function when a
  // pending connection is established
  int Established(HippoLWS *priv) {
    connections_ += connections_increment_;
    connections_ -= connections_pending_increment_;
    return 0;
  }

  WsContext(WsContext const &);    // Don't implement
  void operator=(WsContext const &);    // Don't implement

 private:
  WsContext() :
      facility_(HIPPO_WS), context_(NULL), connections_(0LL) {
  }

  ~WsContext() {
  }

  // This thread's loop will call the LWS loop thread's callback function
  void socket_loop(void) {
    uint32_t timeout_ms = 1000;

    while (true) {
      // this will call the Established/ClientClosed functions above if needed
      lws_service(context_, timeout_ms);
      if (!connections_) {   // atomic if (!connections && !connections_pending)
        std::unique_lock<std::mutex> lock(ctx_mutex_, std::defer_lock);
        if (!CaptureLock(&lock, facility_)) {   // will unlock when out of scope
          if (!connections_) {   // check again while holding the mutex
            lws_context_destroy(context_);
            context_ = NULL;
            break;
          }
        }
      }
    }
  }

  HippoFacility facility_;
  struct lws_context *context_;
  std::mutex ctx_mutex_;

  // 64 bit field containing number of connections in lower 32 bits
  // and number of connections_pending on higher 32 bits
  std::atomic<uint64_t> connections_;
  const uint64_t connections_increment_ = 1;
  const uint64_t connections_pending_increment_ = 1LL << 32;

  std::thread *socket_thread_;
};


//
// Functions for HippoLWS
//
HippoLWS::HippoLWS(HippoFacility facility) :
    facility_(facility),
    lws_(NULL), connected_(false), cancel_read_(false) {
}

HippoLWS::~HippoLWS() {
  (void)Disconnect();
}

int HippoLWS::ClientClosed() {
  std::unique_lock<std::mutex> lock(ws_mutex_, std::defer_lock);
  if (CaptureLock(&lock, facility_)) {
    return -1;
  }
  connected_ = false;

  int err = WsContext::GetInstance().ClientClosed(this);

  lock.unlock();
  ws_condition_.notify_all();

  return err;
}

uint64_t HippoLWS::Connect(const char *host, int port,
                           const char *protocol_name, int timeout) {
  if (Connected()) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_WRONG_STATE_ERROR);
  }
  std::unique_lock<std::mutex> lock(ws_mutex_, std::defer_lock);
  if (CaptureLock(&lock, facility_)) {
    return -1;
  }
  client_data_.hlws_ = this;
  uint64_t err = 0;
  if (err = WsContext::GetInstance().Connect(host, port, protocol_name,
                                             &client_data_, &lws_)) {
    goto clean_up;
  }
  // and wait for an ESTABLISHED callback
  ws_condition_.wait_for(lock,
                         std::chrono::seconds(timeout),
                         [this] {
                           return Connected() || cancel_read_;
                         });
  if (!Connected()) {
    if (cancel_read_) {
      // got a LWS_CALLBACK_CLIENT_CONNECTION_ERROR
      // most likely SoHal server was not up and running
      err = MAKE_HIPPO_ERROR(facility_, HIPPO_OPEN);
      cancel_read_ = false;
    } else {
      err = MAKE_HIPPO_ERROR(facility_, HIPPO_TIMEOUT);
    }
  }
clean_up:
  lock.unlock();
  return err;
}

bool HippoLWS::Connected(void) {
  return connected_;
}

uint64_t HippoLWS::Disconnect() {
  uint64_t err = 0LL;
  if (Connected()) {
    // sending a fixed message to clese connection from inside the callback
    unsigned char *response = NULL;
    size_t resp_len = 0;

    if (err = SendRequest(
            reinterpret_cast<const unsigned char*>(kCloseConnectionStr),
            strlen(kCloseConnectionStr),
            WsConnectionType::TEXT,
            &response, &resp_len)) {
      if (HIPPO_WRITE != (err & 0xfff)) {
        return err;
      }
    }
  }
  return err;
}

// int HippoLWS::Closed() {
//   std::unique_lock<std::mutex> lock(ws_mutex_, std::defer_lock);
//   if (CaptureLock(&lock, facility_)) {
//     return -1;
//   }
//   initialized_ = false;
//   lock.unlock();
//   ws_condition_.notify_all();
//   return 0;
// }

int HippoLWS::Established() {
  std::unique_lock<std::mutex> lock(ws_mutex_, std::defer_lock);
  if (CaptureLock(&lock, facility_)) {
    return -1;
  }
  connected_ = true;
  int err = WsContext::GetInstance().Established(this);
  lock.unlock();
  ws_condition_.notify_all();

  return err;
}

int HippoLWS::ClientConnectionError() {
  std::unique_lock<std::mutex> lock(ws_mutex_, std::defer_lock);
  if (CaptureLock(&lock, facility_)) {
    return -1;
  }
  connected_ = false;
  cancel_read_ = true;
  lock.unlock();
  ws_condition_.notify_all();

  return 0;
}

int HippoLWS::Writable() {
  std::unique_lock<std::mutex> lock(ws_mutex_, std::defer_lock);
  if (CaptureLock(&lock, facility_)) {
    return -1;
  }
  size_t len;
  unsigned char *data;
  lws_write_protocol type;
  client_data_.request_.GetData(&data, &len, &type);

  int err = 0;
  if (0 != len && NULL != data) {
#ifdef VERBOSE_MSG
    fprintf(stderr, "<-+ [%zd](%d): ", len, type);
    if (LWS_WRITE_BINARY == type) {
      for (int i=0; i < len; i++) {
        fprintf(stderr, "%x,", data[LWS_PRE+i]);
      }
      fprintf(stderr, "\n");
    } else {
      fprintf(stderr, "'%*.*s'\n", static_cast<int>(len),
              static_cast<int>(len), data+LWS_PRE);
    }
#endif
    // if we get the 'close connection' string
    if (LWS_WRITE_TEXT == type &&
        len == strlen(kCloseConnectionStr) &&
        0 == memcmp(data+LWS_PRE, kCloseConnectionStr, len)) {
      // we set it back as response
      (void)client_data_.response_.SetData(kCloseConnectionStr, len, true);
      // and return -1 to close the connection. We'll get a CLOSED callback
      // that will send the ws_condition_.notify_all();
      err = -1;
    } else {
      lws_write(lws_, data+LWS_PRE, len, type);
    }
  }
  lock.unlock();

  return err;
}

int HippoLWS::Receive(const char *in, size_t len) {
  std::unique_lock<std::mutex> lock(ws_mutex_, std::defer_lock);
  if (CaptureLock(&lock, facility_)) {
    return -1;
  }
  bool final_fragment = (!lws_remaining_packet_payload(lws_) &&
                         lws_is_final_fragment(lws_));
  if (client_data_.response_.SetData(in, len, final_fragment)) {
    return -1;
  }
  lock.unlock();
  if (final_fragment) {
#ifdef VERBOSE_MSG
    fprintf(stderr, "-+> '%*.*s'\n", static_cast<int>(len),
            static_cast<int>(len), in);
#endif
    ws_condition_.notify_all();
  }
  return 0;
}

uint64_t HippoLWS::StopSignalLoop(void) {
  std::unique_lock<std::mutex> lock(ws_mutex_, std::defer_lock);
  if (CaptureLock(&lock, facility_)) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  cancel_read_ = true;
  lock.unlock();
  ws_condition_.notify_all();

  return 0LL;
}

// will send the request to SoHal and, if the response pointer is not NULL
// will wait for the response back.
uint64_t HippoLWS::SendRequest(const unsigned char *request,
                               size_t req_len,
                               WsConnectionType type,
                               unsigned char **response,
                               size_t *resp_len) {
  int timeout = 10;
  return SendRequest(request, req_len, type, timeout, response, resp_len);
}

// will send the request to SoHal and, if the response pointer is not NULL
// will wait for the response back.
uint64_t HippoLWS::SendRequest(const unsigned char *request,
                               size_t req_len,
                               WsConnectionType type,
                               int timeout,
                               unsigned char **response,
                               size_t *resp_len) {
  uint64_t err = 0LL;
  std::unique_lock<std::mutex> lock(ws_mutex_, std::defer_lock);
  if (CaptureLock(&lock, facility_)) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  if (!connected_) {
    err = MAKE_HIPPO_ERROR(facility_, HIPPO_WRITE);
    goto clean_up;
  }
  // fill up the request
  HippoError hr;
  client_data_.response_.Init();
  if (hr = client_data_.request_.SetData(request, req_len, type)) {
    err = MAKE_HIPPO_ERROR(facility_, hr);
    goto clean_up;
  }
  // request a callback so we can write the command to the ws
  lws_callback_on_writable(lws_);

  if (NULL != response && NULL != resp_len) {
    err = Read_p(&lock, response, resp_len, timeout);
  }
clean_up:
  lock.unlock();

  return err;
}

uint64_t HippoLWS::Read(unsigned char **response, size_t *len,
                        int timeout) {
  uint64_t err = 0;

  client_data_.response_.Init();

  std::unique_lock<std::mutex> lock(ws_mutex_, std::defer_lock);
  if (CaptureLock(&lock, facility_)) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_ERROR);
  }
  err = Read_p(&lock, response, len, timeout);
  lock.unlock();

  return err;
}

// This function expect the lock on the ws_mutex to be captured
uint64_t HippoLWS::Read_p(std::unique_lock<std::mutex> *lock,
                          unsigned char **response, size_t *len,
                          int timeout) {
  uint64_t err = 0;
  *len = 0;
  *response = NULL;
#ifdef VERBOSE_MSG
  int tid = GetCurrentThreadId();
#endif
  if (0 == timeout) {
#ifdef VERBOSE_MSG
    fprintf(stderr, "[%d] %s waiting for signal\n", tid, __FUNCTION__);
#endif
    (void)ws_condition_.wait(
        *lock,
        [this] {
          return (client_data_.response_.Received() ||
                  cancel_read_ || !Connected());
        });
#ifdef VERBOSE_MSG
    fprintf(stderr, "[%d] %s signal!!\n", tid, __FUNCTION__);
#endif
  } else {
#ifdef VERBOSE_MSG
    fprintf(stderr, "[%d] %s waiting for response\n", tid, __FUNCTION__);
#endif
    (void)ws_condition_.wait_for(
        *lock,
        std::chrono::seconds(timeout),
        [this] {
          return (client_data_.response_.Received() ||
                  cancel_read_ || !Connected());
        });
#ifdef VERBOSE_MSG
    fprintf(stderr, "[%d] %s response!!\n", tid, __FUNCTION__);
#endif
  }
  if (client_data_.response_.Received()) {
#ifdef VERBOSE_MSG
    fprintf(stderr, "%s got data %p\n", __FUNCTION__, this);
#endif
    client_data_.response_.GetData(response, len);
  } else  if (cancel_read_) {
#ifdef VERBOSE_MSG
    fprintf(stderr, "%s cancelled %p\n", __FUNCTION__, this);
#endif
    cancel_read_ = false;
    err = MAKE_HIPPO_ERROR(facility_, HIPPO_CANCEL);
  } else if (!Connected()) {
#ifdef VERBOSE_MSG
    fprintf(stderr, "%s connection closed\n", __FUNCTION__);
#endif
    err = MAKE_HIPPO_ERROR(facility_, HIPPO_WRONG_STATE_ERROR);
  } else {
#ifdef VERBOSE_MSG
    fprintf(stderr, "%s timeout\n", __FUNCTION__);
#endif
    err = MAKE_HIPPO_ERROR(facility_, HIPPO_TIMEOUT);
  }
#ifdef VERBOSE_MSG
  fprintf(stderr, "[%d] %s err: %llx  %s!!\n", tid, __FUNCTION__,
          err, *response);
#endif
  return err;
}

//
// Functions for HippoWS
//
HippoWS::HippoWS(HippoFacility facility) :
    facility_(facility), hlws_(NULL) {
}

HippoWS::~HippoWS(void) {
  if (hlws_) {
    delete hlws_;
  }
}

uint64_t HippoWS::Connect(const char *host, uint32_t port,
                          WsConnectionType type, int timeout) {
  return Connect(host, port, type, 0, timeout);
}

uint64_t HippoWS::Connect(const char *host, uint32_t port,
                          WsConnectionType type, uint32_t rx_buffer_size,
                          int timeout) {
  if (!(hlws_ = new (std::nothrow) HippoLWS(facility_))) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_MEM_ALLOC);
  }
  int logs = 0;   // LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE | LLL_INFO;
  lws_set_log_level(logs, NULL);

  return hlws_->Connect(host, port,
                        protocols_[static_cast<uint32_t>(type)].name,
                        timeout);
}

uint64_t HippoWS::Disconnect() {
  uint64_t err = 0LL;

  if (err = hlws_->Disconnect()) {
    return err;
  }
  delete hlws_;
  hlws_ = NULL;

  return err;
}

uint64_t HippoWS::StopSignalLoop() {
  return hlws_->StopSignalLoop();
}

uint64_t HippoWS::SendRequest(const unsigned char *request,
                              WsConnectionType type) {
  int timeout = 10;
  return SendRequest(request, type, timeout, NULL);
}

uint64_t HippoWS::SendRequest(const unsigned char *request,
                              WsConnectionType type, int timeout,
                              unsigned char **response) {
  uint64_t err;
  size_t req_len = strlen(reinterpret_cast<const char*>(request));
  size_t res_len = 0;

#ifdef VERBOSE
  fprintf(stderr, "<--[%lld] '%s'\n", req_len, request);
#endif
  if (err = SendRequest(request, req_len, type, timeout, response, &res_len)) {
    return err;
  }
  if (WsConnectionType::TEXT == type && NULL != response) {
    // the data array is always a byte longer so we can do this ;)
    (*response)[res_len] = '\0';
#ifdef VERBOSE
    fprintf(stderr, "-->[%lld] '%s'\n", res_len, *response);
#endif
  }
  return err;
}

uint64_t HippoWS::SendRequest(const unsigned char *request, size_t req_len,
                              WsConnectionType type, int timeout,
                              unsigned char **response, size_t *res_len) {
  *res_len = 0;

  if (NULL == request || 0 == req_len) {
    return 0LL;
  }
  if (NULL == hlws_) {
    return MAKE_HIPPO_ERROR(facility_, HIPPO_WRITE);
  }
  return hlws_->SendRequest(request, req_len, type, timeout, response, res_len);
}

uint64_t HippoWS::WaitForSignal(unsigned char **response) {
  uint64_t err;
  size_t len;
  int timeout = 0;         // wait forever

  if (err = hlws_->Read(response, &len, timeout)) {
    return err;
  }
  if (*response) {
    // the data array is always a byte longer so we can do this ;)
    (*response)[len] = '\0';
#ifdef VERBOSE
    fprintf(stderr, "*** %s 2 (%p)[%zd] '%*.*s'\n", __FUNCTION__, response, len,
            static_cast<int>(len), static_cast<int>(len), *response);
#endif
  }
  return err;
}

uint64_t HippoWS::ReadResponse(unsigned char **response) {
  uint64_t err;
  size_t len;
  int timeout = 10;      // in seconds

  if (err = hlws_->Read(response, &len, timeout)) {
    return err;
  }
  if (*response) {
    // the data array is always a byte longer so we can do this ;)
    (*response)[len] = '\0';
#ifdef VERBOSE
    fprintf(stderr, "%s (%p)[%zd] '%*.*s'\n", __FUNCTION__, response, len,
            static_cast<int>(len), static_cast<int>(len), *response);
#endif
  }
  return err;
}

bool HippoWS::Connected(void) {
  return (NULL == hlws_) ? false : hlws_->Connected();
}

//
// callback used by the LWS library event loop
//
static int ws_callback(struct lws *wsi, enum lws_callback_reasons reason,
                       void *user, void *in, size_t len) {
#ifdef VERBOSE_2
  const struct lws_protocols *protocol = lws_get_protocol(wsi);
  fprintf(stderr, "**[%s] wsi: %p, p_name: '%s', reason: %d, c_data: %p\n",
          __FUNCTION__,
          wsi,
          protocol ? protocol->name : "?",
          reason, user);
#endif
  ClientData *c_data = reinterpret_cast<ClientData*>(user);

  switch (reason) {
    case LWS_CALLBACK_PROTOCOL_INIT: {
#ifdef VERBOSE
      const struct lws_protocols *protocol = lws_get_protocol(wsi);
      fprintf(stderr, "LWS_CALLBACK_PROTOCOL_INIT "
              "ws: %p, protocol: '%s', c_data: %p\n",
              wsi, protocol->name, c_data);
#endif
      break;
    }
    case LWS_CALLBACK_CLIENT_ESTABLISHED: {
#ifdef VERBOSE
      fprintf(stderr, "LWS_CALLBACK_CLIENT_ESTABLISHED ws: %p, "
              "c_data.private: %p\n", wsi, c_data->hlws_);
#endif
      // connection open from here...
      if (c_data->hlws_->Established()) {
        return 0;
      }
      break;
    }
    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR: {
#ifdef VERBOSE
      fprintf(stderr, "LWS_CALLBACK_CLIENT_CONNECTION_ERROR\n");
#endif
      if (c_data->hlws_->ClientConnectionError()) {
        return 0;
      }
      break;
    }
    case LWS_CALLBACK_WSI_DESTROY: {
#ifdef VERBOSE
      fprintf(stderr, "LWS_CALLBACK_WSI_DESTROY  ws: %p\n", wsi);
#endif
      break;
    }
    case LWS_CALLBACK_CLIENT_CLOSED: {
#ifdef VERBOSE
      fprintf(stderr, "LWS_CALLBACK_CLIENT_CLOSED  ws: %p\n", wsi);
#endif
      // connection open ... until here
      if (c_data->hlws_->ClientClosed()) {
        return 0;
      }
      break;
    }
    case LWS_CALLBACK_CLOSED: {
#ifdef VERBOSE
      fprintf(stderr, "LWS_CALLBACK_CLOSED  ws: %p\n", wsi);
#endif
      //   if (c_data->hlws_->Closed()) {
      //     return 0;
      //   }
      break;
    }
    case LWS_CALLBACK_CLIENT_RECEIVE: {
#ifdef VERBOSE
      fprintf(stderr, "LWS_CALLBACK_CLIENT_RECEIVE  ws:%p, priv:%p [%lld]\n",
              wsi, c_data->hlws_, len);
#endif
      if (c_data->hlws_->Receive(reinterpret_cast<const char*>(in),
                                 len)) {
        return 0;
      }
      break;
    }
    case LWS_CALLBACK_CLIENT_WRITEABLE: {
#ifdef VERBOSE
      fprintf(stderr, "LWS_CALLBACK_WRITABLE  ws: %p, priv: %p\n",
              wsi, c_data->hlws_);
#endif
      if (c_data->hlws_->Writable()) {
        // return -1 to close connection
        return -1;
      }
      break;
    }
    case LWS_CALLBACK_GET_THREAD_ID: {
      int tid = GetCurrentThreadId();
#ifdef VERBOSE_2
      fprintf(stderr, "LWS_CALLBACK_GET_THREAD_ID: %d\n", tid);
#endif
      // lws will detect the lws_callback_on_writable() comes from another
      // thread and wake up the lws_service() thread to handle it
      return tid;
    }
    case LWS_CALLBACK_RECEIVE_PONG: {
#ifdef VERBOSE
      fprintf(stderr, "LWS_CALLBACK_RECEIVE_PONG:\n");
#endif
      break;
    }
    default:
      break;
  }
  return 0;
}

}  // namespace hippo
