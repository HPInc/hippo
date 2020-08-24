
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#include <cstdlib>
#include <cstring>
#include <fstream>
#include "../include/base64.h"

namespace hippo {

// algorithm adapted from:
// https://stackoverflow.com/questions/180947/base64-decode-snippet-in-c/37109258#37109258

static const char* B64chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const int B64index[256] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  62, 63, 62, 62, 63,
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0,  0,  0,  0,  0,  0,
  0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0,  0,  0,  0,  63,
  0,  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
};

uint64_t base64_encode(const b64bytes &bytes, char **get) {
  size_t len = bytes.len;
  size_t encoded_len = (len + 2) / 3 * 4;
  *get = reinterpret_cast<char*>(malloc(encoded_len+1));
  if (*get == NULL) {
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_MEM_ALLOC);
  }
  unsigned char *p = (unsigned char*)bytes.data, *str = (unsigned char*)*get;
  (void)memset(str, '=', encoded_len);

  size_t j = 0, pad = len % 3;
  const size_t last = len - pad;

  // fprintf(stderr, "%s len: %zd, last: %zd, en_len: %zd\n", __FUNCTION__,
  //         len, last, encoded_len);
  for (size_t i = 0; i < last; i += 3) {
    unsigned int n = (static_cast<int>(p[i]) << 16 |
                      static_cast<int>(p[i + 1]) << 8 | p[i + 2]);
    // fprintf(stderr, "i: %zd, j: %zd, n: %d [%d, %d, %d, %d]\n",
    //         i, j, n, n >> 18, n >> 12 & 0x3F, n >> 6 & 0x3F, n & 0x3F);
    str[j++] = B64chars[n >> 18];
    str[j++] = B64chars[n >> 12 & 0x3F];
    str[j++] = B64chars[n >> 6 & 0x3F];
    str[j++] = B64chars[n & 0x3F];
  }
  if (pad) {  /// set padding
    unsigned int n = --pad ?
        static_cast<int>(p[last]) << 8 | p[last + 1] :
        p[last];

    str[j++] = B64chars[pad ? n >> 10 & 0x3F : n >> 2];
    str[j++] = B64chars[pad ? n >> 4 & 0x03F : n << 4 & 0x3F];
    str[j++] = pad ? B64chars[n << 2 & 0x3F] : '=';
  }
  // for (size_t i = 0; i < encoded_len; i ++) {
  //   fprintf(stderr, "%c", str[i]);
  // }
  // fprintf(stderr, "\n");
  str[encoded_len] = '\0';

  return 0LL;
}

uint64_t base64_decode(const char *data, b64bytes *bytes) {
  size_t len = strlen(data);
  if (len == 0) {
    bytes->len = 0;
    bytes->data = NULL;
    return 0LL;
  }
  unsigned char *p = (unsigned char*) data;
  size_t j = 0, pad1 = len % 4 || p[len - 1] == '=',
      pad2 = pad1 && (len % 4 > 2 || p[len - 2] != '=');
  const size_t last = (len - pad1) / 4 << 2;

  // std::string result(last / 4 * 3 + pad1 + pad2, '\0');
  bytes->len = last / 4 * 3 + pad1 + pad2;
  bytes->data = reinterpret_cast<uint8_t*>(malloc(bytes->len));
  if (bytes->data == NULL) {
    bytes->len = 0;
    return MAKE_HIPPO_ERROR(HIPPO_SWDEVICE, HIPPO_MEM_ALLOC);
  }
  (void)memset(bytes->data, '\0', bytes->len);

  // unsigned char *str = (unsigned char*) &result[0];
  unsigned char *str = (unsigned char*) bytes->data;

  for (size_t i = 0; i < last; i += 4) {
    int n = B64index[p[i]] << 18 | B64index[p[i + 1]] << 12 |
        B64index[p[i + 2]] << 6 | B64index[p[i + 3]];
    str[j++] = n >> 16;
    str[j++] = n >> 8 & 0xFF;
    str[j++] = n & 0xFF;
  }
  if (pad1) {
    int n = B64index[p[last]] << 18 | B64index[p[last + 1]] << 12;
    str[j++] = n >> 16;
    if (pad2) {
      n |= B64index[p[last + 2]] << 6;
      str[j++] = n >> 8 & 0xFF;
    }
  }
  return 0LL;   // result;
}

}  // namespace hippo
