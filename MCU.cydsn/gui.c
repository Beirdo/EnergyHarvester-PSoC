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
#include "SSD1306.h"
#include "buttons.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

void setupGui(void) 
{
    SSD1306_initialize();
}

void doTaskGui(void *args) 
{
    (void)args;
    TickType_t xLastWakeTime;
    TickType_t xNextWakeTime;
    TickType_t xNow;
    const TickType_t xPeriod = pdMS_TO_TICKS(100);
    TickType_t thisPeriod;
    buttonItem_t item;
    
    SSD1306_begin();
    
    xLastWakeTime = xTaskGetTickCount();
    
    while(1)
    {
        xNextWakeTime = xLastWakeTime + xPeriod;
        xNow = xTaskGetTickCount();
        
        thisPeriod = xNextWakeTime - xNow;
        
        if (thisPeriod <= 0) {
            if (xQueueReceive(buttonQueue, &item, thisPeriod) == pdPASS) {
                // A button action arrived!
                // do something with it!
                continue;
            }
        }

        // We should get here roughly every 100ms.  Update the screen
    
    }
}

/* [] END OF FILE */
