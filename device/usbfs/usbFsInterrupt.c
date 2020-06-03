#include <usbfs.h>
#include <usb_core_driver.h>
#include <string.h>

void usbFsInterrupt(struct usb_xfer_request *req) {
    struct usbfs *fs = req->private;

    if (req->status == USB_STATUS_SUCCESS && req->actual_size == 0) {
        USBFS_TRACE("Filesystem report received");

	const uchar *data = req->recvbuf;
	uint mod_idx = 0;
	uint count = 0;
	uint i;
    }
    usb_submit_xfer_request(req);
}
