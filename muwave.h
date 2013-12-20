#ifndef MUWAVE_HEADER
#define MUWAVE_HEADER

#include <stdbool.h>

#include "muwave_consts.c"

/* QUAN_WIN_STEP QUAN_MOV_STEP, but not QUAN_WIN_STEP */
#ifdef QUAN_WIN_STEP
#ifndef QUAN_MOV_STEP
#error should_define_both_QUAN_MOV_STEP_and_QUAN_WIN_STEP
#endif
#endif

void complete_recording(void);
void begin_recording(void);
void handle_accel_tick(void);
void cleanup(void);

#endif
