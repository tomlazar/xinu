/**
 * @file framebuffer.h
 * Constants and declarations associated with the framebuffer system.
 */
/* Embedded Xinu, Copyright (C) 2009, 2013.  All rights reserved. */

#ifndef _FRAMEBUFFER_H_
#define _FRAMEBUFFER_H_

#include <device.h>
#include <stddef.h>

extern volatile unsigned int mbox[36];
extern bool screen_initialized;

/* Define states for color codes (escape sequence) */
#define STANDARD	0
#define ESCAPE		1
#define BRKT		2
#define FEATURE		3
#define SEMI		4
#define CL1		5
#define CL2		6
#define MODE		7
#define CHANGE_COLOR	8
#define TERM_CLEAR1	9
#define START_CCHANGE	10

extern uint fb_esc_color1;
extern uint fb_esc_color2;
extern uint fb_icolor;
extern uint fb_state;
extern uint ret_ct;

#define MMIO_BASE       0x3F000000
#define VIDEOCORE_MBOX  (MMIO_BASE+0x0000B880)
#define MBOX_READ       ((volatile unsigned int*)(VIDEOCORE_MBOX+0x0))
#define MBOX_POLL       ((volatile unsigned int*)(VIDEOCORE_MBOX+0x10))
#define MBOX_SENDER     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x14))
#define MBOX_STATUS     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x18))
#define MBOX_CONFIG     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x1C))
#define MBOX_WRITE      ((volatile unsigned int*)(VIDEOCORE_MBOX+0x20))
#define MBOX_RESPONSE   0x80000000
#define MBOX_FULL       0x80000000
#define MBOX_EMPTY      0x40000000
#define MBOX_TAG_LAST           0

/* Framebuffer specific constants and definitions. */
#define MAILBOX_FULL 0x80000000 // set bit in status register if no space in mailbox
#define MAILBOX_EMPTY 0x40000000 // set bit in status register if nothing to read from mailbox
#define MMIO_BASE 0x3F000000 // base address for peripherals

#define MAILBOX_CH_PROPERTY 8
#define MAILBOX_BASE 0xB880 // base address for mailbox registers
#define MBOX_REQUEST 0
#define CHAR_WIDTH 8
#define CHAR_HEIGHT 12

extern unsigned char FONT[];

#define DEFAULT_HEIGHT 768	
#define DEFAULT_WIDTH 1024	
#define BIT_DEPTH 32

#define MAXRETRIES 3 //if screen fails to initialize after three tries, halt

/* Some basic colors. 8 bits transparency, 8 bits blue, 8 bits green, 8 bits red.*/

/*
#define WHITE 0xFFFFFFFF
#define RED 0xFF0000FF
#define GREEN 0xFF00FF00
#define BLUE 0xFFFF0000
#define BLACK 0xFF000000
#define GRAY 0xFF808080
#define SILVER 0xFFC0C0C0
#define YELLOW 0xFF00FFFF
#define CYAN 0xFFFFFF00
#define MAGENTA 0xFFFF00FF
#define INDIGO 0xFF54082E
#define DARKBLUE 0xFFA00000
#define ORANGE 0xFF00A5FF
#define BROWN 0xFF374E6F
#define RASPBERRY 0xFF5C0BE3
#define LEAFGREEN 0xFF009900
#define DARKGREEN 0xFF174125
#define PURPLE 0xFF82004B
#define BLUEIVY 0xFFC79030
#define PINK 0xFF9000FF
*/

/* Reformatted colors. 8 bits transparency, 8 bits red, 8 bits green, 8 bits blue. */

#define WHITE		0xFFFFFFFF
#define RED		0xFFFF0000
#define GREEN		0xFF00FF00
#define BLUE		0xFF0000FF
#define BLACK		0xFF000000
#define GRAY		0xFF808080
#define SILVER		0xFFC0C0C0
#define YELLOW		0xFFFFFF00
#define CYAN		0xFF00FFFF
#define MAGENTA		0xFFFF00FF
#define INDIGO		0xFF2E0854
#define	DARKBLUE	0xFF0000A0
#define	ORANGE		0xFFFFA500
#define BROWN		0xFF6F4E37
#define	RASPBERRY	0xFFE30B5C
#define	LEAFGREEN	0xFF009900
#define	DARKGREEN	0xFF254117
#define	PURPLE		0xFF4B0082
#define	BLUEIVY		0xFF3090C7
#define	PINK		0xFFFF0090

#define ERRORCOLOR 	0x00000000

struct defaultcolor {
	char *colorname;
	ulong colornum;
};

/* flip the bytes to get the blue, green, red order and tack on a 255 transparency. */
#define colorconvert(x) ((((((x)& 0xff)<<24) | (((x)>>24) & 0xff) | \
				(((x) & 0xff0000)>>8) | (((x) & 0xff00)<<8)) >> 8) | 0xff000000)
/* Turtle constants */
#define TURTLE_BODY 0xFF347C2C 
#define TURTLE_HEAD 0xFF438D80 
#define BODY_RADIUS 25
#define HEAD_RADIUS 8

#define PI 3.14159

/* The struct the GPU is looking for. */
struct framebuffer {
	ulong width_p;	/* requested width of physical display */
	ulong height_p;	/* requested height of physical display */
	ulong width_v;	/* requested width of virtual display */
	ulong height_v;	/* requested height of virtual display */
	ulong pitch;	/* zero upon request; in response: # bytes between rows */
	ulong depth;	/* requested depth in bits per pixel */
	ulong x;	/* requested x offset of virtual framebuffer */
	ulong y;	/* requested y offset of virtual framebuffer */
	ulong address;	/* framebuffer address. Zero upon request; failure if zero upon response. */
	ulong size;	/* framebuffer size. Zero upon request.  */
};

extern ulong framebufferAddress;
extern int pitch;
extern struct framebuffer fbtab[];
extern ulong foreground;
extern ulong background;
extern int rows;
extern int cols;
extern int cursor_col; 
extern int cursor_row;
extern bool minishell;

#define MINISHELLMINROW 16 /* When running in minishell mode, only use the bottom fifteen lines of the screen */

// include a "line memory" array that remembers all drawn lines
// currently used by the xsh_turtle function.
// each entry in linemap maps to y * width + x
#define MAPSIZE DEFAULT_HEIGHT * DEFAULT_WIDTH
extern ulong linemap[];

/* Turtle command structs and definitions */
#define COMMANDNAMELENGTH 10    /* max length of procedure name */
#define COMMANDLENGTH 100       /* max length of new procedure */
#define MAXNEWCOMMANDS 10       /* cannot add more than 10 new commands at a time */

struct defaultcommand {
	char commandname[COMMANDNAMELENGTH];
	void (*command) (char*);
}; 

struct newcommand {
	char name[COMMANDNAMELENGTH];
	char text[COMMANDLENGTH];
};
extern struct newcommand newcommandtab[];

/* driver functions */
devcall fbWrite(device *, const uchar *, uint);
devcall fbPutc(device *, char);
syscall fbprintf(char *fmt, ...);

/* other function prototypes */
void screenInit(void);
int framebufferInit(void);
ulong mailboxRead(void);
void mailboxWrite(ulong);
ulong physToBus(void *);
void writeMMIO(ulong, ulong, ulong);
ulong readMMIO(ulong, ulong);
void screenClear(ulong);
void minishellClear(ulong);
void viewlinemap(void); 
void drawPixel(int, int, ulong);
void drawLine(int, int, int, int, ulong);
void initlinemap(void);
void drawSegment(int, int, int, int, ulong);
void drawBody(int, int, int, ulong);
void fillBody(int, int, int, ulong, bool);
void drawChar(char, int, int, ulong);
void drawRect(int, int, int, int, ulong);
void fillRect(int, int, int, int, ulong, bool);
void drawCircle(int, int, int, ulong);
void fillCircle(int, int, int, ulong, bool);
double sin(int);
double cos(int);

#endif /* _FRAMEBUFFER_H_*/
