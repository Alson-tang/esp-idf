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

/*
 * general rounding functions.
 */
#define ROUND_DOWN(x, y)        (((x) / (y)) * (y))
#define ROUND_UP(x, y)          ((((x) + (y) -1) / (y)) * (y))

#define GMON_IN_TEXT(_p, _a)    (((_a) - (_p)->low_pc) <  (_p)->text_size)
#define GMON_OUT_TEXT(_p, _a)   (((_a) - (_p)->low_pc) >= (_p)->text_size)

#include "gprof/types.h"

extern gprof_t g_gprof_data;
