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

#include <string.h>

#include "applet.h"
#include "mpu.h"
#include "rng.h"
#include "systask.h"

#ifdef SYSCALL_DISPATCH

void applet_init(applet_t* applet, applet_header_t* header,
                 applet_layout_t* layout) {
  memset(applet, 0, sizeof(applet_t));

  applet->header = header;
  applet->layout = *layout;

  // !@# check if header refs are valid and inside the layout memory
}

static void applet_clear_memory(applet_t* applet) {
  if (applet->layout.data1_size > 0) {
    memset((void*)applet->layout.data1_start, 0, applet->layout.data1_size);
  }
  if (applet->layout.data2_size > 0) {
    memset((void*)applet->layout.data2_start, 0, applet->layout.data2_size);
  }
}

void applet_reset(applet_t* applet, uint32_t cmd, const void* arg,
                  size_t arg_size) {
  // Clear all memory the applet is allowed to use
  applet_clear_memory(applet);

  // Reset the applet task (stack pointer, etc.)
  systask_init(&applet->task, applet->header->stack_start,
               applet->header->stack_size);

  // Copy the arguments onto the applet stack
  void* arg_copy = NULL;
  if (arg != NULL && arg_size > 0) {
    arg_copy = systask_push_data(&applet->task, arg, arg_size);
  }

  // Schedule the applet task run
  uint32_t arg1 = cmd;
  uint32_t arg2 = (uint32_t)arg_copy;
  uint32_t arg3 = rng_get();

  systask_push_call(&applet->task, applet->header->startup, arg1, arg2, arg3);
}

#endif  // SYSCALL_DISPATCH
