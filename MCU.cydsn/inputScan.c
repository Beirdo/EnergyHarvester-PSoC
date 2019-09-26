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

#include "project.h"
#include "systemTasks.h"
#include "inputs.h"
#include "FreeRTOS.h"
#include "task.h"

uint16 adcReadings[ADC_CHANNELS];   // scaled to original voltages, in mV
uint8 digitalInputs[INPUT_COUNT];
uint16 chargeCurrent;               // in mA
volatile uint8 chargeEnable;        // This is an output, the pin is active high
volatile uint8 selectValue;         // this is an output, and drives 2 pins to select which transformer to use

static uint8 nPGD0_Read(void);
static uint8 nPGD1_Read(void);
\
typedef uint8 (*digitalReadFunc_t)(void);

const digitalReadFunc_t digitalReadFunc[INPUT_COUNT] = {
    nPGD0_Read, nPGD1_Read
};

void setupInputScan(void) {
    memset(adcReadings, 0, sizeof(adcReadings));
    memset(digitalInputs, 0, sizeof(digitalInputs));
    chargeEnable = 0;
    selectValue = 0;    // All off.
    
    ADC_Start();
    ADC_Sleep();
}

void doTaskInputScan(void *args)
{
    (void)args;
    TickType_t xLastWakeTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(100);
    int i;
    
    xLastWakeTime = xTaskGetTickCount();
    
    while(1)
    {
        CHARGE_ENABLE_Write(chargeEnable);
        SEL_Write(selectValue);
        
        // Wake up every 100ms
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
        
        ADC_Wakeup();
        for (i = 0; i < ADC_CHANNELS; i++) {
            // All channels (except VProg) were pre-divided by 2 to make it so we could read up to 6.6V
            adcReadings[i] = ADC_ReadResult_mVolts(i) << (i == ADC_VPROG ? 0 : 1);
        }
        ADC_Sleep();
        
        // i_out = 1000 * iprog  (from LTC1734 datasheet)
        //       = 1000 * ((vprog/1000) [V] / 3k [ohm]) * 1000 [mA/A]
        //       = vprog / 3 [mA]
        chargeCurrent = adcReadings[ADC_VPROG] / 3;   
        
        for (i = 0; i < INPUT_COUNT; i++) {
            digitalInputs[i] = digitalReadFunc[i]();
        }
        
        if (digitalInputs[INPUT_nPGD0] || digitalInputs[INPUT_nPGD1]) {
            chargeEnable = 0;
        } else if (chargeEnable) {
            if (adcReadings[ADC_PACKP] < 500) {
                // No battery connected
                chargeEnable = 0;
            } else if (chargeCurrent < 10) {
                // Charging is complete when we hit 10mA charge current
                chargeEnable = 0;
            } else if (batteryPresent && batteryFault) {
                chargeEnable = 0;
            }
        } else if (adcReadings[ADC_PACKP] > 500) {
            // A battery is connected, enable charging
            chargeEnable = 1;
        }
    }
}

static uint8 nPGD0_Read(void) {
    return !(!(nPGD_Read() & 0x01));
}

static uint8 nPGD1_Read(void) {
    return !(!(nPGD_Read() & 0x02));
}

/* [] END OF FILE */
