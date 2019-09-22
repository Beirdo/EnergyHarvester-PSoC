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

#ifndef __coulombMeter_h__
#define __coulombMeter_h__
    
#include "project.h"
    
extern volatile uint32 pulseCount;
extern volatile uint32 uCoulombs;
extern volatile uint32 uAmpHours;
extern volatile int32 avgCurrent;  // Average current over last 100ms (in uA)
    
    
#endif // __coulombMerter_h__

/* [] END OF FILE */
