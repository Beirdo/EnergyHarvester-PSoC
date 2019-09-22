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
volatile uint8 shutdown;            // This is an output, the pin is active low, this is active high
volatile uint8 selectValue;         // this is an output, and drives 2 pins to select which transformer to use

static uint8 nPGD0_Read(void);
static uint8 nPGD1_Read(void);
static uint8 nPGD2_Read(void);

typedef uint8 (*digitalReadFunc_t)(void);

digitalReadFunc_t digitalReadFunc[INPUT_COUNT] = {
    nPGD0_Read, nPGD1_Read, nPGD2_Read, CHRG_Read, FAULT_Read
};

void setupInputScan(void) {
    memset(adcReadings, 0, sizeof(adcReadings));
    memset(digitalInputs, 0, sizeof(digitalInputs));
    shutdown = 1;
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
    shutdown = 0;
    
    while(1)
    {
        COULOMB_nSHDN_Write(!shutdown);
        SEL_Write(selectValue);
        
        // Wake up every 100ms
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
        
        ADC_Wakeup();
        for (i = 0; i < ADC_CHANNELS; i++) {
            // All channels were pre-divided by 2 to make it so we could read up to 6.6V
            adcReadings[i] = ADC_ReadResult_mVolts(i) << 1;
        }
        ADC_Sleep();
        
        for (i = 0; i < INPUT_COUNT; i++) {
            digitalInputs[i] = digitalReadFunc[i]();
        }
    }
}

static uint8 nPGD0_Read(void) {
    return !(!(nPGD_Read() & 0x01));
}

static uint8 nPGD1_Read(void) {
    return !(!(nPGD_Read() & 0x02));
}

static uint8 nPGD2_Read(void) {
    return !(!(nPGD_Read() & 0x04));
}

/* [] END OF FILE */
