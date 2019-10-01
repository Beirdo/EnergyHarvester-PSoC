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

#ifndef __menuScreen_h__
#define __menuScreen_h__
    
#include "project.h"
  
struct CY_PACKED_ATTR _screen;
struct CY_PACKED_ATTR _menu;
struct CY_PACKED_ATTR _slides;
struct CY_PACKED_ATTR _menu_entry; 
struct CY_PACKED_ATTR _slide_entry;
    
typedef enum {
    SCREEN_MENU,
    SCREEN_SLIDES,
    SCREEN_TOGGLE,
    SCREEN_CHOICES,
} screenType_t;
    
typedef struct CY_PACKED_ATTR _menu {
    const struct CY_PACKED_ATTR _menu_entry *entries;
    uint8 count;
} menu_t;

typedef struct CY_PACKED_ATTR _slides {
    const struct CY_PACKED_ATTR _slide_entry *entries;
    uint8 count;
    uint8 slideCount;
} slides_t;

typedef struct CY_PACKED_ATTR _toggle {
    void *data;
} toggle_t;

typedef struct CY_PACKED_ATTR _choices {
    void *data;
    void *choices;
    uint8 count;
} choices_t;

typedef struct CY_PACKED_ATTR _screen {
    screenType_t type;
    union {
        struct CY_PACKED_ATTR _menu menu;
        struct CY_PACKED_ATTR _slides slides;
        struct CY_PACKED_ATTR _toggle toggle;
        struct CY_PACKED_ATTR _choices choices;
    } item;
} screen_t;

typedef void (*menu_format_func_t)(uint8 *buffer, uint8 width, uint8 *format);
    
typedef struct CY_PACKED_ATTR _menu_entry {
    const uint8 *string;
    menu_format_func_t func;
    const struct CY_PACKED_ATTR _screen *screen;
} menu_entry_t;

typedef void (*slide_display_func_t)(void *, uint8*, uint8);

typedef struct CY_PACKED_ATTR _slide_entry {
    const uint8 *label;
    uint8 slideNum;
    uint8 xLabel;
    uint8 yLabel;
    uint8 x;
    uint8 y;
    uint8 width;
    slide_display_func_t func;
    void *data;    
} slide_entry_t;

typedef struct CY_PACKED_ATTR _screen_queue_item {
    uint8 top;
    uint8 current;
    screen_t *screen;
} screen_queue_item_t;


void menu_screen_reset(void);
void menu_screen_action(uint8 action, uint8 button);
void menu_screen_refresh(void);
    
#endif // __menuScreen_h__

/* [] END OF FILE */
