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
#include <stdbool.h>
#include <sys/param.h>
#include <sys/uio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"
#include "esp_freertos_hooks.h"
#include "esp_private/panic_internal.h"

#include "xtensa/xtensa_context.h"

#include "gprof.h"
#include "gprof/def.h"
#include "gprof/types.h"
#include "gprof/uio.h"
#include "gprof/data.h"
#include "gprof/io.h"
#include "gprof/debug.h"

#define DEPTH_MAX   5

#ifdef __XTENSA__
#define PC_OFF      1
#define RA_OFF      3
#define SP_OFF      4

#define __XTENSA_HIST_BT_PC__
#else
#define PC_OFF      0
#define RA_OFF      1
#define SP_OFF      2
#endif

#define PC_PTR(_p)  ((s_stk_ptr)[0][PC_OFF])
#define RA_PTR(_p)  ((s_stk_ptr)[0][RA_OFF])
#define SP_PTR(_p)  ((s_stk_ptr)[0][SP_OFF])

static uintptr_t **s_stk_ptr;

#ifdef __XTENSA_HIST_BT_PC__
static inline uintptr_t hist_bt_pc(gprof_t *p)
{
    uintptr_t sp;
    uintptr_t pc = RA2PC(RA_PTR());

    if (GMON_IN_TEXT(p, pc)) {
        return pc;
    }

    sp = SP_PTR();

    for (int i = 0; i < DEPTH_MAX; i++) {
        if (!esp_stack_ptr_is_sane(sp)) {
            return 0;
        }

        pc = RA2PC(*((uintptr_t *)(sp - 16)));

        if (GMON_IN_TEXT(p, pc)) {
            break;
        }

        sp = *((uintptr_t *)(sp - 12));
    }

    return pc;
}
#endif

static void IRAM_ATTR hist_sample_cb(void)
{
    size_t i;
    uintptr_t pc;
    gprof_t *p = &g_gprof_data;

#ifdef __XTENSA__
    pc = PC_PTR();
#ifdef __XTENSA_HIST_BT_PC__
    if (GMON_OUT_TEXT(p, pc)) {
        pc = hist_bt_pc(p);
    }
#endif
#else
    pc = PC_PTR();
#endif

    i = (pc - p->low_pc) / HISTFRACTION / sizeof(histcnt_t);

    if (i < p->histcnt_num) {
        ++p->histcnt[i];
    }
}

uint32_t gprof_hist_frequency(void)
{
    return CONFIG_FREERTOS_HZ;
}

int gprof_start_hist(bool start)
{
    int ret;

    if (start) {
        TaskHandle_t task;

        task = xTaskGetCurrentTaskHandle();
        s_stk_ptr = (uintptr_t **)task;

        ret = esp_register_freertos_tick_hook(hist_sample_cb);
        if (ret != ESP_OK) {
            GMON_ABORT();
        }

    } else {
        esp_deregister_freertos_tick_hook(hist_sample_cb);
        s_stk_ptr = NULL;
    }

    return 0;
}

bool IRAM_ATTR task_is_valid(void)
{
    return (TaskHandle_t)s_stk_ptr == xTaskGetCurrentTaskHandle();
}
