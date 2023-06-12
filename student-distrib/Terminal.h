#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "lib.h"
#include "keyboard.h"
#include "system_call.h"

#define NULL_char   '\0'
#define tab_char    '\t'
#define new_line    '\n'        // keyboard input to handle in terminal

typedef struct terminal_t {
    int32_t _screen_x;          // current cursor location
    int32_t _screen_y;
    int32_t _save_y;            // saved previous line
    int32_t _caps_flag;
    int32_t _enter_flag;

    char _keyboard_buf[127];    // current keyboard buffer
    int32_t _buffer_loc;        // current keyboard buffer location
    
} terminal_t;

// clears screen initializes related variables
extern int32_t terminal_open(int fd, const uint8_t* filename);

// scans user input into given buffer
extern int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);

// prints buffer to terminal screen
extern int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);

// clears screen and resets cursor
extern int32_t terminal_close(int32_t fd);


#endif 
