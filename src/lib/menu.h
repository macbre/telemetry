#ifndef _MENU_H
#define _MENU_H
#include "../telemetry.h"

#define MENU_POS_COUNT 5

unsigned char menu_pos, menu_sub_pos, menu_updated;

void menu_init();
void menu_handle_keys();
void menu_update();

#endif
