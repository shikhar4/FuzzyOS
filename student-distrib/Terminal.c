#include "Terminal.h"

/** 
 * terminal_open
 * Description: Clears the screen and initializes terminal variables
 * Inputs: fd, filename
 * Outputs: return 0 
 * Side Effects: clears the screen and resets cursor
 */
int32_t terminal_open(int fd, const uint8_t* filename){
    clear();                // clear screen, set cursor, and initialize vars
    reset_cursor();         
    // enter = 0;
    return -1;
}

/** 
 * terminal_read
 * Description: Reads user input up to 128 bytes
 * Inputs: fd, buf, nbytes
 * Outputs: return number of bytes read
 * Side Effects: Scans user input into the buffer provided (buf)
 */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes){
    if ((int *) buf == NULL_char)           // if null buffer, return error
        return -1;
    if (nbytes < 1)                         // if 0 or less bytes, return 
        return 0;
    sti();                                  // enable interrupts for user keyboard inputs

    terminal_t* terminal = (terminal_t *)terminal_data[getDisplayTerm()];
    if (nbytes > 128) // 128 byte max keyboard buffer
        nbytes = 128;
    char * temp_buf = (char *) buf;
    uint32_t bytes_read = 0;

    while (!(((terminal_t *)terminal_data[getExecuteTerm()])->_enter_flag) || getDisplayTerm() != getExecuteTerm());

    cli();
    int i;
    for (i = 0; i < nbytes; i++) {
        temp_buf[i] = keyboard_buf[i];      // copy keyboard buffer into terminal buffer
        // if (!flag) 
        bytes_read++;
        if (keyboard_buf[i] == '\n')
            break;
    }
    terminal->_buffer_loc = 0;                         // reset keyboard buffer
    ((terminal_t *)terminal_data[getExecuteTerm()])->_enter_flag = 0;                              // reset enter flag for new read
    // enter = 0;
    buf = temp_buf;
    sti();
    return bytes_read;

}

/** 
 * terminal_write
 * Description: Prints from the input buffer onto the screen
 * Inputs: fd, buf, nbytes
 * Outputs: return number of bytes written on success, -1 if invalid param
 * Side Effects: Prints nbytes chars (including newline) to screen, limiting to max of 128 and skipping NULL chars
 */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes){
    if ((int *) buf == NULL_char)           // if not valid buffer, return error
        return -1;
    char * temp_buf = (char *) buf;

    int32_t cur_execute = getExecuteTerm();
    
    int i;
    int count = 0;
    for (i = 0; i < nbytes; i++){    
        count++;
        if (temp_buf[i] == NULL_char)       // don't want to print NULL char
            continue;

        if (temp_buf[i] == tab_char) {      // if tab, print 4 spaces
            terminal_putc(' ', cur_execute);
            terminal_putc(' ', cur_execute);
            terminal_putc(' ', cur_execute);
            terminal_putc(' ', cur_execute); 
            continue;
        }
        terminal_putc(temp_buf[i], cur_execute);                  // print to terminal

    }
    store_deleted_line();                   // prevent overwriting previous Terminal line
    update_cursor();                        // redraw cursor to appropriate location
    return count;
}

/** 
 * terminal_close
 * Description: Closes the terminal
 * Inputs: fd
 * Outputs: return 0
 * Side Effects: clears screen and resets cursor
 */
int32_t terminal_close(int32_t fd){
    // clear screen - not required but is a good thing to do for memory efficiency purposes 
    clear();
    reset_cursor();
    return -1;
}
