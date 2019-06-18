#include <stddef.h>
#include <device.h>
#include <interrupt.h>
#include <usbfs.h>

devcall usbFsFlush(device *devptr) {
    struct usbfs *usbptr;
    device *phw;
    irqmask im;

    usbptr = &usbfstab[devptr->minor];
    phw = usbptr->phw;
    im = disable();

    if (phw == NULL) {
        return SYSERR;
    }

    if (usbptr->ostart > 0) {
        if (SYSERR == 
	    (*phw->write) (phw, (void *)(usbptr->out), usbptr->ostart))
	{
	    restore(im);
	    return SYSERR;
	}

	usbptr->ostart = 0;
    }

    restore(im);

    return OK;
}
