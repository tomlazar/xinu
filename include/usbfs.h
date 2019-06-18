#ifndef _USBFS_H_
#define _USBFS_H_

#include <conf.h>
#include <device.h>
#include <semaphore.h>
#include <usb_util.h>

struct usb_xfer_request;
struct usb_device;

#define USBFS_IFLAG_NOBLOCK  0x0001  /**< do non-blocking input  */
#define USBFS_CTRL_SET_IFLAG 0x0010  /**< set input flags 	 */
#define USBFS_CTRL_CLR_IFLAG 0x0011  /**< clear input flags  	 */
#define USBFS_CTRL_GET_IFLAG 0x0012  /**< get input flags        */

/* Tracing macros */
#define ENABLE_TRACE_USBFS
#ifdef ENABLE_TRACE_USBFS
#   include <kernel.h>
#   include <thread.h>
#   define USBFS_TRACE(...)     { \
                kprintf("%s:%d (%d) ", __FILE__, __LINE__, gettid()); \
		kprintf(__VA_ARGS__); \
		kprintf("\r\n"); }
#else
#  define USBFS_TRACE(...)
#endif

#define USBFS_IBLEN 512
#define USBFS_OBLEN 512

devcall usbFsControl(device *devptr, int func, long arg1, long arg2);
devcall usbFsGetc(device *devptr);
devcall usbFsInit(device *devptr);
devcall usbFsPutc(device *devptr, char ch);
devcall usbFsWrite(device *devptr, void *buf, uint len);
//void    usbFsInterrupt(struct usb_xfer_request *req);
devcall usbFsRead(device *devptr, void *buf, uint len);

usb_status_t usbFsBindDevice(struct usb_device *dev);
void usbFsUnbindDevice(struct usb_device *dev);

struct usbfs {
    device *phw;

    bool initialized;
    bool attached;
    uchar iflags;
    semaphore isema;
    semaphore osema;
    uchar istart;
    uchar icount;
    uchar ostart;
    uchar ocount;
    uchar in[USBFS_IBLEN];
    uchar out[USBFS_OBLEN];
    struct usb_xfer_request *intr;

    uchar recent_usage_ids[6];
};

extern struct usbfs usbfstab[NUSBFS];


#endif
