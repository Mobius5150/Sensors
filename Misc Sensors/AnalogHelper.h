/* 
 * File:   AnalogHelper.h
 * Author: Mike Blouin
 *
 * Holds a helper function for reading analog data.
 */

#ifndef ANALOGHELPER_H
#define	ANALOGHELPER_H

#ifdef	__cplusplus
extern "C" {
#endif

/**
 * Reads the analog pin. For pin AN1 analogInput should be 1 and for AN8 analogInput
 * should be 8.
 *
 * @param analogInput The analog input pin to read from
 * @return Returns 1000 times the voltage read
 */
int ReadAnalog(int analogInput);


#ifdef	__cplusplus
}
#endif

#endif	/* ANALOGHELPER_H */

