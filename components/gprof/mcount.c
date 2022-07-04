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
#include <strings.h>

#include "esp_attr.h"

#include "gprof.h"
#include "gprof/def.h"
#include "gprof/types.h"
#include "gprof/uio.h"
#include "gprof/data.h"
#include "gprof/io.h"
#include "gprof/debug.h"
#include "gprof/histogram.h"

void IRAM_ATTR _mcount(uintptr_t from_pc)
{
    size_t i;
    uintptr_t frompc_off;
    to_t *top, *prev_top;
    from_t *frompc_index;
    from_t to_index;
    gprof_t *p = &g_gprof_data;
    uintptr_t self_pc = (uintptr_t)__builtin_return_address(0);
    if (!task_is_valid()) {
        goto done;
    }

#ifdef __XTENSA__
    from_pc = RA2PC(from_pc);
#endif

    frompc_off = from_pc - p->low_pc;
    if (frompc_off > p->text_size) {
       // GMON_LOGV("from_pc=%p low_pc=%p high_pc=%p\n",
                //   (void *)from_pc, (void *)p->low_pc, (void *)p->high_pc);
        goto done;
    }

    i = frompc_off >> p->log_hashfraction;

    frompc_index = &p->from[i];
    to_index = *frompc_index;
    if (to_index == 0) {
        if ((p->to_ind + 1) >= p->to_num) {
            goto overflow;
        }

        to_index = ++p->to_ind;

        top = &p->to[to_index];
        top->self_pc = self_pc;
        top->count  = 1;
        top->from   = 0;

        *frompc_index = to_index;

        goto done;
    }

    top = &p->to[to_index];
    if (top->self_pc == self_pc) {
        top->count++;
        goto done;
    }

    while (1) {
        if (top->from == 0) {
            if ((p->to_ind + 1) >= p->to_num) {
                goto overflow;
            }

            to_index = ++p->to_ind;

            top = &p->to[to_index];
            top->self_pc = self_pc;
            top->count  = 1;
            top->from   = *frompc_index;

            *frompc_index = to_index;

            goto done;
        }

        prev_top = top;
        top = &p->to[top->from];
        if (top->self_pc == self_pc) {
            top->count++;

            to_index       = prev_top->from;
            prev_top->from = top->from;
            top->from      = *frompc_index;
            *frompc_index  = to_index;

            goto done;
        }

    }
overflow:
    GMON_ABORT();
done:
    return;
}
