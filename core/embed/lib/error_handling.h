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

#ifndef LIB_ERROR_HANDLING_H
#define LIB_ERROR_HANDLING_H

// Suppresses the intellisense error in VSCode
#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif

// Build status code from any 16-bit value.
// This macro is used for generic status codes.
#define TS_BUILD(code) ((code) | (~(code) << 16))

// Status codes
//
// Status code is hardened against fault injections
// by storing the negated value in the upper 16 bits.
typedef enum {
  TS_OK = TS_BUILD(0),
  TS_ERROR = TS_BUILD(1),          // Generic error
  TS_ERROR_BUSY = TS_BUILD(2),     // Busy
  TS_ERROR_TIMEOUT = TS_BUILD(3),  // Timeout
  TS_ERROR_NOTINIT = TS_BUILD(4),  // Not initialized
  TS_ERROR_ARG = TS_BUILD(4),      // Invalid argument
  TS_ERROR_IO = TS_BUILD(5),       // I/O error
} ts_t;

// Check status code consistency and returns its value.
// If invalid status code is detected, it will call `__fatal_error()`.
#define _ts_checked(status)                                       \
  ({                                                              \
    ts_t _checked_status = (status);                              \
    if ((_checked_status & 0xFFFF) != (~_checked_status >> 16)) { \
      __fatal_error("ts_check() error", __FILE_NAME__, __LINE__); \
    }                                                             \
    _checked_status;                                              \
  })

// Returns `true` if status code is `TS_OK`
#define ts_ok(status) (_ts_checked(status) == TS_OK)

// Returns `true` if status code is NOT `TS_OK`
#define ts_error(status) (_ts_checked(status) != TS_OK)

// ----------------------------------------------------
// verify_init(), verify_status() and verify_xxx() macros define
// a simple error handling mechanism
//
// Example:
//
// ts_t my_function(int arg) {
//   // initialize verify mechanism
//   verify_init();
//
//   // check arguments
//   verify_arg(arg > 0);
//
//   ts_t status;
//
//   // verify success
//   status = some_function();
//   verify_ok(status);
//
//   // verify condition
//   verify(another_function() != 0, TS_ERROR_IO);
//
//  error:
//
//   // clean up code comes here
//
//   return verify_status();
// }

// Declares a status variable and initializes it to `TS_OK`.
// This variable is used to store the status
#define verify_init() __attribute__((unused)) ts_t __status = TS_OK;

// Returns the current status.
#define verify_status() (__status)

// Jumps to `error` label if status is not `TS_OK`.
#define verify_ok(status)    \
  do {                       \
    ts_t _status = status;   \
    if (ts_error(_status)) { \
      __status = _status;    \
      goto error;            \
    }                        \
  } while (0)

// Jumps to `error` label if the condition is not `true`.
#define verify(cond, status) \
  do {                       \
    if (!(cond)) {           \
      __status = status;     \
      goto error;            \
    }                        \
  } while (0)

// Jumps to `error` label if the condition is not `true`.
// Sets the status to `TS_ERROR_ARG`.
#define verify_arg(cond)       \
  do {                         \
    if (!(cond)) {             \
      __status = TS_ERROR_ARG; \
      goto error;              \
    }                          \
  } while (0)

// Jumps to `error` label if the condition is not `sectrue`.
#define verify_sec(seccond, status) \
  do {                              \
    if ((seccond) != sectrue) {     \
      __status = status;            \
      goto error;                   \
    }                               \
  } while (0)

// Do not use this function directly, use the `ensure()` macro instead.
void __attribute__((noreturn))
__fatal_error(const char *msg, const char *file, int line);

// Ensures that status code is `TS_OK`.
// If not, it shows an error message and shuts down the device.
#define ensure_ok(status, msg)                     \
  do {                                             \
    if (!ts_ok(status)) {                          \
      __fatal_error(msg, __FILE_NAME__, __LINE__); \
    }                                              \
  } while (0)

// Ensures that condition is evaluated as `true`.
// If not, it shows an error message and shuts down the device.
#define ensure_true(cond, msg)                     \
  do {                                             \
    if (!(cond)) {                                 \
      __fatal_error(msg, __FILE_NAME__, __LINE__); \
    }                                              \
  } while (0)

// Ensures that condition is evaluated as `sectrue`.
// If not, it shows an error message and shuts down the device.
#define ensure(seccond, msg)                       \
  do {                                             \
    if ((seccond) != sectrue) {                    \
      __fatal_error(msg, __FILE_NAME__, __LINE__); \
    }                                              \
  } while (0)

// Shows an error message and shuts down the device.
//
// If the title is NULL, it will be set to "INTERNAL ERROR".
// If the message is NULL, it will be ignored.
// If the footer is NULL, it will be set to "PLEASE VISIT TREZOR.IO/RSOD".
void __attribute__((noreturn))
error_shutdown_ex(const char *title, const char *message, const char *footer);

// Shows an error message and shuts down the device.
//
// Same as `error_shutdown_ex()` but with a default header and footer.
void __attribute__((noreturn)) error_shutdown(const char *message);

// Shows WIPE CODE ENTERED screeen and shuts down the device.
void __attribute__((noreturn)) show_wipe_code_screen(void);

// Shows TOO MANY PIN ATTEMPTS screen and shuts down the device.
void __attribute__((noreturn)) show_pin_too_many_screen(void);

// Shows INSTALL RESTRICTED screen and shuts down the device.
void __attribute__((noreturn)) show_install_restricted_screen(void);

#endif  // LIB_ERRORS_H
