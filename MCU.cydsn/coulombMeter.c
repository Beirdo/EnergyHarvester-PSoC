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
#include "coulombMeter.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// coulombs per count = 1 / (RSense * Gvh) = 1 / (83.5[mOhm] * 33.5[Hz/C])
//                    = 361827 [uC]
#define UCOULOMBS_PER_COUNT 361827

// This has a range of up to 3297mAH before it overflows
volatile uint32 pulseCount;
volatile uint32 uCoulombs;
volatile uint32 uAmpHours;
volatile int32 avgCurrent;  // Average current over last 100ms (in uA)

void setupCoulombMeter(void) {
    pulseCount = 0;
    uCoulombs = 0;
    uAmpHours = 0;
}

void COULOMB_IRQ_Interrupt_InterruptCallback(void) {
    if (COULOMB_POL_Read()) {
        // positive polarity, increment counter;
        pulseCount += 1;
    } else {
        // negative polarity, decrement counter but clamp at 0;
        if (pulseCount) {
            pulseCount -= 1;
        }
    }
}

void doTaskCoulombMeter(void *args)
{
    (void)args;
    TickType_t xLastWakeTime;
    TickType_t xPrevSleepTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(100);
    int32 lastCounter = 0;
    int32 thisCounter;
    int32 delta;
    uint32 deltaTime;
    
    xLastWakeTime = xTaskGetTickCount();
    
    while(1)
    {
        xPrevSleepTime = xTaskGetTickCount();
        
        // Wake up every 100ms
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
        
        thisCounter = pulseCount;
        delta = thisCounter - lastCounter;
        lastCounter = thisCounter;
        
        if (thisCounter <= 0) {
            uCoulombs = 0;
            uAmpHours = 0;
        } else {
            uCoulombs = thisCounter * UCOULOMBS_PER_COUNT;
            uAmpHours = uCoulombs / 3600;
        }
        
        deltaTime = (xLastWakeTime - xPrevSleepTime) / portTICK_PERIOD_MS;
        // 1 amp = 1 coulomb / 1 sec.
        // avgCurrent [uA] = deltaCount [count] * K [uCoulomb/count] * (1 /deltaTime) [1/ms] * 1000 [ms/s]
        avgCurrent = (int32)(((int64)delta * UCOULOMBS_PER_COUNT * 1000) / deltaTime);
    }
}

/* [] END OF FILE */
