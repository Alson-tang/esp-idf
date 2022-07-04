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

#ifdef __XTENSA__
#define RA2PC(_a)   ((_a) - 0x40000000)
#endif

struct iovec;

void gprof_writev_nocancel_nostatus(int fd, struct iovec *iov, int n);
void gprof_write_nocancel(int fd, void *buf, int n);
void gprof_close_nocancel_nostatus(int fd);

void gprof_print_filedata(void);
