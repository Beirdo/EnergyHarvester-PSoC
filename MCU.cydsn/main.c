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
#include "FreeRTOS.h"
#include "task.h"

void prvSetupHardware(void) {
    setupInputScan();
    setupButtonScan();
    setupCoulombMeter();
    CyGlobalIntEnable; /* Enable global interrupts. */
}

int main(void) {
    /* Perform any hardware setup necessary */
    prvSetupHardware();
    
    xTaskCreate(doTaskInputScan, "input-scan", 100, NULL, 8, NULL);
    xTaskCreate(doTaskButtonScan, "button-scan", 100, NULL, 8, NULL);
    xTaskCreate(doTaskCoulombMeter, "coulomb-meter", 100, NULL, 8, NULL);
    
    vTaskStartScheduler();
    
    while(1);
    
    return 0;
}

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
    (void)pxTask;
    (void)pcTaskName;
    
	/* The stack space has been execeeded for a task, considering allocating more. */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}

void vApplicationMallocFailedHook( void )
{
	/* The heap space has been execeeded. */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}



/* [] END OF FILE */