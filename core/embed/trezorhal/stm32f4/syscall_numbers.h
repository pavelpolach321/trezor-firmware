/*
 * This file is part of the Trezor project, https://trezor.io/
 *
 * Copyright (c) SatoshiLabs
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SYSCALL_NUMBERS_H
#define SYSCALL_NUMBERS_H

// Syscall identifiers
typedef enum {

  SYSCALL_SYSTEM_EXIT = 1,
  SYSCALL_SYSTEM_EXIT_ERROR,
  SYSCALL_SYSTEM_EXIT_FATAL,

  SYSCALL_SYSTICK_CYCLES,
  SYSCALL_SYSTICK_MS,
  SYSCALL_SYSTICK_US,
  SYSCALL_SYSTICK_US_TO_CYCLES,

  SYSCALL_SECURE_SHUTDOWN,
  SYSCALL_REBOOT,
  SYSCALL_REBOOT_TO_BOOTLOADER,
  SYSCALL_REBOOT_AND_UPGRADE,

  SYSCALL_SHA256_INIT,
  SYSCALL_SHA256_UPDATE,
  SYSCALL_SHA256_FINAL,
  SYSCALL_SHA256_CALC,

  SYSCALL_DISPLAY_SET_BACKLIGHT,
  SYSCALL_DISPLAY_GET_BACKLIGHT,
  SYSCALL_DISPLAY_SET_ORIENTATION,
  SYSCALL_DISPLAY_GET_ORIENTATION,
  SYSCALL_DISPLAY_GET_FB_INFO,
  SYSCALL_DISPLAY_WAIT_FOR_SYNC,
  SYSCALL_DISPLAY_REFRESH,

  SYSCALL_USB_INIT,
  SYSCALL_USB_DEINIT,
  SYSCALL_USB_START,
  SYSCALL_USB_STOP,
  SYSCALL_USB_CONFIGURED,

  SYSCALL_USB_HID_ADD,
  SYSCALL_USB_HID_CAN_READ,
  SYSCALL_USB_HID_CAN_WRITE,
  SYSCALL_USB_HID_READ,
  SYSCALL_USB_HID_WRITE,
  SYSCALL_USB_HID_READ_SELECT,
  SYSCALL_USB_HID_READ_BLOCKING,
  SYSCALL_USB_HID_WRITE_BLOCKING,
  SYSCALL_USB_VCP_ADD,
  SYSCALL_USB_VCP_CAN_READ,
  SYSCALL_USB_VCP_CAN_WRITE,
  SYSCALL_USB_VCP_READ,
  SYSCALL_USB_VCP_WRITE,
  SYSCALL_USB_VCP_READ_BLOCKING,
  SYSCALL_USB_VCP_WRITE_BLOCKING,
  SYSCALL_USB_WEBUSB_ADD,
  SYSCALL_USB_WEBUSB_CAN_READ,
  SYSCALL_USB_WEBUSB_CAN_WRITE,
  SYSCALL_USB_WEBUSB_READ,
  SYSCALL_USB_WEBUSB_WRITE,
  SYSCALL_USB_WEBUSB_READ_SELECT,
  SYSCALL_USB_WEBUSB_READ_BLOCKING,
  SYSCALL_USB_WEBUSB_WRITE_BLOCKING,

  SYSCALL_SDCARD_POWER_ON,
  SYSCALL_SDCARD_POWER_OFF,
  SYSCALL_SDCARD_IS_PRESENT,
  SYSCALL_SDCARD_GET_CAPACITY,
  SYSCALL_SDCARD_READ_BLOCKS,
  SYSCALL_SDCARD_WRITE_BLOCKS,

  SYSCALL_UNIT_VARIANT_PRESENT,
  SYSCALL_UNIT_VARIANT_GET_COLOR,
  SYSCALL_UNIT_VARIANT_GET_PACKAGING,
  SYSCALL_UNIT_VARIANT_GET_BTCONLY,
  SYSCALL_UNIT_VARIANT_IS_SD_HOTSWAP_ENABLED,

  SYSCALL_SECRET_BOOTLOADER_LOCKED,

  SYSCALL_BUTTON_READ,
  SYSCALL_BUTTON_STATE_LEFT,
  SYSCALL_BUTTON_STATE_RIGHT,

  SYSCALL_TOUCH_GET_EVENT,

  SYSCALL_HAPTIC_SET_ENABLED,
  SYSCALL_HAPTIC_GET_ENABLED,
  SYSCALL_HAPTIC_TEST,
  SYSCALL_HAPTIC_PLAY,
  SYSCALL_HAPTIC_PLAY_CUSTOM,

  SYSCALL_OPTIGA_SIGN,
  SYSCALL_OPTIGA_CERT_SIZE,
  SYSCALL_OPTIGA_READ_CERT,
  SYSCALL_OPTIGA_READ_SEC,
  SYSCALL_OPTIGA_RANDOM_BUFFER,

  SYSCALL_STORAGE_INIT,
  SYSCALL_STORAGE_WIPE,
  SYSCALL_STORAGE_IS_UNLOCKED,
  SYSCALL_STORAGE_LOCK,
  SYSCALL_STORAGE_UNLOCK,
  SYSCALL_STORAGE_HAS_PIN,
  SYSCALL_STORAGE_PIN_FAILS_INCREASE,
  SYSCALL_STORAGE_GET_PIN_REM,
  SYSCALL_STORAGE_CHANGE_PIN,
  SYSCALL_STORAGE_ENSURE_NOT_WIPE_CODE,
  SYSCALL_STORAGE_HAS_WIPE_CODE,
  SYSCALL_STORAGE_CHANGE_WIPE_CODE,
  SYSCALL_STORAGE_HAS,
  SYSCALL_STORAGE_GET,
  SYSCALL_STORAGE_SET,
  SYSCALL_STORAGE_DELETE,
  SYSCALL_STORAGE_SET_COUNTER,
  SYSCALL_STORAGE_NEXT_COUNTER,

  SYSCALL_ENTROPY_GET,

  SYSCALL_TRANSLATIONS_WRITE,
  SYSCALL_TRANSLATIONS_READ,
  SYSCALL_TRANSLATIONS_ERASE,
  SYSCALL_TRANSLATIONS_AREA_BYTESIZE,

  SYSCALL_RNG_GET,

  SYSCALL_FIRMWARE_GET_VENDOR,
  SYSCALL_FIRMWARE_CALC_HASH,

} syscall_number_t;

#endif  // SYSCALL_NUMBERS_H