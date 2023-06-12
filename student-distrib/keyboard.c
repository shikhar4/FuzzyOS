
#include "keyboard.h"

static int shift_flag = 0; // global variable to keep track of shift, caps lock, and control keys 
int caps_flag = 0;
static int control_flag = 0;
static int alt_flag = 0;

int buffer_loc;
char keyboard_buf[127];

// keyboard map of scancodes to their corresponding ASCII chars
char keyboard_map[87] = {0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', /*backspace*/
'\t', /*tab*/
'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /*enter*/
0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 
0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 
0, 0, 0, ' ', 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0};

char keyboard_map_caps[87] = {0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', /*backspace*/
'\t', /*tab*/
'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n', /*enter*/
0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', 
0, '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', 
0, 0, 0, ' ', 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0};

char keyboard_map_shift[87] = {0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', /*backspace*/
'\t', /*tab*/
'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', /*enter*/
0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', 
0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',
0, 0, 0, ' ', 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0};

char keyboard_map_shift_caps[87] = {0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', /*backspace*/
'\t', /*tab*/
'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\n', /*enter*/
0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':', '\"', '~', 
0, '|', 'z', 'x', 'c', 'v', 'b', 'n', 'm', '<', '>', '?',
0, 0, 0, ' ', 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0};


/** 
 * init_keyboard
 * 
 * Description: Initializes the keyboard
 * Inputs: none
 * Outputs: none
 * Side Effects: Enables port 1 on master
 */
void init_keyboard() {
    enable_irq(1);                  //enabling the keyboard
    buffer_loc = 0;                 // point to start of buffer
    // enter[0] = 0;                      // clear enter flag
    // enter[1] = 0;
    // enter[2] = 0;
    int i;
    terminal_t* terminal;
    for (i = 0; i < NUM_TERMINALS; i++) {
        terminal = (terminal_t *)terminal_data[i];
        terminal->_screen_x = 0;
        terminal->_screen_y = 0;
        terminal->_save_y = 0;
        terminal->_caps_flag = 0;
        terminal->_enter_flag = 0;
        terminal->_buffer_loc = 0;
    }
}

/** 
 * get_keyboard_input
 * 
 * Description: Gets user input 
 * Inputs: none
 * Outputs: none
 * Side Effects: Echoes user input on screen
 */
void get_keyboard_input(){
    unsigned int user_input;
    user_input = inb(0x60);                     // gets user input from keyboard port (0x60)
    
    terminal_t* terminal = ((terminal_t *)terminal_data[getDisplayTerm()]); 
    if (user_input == tab_key) {                // if tab pressed
        putc(' ');
        putc(' ');
        putc(' ');
        putc(' ');                              // print 4 spaces and store one char representing tab to keyboard buffer
        keyboard_buf[terminal->_buffer_loc] = tab_char;
		terminal->_buffer_loc++;
        send_eoi(1);
        update_cursor();                        // update cursor after printing spaces
        return;
    }

    if (user_input == backspace_key) {          // if backspace pressed
        // go back, print space, move cursor back again
        
        if (keyboard_buf[terminal->_buffer_loc - 1] == '\t') {
            deletec();
            deletec();
            deletec();                            	// remove character from terminal screen
			deletec();                              // remove character from terminal screen
			update_cursor();                        // update cursor location
			send_eoi(1); 

			if (terminal->_buffer_loc > 0)
            	terminal->_buffer_loc--;                       // remove character from buffer if not empty
			return;
        }
		if (terminal->_buffer_loc == 0) {
            send_eoi(1);
            return;
        }
        if (terminal->_buffer_loc > 0)
            terminal->_buffer_loc--;                       // remove character from buffer if not empty
        deletec();                              // remove character from terminal screen
        send_eoi(1); 
        update_cursor();                        // update cursor location
        return;
    }

    if (user_input == left_ctrl_press || user_input == right_ctrl_press)   
        control_flag = 1;                       // left or right control pressed

    if (user_input == left_ctrl_release || user_input == right_ctrl_release)   
        control_flag = 0;                       // left or right control released

    if(user_input == caps_key && caps_flag == 0){       
        caps_flag = 1;                          // caps lock on
    }  
    else if(user_input == caps_key && caps_flag == 1){  
        caps_flag = 0;                          // caps lock off
    }                              
    if(user_input == left_shift_press || user_input == right_shift_press){   
        shift_flag = 1;                         // left or right shift pressed
    }
    if(user_input == left_shift_release || user_input == right_shift_release){   
        shift_flag = 0;                         // left or right shift released
    }
    if (user_input == left_alt_press || user_input == right_alt_press) 
        alt_flag = 1;
    if (user_input == left_alt_release || user_input == right_alt_release)
        alt_flag = 0;
    
    if (user_input == F1 && alt_flag) { // change display terminal to 0
        if (getDisplayTerm() == 0) {
            send_eoi(1);
            return;
        }
        change_display_terminal(0);
        setDisplayTerm(0);
        update_cursor();
        send_eoi(1);
        return;
    }
    if (user_input == F2 && alt_flag) { // change display terminal to 1
        if (getDisplayTerm() == 1) {
            send_eoi(1);
            return;
        }
        change_display_terminal(1);
        setDisplayTerm(1);
        update_cursor();
        send_eoi(1);
        return;
    }
    if (user_input == F3 && alt_flag) { // change display terminal to 2
        if (getDisplayTerm() == 2) {
            send_eoi(1);
            return;
        }
        change_display_terminal(2);
        setDisplayTerm(2);
        update_cursor();
        send_eoi(1);
        return;
    }

    if ((user_input == L && control_flag) || (user_input == L && control_flag && shift_flag)) {   
        // if control + l or control + L
        clear();                                        // clear screen
        reset_cursor();                                 // reset cursor to top left of screen
        puts((int8_t*)"391OS> \0");
        terminal->_buffer_loc = 0;
        keyboard_puts((int8_t*)keyboard_buf);
        // printf("%s", keyboard_buf);
        terminal->_save_y = 0;
        send_eoi(1);
                                     
        update_cursor();                                // update cursor 
        return;
    }

    if (keyboard_map[user_input] != 0 && user_input < key_press && terminal->_buffer_loc < buffer_size){    
        // if valid key pressed (not released) and buffer not full
        if(shift_flag == 1 && caps_flag == 0){                                      // shift pressed
            keyboard_buf[terminal->_buffer_loc] = keyboard_map_shift[user_input];              // write to keyboard buffer and print char
            terminal->_buffer_loc++;
            putc(keyboard_buf[terminal->_buffer_loc - 1]);
        }
        else if(caps_flag == 1 && shift_flag == 0){                                 // caps lock pressed
            keyboard_buf[terminal->_buffer_loc] = keyboard_map_caps[user_input];
            terminal->_buffer_loc++;
            putc(keyboard_buf[terminal->_buffer_loc - 1]);
        }
        else if((shift_flag == 1 && caps_flag == 1)){                               // caps lock on and shift pressed
            keyboard_buf[terminal->_buffer_loc] = keyboard_map_shift_caps[user_input];
            terminal->_buffer_loc++;
            putc(keyboard_buf[terminal->_buffer_loc - 1]);
        }
        else{
            keyboard_buf[terminal->_buffer_loc] = keyboard_map[user_input];                    // normal keyboard input
            terminal->_buffer_loc++;
            putc(keyboard_buf[terminal->_buffer_loc - 1]);
        }
        if (keyboard_buf[terminal->_buffer_loc-1] == new_line) {                               // if newline char, set enter flag for Terminal
            ((terminal_t *)terminal_data[getDisplayTerm()])->_enter_flag = 1; 
            store_deleted_line();                                                   // keep track of which line to stop deleting at 
            terminal->_buffer_loc = 0;                                                         // reset keyboard buffer
        }

        update_cursor();

    }
    send_eoi(1);            // send eoi to port 1 on master
}

