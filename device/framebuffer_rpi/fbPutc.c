/**
 * @file fbPutc.c
 */

/* Embedded Xinu, Copyright (C) 2009, 2013.  All rights reserved. */

#include <stddef.h>
#include <framebuffer.h>
#include <device.h>

extern int rows;
extern int cols;
extern int cursor_col;
extern int cursor_row;
extern ulong foreground;
extern ulong background;
extern bool screen_initialized;
uint fb_esc_color1;
uint fb_esc_color2;
uint fb_icolor;
uint fb_state;

/**
 * @ingroup framebuffer
 *
 * Write a single character to the framebuffer
 * @param  devptr  pointer to framebuffer device
 * @param  ch    character to write
 */
devcall fbPutc(device *devptr, char ch)
{
    if (screen_initialized)
    {

	/* Handle special escape codes using states (for color code parsing)
	 * =====================================================
	 * Entire sequence looks like this: \033[1;32m
	 * States (in order):
	 * \033 is the escape sequence (ESCAPE state)
	 * The opening bracket (BRKT)
	 * The feature of the text to display, such as bold (FEATURE)
	 * The semicolon (SEMI)
	 * The first integer of the color to display (CL1)
	 * The second integer of the color to display (CL2)
	 * The graphics mode (MODE)
	 *
	 */
	if (ch == 033){
		fb_state = ESCAPE;
		return '\0';
	}

	if (fb_state == ESCAPE){
		if (ch == '['){
			fb_state = BRKT;
			return '\0';
		}
	}
	else if (fb_state == BRKT){
		if (ch == '1' || ch == '0'){
			fb_state == FEATURE;
			return '\0';
		}
	}
	else if (fb_state == FEATURE){
		if (ch == ';'){
			fb_state == SEMI;
			return '\0';
		}
	}
	else if (fb_state == SEMI){
		// Expecting a color to follow the semicolon
		fb_state = CL1;
		fb_esc_color1 = (ch - '0') * 10;
		return '\0';
	}
	else if (fb_state == CL2){
		fb_state = MODE;
		fb_esc_color2 = (ch - '0');
		return '\0';
	}
	else if (fb_state == MODE){
		if (ch == ';'){
			fb_state = CHANGE_COLOR;
			return '\0';
		}
	}
	else if (fb_state == CHANGE_COLOR){
		fb_icolor = fb_esc_color1 + fb_esc_color2;
		switch (fb_icolor)
		{
		case 31: // Red
			foreground = RED;
			break;
		case 32: // Green
			foreground = GREEN;
			break;
		case 39: // Default (white)
			foreground = WHITE;
			break;
		case 96: // Bright cyan
			foreground = CYAN;
			break;
		default:
			break;
		}
		fb_state = STANDARD;
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
