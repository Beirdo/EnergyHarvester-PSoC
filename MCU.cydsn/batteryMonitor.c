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
#include "ltc2942-1.h"
#include "i2cRegisters.h"
#include "inputs.h"

#include "FreeRTOS.h"
#include "task.h"


volatile uint8 batteryPresent;
volatile uint8 batteryFault;
uint8 batteryStatus;
uint32 batteryCharge;
uint16 batteryVoltage;
int16 batteryTemperature;

void initializeBatteryMonitor(void);

void setupBatteryMonitor(void)
{
    batteryPresent = 0;
    batteryFault = 0;
    initializeBatteryMonitor();
}

void initializeBatteryMonitor(void) {
    uint8 oldBatteryPresent = batteryPresent;

    // Attempt to see if the monitor is on the I2C bus
    if (!i2c_register_test_device(LTC2942_I2C_ADDR)) {
        batteryPresent = 0;
        return;
    }
   
    if (!oldBatteryPresent) {
        batteryPresent = 1;
        
        // Let's setup the batteryMonitor
        // First the control register: automatic readings, prescale at 128, disable alarm pin.
        i2c_register_write(LTC2942_I2C_ADDR, LTC2942_CONTROL, 0xF8);
        
        // Now let's set the thresholds
        // Leave charge thresholds at limits for now
        // Voltage limits: min 3V, max 4.22V
        i2c_register_write(LTC2942_I2C_ADDR, LTC2942_VOLTAGE_THRESH_LO, 0x80);
        i2c_register_write(LTC2942_I2C_ADDR, LTC2942_VOLTAGE_THRESH_LO, 0xB4);

        // Temperature thresholds: min -31.59C, max 59.82C
        i2c_register_write(LTC2942_I2C_ADDR, LTC2942_TEMP_THRESH_LO, 0x67);
        i2c_register_write(LTC2942_I2C_ADDR, LTC2942_TEMP_THRESH_LO, 0x8E);
    }

    // Read (and clear) status
    batteryStatus = i2c_register_read(LTC2942_I2C_ADDR, LTC2942_STATUS);
}

void doTaskBatteryMonitor(void *args){
    (void)args;
    TickType_t xLastWakeTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(1000);
    uint16 raw_voltage;
    uint16 raw_temperature;
    uint16 raw_charge;
    
    xLastWakeTime = xTaskGetTickCount();
    
    while(1)
    {
        // Wake up every 1s (the onboard ADC readings are on a 2s cycle)
        vTaskDelayUntil(&xLastWakeTime, xPeriod);

        // Check that the battery is still there
        initializeBatteryMonitor();
        
        if (!batteryPresent) {
            continue;
        }
        
        // Read the raw values
        raw_charge = i2c_register_read16be(LTC2942_I2C_ADDR, LTC2942_ACCUM_CHARGE);
        raw_voltage = i2c_register_read16be(LTC2942_I2C_ADDR, LTC2942_VOLTAGE);
        raw_temperature = i2c_register_read16be(LTC2942_I2C_ADDR, LTC2942_TEMP);
        
        // Convert to the units we want
        // batteryCharge is in uAH
        // from datasheet:  Q [mAH] = 0.085 * M/128 * raw_charge
        batteryCharge = 85 * raw_charge;  // M = 128
        
        // We want battery voltage in mV
        // from datasheet:  Vsense [V] = 6V * raw_voltage / 0xFFFF
        batteryVoltage = ((uint32)raw_voltage * 6000) / 0xFFFF;
        
        // We want temperature in degC/100
        // from datasheet:  temp [degC] = 600 [K] * raw_temperature / 0xFFFF - 273.15
        batteryTemperature = (((uint32)raw_temperature * 60000) / 0xFFFF) - 27315;
    
        // Mask off the status bits that should not cause shutdown
        batteryFault = batteryStatus & 0x3E;
    }
}

/* [] END OF FILE */
