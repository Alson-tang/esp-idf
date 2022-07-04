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

#include <stdint.h>
#include <string.h>
#include <sys/param.h>

#include "gprof.h"
#include "gprof/def.h"
#include "gprof/types.h"
#include "gprof/uio.h"
#include "gprof/data.h"
#include "gprof/io.h"
#include "gprof/debug.h"

static size_t dump_head(const uint8_t *buf)
{
    ghdr_t *ghdr = (ghdr_t *)buf;

    GMON_PRINT("magic: %c%c%c%c\n", ghdr->magic[0], ghdr->magic[1], ghdr->magic[2], ghdr->magic[3]);
    GMON_PRINT("version: %x\n", ghdr->version);
    GMON_PRINT("spare:\n");
    for (int i = 0; i < 12; i++) {
        GMON_PRINT("%02x ", ghdr->spare[i]);
    }
    GMON_PRINT("\n");

    return sizeof(ghdr_t);
}

static size_t dump_time_hist(const uint8_t *buf)
{
    histcnt_t *hcnt;
    hist_hdr_t *ghhdr = (hist_hdr_t *)buf;

    GMON_PRINT("low pc: %p\n", (void *)ghhdr->low_pc);
    GMON_PRINT("high pc: %p\n", (void *)ghhdr->high_pc);
    GMON_PRINT("hist size: %d\n", ghhdr->hist_size);
    GMON_PRINT("prof rate: %d\n", ghhdr->prof_rate);
    GMON_PRINT("dimen:\n");
    for (int i = 0; i < 15; i++) {
        GMON_PRINT("%02x(%c)", ghhdr->dimen[i], ghhdr->dimen[i]);
    }
    GMON_PRINT("\n");
    GMON_PRINT("dimen abbrev: %02x(%c)\n", ghhdr->dimen_abbrev, ghhdr->dimen_abbrev);

    GMON_PRINT("hist samples:\n");
    hcnt = (histcnt_t *)buf + sizeof(hist_hdr_t);
    for (int i = 0; i < ghhdr->hist_size; i += 8) {
        int n = MIN(ghhdr->hist_size - i, 8);

        GMON_PRINT("0x%04x\t", i);
        for (int j = 0; j < n; j++) {
            GMON_PRINT("%04x ", hcnt[i + j]);
        }
        GMON_PRINT("\n");
    }

    return sizeof(hist_hdr_t) + ghhdr->hist_size * sizeof(histcnt_t);
}

static int dump_cg_arc(const uint8_t *buf)
{
    cg_hdr_t *arc = (cg_hdr_t *)buf;

    GMON_PRINT("from_pc: %p\n", (void *)arc->from_pc);
    GMON_PRINT("self_pc: %p\n", (void *)arc->self_pc);
    GMON_PRINT("count: %u(0x%08x)\n", arc->count, arc->count);

    return sizeof(cg_hdr_t);
}

void gprof_dump_info(const uint8_t *buf, size_t len)
{
    size_t n;
    const uint8_t *p = buf;
    const uint8_t *end = buf + len;

    n = dump_head(p);
    p += n;

    while (p < end) {
        const uint8_t *tag;

        tag = p;
        p  += sizeof(uint8_t);

        GMON_PRINT("\noffet: 0x%x\n", (uint32_t)(p - buf));
        GMON_PRINT("tag: %02x\n", tag[0]);
        switch (tag[0]) {
        case GPROF_TAG_TIME_HIST:
            GMON_PRINT("record: time histogram\n");
            n = dump_time_hist(p);
            p += n;
            break;
        case GPROF_TAG_CG_ARC:
            GMON_PRINT("record: call-graph\n");
            n = dump_cg_arc(p);
            p += n;
            break;
        default:
            GMON_PRINT("record=%d not support\n", tag[0]);
            return ;
            break;
        }
    }
}
