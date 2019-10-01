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
#include "menuScreen.h"
#include "SSD1306.h"
#include "inputs.h"
#include "buttons.h"
#include "utils.h"
#include <stdio.h>

#include "FreeRTOS.h"
#include "queue.h"

extern const screen_t mainMenu;
extern const menu_entry_t mainMenuEntries[];
extern const screen_t slidesReadings;
extern const screen_t ejectBattery;
extern const screen_t chooseInput;
extern const screen_t overrideToggle;

QueueHandle_t screenStack;
screen_t *currentScreen;
uint8 currentChoice;
uint8 topChoice;

static uint8 line_buffer[32];
volatile uint8 connectBattery;       // input from the menu, defaults to 0 (no battery), and can be set to 1 when connected
volatile uint8 overrideDisabled;     // input from the menu, let the user manually override the enable (set 1 to disable)
static const uint8 *selector[] = { 
    (const uint8 *)"Disabled", 
    (const uint8 *)"1:20", 
    (const uint8 *)"1:50", 
    (const uint8 *)"1:100" 
};


void menu_push(const screen_t *screen);
void menu_pop(void);

void menu_screen_menu_action(uint8 action, uint8 button);
void menu_screen_slides_action(uint8 action, uint8 button);
void menu_screen_draw_menu(void);
void menu_screen_draw_slide(void);

void displayVolts(void *data, uint8 *buffer, uint8 width);
void displayAmps(void *data, uint8 *buffer, uint8 width);
void displayActiveLow(void *data, uint8 *buffer, uint8 width);
void displayActiveHigh(void *data, uint8 *buffer, uint8 width);
void displaySelected(void *data, uint8 *buffer, uint8 width);

void displayBattVolts(void *data, uint8 *buffer, uint8 width);
void displayBattAmpHours(void *data, uint8 *buffer, uint8 width);
void displayBattActiveHigh(void *data, uint8 *buffer, uint8 width);
void displayBattTemperature(void *data, uint8 *buffer, uint8 width);

void _displayFromMillis(uint16 value, uint8 *buffer, uint8 width, uint8 *unit);
void _displayFromMicros(uint32 value, uint8 *buffer, uint8 width, uint8 *unit);
void _displaySignal(uint8 active, uint8 *buffer, uint8 width);

void formatEjectConnect(uint8 *buffer, uint8 width, uint8 *format);
void formatInputSelector(uint8 *buffer, uint8 width, uint8 *format);
void formatOverride(uint8 *buffer, uint8 width, uint8 *format);

const menu_entry_t mainMenuEntries[] = {
    { .string = (const uint8 *)"View Readings", .screen = &slidesReadings },
    { .string = (const uint8 *)"%s Battery", .func = formatEjectConnect, .screen = &ejectBattery },
    { .string = (const uint8 *)"Override: %s", .func = formatOverride, .screen = &overrideToggle },
    { .string = (const uint8 *)"Input: %s", .func = formatInputSelector, .screen = &chooseInput },
};

const screen_t mainMenu = {
    .type = SCREEN_MENU,
    .item.menu = {
        .entries = mainMenuEntries,
        .count = NELEMS(mainMenuEntries)
    }
};

const slide_entry_t slidesReadingsEntries[] = {
    { .label = (const uint8 *)"Vin:", .slideNum = 0, .xLabel = 0, .yLabel = 0, .x = 10, .y = 0, .width = 10, .func = displayVolts, .data = &adcReadings[ADC_VIN] },
    { .label = (const uint8 *)"Vstore:", .slideNum = 0, .xLabel = 0, .yLabel = 1, .x = 10, .y = 1, .width = 10, .func = displayVolts, .data = &adcReadings[ADC_VSTORE] },
    { .label = (const uint8 *)"Vout:", .slideNum = 0, .xLabel = 0, .yLabel = 2, .x = 10, .y = 2, .width = 10, .func = displayVolts, .data = &adcReadings[ADC_VOUT] },
    { .label = (const uint8 *)"Vharvest:", .slideNum = 0, .xLabel = 0, .yLabel = 3, .x = 10, .y = 3, .width = 10, .func = displayVolts, .data = &adcReadings[ADC_VHARVEST] },
    { .label = (const uint8 *)"Vcap:", .slideNum = 0, .xLabel = 0, .yLabel = 4, .x = 10, .y = 4, .width = 10, .func = displayVolts, .data = &adcReadings[ADC_VCAP] },
    { .label = (const uint8 *)"Vbatt:", .slideNum = 0, .xLabel = 0, .yLabel = 5, .x = 10, .y = 5, .width = 10, .func = displayVolts, .data = &adcReadings[ADC_PACKP] },
    { .label = (const uint8 *)"+5V:", .slideNum = 0, .xLabel = 0, .yLabel = 6, .x = 10, .y = 6, .width = 10, .func = displayVolts, .data = &adcReadings[ADC_5V] },
    { .label = (const uint8 *)"Icharge:", .slideNum = 0, .xLabel = 0, .yLabel = 7, .x = 10, .y = 7, .width = 10, .func = displayAmps, .data = &chargeCurrent },
    { .label = (const uint8 *)"Selected:", .slideNum = 1, .xLabel = 0, .yLabel = 0, .x = 10, .y = 0, .width = 10, .func = displaySelected, .data = (void *)&selectValue },
    { .label = (const uint8 *)"PGD0:", .slideNum = 1, .xLabel = 0, .yLabel = 1, .x = 10, .y = 1, .width = 10, .func = displayActiveLow, .data = &digitalInputs[INPUT_nPGD0] },
    { .label = (const uint8 *)"PGD1:", .slideNum = 1, .xLabel = 0, .yLabel = 2, .x = 10, .y = 2, .width = 10, .func = displayActiveLow, .data = &digitalInputs[INPUT_nPGD1] },
    { .label = (const uint8 *)"Enabled:", .slideNum = 1, .xLabel = 0, .yLabel = 3, .x = 10, .y = 3, .width = 10, .func = displayActiveHigh, .data = (void *)&chargeEnable },
    { .label = (const uint8 *)"BattPresent:", .slideNum = 2, .xLabel = 0, .yLabel = 0, .x = 13, .y = 0, .width = 10, .func = displayActiveHigh, .data = (void *)&batteryPresent },
    { .label = (const uint8 *)"BattFault:", .slideNum = 2, .xLabel = 0, .yLabel = 1, .x = 13, .y = 1, .width = 10, .func = displayBattActiveHigh, .data = (void *)&batteryFault },
    { .label = (const uint8 *)"BattCharge:", .slideNum = 2, .xLabel = 0, .yLabel = 2, .x = 13, .y = 2, .width = 10, .func = displayBattAmpHours, .data = (void *)&batteryCharge },
    { .label = (const uint8 *)"BattVoltage:", .slideNum = 2, .xLabel = 0, .yLabel = 3, .x = 13, .y = 3, .width = 10, .func = displayBattVolts, .data = (void *)&batteryVoltage },
    { .label = (const uint8 *)"BattTemp:", .slideNum = 2, .xLabel = 0, .yLabel = 4, .x = 13, .y = 4, .width = 10, .func = displayBattTemperature, .data = (void *)&batteryTemperature },
};

    
const screen_t slidesReadings = {
    .type = SCREEN_SLIDES,
    .item.slides = {
        .entries = slidesReadingsEntries,
        .count = NELEMS(slidesReadingsEntries),
        .slideCount = 3,
    }
};

const screen_t ejectBattery = {
    .type = SCREEN_TOGGLE,
    .item.toggle = {
        .data = (void *)&connectBattery
    }
};

const screen_t chooseInput = {
    .type = SCREEN_CHOICES,
    .item.choices = {
        .data = (void *)&selectValue,
        .choices = selector,
        .count = NELEMS(selector),
    }
};

const screen_t overrideToggle = {
    .type = SCREEN_TOGGLE,
    .item.toggle = {
        .data = (void *)&overrideDisabled
    }
};

// ==============================================================

void menu_screen_reset(void)
{
    screenStack = xQueueCreate(16, sizeof(screen_queue_item_t));
    currentScreen = NULL;
    connectBattery = 0;
    overrideDisabled = 0;
    menu_push(&mainMenu);
}

void menu_screen_action(uint8 action, uint8 button)
{
    if (!currentScreen) {
        return;
    }
    
    switch (currentScreen->type) {
        case SCREEN_MENU:
            menu_screen_menu_action(action, button);
            break;
        case SCREEN_SLIDES:
            menu_screen_slides_action(action, button);
            break;
        case SCREEN_TOGGLE:
        case SCREEN_CHOICES:
            // should never happen.  if it does, get back to the menu
            menu_pop();
            break;
        default:
            break;
    }
}

void menu_screen_menu_action(uint8 action, uint8 button)
{
    if (action != ACTION_PRESSED) {
        return;
    }
    
    const menu_entry_t *menuItems = currentScreen->item.menu.entries;
    int line = currentChoice - topChoice;
    int bottomChoice = topChoice + 7;
    
    switch(button) {
        case BUTTON_UP:
            if (line == 0) {
                if (topChoice != 0) {
                    topChoice--;
                    currentChoice--;
                }
            } else {
                currentChoice--;
            }
            break;
        case BUTTON_DOWN:
            if (line == 7) {
                if (bottomChoice != currentChoice) {
                    topChoice++;
                    currentChoice++;
                }
            } else {
                currentChoice++;
            }
            break;
        case BUTTON_LEFT:
            menu_pop();
            break;
        case BUTTON_ENTER:
        case BUTTON_RIGHT:
            {
                const screen_t *screen = menuItems[currentChoice].screen;
                switch(screen->type) {
                    case SCREEN_TOGGLE: 
                        {
                            volatile uint8 *data = (volatile uint8 *)screen->item.toggle.data;
                            *data = !(*data);
                        }
                        break;
                    case SCREEN_CHOICES:
                        {
                            volatile uint8 *data = (volatile uint8 *)screen->item.choices.data;
                            *data = (*data + 1) % screen->item.choices.count;
                        }
                        break;
                    default:
                        menu_push(menuItems[currentChoice].screen);
                        break;
                }
            }
            break;
        default:
            break;
    }
}

void menu_screen_slides_action(uint8 action, uint8 button)
{
    if (action != ACTION_PRESSED) {
        return;
    }
    
    const slides_t *slides = &currentScreen->item.slides;
    
    switch(button) {
        case BUTTON_UP:
            if (currentChoice == 0) {
                currentChoice = slides->slideCount;
            }
            currentChoice--;
            break;
        case BUTTON_ENTER:
        case BUTTON_DOWN:
            currentChoice++;
            if (currentChoice == slides->slideCount) {
                currentChoice = 0;
            }
            break;
        case BUTTON_LEFT:
            menu_pop();
            break;
        case BUTTON_RIGHT:
        default:
            break;
    }
}


void menu_screen_refresh(void)
{
    SSD1306_clearDisplay();

    if (!currentScreen) {
        return;
    }
    
    switch (currentScreen->type) {
        case SCREEN_MENU:
            menu_screen_draw_menu();
            break;
        case SCREEN_SLIDES:
            menu_screen_draw_slide();
            break;
        default:
            break;
    }
}

void menu_screen_draw_menu(void) {
    const menu_entry_t *menuItems = currentScreen->item.menu.entries;
    int count = currentScreen->item.menu.count;
    int i;
    int j;
    int k;
    int col;
    int line = currentChoice - topChoice;
    int color;
    int max_col = SSD1306_width() / 6;
    uint8 *buf;
    
    for (i = 0, j = topChoice; i < 8 && j < count; i++, j++) {
        const uint8 *string = menuItems[j].string;
        menu_format_func_t func = menuItems[j].func;
        if (func) {
            buf = line_buffer;
            func(buf, max_col, (uint8 *)string);
            buf[max_col] = 0;
        } else {
            buf = (uint8 *)string;
        }
        int len = ustrlen(buf);
        
        color = (i == line) ? INVERSE : WHITE;
        SSD1306_setCursor(0, i);
        SSD1306_setTextColor(color, color);
        for (col = 0, k = -2; col < max_col; k++, col++) {
            if (k < 0 || k >= len) {
                if (color == INVERSE) {
                    SSD1306_write(' ');
                } else if (col < max_col - 1) {
                    SSD1306_setCursor(col + 1, i);
                }
            } else {
                SSD1306_write(*(buf++));
            }
        }
    }
}

void menu_screen_draw_slide(void) {
    int count = currentScreen->item.slides.count;
    const slide_entry_t *entries = currentScreen->item.slides.entries;
    int i;
    int j;
    int col;
    int max_col = SSD1306_width() / 6;
    int len;
    const uint8 *buf;
    
    SSD1306_setTextColor(WHITE, WHITE);
    for (i = 0; i < count; i++) {
        const slide_entry_t *entry = &entries[i];
        
        if (entry->slideNum > currentChoice) {
            break;
        }
        
        if (entry->slideNum != currentChoice) {
            continue;
        }
        
        SSD1306_setCursor(entry->xLabel, entry->yLabel);
        buf = entry->label;
        len = ustrlen(buf);
        for (col = entry->xLabel, j = 0; j < len && col < max_col; j++, col++) {
            SSD1306_write(*(buf++));
        }
        
        if (!entry->func) {
            continue;
        }
        
        SSD1306_setCursor(entry->x, entry->y);
        entry->func(entry->data, line_buffer, min(entry->width, max_col));
        line_buffer[max_col] = 0;
        buf = line_buffer;
        len = ustrlen(buf);
        for (col = entry->x, j = 0; j < len && col < max_col; j++, col++) {
            SSD1306_write(*(buf++));
        }
    }
}

void menu_push(const screen_t *screen)
{
    if (currentScreen) {
        screen_queue_item_t item = { .top = topChoice, .current = currentChoice, .screen = currentScreen };
        xQueueSendToFront(screenStack, &item, portMAX_DELAY);
    }
    topChoice = 0;
    currentChoice = 0;
    currentScreen = (screen_t *)screen;
}
        
void menu_pop(void)
{
    if (currentScreen == (screen_t *)&mainMenu) {
        return;
    }
    
    screen_queue_item_t item;
    xQueueReceive(screenStack, &item, portMAX_DELAY);
    currentScreen = item.screen;
    topChoice = item.top;
    currentChoice = item.current;
}

// ====================================================================

void displayVolts(void *data, uint8 *buffer, uint8 width)
{
    uint16 value = *(uint16 *)data;
    
    if(!buffer) {
        return;
    }
    _displayFromMillis(value, buffer, width, (uint8 *)"V");
}

void displayBattVolts(void *data, uint8 *buffer, uint8 width)
{
    if(!buffer) {
        return;
    }

    if (!batteryPresent) {
        snprintf((char *)buffer, width, "----");
    } else {
        displayVolts(data, buffer, width);
    }
}

void _displayFromMillis(uint16 value, uint8 *buffer, uint8 width, uint8 *unit)
{
    if (value < 1000) {
        snprintf((char *)buffer, width, "%6d m%s", value, unit);
    } else {
        snprintf((char *)buffer, width, "%2d.%3d  %s", value / 1000, value % 1000, unit);
    }
}

void displayAmps(void *data, uint8 *buffer, uint8 width)
{
    uint16 value = *(uint16 *)data;
    
    if(!buffer) {
        return;
    }
    _displayFromMillis(value, buffer, width, (uint8 *)"A");
}

void displayBattAmpHours(void *data, uint8 *buffer, uint8 width)
{
    uint32 value = *(uint32 *)data;
    
    if(!buffer) {
        return;
    }
    
    if (!batteryPresent) {
        snprintf((char *)buffer, width, "----");
    } else {
        _displayFromMicros(value, buffer, width, (uint8 *)"AH");
    }
}

void _displayFromMicros(uint32 value, uint8 *buffer, uint8 width, uint8 *unit)
{
    if (value < 1000) {
        snprintf((char *)buffer, width, "%6ld m%s", value, unit);
    } else {
        _displayFromMillis(value / 1000, buffer, width, unit);
    }
}

void displayBattTemperature(void *data, uint8 *buffer, uint8 width)
{
    int16 value = *(int32 *)data;
    uint16 absValue = _abs(value);
    
    if(!buffer) {
        return;
    }

    if (!batteryPresent) {
        snprintf((char *)buffer, width, "----");
    } else {
        snprintf((char *)buffer, width, "%4d.%02d C", value / 100, absValue % 100);
    }
}

void _displaySignal(uint8 active, uint8 *buffer, uint8 width)
{
    snprintf((char *)buffer, width, "%s", active ? "TRUE" : "FALSE");
}

void displayActiveLow(void *data, uint8 *buffer, uint8 width)
{
    uint8 value = *(volatile uint8 *)data;
    
    if(!buffer) {
        return;
    }
    _displaySignal(!value, buffer, width);
}

void displayActiveHigh(void *data, uint8 *buffer, uint8 width)
{
    uint8 value = *(volatile uint8 *)data;
    
    if(!buffer) {
        return;
    }
    _displaySignal(!(!value), buffer, width);
}

void displayBattActiveHigh(void *data, uint8 *buffer, uint8 width)
{
        if(!buffer) {
        return;
    }

    if (!batteryPresent) {
        snprintf((char *)buffer, width, "----");
    } else {
        displayActiveHigh(data, buffer, width);
    }
}

void displaySelected(void *data, uint8 *buffer, uint8 width)
{
    uint8 value = *(volatile uint8 *)data;
    
    if(!buffer) {
        return;
    }
    snprintf((char *)buffer, width, "%s", selector[value & 0x03]);
}

void formatEjectConnect(uint8 *buffer, uint8 width, uint8 *format)
{
    snprintf((char *)buffer, width, (char *)format, (connectBattery ? "Eject" : "Connect"));
}

void formatInputSelector(uint8 *buffer, uint8 width, uint8 *format)
{
    selectValue &= 0x3;
    snprintf((char *)buffer, width, (char *)format, selector[selectValue]);
}

void formatOverride(uint8 *buffer, uint8 width, uint8 *format)
{
    snprintf((char *)buffer, width, (char *)format, (overrideDisabled ? "re-enable" : "disable"));
}

/* [] END OF FILE */
