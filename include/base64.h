
// Copyright 2019 HP Development Company, L.P.
// SPDX-License-Identifier: MIT

#ifndef INCLUDE_BASE64_H_
#define INCLUDE_BASE64_H_

#include "../include/hippo.h"
#include "../include/hippo_swdevice.h"

namespace hippo {

uint64_t base64_encode(const b64bytes &bytes, char **get);
uint64_t base64_decode(const char *data, b64bytes *bytes);

}   // namespace hippo

#endif   // INCLUDE_BASE64_H_
