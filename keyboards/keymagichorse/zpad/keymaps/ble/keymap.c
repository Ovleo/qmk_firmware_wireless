/* Copyright 2024 keymagichorse
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 #include QMK_KEYBOARD_H
 #include "config.h"
 #include "bhq.h"
 #include "outputselect.h"
 #include "usb_main.h"
 #include "km_printf.h"
 #include "ws2812.h"

 #if defined(BLUETOOTH_BHQ)
 #   include "bhq.h"
 #endif

 #   if defined(KB_LPM_ENABLED)
 #   include "lpm.h"
 #endif

 #include "analog.h"

 // 键盘传输模式枚举
 enum  kb_transfer_mode_enum {
     KB_USB_MODE = 1,
     KB_BLE_1_MODE,
     KB_BLE_2_MODE,
     KB_BLE_3_MODE,
     KB_RF_MODE,
 };

 typedef union {
   uint32_t raw;
   struct {
      uint8_t transfer_mode :8;  // 保存 键盘传输模式
      bool rgb_anim :1;          // 保存 自定义的 RGB 灯 动画
   };
 } user_config_t;
 user_config_t user_config = {0};
 uint32_t battery_timer = 0;

 enum keyboard_keycodes {
     BT_1 = QK_KB_0,
     BT_2,
     BT_3,
     BT_4,
     BT_5,
     BT_6,
     BT_7,
     BT_8,
     BT_9,
     RGB_TOGG,
 };
 #define BLE_TOG     BT_1    // 切换蓝牙输出 并 开启蓝牙广播（非配对类型）
 #define RF_TOG      BT_2    // 切换 2.4ghz输出
 #define USB_TOG     BT_3    // 打开USB
 #define BL_SW_0     BT_4    // 开启蓝牙通道0（需要打开蓝牙的条件下才行） 短按打开广播 长按开启配对广播
 #define BL_SW_1     BT_5    // 开启蓝牙通道1（需要打开蓝牙的条件下才行） 短按打开广播 长按开启配对广播
 #define BL_SW_2     BT_6    // 开启蓝牙通道2（需要打开蓝牙的条件下才行） 短按打开广播 长按开启配对广播
 // #define BLE_DEL     BT_7    // 删除当前蓝牙绑定
 // #define BLE_CLR     BT_8    // 清空所有蓝牙绑定
 #define BLE_OFF     BT_9    // 关闭蓝牙连接
 #define RGB_TOGG    RGB_TOGG

 const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
     [0] = LAYOUT(
       KC_ESC,                    MO(1),
       KC_NUM, BLE_TOG , RGB_VAD, RGB_M_R,
       RGB_RMOD, RGB_MOD, RGB_TOGG, RGB_HUD,
       RGB_HUI, RGB_SAD, USB_TOG,
       BL_SW_0, BL_SW_1, BL_SW_2, KC_P7,
       KC_SCRL,            KC_CAPS),
     [1] = LAYOUT(
       KC_TRNS,                   KC_TRNS,
       BLE_TOG, RF_TOG, BL_SW_0, BL_SW_1,
       BL_SW_2, BLE_OFF, RGB_TOGG, KC_TRNS,
       KC_TRNS, KC_TRNS, KC_TRNS,
       KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
       KC_TRNS,          KC_TRNS),
     [2] = LAYOUT(
       KC_TRNS,                   KC_TRNS,
       KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
       KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
       KC_TRNS, KC_TRNS, KC_TRNS,
       KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
       KC_TRNS,          KC_TRNS),
     [3] = LAYOUT(
       KC_TRNS,                   KC_TRNS,
       KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
       KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
       KC_TRNS, KC_TRNS, KC_TRNS,
       KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
       KC_TRNS,          KC_TRNS),
   };


   void eeconfig_init_kb(void)
   {
       // 上电先读取一次
       user_config.raw = eeconfig_read_user();
       // 判断传输模式有没有被初始化过
       if(user_config.transfer_mode == 0 || user_config.transfer_mode == 0xff)
       {
           // 表示没有被初始化过
           user_config.transfer_mode = KB_USB_MODE;    // 默认先初始化为USB
           set_output(OUTPUT_USB);
           eeconfig_update_user(user_config.raw);
           return;
       }

       if(user_config.transfer_mode == KB_USB_MODE)
       {
           set_output(OUTPUT_USB);
       }

       if(user_config.transfer_mode > KB_USB_MODE && user_config.transfer_mode != 0xff)
       {
           set_output(OUTPUT_BLUETOOTH);
       }

   }


   // --------------------  都是用于处理按键触发的变量 --------------------
   // 这几个变量大致的功能就是用来 作长按短按的。
   // 长按打开配对蓝牙广播  短按打开非配对蓝牙广播
   static uint32_t key_output_mode_press_time = 0; // 输出模式按下计时器
   static uint32_t key_ble_switch_press_time = 0;  // 蓝牙切换通道 按下计时器
   static uint8_t key_ble_host_index = 0;          // 这里是用于按键按下的
   // --------------------  都是用于处理按键触发的变量 --------------------


   // --------------------  蓝牙模块返回的状态 --------------------
   uint8_t advertSta = 0;      // 蓝牙广播状态
   uint8_t connectSta = 0;     // 连接状态
   uint8_t pairingSta = 0;     // 蓝牙是否开启配对广播
   uint8_t host_index = 255;   // 蓝牙通道 这里是模块返回的
   // --------------------  蓝牙模块返回的状态 --------------------

   //保存灯光效果避免每次上电都亮
void keyboard_post_init_user(void) {
    user_config.raw = eeconfig_read_user(); // 读取保存的配置
}
void eeconfig_init_user(void) {
    user_config.raw = 0;
    user_config.rgb_anim = 1;
    eeconfig_update_user(user_config.raw);
}

   bool process_record_user(uint16_t keycode, keyrecord_t *record) {
       battery_timer = 0;
   #   if defined(KB_LPM_ENABLED)
       lpm_timer_reset();  // 这里用于低功耗，按下任何按键刷新低功耗计时器
   #endif

       // 没有连接
       if (connectSta == 0)
       {
           // 检查按键值是否不在 BT_1 到 BT_11 范围
          if (keycode < BT_1 || keycode > BT_9)
           {
               // 检查传输模式是否为蓝牙模式
               if (
                   user_config.transfer_mode == KB_BLE_1_MODE ||
                   user_config.transfer_mode == KB_BLE_2_MODE ||
                   user_config.transfer_mode == KB_BLE_3_MODE
               )
               {
                   // KB_BLE_1_MODE 在枚举 里面是2、在蓝牙通道内是0
                   // 那么 2 - 2 = 0 那就是host = 0;
                   // 重新打开非配对蓝牙广播。如已开启蓝牙广播或已连接，那么不会断开当前的蓝牙连接。
                   bhq_AnewOpenBleAdvertising(user_config.transfer_mode - 2, 15);
               }
           }
       }


       switch (keycode)
       {
           case BLE_TOG:
           {
               if(record->event.pressed)
               {
                   key_output_mode_press_time = timer_read32();
               }
               else
               {
                   // gpio_write_pin_high(QMK_RUN_OUTPUT_PIN);
                   if(timer_elapsed32(key_output_mode_press_time) >= 300)
                   {
                       key_ble_host_index = 0;
                       // 打开非配对模式蓝牙广播 10 = 10S
                       bhq_OpenBleAdvertising(key_ble_host_index, 30);
                       set_output(OUTPUT_BLUETOOTH);

                       // 这里切换蓝牙模式，默认是打开第一个蓝牙通道
                       user_config.transfer_mode = KB_BLE_1_MODE;
                       eeconfig_update_user(user_config.raw);
                   }
               }
               return false;
           }
           case RF_TOG:
           {
               if(record->event.pressed)
               {
                   key_output_mode_press_time = timer_read32();
               }
               else
               {
                   // gpio_write_pin_high(QMK_RUN_OUTPUT_PIN);
                   if(timer_elapsed32(key_output_mode_press_time) >= 300)
                   {
                       // TODO: 等待bhq驱动完善，这里还是用蓝牙输出来作为qmk的模式切换，在蓝牙模块内会切换成2.4ghz私有连接
                       bhq_switch_rf_easy_kb();
                       set_output(OUTPUT_BLUETOOTH);

                       user_config.transfer_mode = KB_RF_MODE;
                       eeconfig_update_user(user_config.raw);
                   }
               }
               return false;
           }
           case USB_TOG:
           {
               if(record->event.pressed)
               {
                   key_output_mode_press_time = timer_read32();
               }
               else
               {
                   if(timer_elapsed32(key_output_mode_press_time) >= 300)
                   {
                       // 切换到usb模式 并 关闭蓝牙广播
                       set_output(OUTPUT_USB);
                       bhq_CloseBleAdvertising();

                       user_config.transfer_mode = KB_USB_MODE;
                       eeconfig_update_user(user_config.raw);
                   }
               }
               return false;
           }
           case BLE_OFF:
           {
               if(record->event.pressed)
               {
                   key_output_mode_press_time = timer_read32();
               }
               else
               {
                   if(timer_elapsed32(key_output_mode_press_time) >= 500)
                   {
                       // 关闭蓝牙广播
                       bhq_CloseBleAdvertising();
                   }
               }
               return false;
           }
           case BL_SW_0:
           case BL_SW_1:
           case BL_SW_2:
               if(record->event.pressed)
               {
                   key_ble_switch_press_time = timer_read32();
               }
               else
               {
                   switch (keycode)
                   {
                       case BL_SW_0:
                           key_ble_host_index = 0;
                           break;
                       case BL_SW_1:
                           key_ble_host_index = 1;
                           break;
                       case BL_SW_2:
                           key_ble_host_index = 2;
                           break;
                   }
                   if(timer_elapsed32(key_ble_switch_press_time) >= 100 && timer_elapsed32(key_ble_switch_press_time) <= 800)
                   {
                       // 打开非配对模式蓝牙广播 10 = 10S
                       bhq_OpenBleAdvertising(key_ble_host_index, 30);
                       set_output(OUTPUT_BLUETOOTH);

                       // 这里枚举 + 蓝牙通道就能计算出 KB_BLE_1_MODE、KB_BLE_2_MODE、KB_BLE_3_MODE
                       user_config.transfer_mode = KB_BLE_1_MODE + key_ble_host_index;
                       eeconfig_update_user(user_config.raw);
                   }
                   else if(timer_elapsed32(key_ble_switch_press_time) >= 1000)
                   {
                       // 打开 配对模式蓝牙广播 10 = 10S
                       bhq_SetPairingMode(key_ble_host_index, 30);
                       set_output(OUTPUT_BLUETOOTH);

                       // 这里枚举 + 蓝牙通道就能计算出 KB_BLE_1_MODE、KB_BLE_2_MODE、KB_BLE_3_MODE
                       user_config.transfer_mode = KB_BLE_1_MODE + key_ble_host_index;
                       eeconfig_update_user(user_config.raw);
                   }
               }
               return false;
           case RGB_TOGG:
               if (record->event.pressed) { // 按下
                   user_config.rgb_anim ^= 1; // 切换状态
                   eeconfig_update_user(user_config.raw); // 保存状态
               }
               return false;
       }
       if(connectSta == 0 && user_config.transfer_mode > KB_USB_MODE)
       {
           return false;
       }
       return true;
   }

// BHQ Status callback   BHQ状态回调函数
void BHQ_State_Call(uint8_t cmdid, uint8_t *dat)
{
    if(cmdid == BHQ_ACK_RUN_STA_CMDID)
    {
        advertSta = BHQ_GET_BLE_ADVERT_STA(dat[1]);
        connectSta = BHQ_GET_BLE_CONNECT_STA(dat[1]);
        pairingSta = BHQ_GET_BLE_PAIRING_STA(dat[1]);
        host_index = dat[2];
    }else if(cmdid == BHQ_ACK_LED_LOCK_CMDID){
    }
}
bool rgb_matrix_indicators_kb(void) {
    static uint32_t led_timer = 0;
    static bool led_state = false;
    uint16_t fast_blink_interval = 200;  // 快速闪烁间隔(ms)
    uint16_t slow_blink_interval = 500;  // 慢速闪烁间隔(ms)

    // 更新LED状态
    if (timer_elapsed32(led_timer) >= (pairingSta ? fast_blink_interval : slow_blink_interval)) {
        led_timer = timer_read32();
        led_state = !led_state;
    }

    // 如果RGB动画关闭，清除所有LED
    if (!user_config.rgb_anim) {
        rgb_matrix_set_color_all(0, 0, 0);
    }

    if (!rgb_matrix_indicators_user()) {
        return false;
    }

    // 处理指示灯，无论RGB动画是否开启都要处理
    // CAPS LOCK 指示灯
    if (host_keyboard_led_state().caps_lock) {
        rgb_matrix_set_color(2, 255, 0, 0);  // 红色
    }
    // NUM LOCK 指示灯
    if (host_keyboard_led_state().num_lock) {
        rgb_matrix_set_color(3, 0, 255, 0);  // 绿色
    }
    // SCROLL LOCK 指示灯
    if (host_keyboard_led_state().scroll_lock) {
        rgb_matrix_set_color(1, 0, 0, 255);  // 蓝色
    }

    // 蓝牙状态指示灯
    static uint32_t connect_led_timer = 0;
    static bool last_connect_state = false;
    void set_ble_indicator(uint8_t led_index) {
        if (connectSta == 1) {
            // 检测连接状态变化
            if (last_connect_state != connectSta) {
                connect_led_timer = timer_read32();
                last_connect_state = connectSta;
            }
            // 已连接状态 - 闪烁5S，然后常亮2S，最后关闭
            uint32_t elapsed_time = timer_elapsed32(connect_led_timer);
            if (elapsed_time < 5000) {
                // 前5秒闪烁
                if (led_state) {
                    rgb_matrix_set_color(led_index, 0, 0, 255);
                }
            } else if (elapsed_time < 7000) {
                // 5-7秒常亮
                rgb_matrix_set_color(led_index, 0, 0, 255);
            }
        } else {
            last_connect_state = connectSta;
            // 未连接状态 - 蓝牙广播状态指示灯
            if (advertSta == 1) {
                if (led_state) {
                    if (pairingSta == 1) {
                        // 配对模式 - 快速闪烁紫色
                        rgb_matrix_set_color(led_index, 0, 0, 255);
                    } else {
                        // 广播模式 - 慢速闪烁蓝色
                        rgb_matrix_set_color(led_index, 0, 0, 255);
                    }
                }
            }
        }
    }

    // 根据当前活动的蓝牙通道设置对应LED
    if (host_index == 0) {
        set_ble_indicator(0);
    } else if (host_index == 1) {
        set_ble_indicator(1);
    } else if (host_index == 2) {
        set_ble_indicator(2);
    }

    // 根据当前层设置指示灯
    uint8_t layer = get_highest_layer(layer_state);
    if (layer > 0) {
        rgb_matrix_set_color(3, 0, 0, 255);    // 蓝色指示当前层
    }

    return true;
}


bool led_update_user(led_t led_state) {
    // 强制更新 RGB Matrix 指示灯状态
    rgb_matrix_indicators_kb();

    // 记录状态变化
    static led_t last_state;
    if (last_state.raw != led_state.raw) {
        last_state = led_state;
        km_printf("LED state changed: CAPS:%d NUM:%d SCROLL:%d\r\n",
                 led_state.caps_lock,
                 led_state.num_lock,
                 led_state.scroll_lock);
    }

    return true;  // 允许默认的 LED 处理继续进行
}





   // After initializing the peripheral
   void keyboard_post_init_kb(void)
   {
       ws2812_init();
       gpio_set_pin_output(WS2812_POWER_PIN);        // ws2812 power
       gpio_write_pin_low(WS2812_POWER_PIN);         // 默认开启WS2812电源

       gpio_set_pin_input_low(BHQ_IQR_PIN);        // Module operating status.
       gpio_set_pin_output(BHQ_INT_PIN);            // The qmk has a data request.

       gpio_set_pin_output(BHQ_INT_PIN);
       gpio_write_pin_high(BHQ_INT_PIN);

   #   if defined(KM_DEBUG)
       km_printf_init();
       km_printf("hello rtt log1111111\r\n");
   #   endif

   #if defined(BLUETOOTH_BHQ)
       bhkDevConfigInfo_t model_parma = {
           .vendor_id_source   = 1,
           .verndor_id         = VENDOR_ID,
           .product_id         = PRODUCT_ID,

           .le_connection_interval_min = 100,
           .le_connection_interval_max =300,
           .le_connection_interval_timeout =500,
           .tx_poweer = 0x3D,

           .mk_is_read_battery_voltage = FALSE,
           .mk_adc_pga = 1,
           .mk_rvd_r1 = 0,
           .mk_rvd_r2 = 0,

           .sleep_1_s = 30,
           .sleep_2_s = 300,

           .bleNameStrLength = strlen(PRODUCT),
           .bleNameStr = PRODUCT
       };
       bhq_ConfigRunParam(model_parma);    // 将配置信息发送到无线模块中
   #endif

       // 初始化RGB Matrix
       rgb_matrix_enable();

       // 读取保存的配置
       user_config.raw = eeconfig_read_user();

    //    // 根据保存的状态设置RGB
    //    if (user_config.rgb_anim) {
    //        rgb_matrix_mode(RGB_MATRIX_BREATHING); // 使用默认的呼吸效果
    //    } else {
    //        rgb_matrix_mode(RGB_MATRIX_NONE);
    //    }

       keyboard_post_init_user();
   }

   #   if defined(KB_LPM_ENABLED)
   // 修改低功耗控制函数
void lpm_device_power_open(void)
{
#if defined(RGB_MATRIX_ENABLE)
    // ws2812电源开启
    ws2812_init();
    gpio_set_pin_output(WS2812_POWER_PIN);        // ws2812 power
    gpio_write_pin_low(WS2812_POWER_PIN);

    // 恢复 RGB Matrix 状态
    rgb_matrix_enable_noeeprom();
#endif
}

void lpm_device_power_close(void)
{
#if defined(RGB_MATRIX_ENABLE)
    // 关闭所有 LED
    rgb_matrix_set_color_all(0, 0, 0);
    rgb_matrix_disable_noeeprom();

    // ws2812电源关闭
    gpio_set_pin_output(WS2812_POWER_PIN);        // ws2812 power
    gpio_write_pin_high(WS2812_POWER_PIN);

    gpio_set_pin_output(WS2812_DI_PIN);           // ws2812 DI Pin
    gpio_write_pin_low(WS2812_DI_PIN);
#endif
}

   // 将未使用的引脚设置为输入模拟
   // PS：在6095中，如果不加以下代码休眠时是102ua。如果加了就是30ua~32ua浮动
   void lpm_set_unused_pins_to_input_analog(void)
   {
       // 禁用调试功能以降低功耗
       DBGMCU->CR &= ~DBGMCU_CR_DBG_SLEEP;   // 禁用在Sleep模式下的调试
       DBGMCU->CR &= ~DBGMCU_CR_DBG_STOP;    // 禁用在Stop模式下的调试
       DBGMCU->CR &= ~DBGMCU_CR_DBG_STANDBY; // 禁用在Standby模式下的调试
       // 在系统初始化代码中禁用SWD接口
       palSetLineMode(A13, PAL_MODE_INPUT_ANALOG);
       palSetLineMode(A14, PAL_MODE_INPUT_ANALOG);

       palSetLineMode(A0, PAL_MODE_INPUT_ANALOG);
       palSetLineMode(A1, PAL_MODE_INPUT_ANALOG);
       palSetLineMode(A2, PAL_MODE_INPUT_ANALOG);
       palSetLineMode(A3, PAL_MODE_INPUT_ANALOG);
       palSetLineMode(A4, PAL_MODE_INPUT_ANALOG);
       palSetLineMode(A5, PAL_MODE_INPUT_ANALOG);
       palSetLineMode(A6, PAL_MODE_INPUT_ANALOG);
       palSetLineMode(A7, PAL_MODE_INPUT_ANALOG);
       palSetLineMode(A8, PAL_MODE_INPUT_ANALOG);
       palSetLineMode(A9, PAL_MODE_INPUT_ANALOG);
       palSetLineMode(A10, PAL_MODE_INPUT_ANALOG);
       palSetLineMode(A11, PAL_MODE_INPUT_ANALOG);
       palSetLineMode(A13, PAL_MODE_INPUT_ANALOG);
       palSetLineMode(A14, PAL_MODE_INPUT_ANALOG);
       palSetLineMode(A15, PAL_MODE_INPUT_ANALOG);

       palSetLineMode(B0, PAL_MODE_INPUT_ANALOG);
       palSetLineMode(B1, PAL_MODE_INPUT_ANALOG);
       palSetLineMode(B2, PAL_MODE_INPUT_ANALOG);
       palSetLineMode(B3, PAL_MODE_INPUT_ANALOG);
       palSetLineMode(B4, PAL_MODE_INPUT_ANALOG);
       palSetLineMode(B5, PAL_MODE_INPUT_ANALOG);
       palSetLineMode(B6, PAL_MODE_INPUT_ANALOG);
       palSetLineMode(WS2812_POWER_PIN, PAL_MODE_INPUT_ANALOG);
       palSetLineMode(B8, PAL_MODE_INPUT_ANALOG);
       palSetLineMode(B9, PAL_MODE_INPUT_ANALOG);
       palSetLineMode(B10, PAL_MODE_INPUT_ANALOG);
       palSetLineMode(B11, PAL_MODE_INPUT_ANALOG);
       palSetLineMode(B13, PAL_MODE_INPUT_ANALOG);
       palSetLineMode(B14, PAL_MODE_INPUT_ANALOG);
       palSetLineMode(B15, PAL_MODE_INPUT_ANALOG);
   }

   #endif

   // ------------------------ 电池分压电阻的配置 ------------------------
   /* Battery voltage resistive voltage divider setting of MCU */
   #ifndef BAT_R_UPPER
   // Upper side resitor value (uint: KΩ)
   #   define BAT_R_UPPER 100
   #endif
   #ifndef BAT_R_LOWER
    // Lower side resitor value (uint: KΩ)
   #   define BAT_R_LOWER 100
   #endif
   // ------------------------ 电池分压电阻的配置 ------------------------

   // ------------------------ 电池电压读取的引脚 ------------------------
   #ifndef BATTER_ADC_PIN
   #    define BATTER_ADC_PIN     B1
   #endif
   // https://docs.qmk.fm/drivers/adc#stm32
   #ifndef BATTER_ADC_DRIVER
   #    define BATTER_ADC_DRIVER     ADCD1
   #endif
   // ------------------------ 电池电压读取的引脚 ------------------------

   // 电池电压最高最低 mv
   #define BATTER_MAX_MV   4150
   #define BATTER_MIN_MV   3500

   // 电池电压转百分比
   uint8_t calculate_battery_percentage(uint16_t current_mv) {
       if (current_mv >= BATTER_MAX_MV) {
           return 100;
       } else if (current_mv <= BATTER_MIN_MV) {
           return 0;
       } else {
           uint16_t percentage = ((current_mv - BATTER_MIN_MV) * 100) / (BATTER_MAX_MV - BATTER_MIN_MV);
           // 如果百分比超过100，确保其被限制在100以内
           if (percentage > 100) {
               percentage = 100;
           }
           return (uint8_t)percentage;
       }
   }
   void battery_percent_read_task(void)
   {
       if(battery_timer == 0)
       {
           battery_timer = timer_read32();
       }

       if (timer_elapsed32(battery_timer) > 2000)
       {
           battery_timer = 0;
           uint16_t adc = analogReadPin(BATTER_ADC_PIN);
           adc = analogReadPin(BATTER_ADC_PIN);

           uint16_t voltage_mV_Fenya = (adc * 3300) / 1023;
           uint16_t voltage_mV_actual = voltage_mV_Fenya  * (1 + (BAT_R_UPPER / BAT_R_LOWER));

           // voltage_mV_actual = voltage_mV_actual;  //
           // km_printf("adc:%d   fymv:%d  sjmv:%d  bfb:%d  \r\n",
           // adc,voltage_mV_Fenya,voltage_mV_actual,calculate_battery_percentage(voltage_mV_actual));
           // km_printf("adcState:%d\r\n",ADCD1.state);
           // 上报电池百分比到模块中
           bhq_update_battery_percent(calculate_battery_percentage(voltage_mV_actual),voltage_mV_actual);
       }
   }

   // Keyboard level code can override this, but shouldn't need to.
   // Controlling custom features should be done by overriding
   // via_custom_value_command_kb() instead.
   __attribute__((weak)) bool via_command_kb(uint8_t *data, uint8_t length) {
       uint8_t command_id   = data[0];
       uint8_t i = 0;

       // 此逻辑删除 会失去蓝牙模块升级功能 以及蓝牙改键功能！！！！！！！
       km_printf("cmdid:%02x  length:%d\r\n",command_id,length);
       km_printf("read host app of data \r\n[");
       for (i = 0; i < length; i++)
       {
           km_printf("%02x ",data[i]);
       }
       km_printf("]\r\n");

       if(command_id == 0xF1)
       {
           // cmdid + 2 frame headers
           // The third one is isack the fourth one is length and the fifth one is data frame
           BHQ_SendCmd(0, &data[4], data[3]);
           return true;
       }
       return false;
   }






led_config_t g_led_config = { {
    // Key Matrix to LED Index - 4颗LED的矩阵映射
    { 0, 1, 2, 3 }
}, {
    // LED Index to Physical Position - LED物理位置坐标
    { 0,  0 },  // LED 0
    { 74, 0 },  // LED 1
    { 149, 0 }, // LED 2
    { 224, 0 }  // LED 3
}, {
    // LED Index to Flag - LED类型标志
    LED_FLAG_ALL, LED_FLAG_ALL, LED_FLAG_ALL, LED_FLAG_ALL
} };

