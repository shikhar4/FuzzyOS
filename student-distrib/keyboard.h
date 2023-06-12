#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "i8259.h"
#include "lib.h"
#include "Terminal.h"
#include "system_call.h"

#define buffer_size         128
#define tab_key             0x0F
#define backspace_key       0x0E
#define left_ctrl_press     0x1D
#define right_ctrl_press    0xE0
#define left_ctrl_release   0x9D
#define right_ctrl_release  0xE0
#define caps_key            0x3A
#define left_shift_press    0x2A
#define right_shift_press   0x36
#define left_shift_release  0xAA
#define right_shift_release 0xB6
#define L                   0x26
#define key_press           0x59
#define left_alt_press      0x38
#define right_alt_press     0xE0
#define left_alt_release    0xB8
#define right_alt_release   0xE0
#define F1                  0x3B
#define F2                  0x3C
#define F3					0x3D

// initializes keyboard irq on PIC
void init_keyboard();

// gets user input from keyboard
void get_keyboard_input();

int enter;
extern int caps_flag;
extern int buffer_loc;
extern char keyboard_buf[127];         // global variables needed for Terminal communication/synchronization

#endif 
