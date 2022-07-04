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

#include "esp_spi_flash.h"

#include "gprof.h"
#include "gprof/def.h"
#include "gprof/types.h"
#include "gprof/uio.h"
#include "gprof/data.h"
#include "gprof/io.h"
#include "gprof/debug.h"

// #define WR_OFFSET       (0x180000)
#define WR_OFFSET       (0x120000)

#define WR_BUFFER_MAX   (150 * 1024)

static size_t s_size;
static uint8_t s_buffer[WR_BUFFER_MAX];

void gprof_writev_nocancel_nostatus(int fd, struct iovec *iov, int n)
{
    for (int i = 0; i < n; i++) {
        if (s_size + iov[i].iov_len > WR_BUFFER_MAX) {
            GMON_LOGI("len is: %d\n", s_size + iov[i].iov_len);
            GMON_ABORT();
        }

        memcpy(s_buffer + s_size, iov[i].iov_base, iov[i].iov_len);
        s_size += iov[i].iov_len;
    }
}

void gprof_write_nocancel(int fd, void *buf, int n)
{
    if (s_size + n > WR_BUFFER_MAX) {
        GMON_ABORT();
    }

    memcpy(s_buffer + s_size, buf, n);
    s_size += n;
}

void gprof_close_nocancel_nostatus(int fd)
{
    int ret;
    size_t n = ROUND_UP(s_size, SPI_FLASH_SEC_SIZE);

    ret = spi_flash_erase_range(WR_OFFSET, n);
    if (ret != ESP_OK) {
        GMON_ABORT();
    } else {
        GMON_LOGI("erase from 0x%x to 0x%x\n", WR_OFFSET, WR_OFFSET + n);
    }

    ret = spi_flash_write(WR_OFFSET, s_buffer, s_size);
    if (ret != ESP_OK) {
        GMON_ABORT();
    } else {
        GMON_LOGI("save %zu data to 0x%x\n", s_size, WR_OFFSET);
    }
}

void gprof_print_filedata(void)
{
    gprof_dump_info(s_buffer, s_size);
}
