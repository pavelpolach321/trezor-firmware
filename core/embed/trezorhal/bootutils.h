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

#ifndef TREZORHAL_BOOTUTILS_H
#define TREZORHAL_BOOTUTILS_H

#include <stddef.h>
#include <stdint.h>

// Immediately resets the device and initiates the normal boot sequence.
void __attribute__((noreturn)) reboot(void);

// Resets the device and enters the bootloader,
// halting there and waiting for further user instructions.
void __attribute__((noreturn)) reboot_to_bootloader(void);

// Resets the device into the bootloader and automatically continues
// with the installation of new firmware (also known as an
// interaction-less upgrade).
//
// If the provided hash is NULL or invalid, the device will stop
// at the bootloader and will require user acknowledgment to proceed
// with the firmware installation.
void __attribute__((noreturn)) reboot_and_upgrade(const uint8_t hash[32]);

// Allows the user to see the displayed error message and then
// safely shuts down the device (clears secrets, memory, etc.).
//
// This function is called when the device eneters an
// unrecoverable error state.
void __attribute__((noreturn)) secure_shutdown(void);

#endif  // TREZORHAL_BOOTUTILS_H
