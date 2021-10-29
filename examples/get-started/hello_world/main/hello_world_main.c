/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "nvs_flash.h"
#include "esp_partition.h"

static bool nvs_test(void)
{
    esp_err_t err;

    printf("enter func nvs_test()\r\n");

    esp_partition_t partition = {
        .flash_chip = NULL,
        .type = ESP_PARTITION_TYPE_DATA,
        .subtype = ESP_PARTITION_SUBTYPE_DATA_NVS,
        .address = 0x0003E000,
        .size = 0x00009000,
        .label = "fctry",
        .encrypted = false,
    };

    err = nvs_flash_init_partition_ptr(&partition);
    if (err != ESP_OK) {
        printf("nvs flash init failed: %x\r\n", err);
    } else {
        printf("nvs flash init success\r\n");
    }

    return true;
}

void app_main(void)
{
    printf("Hello world!\n");

    nvs_test();
}
