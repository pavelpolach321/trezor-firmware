  .syntax unified

  .text

  .global reset_handler
  .type reset_handler, STT_FUNC
reset_handler:
  // set the stack protection
  ldr r0, =_sstack
  add r0, r0, #128       // safety margin for the exception frame
  msr MSPLIM, r0

  // setup environment for subsequent stage of code
  ldr r2, =0             // r2 - the word-sized value to be written
  ldr r0, =_startup_clean_ram_0_start
  ldr r1, =_startup_clean_ram_0_end
  bl memset_reg
  ldr r0, =_startup_clean_ram_1_start
  ldr r1, =_startup_clean_ram_1_end
  bl memset_reg
  ldr r0, =_startup_clean_ram_2_start
  ldr r1, =_startup_clean_ram_2_end
  bl memset_reg

  // copy data in from flash
  ldr r0, =data_vma     // dst addr
  ldr r1, =data_lma     // src addr
  ldr r2, =data_size    // size in bytes
  bl memcpy

  // copy confidential data in from flash
  ldr r0, =confidential_vma     // dst addr
  ldr r1, =confidential_lma     // src addr
  ldr r2, =confidential_size    // size in bytes
  bl memcpy

  // setup the stack protector (see build script "-fstack-protector-all") with an unpredictable value
  bl rng_get
  ldr r1, = __stack_chk_guard
  str r0, [r1]

  // re-enable exceptions
  // according to "ARM Cortex-M Programming Guide to Memory Barrier Instructions" Application Note 321, section 4.7:
  // "If it is not necessary to ensure that a pended interrupt is recognized immediately before
  // subsequent operations, it is not necessary to insert a memory barrier instruction."
  cpsie f

  // enter the application code
  bl main

  b shutdown_privileged

  .end
