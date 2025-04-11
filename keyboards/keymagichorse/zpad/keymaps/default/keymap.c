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

// 在EEPROM中保存RGB配置
void eeconfig_update_rgb_state(void) {
    eeconfig_update_rgb_matrix(rgb_matrix_config.raw);
}

// RGB矩阵效果变化时的回调函数
void rgb_matrix_indicators_user(void) {
    // 当RGB状态发生变化时，保存到EEPROM
    eeconfig_update_rgb_state();
}

void keyboard_post_init_user(void) {
    // 从EEPROM中读取RGB配置
    rgb_matrix_config.raw = eeconfig_read_rgb_matrix();
    
    // 应用保存的配置
    rgb_matrix_mode_noeeprom(rgb_matrix_config.mode);
    rgb_matrix_sethsv_noeeprom(rgb_matrix_config.hsv.h, rgb_matrix_config.hsv.s, rgb_matrix_config.hsv.v);
    rgb_matrix_set_speed_noeeprom(rgb_matrix_config.speed);
}

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [0] = LAYOUT_19_tsangan_split_rshift(
    KC_ESC,                    MO(1),
    KC_NUM,  KC_PSLS, KC_PAST, KC_PMNS,
    KC_P7,   KC_P8,   KC_P9,   KC_PPLS,
    KC_P4,   KC_P5,   KC_P6,      
    KC_P1,   KC_P2,   KC_P3,   KC_PENT,
    KC_P0,            KC_PDOT),
  [1] = LAYOUT_19_tsangan_split_rshift(
    KC_TRNS,                   KC_TRNS, 
    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, 
    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, 
    KC_TRNS, KC_TRNS, KC_TRNS, 
    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
    KC_TRNS,          KC_TRNS)
};
