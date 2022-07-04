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

#include <stdint.h>

typedef uint16_t    histcnt_t;

typedef uintptr_t   from_t;

#pragma pack(4)
typedef struct ghdr {
    uint8_t         magic[4];               /* magic */
    uint32_t        version;                /* version number */
    uint8_t         spare[12];              /* reserved */
} ghdr_t;

typedef struct hist_hdr {
    uintptr_t       low_pc;
    uintptr_t       high_pc;

    uint32_t        hist_size;
    uint32_t        prof_rate;

    uint8_t         dimen[15];
    uint8_t         dimen_abbrev;
} hist_hdr_t;

typedef struct cg_hdr {
    uintptr_t       from_pc;               /* address within caller's body */
    uintptr_t       self_pc;               /* address within callee's body */
    uint32_t        count;                  /* number of arc traversals */
} cg_hdr_t;
#pragma pack()

typedef struct to {
    uintptr_t       self_pc;
    uint32_t        count;

    from_t          from;
} to_t;

typedef struct gprof {
    histcnt_t       *histcnt;       /* histogram counter buffer */
    size_t          histcnt_bs;     /* histogram counter buffer size by byte */
    size_t          histcnt_num;    /* histogram counter number */

    from_t          *from;
    size_t          from_bs;
    size_t          from_num;

    to_t            *to;
    size_t          to_bs;
    size_t          to_num;
    size_t          to_ind;

    uintptr_t       low_pc;
    uintptr_t       high_pc;
    size_t          text_size;

    uint8_t         hashfraction;
    uint8_t         log_hashfraction;
} gprof_t;
