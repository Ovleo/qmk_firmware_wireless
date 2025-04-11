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
#pragma once


#ifdef BLUETOOTH_BHQ
// Its active level is "BHQ_IRQ_AND_INT_LEVEL of bhq.h "
#   define BHQ_IQR_PIN          A1
#   define BHQ_INT_PIN          A0
#   define USB_POWER_SENSE_PIN  A10     // USB插入检测引脚

#   define UART_DRIVER          SD2
#   define UART_TX_PIN          A2
#   define UART_TX_PAL_MODE     7
#   define UART_RX_PIN          A3
#   define UART_RX_PAL_MODE	    7

#   define WS2812_POWER_PIN     B7
// STM32使用到的高速晶振引脚号，做低功耗需要用户配置，每款芯片有可能不一样的
#   define LPM_STM32_HSE_PIN_IN     H1
#   define LPM_STM32_HSE_PIN_OUT    H0

#endif

/*************************matrix**************************/
/* key matrix size */
#define MATRIX_ROWS 6
#define MATRIX_COLS 4
/* COL2ROW or ROW2COL */
#define DIODE_DIRECTION ROW2COL

/* key matrix pins */
#define MATRIX_ROW_PINS { NO_PIN, A8, B15, B8, B9, C13 }
                        //  Q7      Q6      Q5     Q4
#define MATRIX_COL_PINS { NO_PIN, NO_PIN, NO_PIN, NO_PIN}
// 列 接到 74HC595 的哪个 Qx
#define COL_TO_74HC595_PINS  { _1_595_Q7, _1_595_Q6, _1_595_Q5, _1_595_Q4}
// 定义列映射到 74HC595 的管脚数量
#define COL_TO_74HC595_PINS_COUNT   4
// 第几个列开始接到595
#define COL_TO_74HC595_START_INDEX  0
// 到第几个列结束到595
#define COL_TO_74HC595_END_INDEX    3
#define I_595_NUM   1       // 用了多少个595
/* Pin connected to DS of 74HC595 */
#define DS_PIN_74HC595      A4
/* Pin connected to SH_CP of 74HC595 */
#define SHCP_PIN_74HC595    A6
/* Pin connected to ST_CP of 74HC595 */
#define STCP_PIN_74HC595    A5


/*************************matrix**************************/



// #define WS2812_BYTE_ORDER       WS2812_BYTE_ORDER_GRB

// #define RGBLIGHT_LAYERS
// #define RGBLIGHT_LAYER_BLINK
// #define RGBLIGHT_LAYERS_OVERRIDE_RGB_OFF
// #define RGBLIGHT_LAYERS_RETAIN_VAL

// #define RGB_MATRIX_ENABLE
// #define RGB_MATRIX_LED_COUNT 4  // 根据实际LED数量修改
// #define RGB_MATRIX_DEFAULT_MODE RGB_MATRIX_CUSTOM_effects
// #define RGB_MATRIX_DEFAULT_HUE 0
// #define RGB_MATRIX_DEFAULT_SAT 255
// #define RGB_MATRIX_DEFAULT_VAL 128
// #define RGB_MATRIX_DEFAULT_SPD 127
#define ENABLE_RGB_MATRIX_CUSTOM_effects  // 启用自定义效果
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// RGB Matrix相关
// #define WS2812_DI_PIN B6                            //
#define RGB_MATRIX_LED_COUNT 4                    // 总共驱动的LED数量

//#define RGB_MATRIX_CENTER { 112, 32 }             //

//#define RGB_MATRIX_KEYRELEASES                    // 针对按键触发的灯光效果,改为从松开按键执行灯光而不是按下按键执行灯光.
//#define RGB_MATRIX_TIMEOUT 0                      // RGB是否在X毫秒后自动关闭,默认为0(常开)
//#define RGB_MATRIX_SLEEP                          // 待机时关闭.
#define RGB_MATRIX_SLEEP             // USB待机（电脑没开机）时禁用灯光确保宝宝睡眠
//#define RGB_MATRIX_LED_PROCESS_LIMIT (RGB_MATRIX_LED_COUNT + 4) / 5 // 限制每个灯效动画中同时要处理的 LED 数量(降低可以提高键盘响应能力)
//#define RGB_MATRIX_LED_FLUSH_LIMIT 16             // 以ms为单位限制灯效动画更新 LED 的频率.16(16ms)相当于限制为60fps(降低可以提高键盘响应能力)
#define RGB_MATRIX_MAXIMUM_BRIGHTNESS 255           // RGB最大亮度,如果键盘灯太多会造成包括但不限于闪瞎眼睛,USB耗电量剧增,寿命下降等问题,但RGB键盘亮度不设置为255那叫个毛RGB键盘啊
//#define RGB_MATRIX_DEFAULT_ON true                // 设置默认是否自动打开RGB,适用于配置了RGB但默认不启动的情况,默认为true
//#define RGB_MATRIX_DEFAULT_MODE RGB_MATRIX_CYCLE_LEFT_RIGHT // 设置默认启动的RGB模式,默认为RGB_MATRIX_CYCLE_LEFT_RIGHT
//#define RGB_MATRIX_DEFAULT_HUE 0                  // 设置默认的色调值,默认为0
//#define RGB_MATRIX_DEFAULT_SAT 255                // 设置默认的饱和度,默认为0
//#define RGB_MATRIX_DEFAULT_VAL RGB_MATRIX_MAXIMUM_BRIGHTNESS // 设置默认的亮度,默认为上文定义的RGB_MATRIX_MAXIMUM_BRIGHTNESS(最大亮度)
//#define RGB_MATRIX_DEFAULT_SPD 127                // 设置默认的效果速度
//#define RGB_MATRIX_DEFAULT_FLAGS LED_FLAG_ALL     // 给所有按键默认设置分组,默认全分组
//#define RGB_MATRIX_DISABLE_KEYCODES               // 禁用通过按键调整RGB Matrix的功能,强制锁定RGB模式用
//#define RGB_TRIGGER_ON_KEYDOWN                    // RGB将在被按下后触发事件,能让RGB工作的更加领命,但存在兼容性问题部分MCU无法使用,尚未测试

//仅限分体式键盘
//#define RGB_MATRIX_SPLIT { X, Y }                 // 分体式键盘设置左右各有多少个LED,其中X = 左侧,Y = 右侧.
//如果启用了 RGB_MATRIX_SOLID_REACTIVE 这类按键触发灯效,需要同时设置 #define SPLIT_TRANSPORT_MIRROR

//                                                     效果名称                                       调整色调（HUI/HUD）          调整饱和度（SAI/SAD）          调整效果速度（SPI/SPD）
#define ENABLE_RGB_MATRIX_ALPHAS_MODS               // 单色                                           调整色调                    调整饱和度                     没用
#define ENABLE_RGB_MATRIX_GRADIENT_UP_DOWN          // 上下渐变                                       调整色调                     调整饱和度                     调整上下渐变宽度
#define ENABLE_RGB_MATRIX_GRADIENT_LEFT_RIGHT       // 左右渐变                                       调整色调                     调整饱和度                     调整左右渐变宽度
#define ENABLE_RGB_MATRIX_BREATHING                 // 单色呼吸循环                                   调整色调                    调整饱和度                    调整上下渐变宽度
#define ENABLE_RGB_MATRIX_BAND_SAT                  // 有一个固定底色,同时上面有左向右的单色调带状循环   没用                        没用                          调整循环速度
#define ENABLE_RGB_MATRIX_BAND_VAL                  // 没有固定背光,同时上面有左向右的单色调带状循环     修改渐变带的色调             修改渐变带的饱和度             调整循环速度
#define ENABLE_RGB_MATRIX_BAND_PINWHEEL_SAT         // 有一个规定底色,同时上面有三个循环风扇一样的循环   修改叶轮的色调               修改叶轮的饱和度               调整旋转速度
#define ENABLE_RGB_MATRIX_BAND_PINWHEEL_VAL         // 没有固定背光,同时上面有三个循环风扇一样的循环     修改叶轮的色调               修改叶轮的饱和度               调整旋转速度
#define ENABLE_RGB_MATRIX_BAND_SPIRAL_SAT           // 有一个固定底色,同时上面有螺旋状的单色循环         修改螺旋的色调              修改螺旋的饱和度               调整旋转速度
#define ENABLE_RGB_MATRIX_BAND_SPIRAL_VAL           // 没有固定背光,同时上面有螺旋状的单色循环           修改螺旋的色调              修改螺旋的饱和度               调整旋转速度
#define ENABLE_RGB_MATRIX_CYCLE_ALL                 // 全键盘纯色循环                                   没用                       没用                          调整渐变切换速度
#define ENABLE_RGB_MATRIX_CYCLE_LEFT_RIGHT          // 左往右全键盘纯色循环                             没用                        没用                         调整渐变切换速度
#define ENABLE_RGB_MATRIX_CYCLE_UP_DOWN             // 上往下全键盘纯色循环                             没用                        没用                         调整渐变切换速度
#define ENABLE_RGB_MATRIX_RAINBOW_MOVING_CHEVRON    // 以V形从左往右进行渐变                            调整背光的色调               调整渐变区的色调              调整渐变切换速度
#define ENABLE_RGB_MATRIX_CYCLE_OUT_IN              // 从外侧往内侧滚动渐变                             没用                        没用                         调整渐变切换速度
#define ENABLE_RGB_MATRIX_CYCLE_OUT_IN_DUAL         // 同时由外向内和由内向外的滚动渐变                  没用                        没用                         调整渐变切换速度
#define ENABLE_RGB_MATRIX_CYCLE_PINWHEEL            // 围绕中心,叶轮状的循环                           没用                        没用                         调整螺旋循环速度
#define ENABLE_RGB_MATRIX_CYCLE_SPIRAL              // 围绕中心,螺旋状的循环                           没用                        没用                         调整螺旋循环速度
#define ENABLE_RGB_MATRIX_DUAL_BEACON               // 双梯度循环,具体意义没看懂                       似乎有用                     没用                         调整循环渐变速度
#define ENABLE_RGB_MATRIX_RAINBOW_BEACON            // 左右侧有两个梯度循环                             没用                        没用                         调整梯度循环苏大夫
#define ENABLE_RGB_MATRIX_RAINBOW_PINWHEELS         // 左右中间有两个色彩叶轮循环                       没用                        没用                          调整叶轮旋转的速度
#define ENABLE_RGB_MATRIX_RAINDROPS                 // 随机雨滴修改某个按键的色调                       没用                        没用                          没用
#define ENABLE_RGB_MATRIX_JELLYBEAN_RAINDROPS       // 随机雨滴,但是会修改色调和饱和度                  没用                        没用                         没用
#define ENABLE_RGB_MATRIX_HUE_BREATHING             // 带呼吸效果的色调渐变                             调整底色的色调               调整呼吸色的饱和度             调整呼吸的速度
#define ENABLE_RGB_MATRIX_HUE_PENDULUM              // 带一根颜色条移动的色调渐变                       调整底色的色调               调整呼吸色的饱和度              调整呼吸的速度
#define ENABLE_RGB_MATRIX_HUE_WAVE                  // 带从左往右波动的色调渐变                         调整底色的色调               调整呼吸色的饱和度             调整呼吸的速度
#define ENABLE_RGB_MATRIX_PIXEL_FRACTAL             // 全按键随机灯光？                                没用                         没用                        没用
#define ENABLE_RGB_MATRIX_PIXEL_FLOW                // 沿着LED布线顺序的随机灯光                        没用                        没用                        没用
#define ENABLE_RGB_MATRIX_PIXEL_RAIN                // 随机按键,然后沿着LED布线顺序左右渐变             调整色调                    调整饱和度                    调整速度

#define RGB_MATRIX_FRAMEBUFFER_EFFECTS              //

#define ENABLE_RGB_MATRIX_TYPING_HEATMAP            // 没有底色,但会根据按键提升区域热度,从篮往红       没用                          没用                        没用
#define ENABLE_RGB_MATRIX_DIGITAL_RAIN              // 如同下雨一般的从上往下下雨                        没用                          没用                       没用

#define RGB_MATRIX_KEYPRESSES                       //

#define ENABLE_RGB_MATRIX_SOLID_REACTIVE_SIMPLE     // 按键灯就亮                                     调整色调                        调整饱和度                  调整灯消失的速度
#define ENABLE_RGB_MATRIX_SOLID_REACTIVE            // 有一个背光的情况下,按键灯就亮                   调整背景色和按键的色调           调整背景色和按键的饱和度     调整灯消失的速度
#define ENABLE_RGB_MATRIX_SOLID_REACTIVE_WIDE       // 按键灯,区域亮灯                                调整亮灯的色调                  调整亮灯的饱和度            调整灯消失的速度
#define ENABLE_RGB_MATRIX_SOLID_REACTIVE_MULTIWIDE  // 同上,但是同时允许多个区域                       调整亮灯的色调                  调整亮灯的饱和度            调整灯消失的速度
#define ENABLE_RGB_MATRIX_SOLID_REACTIVE_CROSS      // 按键灯,但渐变扩散到整排                         调整亮灯的色调                  调整亮灯的饱和度            调整灯消失的速度
#define ENABLE_RGB_MATRIX_SOLID_REACTIVE_MULTICROSS // 同上,但是同时允许多个区域                       调整亮灯的色调                  调整亮灯的饱和度            调整灯消失的速度
#define ENABLE_RGB_MATRIX_SOLID_REACTIVE_NEXUS      // 从中间往旁边扩散的,广域按键灯                    调整亮灯的色调                  调整亮灯的饱和度            调整灯消失的速度
#define ENABLE_RGB_MATRIX_SOLID_REACTIVE_MULTINEXUS // 同上,但同时允许多个区域                         调整亮灯的色调                  调整亮灯的饱和度            调整灯消失的速度
#define ENABLE_RGB_MATRIX_SPLASH                    // 按键后,扩散到全键盘的彩色水波效果                调整亮灯的色调                  调整亮灯的饱和度            调整灯消失的速度
#define ENABLE_RGB_MATRIX_MULTISPLASH               // 同上,但同时允许多个区域                         调整亮灯的色调                  调整亮灯的饱和度            调整灯消失的速度
#define ENABLE_RGB_MATRIX_SOLID_SPLASH              // 按键后,扩散到全键盘的单色水波效果                调整亮灯的色调                  调整亮灯的饱和度            整灯消失的速度
#define ENABLE_RGB_MATRIX_SOLID_MULTISPLASH         // 同上,但同时允许多个区域                         调整亮灯的色调                  调整亮灯的饱和度            调整灯消失的速度

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MIDI功能相关
//#define MIDI_ADVANCED                              // 启动高级MIDI
