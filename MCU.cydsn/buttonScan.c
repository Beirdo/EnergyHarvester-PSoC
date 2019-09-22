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
#include "buttons.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

QueueHandle_t buttonQueue;

uint8 shiftRegister[BUTTON_COUNT];
uint8 debounced[BUTTON_COUNT];

void setupButtonScan(void)
{
    int i;
    
    for (i = 0; i < BUTTON_COUNT; i++) {
        // Default to all buttons off.
        shiftRegister[i] = 0xFF;
        debounced[i] = 1;
    }
    
    buttonQueue = xQueueCreate(16, sizeof(buttonItem_t));
}

void doTaskButtonScan(void *args)
{
    (void)args;
    TickType_t xLastWakeTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(10);
    int i;
    uint8 buttons;
    uint8 bit;
    uint8 oldValue;
    buttonItem_t item;
    
    xLastWakeTime = xTaskGetTickCount();
    
    while(1)
    {
        // Wake up every 100ms
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
        
        buttons = BUTTON_Read();
        
        for (i = 0; i < BUTTON_COUNT; i++) {
            bit = !(!(buttons & (1 << i)));
            shiftRegister[i] <<= 1;
            shiftRegister[i] |= bit;
            oldValue = debounced[i];
            debounced[i] = !(!shiftRegister[i]);
            if (oldValue != debounced[i]) {
                item.button = i;
                item.action = (debounced[i] ? ACTION_RELEASED : ACTION_PRESSED);
                xQueueSend(buttonQueue, &item, 0);
            }
        }
    }   
}


/* [] END OF FILE */
