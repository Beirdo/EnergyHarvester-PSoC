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

#ifndef __systemTasks_h__
#define __systemTasks_h__
    
void setupInputScan(void);
void doTaskInputScan(void *args);

void setupButtonScan(void);
void doTaskButtonScan(void *args);

void setupBatteryMonitor(void);
void doTaskBatteryMonitor(void *args);
void batteryEject(void);

void setupGui(void);
void doTaskGui(void *args);

#endif // __systemTasks_h__

/* [] END OF FILE */