/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

/**
 * @file tft_lvgl_configuration.cpp
 * @date    2020-02-21
 */

#include "../../../../inc/MarlinConfigPre.h"

#if HAS_TFT_LVGL_UI

#include "SPI_TFT.h"

#include "tft_lvgl_configuration.h"
#include "draw_ready_print.h"
#include "pic_manager.h"
#include "mks_hardware_test.h"
#include "draw_ui.h"
#include <lvgl.h>

#include "../../../../inc/MarlinConfig.h"

#include HAL_PATH(../../HAL, tft/xpt2046.h)
XPT2046 touch;

#if ENABLED(POWER_LOSS_RECOVERY)
  #include "../../../../feature/powerloss.h"
#endif

#include <SPI.h>

#ifndef TFT_WIDTH
  #define TFT_WIDTH  480
#endif
#ifndef TFT_HEIGHT
  #define TFT_HEIGHT 320
#endif

#if HAS_SPI_FLASH_FONT
  extern void init_gb2312_font();
#endif

static lv_disp_buf_t disp_buf;
#if ENABLED(SDSUPPORT)
  extern void UpdateAssets();
#endif
uint16_t DeviceCode = 0x9488;
extern uint8_t sel_id;

extern uint8_t gcode_preview_over, flash_preview_begin, default_preview_flg;

void SysTick_Callback() {
  lv_tick_inc(1);
  print_time_count();
}

extern uint8_t bmp_public_buf[17 * 1024];

void tft_lvgl_init() {

  //uint16_t test_id=0;
  W25QXX.init(SPI_QUARTER_SPEED);
  //test_id=W25QXX.W25QXX_ReadID();

  gCfgItems_init();
  ui_cfg_init();
  disp_language_init();

  //init tft first!
  SPI_TFT.spi_init(SPI_FULL_SPEED);
  SPI_TFT.LCD_init();

  #if ENABLED(SDSUPPORT)
    UpdateAssets();
  #endif
  mks_test_get();

  //spi_flash_read_test();

  touch.Init();

  lv_init();

  lv_disp_buf_init(&disp_buf, bmp_public_buf, NULL, LV_HOR_RES_MAX * 18); /*Initialize the display buffer*/

  lv_disp_drv_t disp_drv;     /*Descriptor of a display driver*/
  lv_disp_drv_init(&disp_drv);    /*Basic initialization*/
  disp_drv.flush_cb = my_disp_flush; /*Set your driver function*/
  disp_drv.buffer = &disp_buf;    /*Assign the buffer to the display*/
  lv_disp_drv_register(&disp_drv);  /*Finally register the driver*/

  lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);     /*Descriptor of a input device driver*/
  indev_drv.type = LV_INDEV_TYPE_POINTER; /*Touch pad is a pointer-like device*/
  indev_drv.read_cb = my_touchpad_read;  /*Set your driver function*/
  lv_indev_drv_register(&indev_drv);   /*Finally register the driver*/

  systick_attach_callback(SysTick_Callback);

  #if HAS_SPI_FLASH_FONT
    init_gb2312_font();
  #endif

  tft_style_init();

  filament_pin_setup();

  #if ENABLED(POWER_LOSS_RECOVERY)
    if (recovery.valid()) {
      if (gCfgItems.from_flash_pic == 1)
        flash_preview_begin = 1;
      else
        default_preview_flg = 1;

      uiCfg.print_state = REPRINTING;

      ZERO(public_buf_m);
      strncpy(public_buf_m, recovery.info.sd_filename, sizeof(public_buf_m));
      card.printLongPath(public_buf_m);

      strncpy(list_file.long_name[sel_id], card.longFilename, sizeof(list_file.long_name[sel_id]));

      lv_draw_printing();
    }
    else
  #endif
    lv_draw_ready_print();

  if (mks_test_flag == 0x1E)
    mks_gpio_test();
}

void my_disp_flush(lv_disp_drv_t * disp, const lv_area_t * area, lv_color_t * color_p) {
  uint16_t i, width, height;

  width = area->x2 - area->x1 + 1;
  height = area->y2 - area->y1 + 1;

  SPI_TFT.SetWindows((uint16_t)area->x1, (uint16_t)area->y1, width, height);
  for (i = 0; i < height; i++) {
    SPI_TFT.tftio.WriteSequence((uint16_t*)(color_p + width * i), width);
  }
  lv_disp_flush_ready(disp);       /* Indicate you are ready with the flushing*/

  W25QXX.init(SPI_QUARTER_SPEED);
}

#define TICK_CYCLE 1

static int32_t touch_time1 = 0;

unsigned int getTickDiff(unsigned int curTick, unsigned int lastTick) {
  return TICK_CYCLE * (lastTick <= curTick ? (curTick - lastTick) : (0xFFFFFFFF - lastTick + curTick));
}

static bool get_point(int16_t *x, int16_t *y) {
  bool is_touched = touch.getRawPoint(x, y);

  if (is_touched) {
    *x = int16_t((int32_t(*x) * XPT2046_X_CALIBRATION) >> 16) + XPT2046_X_OFFSET;
    *y = int16_t((int32_t(*y) * XPT2046_Y_CALIBRATION) >> 16) + XPT2046_Y_OFFSET;
  }

  #if (TFT_ROTATION & TFT_ROTATE_180)
    x = (TFT_WIDTH) - x;
    y = (TFT_HEIGHT) - y;
  #endif

  return is_touched;
}

static int16_t last_x = 0, last_y = 0;
bool my_touchpad_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data) {
  uint32_t tmpTime, diffTime = 0;

  tmpTime = millis();
  diffTime = getTickDiff(tmpTime, touch_time1);
  /*Save the state and save the pressed coordinate*/
  //data->state = TOUCH_PressValid(last_x, last_y) ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
  //if (data->state == LV_INDEV_STATE_PR)  ADS7843_Rd_Addata((u16 *)&last_x, (u16 *)&last_y);
  //touchpad_get_xy(&last_x, &last_y);
  /*Save the pressed coordinates and the state*/
  if (diffTime > 10) {
    if (get_point(&last_x, &last_y)) {

      data->state = LV_INDEV_STATE_PR;

      // Set the coordinates (if released use the last-pressed coordinates)

      data->point.x = last_x;
      data->point.y = last_y;

      last_x = last_y = 0;
    }
    else
      data->state = LV_INDEV_STATE_REL;

    touch_time1 = tmpTime;
  }

  return false; // Return `false` since no data is buffering or left to read
}

#endif // HAS_TFT_LVGL_UI
