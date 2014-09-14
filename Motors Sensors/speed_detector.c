#include "speed_detector.h"

SPEED_DATA_TYPE speed_moving_averages[SPEED_LEN] = { 0, 0, 0, 0, 0 };
int speed_index = 0;

void log_speed(SPEED_DATA_TYPE speed) {
    speed_index = ++speed_index % SPEED_LEN;
    speed_moving_averages[speed_index] = speed;
}

SPEED_DATA_TYPE get_average_speed() {
    SPEED_DATA_TYPE speed = 0;
    int i = 0;

    for (i = 0; i < SPEED_LEN; ++i) {
        speed += speed_moving_averages[(speed_index - i) % SPEED_LEN];
    }

    return speed/SPEED_LEN;
}