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

#ifndef __ltc2942_1_h__
#define __ltc2942_1_h__
    
#include "project.h"
    
#define LTC2942_I2C_ADDR 0x69
    
typedef enum {
    LTC2942_STATUS,
    LTC2942_CONTROL,
    LTC2942_ACCUM_CHARGE,
    LTC2942_CHARGE_THRESH_HI = 4,
    LTC2942_CHARGE_THRESH_LO = 6,
    LTC2942_VOLTAGE = 8,
    LTC2942_VOLTAGE_THRESH_HI = 10,
    LTC2942_VOLTAGE_THRESH_LO,
    LTC2942_TEMP,
    LTC2942_TEMP_THRESH_HI = 14,
    LTC2942_TEMP_THRESH_LO,
} ltc2942Registers_t;

#endif // __ltc2942_1_h__

/* [] END OF FILE */
