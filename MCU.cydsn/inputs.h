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
    
typedef enum {
    ADC_VIN,
    ADC_VSTORE,
    ADC_VOUT,
    ADC_VHARVEST,
    ADC_VCAP,
    ADC_PACKP,
    ADC_5V,
    ADC_VPROG,
    ADC_CHANNELS
} analogInputs_t;
    
typedef enum {
    INPUT_nPGD0,
    INPUT_nPGD1,
    INPUT_COUNT,
} digitalInputs_t;

// in inputScan.c
extern uint16 adcReadings[ADC_CHANNELS];    // scaled to original voltages, in mV
extern uint8 digitalInputs[INPUT_COUNT];
extern uint16 chargeCurrent;
extern volatile uint8 chargeEnable;         // this is an output, the pin is active high
extern volatile uint8 selectValue;          // this is an output, and drives 2 pins to select which transformer to use

// in menuScreen.c
extern volatile uint8 connectBattery;       // input from the menu, defaults to 0 (no battery), and can be set to 1 when connected
extern volatile uint8 overrideDisabled;     // input from the menu, let the user manually override the enable (set 1 to disable)

// in batteryMonitor.c
extern volatile uint8 batteryPresent;
extern volatile uint8 batteryFault;
extern uint8 batteryStatus;
extern uint32 batteryCharge;        // in uAH
extern uint16 batteryVoltage;       // in mV
extern int16 batteryTemperature;    // in degC/100

#endif // __inputs_h__

/* [] END OF FILE */
