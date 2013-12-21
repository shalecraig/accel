#ifndef MUWAVE_HEADER
#define MUWAVE_HEADER

#include <stdbool.h>

#include "muwave_consts.c"

#ifndef MUWAVE_QUAN_POINT_SIZE
#define MUWAVE_QUAN_POINT_SIZE ((uint32_t)MUWAVE_FREQUENCY_HZ/25)
#endif

/* TODO: where am I doing this wrong? I should theoretically be using this... */
#ifndef MUWAVE_QUAN_MOVING_AVG_SIZE
#define MUWAVE_QUAN_MOVING_AVG_SIZE 2
#endif

/*
 * At 100Hz, this should be 20s
 */
/* TODO: move this to the top of the file. */
#ifndef MUWAVE_RECORD_MAX_LENGTH
#define MUWAVE_RECORD_MAX_LENGTH MUWAVE_FREQUENCY_HZ*MUWAVE_RECORDING_S
#endif

void complete_recording(void);
void begin_recording(void);
void handle_accel_tick(void);
void cleanup(void);

#endif
