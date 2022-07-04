// Copyright 2021 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <stdio.h>

#define GMON_LOGE(_fmt, ...)    printf("E(%s:%d): " _fmt, __func__, __LINE__, ##__VA_ARGS__)
#define GMON_LOGI(_fmt, ...)    printf("I(%s:%d): " _fmt, __func__, __LINE__, ##__VA_ARGS__)
#define GMON_LOGD(_fmt, ...)    printf("D(%s:%d): " _fmt, __func__, __LINE__, ##__VA_ARGS__)
#define GMON_LOGV(_fmt, ...)    printf("V(%s:%d): " _fmt, __func__, __LINE__, ##__VA_ARGS__)

#define GMON_PRINT(_fmt, ...)   printf(_fmt, ##__VA_ARGS__)

#define GMON_ABORT()           ({printf("A(%s:%d)\n", __func__, __LINE__); abort();})

void gprof_dump_info(const uint8_t *buf, size_t len);
