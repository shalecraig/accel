#ifndef MUWAVE_AVG_TICKER
#define MUWAVE_AVG_TICKER

#include "muwave.h"

moving_avg_values *allocate_moving_avg(int num_wbuf, int subtotal_sizes);

void reset_moving_avg(moving_avg_values * reset);

bool append_to_moving_avg(moving_avg_values *value, int appended);

int get_latest_frame_moving_avg(moving_avg_values *value);

#endif
