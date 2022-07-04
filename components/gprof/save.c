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

#include "gprof.h"
#include "gprof/def.h"
#include "gprof/types.h"
#include "gprof/uio.h"
#include "gprof/data.h"
#include "gprof/io.h"
#include "gprof/debug.h"
#include "gprof/histogram.h"

gprof_t g_gprof_data;

static void write_ghdr(int fd, gprof_t *p)
{
    ghdr_t ghdr;

    memcpy (&ghdr.magic[0], GPROF_MAGIC, sizeof (ghdr.magic));
    ghdr.version = GPROF_VERSION;
    memset (ghdr.spare, '\0', sizeof (ghdr.spare));

    gprof_write_nocancel (fd, &ghdr, sizeof (ghdr_t));
}

static void write_hist(int fd, gprof_t *p)
{
    uint8_t tag = GPROF_TAG_TIME_HIST;
    hist_hdr_t hdr;

    struct iovec iov[3] = {
        { &tag, sizeof (tag) },
        { &hdr, sizeof (hist_hdr_t) },
        { p->histcnt, p->histcnt_bs }
    };

    hdr.low_pc    = p->low_pc;
    hdr.high_pc   = p->high_pc;
    hdr.hist_size = p->histcnt_num;
    hdr.prof_rate = gprof_hist_frequency();
    strncpy((char *)hdr.dimen, "seconds", sizeof(hdr.dimen));
    hdr.dimen_abbrev = 's';

    gprof_writev_nocancel_nostatus(fd, iov, 3);
}

static void write_callgraph(int fd, gprof_t *p)
{
    cg_hdr_t hdr_buf[NARCS_PER_WRITEV];
    struct iovec iov[NARCS_PER_WRITEV * 2];
    int nfilled = 0;
    uint8_t tag = GPROF_TAG_CG_ARC;

    for (int i = 0; i < NARCS_PER_WRITEV; i++) {
        iov[2 * i].iov_base = &tag;
        iov[2 * i].iov_len = sizeof(tag);

        iov[2 * i + 1].iov_base = &hdr_buf[i];
        iov[2 * i + 1].iov_len = sizeof(cg_hdr_t);
    }

    for (int i = 0; i < p->from_num; i++) {
        if (p->from[i] == 0) {
            continue;
        }

        uintptr_t from_pc = p->low_pc + i * p->hashfraction * sizeof(from_t);
        for (from_t index = p->from[i];
                index != 0;
                index = p->to[index].from) {
            cg_hdr_t arc;

            arc.from_pc = from_pc;
            arc.self_pc = p->to[index].self_pc;
            arc.count   = p->to[index].count;
            memcpy (hdr_buf + nfilled, &arc, sizeof(cg_hdr_t));

            if (++nfilled == NARCS_PER_WRITEV) {
                gprof_writev_nocancel_nostatus (fd, iov, 2 * nfilled);
                nfilled = 0;
            }
        }
    }

    if (nfilled > 0) {
        gprof_writev_nocancel_nostatus (fd, iov, 2 * nfilled);
    }
}

void gprof_save_data(void)
{
    int fd = -1;

    gprof_t *p = &g_gprof_data;

    gprof_start_hist(false);

    write_ghdr(fd, p);

    write_hist(fd, p);

    write_callgraph(fd, p);

    gprof_close_nocancel_nostatus(fd);
}
