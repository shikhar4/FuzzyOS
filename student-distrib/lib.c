/* lib.c - Some basic library functions (printf, strlen, etc.)
 * vim:ts=4 noexpandtab */

#include "lib.h"
#include "rtc.h"
#include "keyboard.h"

// int screen_x;
// int screen_y;
// int save_y = NUM_COLS;
static char* video_mem = (char *)VIDEO;

int ATTRIB = 0x17;

/* void clear(void);
 * Inputs: void
 * Return Value: none
 * Function: Clears video memory */
void clear(void) {
    int32_t i;
    video_mem = (char *) VIDEO;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        *(uint8_t *)(video_mem + (i << 1)) = ' ';
        *(uint8_t *)(video_mem + (i << 1) + 1) = colors[getDisplayTerm()];
    }
}

/* Standard printf().
 * Only supports the following format strings:
 * %%  - print a literal '%' character
 * %x  - print a number in hexadecimal
 * %u  - print a number as an unsigned integer
 * %d  - print a number as a signed integer
 * %c  - print a character
 * %s  - print a string
 * %#x - print a number in 32-bit aligned hexadecimal, i.e.
 *       print 8 hexadecimal digits, zero-padded on the left.
 *       For example, the hex number "E" would be printed as
 *       "0000000E".
 *       Note: This is slightly different than the libc specification
 *       for the "#" modifier (this implementation doesn't add a "0x" at
 *       the beginning), but I think it's more flexible this way.
 *       Also note: %x is the only conversion specifier that can use
 *       the "#" modifier to alter output. */
int32_t printf(int8_t *format, ...) {

    /* Pointer to the format string */
    int8_t* buf = format;

    /* Stack pointer for the other parameters */
    int32_t* esp = (void *)&format;
    esp++;

    while (*buf != '\0') {
        switch (*buf) {
            case '%':
                {
                    int32_t alternate = 0;
                    buf++;

format_char_switch:
                    /* Conversion specifiers */
                    switch (*buf) {
                        /* Print a literal '%' character */
                        case '%':
                            putc('%');
                            break;

                        /* Use alternate formatting */
                        case '#':
                            alternate = 1;
                            buf++;
                            /* Yes, I know gotos are bad.  This is the
                             * most elegant and general way to do this,
                             * IMHO. */
                            goto format_char_switch;

                        /* Print a number in hexadecimal form */
                        case 'x':
                            {
                                int8_t conv_buf[64];
                                if (alternate == 0) {
                                    itoa(*((uint32_t *)esp), conv_buf, 16);
                                    puts(conv_buf);
                                } else {
                                    int32_t starting_index;
                                    int32_t i;
                                    itoa(*((uint32_t *)esp), &conv_buf[8], 16);
                                    i = starting_index = strlen(&conv_buf[8]);
                                    while(i < 8) {
                                        conv_buf[i] = '0';
                                        i++;
                                    }
                                    puts(&conv_buf[starting_index]);
                                }
                                esp++;
                            }
                            break;

                        /* Print a number in unsigned int form */
                        case 'u':
                            {
                                int8_t conv_buf[36];
                                itoa(*((uint32_t *)esp), conv_buf, 10);
                                puts(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a number in signed int form */
                        case 'd':
                            {
                                int8_t conv_buf[36];
                                int32_t value = *((int32_t *)esp);
                                if(value < 0) {
                                    conv_buf[0] = '-';
                                    itoa(-value, &conv_buf[1], 10);
                                } else {
                                    itoa(value, conv_buf, 10);
                                }
                                puts(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a single character */
                        case 'c':
                            putc((uint8_t) *((int32_t *)esp));
                            esp++;
                            break;

                        /* Print a NULL-terminated string */
                        case 's':
                            puts(*((int8_t **)esp));
                            esp++;
                            break;

                        default:
                            break;
                    }

                }
                break;

            default:
                putc(*buf);
                break;
        }
        buf++;
    }
    return (buf - format);
}

/* int32_t puts(int8_t* s);
 *   Inputs: int_8* s = pointer to a string of characters
 *   Return Value: Number of bytes written
 *    Function: Output a string to the console */
int32_t puts(int8_t* s) {
    register int32_t index = 0;
    while (s[index] != '\0') {
        putc(s[index]);
        index++;
    }
    return index;
}

int32_t keyboard_puts(int8_t* s) {
    register int32_t index = 0;
    while (s[index] != '\0') {
        if (s[index == '\n'])
            break;
        putc(s[index]);
        index++;
    }
    return index;
}

/* void putc(uint8_t c);
 * Inputs: uint_8* c = character to print
 * Return Value: void
 *  Function: Output a character to the console */
void putc(uint8_t c) {
    // char * temp_vidmem = (char*) VIDEO;
    video_mem = (char *) VIDEO;
    terminal_t* terminal = (terminal_t *)terminal_data[getDisplayTerm()];
    if(c == '\n' || c == '\r') {
        // if enter is pressed, update screen_x to the start of the next line
        (terminal->_screen_y)++;
        terminal->_screen_x = 0;  
    } else {
        *(uint8_t *)(video_mem + ((NUM_COLS * terminal->_screen_y + terminal->_screen_x) << 1)) = c;
        *(uint8_t *)(video_mem + ((NUM_COLS * terminal->_screen_y + terminal->_screen_x) << 1) + 1) = colors[getDisplayTerm()];
        terminal->_screen_x++;
         // if end of last line reached, update screen_x to the start of the line and increment screen_y by 1
        if (terminal->_screen_x == NUM_COLS) {
            terminal->_screen_x = 0;
            (terminal->_screen_y)++;
            if (terminal->_screen_y == NUM_ROWS) { // if end of terminal reached, scroll down by 1 line 
                scroll_down(1, -1, video_mem);
                // terminal->_save_y--;
            }
            return; 
        }
        terminal->_screen_x %= NUM_COLS;
        terminal->_screen_y = (terminal->_screen_y + (terminal->_screen_x / NUM_COLS)) % NUM_ROWS;
        
    }
    if (terminal->_screen_y == NUM_ROWS) { // if end of terminal reached, scroll down by one line 
        scroll_down(1, -1, video_mem);
        // terminal->_save_y--;
    }
}

/* void terminal_putc(uint8_t c);
 * Inputs: uint_8* c = character to print, cur execute termimnal
 * Return Value: void
 * Function: Output a character to the terminal memory */
void terminal_putc(uint8_t c, int32_t video) {
    char * temp_vidmem = (char*) VIDEO;
    int32_t cur_execute = video;
    if (video == getDisplayTerm())
        video = 3; // 3 is just used as a flag
    
    int32_t flag = 0;           // write to current execute termainal memory, or physical VIDEO memory if execute=display
    switch (video) {
        
        case 0:
            temp_vidmem = (char *)terminal_vidmem[0];
            break;
        case 1:
            temp_vidmem = (char *)terminal_vidmem[1];
            break;
        case 2:
            temp_vidmem = (char *)terminal_vidmem[2];
            break;
        case 3:
            temp_vidmem = (char *)VIDEO;
            flag = 1;
            break;
        default: 
            temp_vidmem = (char *)VIDEO;
    }
    terminal_t* terminal = (terminal_t *)terminal_data[cur_execute];
    
    if(c == '\n' || c == '\r') {
        // if enter is pressed, update screen_x to the start of the next line
        (terminal->_screen_y)++;
        terminal->_screen_x = 0;  
    } else {
        *(uint8_t *)(temp_vidmem + ((NUM_COLS * terminal->_screen_y + terminal->_screen_x) << 1)) = c;
        *(uint8_t *)(temp_vidmem + ((NUM_COLS * terminal->_screen_y + terminal->_screen_x) << 1) + 1) = colors[cur_execute];
        (terminal->_screen_x)++;
         // if end of last line reached, update screen_x to the start of the line and increment screen_y by 1
        if (terminal->_screen_x == NUM_COLS) {
            terminal->_screen_x = 0;
            (terminal->_screen_y)++;
            if (terminal->_screen_y == NUM_ROWS) // if end of terminal reached, scroll down by 1 line 
                scroll_down(flag, cur_execute, temp_vidmem);
            return; 
        }
        (terminal->_screen_x) %= NUM_COLS;
        terminal->_screen_y = (terminal->_screen_y + (terminal->_screen_x / NUM_COLS)) % NUM_ROWS;
        
    }
    if (terminal->_screen_y == NUM_ROWS) // if end of terminal reached, scroll down by one line 
        scroll_down(flag, cur_execute, temp_vidmem);
    // temp_vidmem = (char *)VIDEO;
}

/* void deletec()
 * Inputs: None
 * Return Value: void
 *  Function: deletes the previous character */
void deletec() {
    // char * temp_vidmem = (char*) VIDEO;
    video_mem = (char *) VIDEO;
    terminal_t* terminal = (terminal_t *)terminal_data[getDisplayTerm()];
    (terminal->_screen_x)--;
    if (terminal->_screen_x < 0 && terminal->_screen_y > terminal->_save_y) {                // if at beginning of current line and want to delete more, then
        terminal->_screen_x += NUM_COLS;                               // delete from end of previous line
        (terminal->_screen_y)--;
    }
    else if (terminal->_screen_x < 0 && terminal->_screen_y <= terminal->_save_y)            // if at beginning of current line and want to delete more, but save_y
        terminal->_screen_x = 0;                                       // prevents, then reset x to 0 (keep at left side of current line)
    char clear = ' ';
    *(uint8_t *)(video_mem + ((NUM_COLS * terminal->_screen_y + terminal->_screen_x) << 1)) = clear;  // replacing the currentl line with an empty string 
    // *(uint8_t *)(video_mem + ((NUM_COLS * screen_y + screen_x-1) << 1) + 1) = ATTRIB;
    
}

/* void reset_cursor()
 * Inputs: None
 * Return Value: void
 *  Function: Resets the cursor to the top left of the screen */
void reset_cursor() { 
    terminal_t* terminal = (terminal_t *)terminal_data[getDisplayTerm()];
    /// update cursor to the (0, 0) point on the terminal 
    terminal->_screen_x = 0;
    terminal->_screen_y = 0;
    // *(uint8_t *)(video_mem + ((NUM_COLS * screen_y + screen_x) << 1)) = '_';
}

/* void update_cursor()
 * Inputs: None
 * Return Value: void
 *  Function: updates the cursor depending on the current Screen_x and screen_y values */
void update_cursor() {
    terminal_t* terminal = (terminal_t *)terminal_data[getDisplayTerm()];
    // the following code is picked up from OSDev 
	uint16_t pos = (terminal->_screen_y ) * NUM_COLS + terminal->_screen_x; // find the current position of the cursor 
	outb(lower_8_cursor_pos, cursor_port); // clearing current cursor position 
	outb((uint8_t) (pos & cursor_mask), cursor_data); //splitting cursor coordinates
	outb(upper_8_cursor_pos, cursor_port);
	outb((uint8_t) ((pos >> 8) & cursor_mask), cursor_data); // splitting cursor coordinates by bitshifting by 8 bits
}

/* void store_deleted_line()
 * Inputs: None
 * Return Value: void
 *  Function: stores the last line of the terminal (the line at the current screen_y value) */
void store_deleted_line() {
    terminal_t* terminal = (terminal_t *)terminal_data[getDisplayTerm()];
    terminal->_save_y = terminal->_screen_y; // save the last line of the terminal 
}

/* void scroll_down()
 * Inputs: None
 * Return Value: void
 *  Function: scrolls the screen one line up */
void scroll_down (int32_t flag, int32_t execute, char * vidmem) {
    terminal_t* terminal;
    if (!flag){
        terminal = (terminal_t *)terminal_data[execute];
    }
    else{
        terminal = (terminal_t *) terminal_data[getDisplayTerm()];
    }

    int video = getExecuteTerm();
    if (getExecuteTerm() == getDisplayTerm())
        video = 3;

    switch (video) {
        
        case 0:
            video_mem = (char *)terminal_vidmem[0];
            break;
        case 1:
            video_mem = (char *)terminal_vidmem[1];
            break;
        case 2:
            video_mem = (char *)terminal_vidmem[2];
            break;
        case 3:
            video_mem = (char *)VIDEO;
            break;
        default: 
            video_mem = (char *)VIDEO;
    }

    memcpy(vidmem, vidmem + scroll_one_down, NUM_COLS * scroll_limit * 2); // move every line on the screen by one line above 
    // screen_y--;
    // char clear_line[80];
    int i;
    char clear = ' ';  // empty string 
    // for (i = 0; i < 80; i++)
    //     clear_line[i] = clear;
    terminal->_screen_x = 0;
    terminal->_screen_y = scroll_limit; // reach last line before scroll
    
    // puts(clear_line);
    
    // memset(video_mem + (NUM_COLS * screen_y * 2), clear, 80 * 2);

    for (i = 0; i < NUM_COLS; i++) {
        *(uint8_t *)(vidmem + ((NUM_COLS * scroll_limit + i) << 1)) = clear; // clear the last line of the terminal 
    }
    // update_cursor();
    
}


/* int8_t* itoa(uint32_t value, int8_t* buf, int32_t radix);
 * Inputs: uint32_t value = number to convert
 *            int8_t* buf = allocated buffer to place string in
 *          int32_t radix = base system. hex, oct, dec, etc.
 * Return Value: number of bytes written
 * Function: Convert a number to its ASCII representation, with base "radix" */
int8_t* itoa(uint32_t value, int8_t* buf, int32_t radix) {
    static int8_t lookup[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int8_t *newbuf = buf;
    int32_t i;
    uint32_t newval = value;

    /* Special case for zero */
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return buf;
    }

    /* Go through the number one place value at a time, and add the
     * correct digit to "newbuf".  We actually add characters to the
     * ASCII string from lowest place value to highest, which is the
     * opposite of how the number should be printed.  We'll reverse the
     * characters later. */
    while (newval > 0) {
        i = newval % radix;
        *newbuf = lookup[i];
        newbuf++;
        newval /= radix;
    }

    /* Add a terminating NULL */
    *newbuf = '\0';

    /* Reverse the string and return */
    return strrev(buf);
}

/* int8_t* strrev(int8_t* s);
 * Inputs: int8_t* s = string to reverse
 * Return Value: reversed string
 * Function: reverses a string s */
int8_t* strrev(int8_t* s) {
    register int8_t tmp;
    register int32_t beg = 0;
    register int32_t end = strlen(s) - 1;

    while (beg < end) {
        tmp = s[end];
        s[end] = s[beg];
        s[beg] = tmp;
        beg++;
        end--;
    }
    return s;
}

/* uint32_t strlen(const int8_t* s);
 * Inputs: const int8_t* s = string to take length of
 * Return Value: length of string s
 * Function: return length of string s */
uint32_t strlen(const int8_t* s) {
    register uint32_t len = 0;
    while (s[len] != '\0')
        len++;
    return len;
}

/* void* memset(void* s, int32_t c, uint32_t n);
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set n consecutive bytes of pointer s to value c */
void* memset(void* s, int32_t c, uint32_t n) {
    c &= 0xFF;
    asm volatile ("                 \n\
            .memset_top:            \n\
            testl   %%ecx, %%ecx    \n\
            jz      .memset_done    \n\
            testl   $0x3, %%edi     \n\
            jz      .memset_aligned \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            subl    $1, %%ecx       \n\
            jmp     .memset_top     \n\
            .memset_aligned:        \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            movl    %%ecx, %%edx    \n\
            shrl    $2, %%ecx       \n\
            andl    $0x3, %%edx     \n\
            cld                     \n\
            rep     stosl           \n\
            .memset_bottom:         \n\
            testl   %%edx, %%edx    \n\
            jz      .memset_done    \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            subl    $1, %%edx       \n\
            jmp     .memset_bottom  \n\
            .memset_done:           \n\
            "
            :
            : "a"(c << 24 | c << 16 | c << 8 | c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memset_word(void* s, int32_t c, uint32_t n);
 * Description: Optimized memset_word
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set lower 16 bits of n consecutive memory locations of pointer s to value c */
void* memset_word(void* s, int32_t c, uint32_t n) {
    asm volatile ("                 \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            cld                     \n\
            rep     stosw           \n\
            "
            :
            : "a"(c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memset_dword(void* s, int32_t c, uint32_t n);
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set n consecutive memory locations of pointer s to value c */
void* memset_dword(void* s, int32_t c, uint32_t n) {
    asm volatile ("                 \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            cld                     \n\
            rep     stosl           \n\
            "
            :
            : "a"(c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memcpy(void* dest, const void* src, uint32_t n);
 * Inputs:      void* dest = destination of copy
 *         const void* src = source of copy
 *              uint32_t n = number of byets to copy
 * Return Value: pointer to dest
 * Function: copy n bytes of src to dest */
void* memcpy(void* dest, const void* src, uint32_t n) {
    asm volatile ("                 \n\
            .memcpy_top:            \n\
            testl   %%ecx, %%ecx    \n\
            jz      .memcpy_done    \n\
            testl   $0x3, %%edi     \n\
            jz      .memcpy_aligned \n\
            movb    (%%esi), %%al   \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            addl    $1, %%esi       \n\
            subl    $1, %%ecx       \n\
            jmp     .memcpy_top     \n\
            .memcpy_aligned:        \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            movl    %%ecx, %%edx    \n\
            shrl    $2, %%ecx       \n\
            andl    $0x3, %%edx     \n\
            cld                     \n\
            rep     movsl           \n\
            .memcpy_bottom:         \n\
            testl   %%edx, %%edx    \n\
            jz      .memcpy_done    \n\
            movb    (%%esi), %%al   \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            addl    $1, %%esi       \n\
            subl    $1, %%edx       \n\
            jmp     .memcpy_bottom  \n\
            .memcpy_done:           \n\
            "
            :
            : "S"(src), "D"(dest), "c"(n)
            : "eax", "edx", "memory", "cc"
    );
    return dest;
}

/* void* memmove(void* dest, const void* src, uint32_t n);
 * Description: Optimized memmove (used for overlapping memory areas)
 * Inputs:      void* dest = destination of move
 *         const void* src = source of move
 *              uint32_t n = number of bytes to move
 * Return Value: pointer to dest
 * Function: move n bytes of src to dest */
void* memmove(void* dest, const void* src, uint32_t n) {
    asm volatile ("                             \n\
            movw    %%ds, %%dx                  \n\
            movw    %%dx, %%es                  \n\
            cld                                 \n\
            cmp     %%edi, %%esi                \n\
            jae     .memmove_go                 \n\
            leal    -1(%%esi, %%ecx), %%esi     \n\
            leal    -1(%%edi, %%ecx), %%edi     \n\
            std                                 \n\
            .memmove_go:                        \n\
            rep     movsb                       \n\
            "
            :
            : "D"(dest), "S"(src), "c"(n)
            : "edx", "memory", "cc"
    );
    return dest;
}

/* int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n)
 * Inputs: const int8_t* s1 = first string to compare
 *         const int8_t* s2 = second string to compare
 *               uint32_t n = number of bytes to compare
 * Return Value: A zero value indicates that the characters compared
 *               in both strings form the same string.
 *               A value greater than zero indicates that the first
 *               character that does not match has a greater value
 *               in str1 than in str2; And a value less than zero
 *               indicates the opposite.
 * Function: compares string 1 and string 2 for equality */
int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n) {
    int32_t i;
    for (i = 0; i < n; i++) {
        if ((s1[i] != s2[i]) || (s1[i] == '\0') /* || s2[i] == '\0' */) {

            /* The s2[i] == '\0' is unnecessary because of the short-circuit
             * semantics of 'if' expressions in C.  If the first expression
             * (s1[i] != s2[i]) evaluates to false, that is, if s1[i] ==
             * s2[i], then we only need to test either s1[i] or s2[i] for
             * '\0', since we know they are equal. */
            return s1[i] - s2[i];
        }
    }
    return 0;
}

/* int8_t* strcpy(int8_t* dest, const int8_t* src)
 * Inputs:      int8_t* dest = destination string of copy
 *         const int8_t* src = source string of copy
 * Return Value: pointer to dest
 * Function: copy the source string into the destination string */
int8_t* strcpy(int8_t* dest, const int8_t* src) {
    int32_t i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return dest;
}

/* int8_t* strcpy(int8_t* dest, const int8_t* src, uint32_t n)
 * Inputs:      int8_t* dest = destination string of copy
 *         const int8_t* src = source string of copy
 *                uint32_t n = number of bytes to copy
 * Return Value: pointer to dest
 * Function: copy n bytes of the source string into the destination string */
int8_t* strncpy(int8_t* dest, const int8_t* src, uint32_t n) {
    int32_t i = 0;
    while (src[i] != '\0' && i < n) {
        dest[i] = src[i];
        i++;
    }
    while (i < n) {
        dest[i] = '\0';
        i++;
    }
    return dest;
}

/* void test_interrupts(void)
 * Inputs: void
 * Return Value: void
 * Function: increments video memory. To be used to test rtc */
void test_interrupts(void) {
    int32_t i;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        video_mem[i << 1]++;
    }
}

/* void test_rtc_video
 * Inputs: none
 * Return Value: void
 * Function: To be used to test rtc */
void test_rtc_video() {
    int32_t i;
    for (i = 0; i < NUM_COLS ; i++) 
        video_mem[i] = '1';
    
}
