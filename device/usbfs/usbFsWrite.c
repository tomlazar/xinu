#include <stddef.h>
#include <ctype.h>
#include <device.h>
#include <usbfs.h>
#include <thread.h>
#include <stdio.h>

devcall usbFsWrite(device *devptr, void *buf, uint len) {
    struct usbfs *usbptr;
    device *phw;
    uchar ch = 0;
    uint count = 0;
    uchar *buffer = buf;

    usbptr = &usbfstab[devptr->minor];
    phw = usbptr->phw;
    if (phw == NULL) {
         return SYSERR;
    }

    USBFS_TRACE("Writing to device");

    wait(usbptr->osema); 

    while (count < len) {
        ch = buffer[count++];

	if (usbptr->ostart >= USBFS_OBLEN - 1) {
	    if (usbFsFlush(devptr) == SYSERR)
	        return SYSERR;
	}

	switch (ch) {
	    case '\n':
	        usbptr->out[usbptr->ostart++] = '\r';
		usbptr->out[usbptr->ostart++] = '\n';

		if (usbFsFlush(devptr) == SYSERR) {
		    return SYSERR;
		}
		break;
	    default:
	        usbptr->out[usbptr->ostart++] = ch;
		break;
	}
    }
    signal(usbptr->osema);

    return count;
}
