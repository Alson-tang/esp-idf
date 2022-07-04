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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/param.h>

#include "gprof.h"
#include "gprof/def.h"
#include "gprof/types.h"
#include "gprof/uio.h"
#include "gprof/data.h"
#include "gprof/io.h"
#include "gprof/debug.h"
#include "gprof/histogram.h"

static void gprof_init_ll(gprof_t *p, void *low_pc, void *high_pc)
{
    size_t n;
    uint8_t *cp;

    GMON_LOGD("low_pc=%p, high_pc=%p\n", low_pc, high_pc);

    p->low_pc    = (uintptr_t)low_pc;
    p->high_pc   = (uintptr_t)high_pc;
    p->text_size = ROUND_UP(high_pc - low_pc, sizeof(from_t));

    p->histcnt_bs  = ROUND_UP(p->text_size / HISTFRACTION, sizeof(from_t));
    p->histcnt_num = p->histcnt_bs / sizeof(histcnt_t);

    GMON_LOGD("text_size=%zu, histcnt_num=%zu\n", p->text_size, p->histcnt_bs);

    p->from_bs  = p->text_size / HASHFRACTION;
    p->from_num = p->from_bs / sizeof (from_t);

    p->to_num = p->text_size * ARCDENSITY / 100;
    p->to_num = MAX(p->to_num, MINARCS);
    p->to_num = MIN(p->to_num, MAXARCS);
    p->to_bs  = p->to_num * sizeof(to_t);

    n = p->histcnt_bs + p->from_bs + p->to_bs;
    GMON_LOGD("total buffer size=%zu\n", n);
    cp = calloc(n, 1);
    if (! cp) {
        GMON_ABORT();
    }

    p->to = (to_t *)cp;

    cp += p->to_bs;
    p->histcnt = (histcnt_t *)cp;

    cp += p->histcnt_bs;
    p->from = (from_t *)cp;

    p->hashfraction     = HASHFRACTION;
    p->log_hashfraction = ffs(p->hashfraction * sizeof(from_t)) - 1;

    GMON_LOGD("hashfraction=%d, log_hashfraction=%d\n",
              p->hashfraction, p->log_hashfraction);

    gprof_start_hist(true);
}

void gprof_init(void)
{
    extern uint32_t _gprof_text_start, _gprof_text_end;

    gprof_init_ll(&g_gprof_data, &_gprof_text_start, &_gprof_text_end);
}
