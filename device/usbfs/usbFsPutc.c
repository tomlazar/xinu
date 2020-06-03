#include <usbfs.h>
#include <device.h>

devcall usbFsPutc(device *devptr, char ch) {
    int ret;

    USBFS_TRACE("usbFsPutc has been called on device");

    USBFS_TRACE("Calling usbFsWrite(%c, 1)", ch);

    ret = usbFsWrite(devptr, &ch, 1);
    if (ret == 1) {
        return (uchar)ch;
    } else {
        return SYSERR;
    }
}
