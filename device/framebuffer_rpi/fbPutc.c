/**
 * @file fbPutc.c
 */

/* Embedded Xinu, Copyright (C) 2009, 2013.  All rights reserved. */

#include <stddef.h>
#include <framebuffer.h>
#include <device.h>
#include <platform.h>

extern int rows;
extern int cols;
extern int cursor_col;
extern int cursor_row;
extern ulong foreground;
extern ulong background;
extern bool screen_initialized;
uint fb_esc_color1 = 0;
uint fb_esc_color2 = 0;
uint fb_icolor = 0;
uint fb_state = 0;
uint ret_ct = 0;

/**
 * @ingroup framebuffer
 *
 * Write a single character to the framebuffer
 * @param devptr	pointer to framebuffer device
 * @param ch		character to write
 */
devcall fbPutc(device *devptr, char ch)
{

    if (screen_initialized)
    {

	/* Parse ANSI escape codes using states.
	 * There are many cases in which the frame buffer has to do something fancy,
	 * like print colored text or clear the screen. The terminal (CONSOLE) can 
	 * do this with escape codes. This section of code parses those escape codes
	 * to prevent redundant code. Thus, only one 'printf' is necessary
	 * in the calling function, as this handles the special framebuffer cases. 
	 * Colors can be added in the CHANGE_COLOR switch statement below. */
	if (ch == 033){
		fb_state = ESCAPE;
		return '\0';
	}

	switch (fb_state){
		case ESCAPE:
		// Assume the escape sequence is a color code with the format mentioned above
			
			// Skip the opening bracket
			if (ch == '['){
				fb_state = BRKT;
				return '\0';
			}
			break;

		case BRKT:
			if(ch == '2'){
				fb_state = TERM_CLEAR1;
				return '\0';
				break;
			}
			else {	// Color assumed to look like: \033[1;32m
				fb_state = START_CCHANGE;
				return '\0'; // Return null terminator instead of '1'
			}

		case TERM_CLEAR1: // \033[2J = screen clear terminal code
			if(ch == 'J'){
				screenClear(background);
				cursor_row = 0;
				return '\0';
			}
			break;

		case START_CCHANGE:
				// Return the null terminator one more time to skip the semicolon and get to the color code
				if(ret_ct <= 0){
					ret_ct++;
					return '\0';
				}
				else{
					ret_ct = 0;
					fb_state = CL1;
				}
				// Don't break here because the current char is the first color digit (CL1)
		case CL1:
			fb_esc_color1 = (ch - '0') * 10;
			fb_state = CL2;
			return '\0';
			break;
		case CL2: // Convert second color character to proper integer
			fb_esc_color2 = (ch - '0');
			fb_state = CHANGE_COLOR;
			return '\0';
			break;
		case CHANGE_COLOR:
			// Get the final value of the color and change it accordingly
			fb_icolor = fb_esc_color1 + fb_esc_color2;
			switch (fb_icolor)
			{
			case 31: // Red
				foreground = RED;
				fb_state = STANDARD;
				return '\0';
				break;
			case 32: // Green
				foreground = GREEN;
				fb_state = STANDARD;
				return '\0';
				break;
			case 39: // Default (white)
				foreground = WHITE;
				fb_state = STANDARD;
				return '\0';
				break;
			case 96: // Bright cyan
				foreground = CYAN;
				fb_state = STANDARD;	// Set back to standard before returning
				return '\0';
				break;
			default:
				break;
			}
			break;
		default:
			break;
	}

	/* Standard character processing */
        if (ch == '\n')
        {
            cursor_row++;
            cursor_col = 0;
        }
        else if (ch == '\t')
        {
            cursor_col += 4;
        }
        drawChar(ch, cursor_col * CHAR_WIDTH,
                 cursor_row * CHAR_HEIGHT, foreground);
        cursor_col++;
        if (cursor_col == cols)
        {
            cursor_col = 0;
            cursor_row += 1;
        }
        if ( (minishell == TRUE) && (cursor_row == rows) )
        {
            minishellClear(background);
            cursor_row = rows - MINISHELLMINROW;
        }
        else if (cursor_row == rows)
        {
            screenClear(background);
            cursor_row = 0;
        }
        return (uchar)ch;
    }
    return SYSERR;
}
