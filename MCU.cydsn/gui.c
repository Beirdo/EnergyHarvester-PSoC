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
#include "menuScreen.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

uint8 screenOn;

void setupGui(void) 
{
    SSD1306_initialize();
    screenOn = 1;
}

void doTaskGui(void *args) 
{
    (void)args;
    TickType_t xLastWakeTime;
    TickType_t xNextWakeTime;
    TickType_t xLastButtonPush;
    TickType_t xNow;
    const TickType_t xPeriod = pdMS_TO_TICKS(100);
    const TickType_t xBlankPeriod = pdMS_TO_TICKS(15000);
    TickType_t thisPeriod;
    buttonItem_t item;
    
    SSD1306_begin();
    menu_screen_reset();
    
    xLastWakeTime = xTaskGetTickCount();
    xLastButtonPush = xTaskGetTickCount();
    
    while(1)
    {
        xNextWakeTime = xLastWakeTime + xPeriod;
        xNow = xTaskGetTickCount();
        
        thisPeriod = xNextWakeTime - xNow;
        
        if (thisPeriod > 0) {
            if (xQueueReceive(buttonQueue, &item, thisPeriod) == pdPASS) {
                // A button action arrived!
                xLastButtonPush = xTaskGetTickCount();
                
                // Turn the screen on if it's off.
                if (!screenOn) {
                    SSD1306_displayOn();
                    screenOn = 1;
                }
                
                // do something with it!
                menu_screen_action(item.action, item.button);
                continue;
            }
        }

        // If the screen is idle, shut it off.
        xNow = xTaskGetTickCount();
        if (screenOn && xNow - xLastButtonPush >= xBlankPeriod) {
            SSD1306_displayOff();
            screenOn = 0;
        }
        
        // Update the cache for the current screen
        menu_screen_refresh();
        
        // Don't bother updating the screen if it's off.  (Probably need to update the cache, but not screen)
        if (!screenOn) {
            continue;
        }

        // We should get here roughly every 100ms.  Update the screen
        SSD1306_display();            
    }
}

/* [] END OF FILE */
