/* 
 * File:   speed_detector.h
 * Author: Mike
 *
 * Created on April 26, 2014, 9:34 PM
 */

#ifndef SPEED_DETECTOR_H
#define	SPEED_DETECTOR_H

#ifdef	__cplusplus
extern "C" {
#endif

#ifndef SPEED_DATA_TYPE
#define SPEED_DATA_TYPE int
#endif

#define SPEED_LEN 5

void log_speed(SPEED_DATA_TYPE);
SPEED_DATA_TYPE get_average_speed();

#ifdef	__cplusplus
}
#endif

#endif	/* SPEED_DETECTOR_H */

