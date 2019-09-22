/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#ifndef __inputs_h__
#define __inputs_h__
    
#include "project.h"
    
#define ADC_CHANNELS 7
    
typedef enum {
    INPUT_nPGD0,
    INPUT_nPGD1,
    INPUT_nPGD2,
    INPUT_CHRG,
    INPUT_FAULT,
    INPUT_COUNT,
} digitalInputs_t;
    
extern uint16 adcReadings[ADC_CHANNELS];    // scaled to original voltages, in mV
extern uint8 digitalInputs[INPUT_COUNT];
extern volatile uint8 shutdown;             // this is an output, the pin is active low, this is active high
extern volatile uint8 selectValue;          // this is an output, and drives 2 pins to select which transformer to use

#endif // __inputs_h__

/* [] END OF FILE */
