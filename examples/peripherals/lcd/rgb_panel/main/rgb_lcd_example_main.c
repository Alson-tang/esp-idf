/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lvgl.h"

#include "gprof.h"
#include "gprof/io.h"

static const char *TAG = "example";

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Please update the following configuration according to your LCD spec //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ     (10 * 1000 * 1000)
#define EXAMPLE_PIN_NUM_HSYNC          (46)
#define EXAMPLE_PIN_NUM_VSYNC          (3)
#define EXAMPLE_PIN_NUM_DE             (0)
#define EXAMPLE_PIN_NUM_PCLK           (9)
#define EXAMPLE_PIN_NUM_DATA0          (14) // B0
#define EXAMPLE_PIN_NUM_DATA1          (13) // B1
#define EXAMPLE_PIN_NUM_DATA2          (12) // B2
#define EXAMPLE_PIN_NUM_DATA3          (11) // B3
#define EXAMPLE_PIN_NUM_DATA4          (10) // B4
#define EXAMPLE_PIN_NUM_DATA5          (39) // G0
#define EXAMPLE_PIN_NUM_DATA6          (38) // G1
#define EXAMPLE_PIN_NUM_DATA7          (45) // G2
#define EXAMPLE_PIN_NUM_DATA8          (48) // G3
#define EXAMPLE_PIN_NUM_DATA9          (47) // G4
#define EXAMPLE_PIN_NUM_DATA10         (21) // G5
#define EXAMPLE_PIN_NUM_DATA11         (1)  // R0
#define EXAMPLE_PIN_NUM_DATA12         (2)  // R1
#define EXAMPLE_PIN_NUM_DATA13         (42) // R2
#define EXAMPLE_PIN_NUM_DATA14         (41) // R3
#define EXAMPLE_PIN_NUM_DATA15         (40) // R4
#define EXAMPLE_PIN_NUM_DISP_EN        (-1)

// The pixel number in horizontal and vertical
#define EXAMPLE_LCD_H_RES              800
#define EXAMPLE_LCD_V_RES              480

#define EXAMPLE_LVGL_TICK_PERIOD_MS    2

#define GPROF_TIME  (10 * 1000 * 1000)

static SemaphoreHandle_t xSemaphore = NULL;
static esp_timer_handle_t gprof_timer = NULL;

extern void example_lvgl_demo_ui(lv_obj_t *scr);

static void gprof_end(void *pvParameters)
{
    xSemaphoreTake(xSemaphore, portMAX_DELAY);

    gprof_save_data();

    vTaskDelete(NULL);
}

static void gprof_timer_callback(void* arg)
{
    xSemaphoreGive(xSemaphore);
}

static void gprof_timer_init(void)
{
    const esp_timer_create_args_t gprof_timer_args = {
        .callback = &gprof_timer_callback,
        .name = "gprof_timer"
    };

    ESP_ERROR_CHECK(esp_timer_create(&gprof_timer_args, &gprof_timer));
}

static void example_lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) drv->user_data;
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    // copy a buffer's content to a specific area of the display
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);
    lv_disp_flush_ready(drv);
}

static void example_increase_lvgl_tick(void *arg)
{
    /* Tell LVGL how many milliseconds has elapsed */
    lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}

void app_main(void)
{
    gprof_init();

    xSemaphore = xSemaphoreCreateBinary();
    if (xSemaphore == NULL) {
        ESP_LOGE(TAG, "xSemaphore create failed");
        return;
    }

    xTaskCreate(gprof_end, "gprof_end", 1024 * 4, NULL, 3, NULL);

    gprof_timer_init();

    esp_timer_start_once(gprof_timer, GPROF_TIME);

    static lv_disp_draw_buf_t disp_buf; // contains internal graphic buffer(s) called draw buffer(s)
    static lv_disp_drv_t disp_drv;      // contains callback functions

    ESP_LOGI(TAG, "Install RGB panel driver");
    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_rgb_panel_config_t panel_config = {
        .data_width = 16, // RGB565 in parallel mode, thus 16bit in width
        .psram_trans_align = 64,
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .disp_gpio_num = EXAMPLE_PIN_NUM_DISP_EN,
        .pclk_gpio_num = EXAMPLE_PIN_NUM_PCLK,
        .vsync_gpio_num = EXAMPLE_PIN_NUM_VSYNC,
        .hsync_gpio_num = EXAMPLE_PIN_NUM_HSYNC,
        .de_gpio_num = EXAMPLE_PIN_NUM_DE,
        .data_gpio_nums = {
            EXAMPLE_PIN_NUM_DATA0,
            EXAMPLE_PIN_NUM_DATA1,
            EXAMPLE_PIN_NUM_DATA2,
            EXAMPLE_PIN_NUM_DATA3,
            EXAMPLE_PIN_NUM_DATA4,
            EXAMPLE_PIN_NUM_DATA5,
            EXAMPLE_PIN_NUM_DATA6,
            EXAMPLE_PIN_NUM_DATA7,
            EXAMPLE_PIN_NUM_DATA8,
            EXAMPLE_PIN_NUM_DATA9,
            EXAMPLE_PIN_NUM_DATA10,
            EXAMPLE_PIN_NUM_DATA11,
            EXAMPLE_PIN_NUM_DATA12,
            EXAMPLE_PIN_NUM_DATA13,
            EXAMPLE_PIN_NUM_DATA14,
            EXAMPLE_PIN_NUM_DATA15,
        },
        .timings = {
            .pclk_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
            .h_res = EXAMPLE_LCD_H_RES,
            .v_res = EXAMPLE_LCD_V_RES,
            // The following parameters should refer to LCD spec
            .hsync_back_porch = 88,
            .hsync_front_porch = 40,
            .hsync_pulse_width = 48,
            .vsync_back_porch = 32,
            .vsync_front_porch = 13,
            .vsync_pulse_width = 3,
            .flags.pclk_active_neg = true,
        },
        .flags.fb_in_psram = 1, // allocate frame buffer in PSRAM
    };
    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&panel_config, &panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

    ESP_LOGI(TAG, "Initialize LVGL library");
    lv_init();
    // alloc draw buffers used by LVGL from PSRAM
    lv_color_t *buf1 = heap_caps_malloc(EXAMPLE_LCD_H_RES * 100 * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    assert(buf1);
    lv_color_t *buf2 = heap_caps_malloc(EXAMPLE_LCD_H_RES * 100 * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    assert(buf2);
    // initialize LVGL draw buffers
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, EXAMPLE_LCD_H_RES * 100);

    ESP_LOGI(TAG, "Register display driver to LVGL");
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = EXAMPLE_LCD_H_RES;
    disp_drv.ver_res = EXAMPLE_LCD_V_RES;
    disp_drv.flush_cb = example_lvgl_flush_cb;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.user_data = panel_handle;
    lv_disp_t *disp = lv_disp_drv_register(&disp_drv);

    ESP_LOGI(TAG, "Install LVGL tick timer");
    // Tick interface for LVGL (using esp_timer to generate 2ms periodic event)
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &example_increase_lvgl_tick,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000));

    ESP_LOGI(TAG, "Display LVGL Scatter Chart");
    lv_obj_t *scr = lv_disp_get_scr_act(disp);
    example_lvgl_demo_ui(scr);

    // lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x0000FF), 0);

    while (1) {
        // raise the task priority of LVGL and/or reduce the handler period can improve the performance
        vTaskDelay(pdMS_TO_TICKS(10));
        // The task running lv_timer_handler should have lower priority than that running `lv_tick_inc`
        lv_timer_handler();
    }
}
