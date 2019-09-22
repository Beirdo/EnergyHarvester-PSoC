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

#ifndef __buttons_h__
#define __buttons_h__
    
#include "project.h"
#include "FreeRTOS.h"
#include "queue.h"
    
typedef enum {
    BUTTON_UP,
    BUTTON_LEFT,
    BUTTON_ENTER,
    BUTTON_RIGHT,
    BUTTON_DOWN,
    BUTTON_COUNT,
} button_t;

typedef enum {
    ACTION_PRESSED,
    ACTION_RELEASED,
    ACTION_COUNT,
} buttonAction_t;

typedef struct CY_PACKED_ATTR _buttonItem {
    button_t button;
    buttonAction_t action;
} buttonItem_t;

extern QueueHandle_t buttonQueue;
    
#endif // __buttons_h__

/* [] END OF FILE */
