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

#define GPROF_MAGIC         "gmon"
#define GPROF_VERSION       1

#define HISTFRACTION        2

#define HASHFRACTION        2

#define ARCDENSITY          3

#define MINARCS             50

#define MAXARCS             (1 << 20)

#define NARCS_PER_WRITEV    32

enum gprof_tag {
    GPROF_TAG_TIME_HIST = 0,
    GPROF_TAG_CG_ARC = 1,
    GPROF_TAG_BB_COUNT = 2
};
