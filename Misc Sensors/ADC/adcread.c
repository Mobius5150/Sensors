#include <p18cxxx.h>
#include <adc.h>

#if defined (ADC_V1) || defined (ADC_V2) || defined (ADC_V3) ||\
    defined (ADC_V4) || defined (ADC_V5) || defined (ADC_V6) ||\
    defined (ADC_V7) || defined (ADC_V7_1) || defined (ADC_V8) ||\
    defined (ADC_V9) || defined (ADC_V10) || defined (ADC_V11) ||\
	defined (ADC_V12) || defined (ADC_V13) || defined (ADC_V13_1)\
	|| defined (ADC_V11_1) || defined (ADC_V13_2) || defined (ADC_V13_3)\
	|| defined (ADC_V14) || defined (ADC_V14_1)
/*************************************************************************************
Function:       int ReadADC(void)

Overview:    This function reads the ADC Buffer register which contains
                      the conversion value.

Parameters:  None

Returns:        Returns the ADC Buffer value

Remarks:     None
            
***************************************************************************************/	
int ReadADC(void)
{
  return (((unsigned int)ADRESH)<<8)|(ADRESL);
}

#endif
