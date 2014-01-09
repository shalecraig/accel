#ifndef ACCEL_AVG_TICKER
#define ACCEL_AVG_TICKER

#include "accel.h"

moving_avg_values *allocate_moving_avg(int num_wbuf, int subtotal_sizes);

void reset_moving_avg(moving_avg_values * reset);

bool append_to_moving_avg(moving_avg_values *value, int appended);

int get_latest_frame_moving_avg(moving_avg_values *value);

#endif
