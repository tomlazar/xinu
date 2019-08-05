/**
 * @file bcm2837_power.c
 *
 * This driver provides the ability to power on and power off hardware, such as
 * the USB Controller, on the BCM2837B0 SoC used on the Raspberry Pi 3 B+.  This makes
 * use of the BCM2837B0's mailbox mechanism.
 */
#include "bcm2837.h"
#include "bcm2837_mbox.h"
#include "mmu.h"

/**
 * @ingroup bcm2837
 *
 * Write to the specified channel of the mailbox.  
 * @param channel	Mailbox channel to write to
 * @param value		Value to write to the mailbox
 */
void
bcm2837_mailbox_write(uint channel, uint value)
{
    while (mailbox_regs[MAILBOX_STATUS] & MAILBOX_FULL)
    {
    }
    mailbox_regs[MAILBOX_WRITE] = (value & ~MAILBOX_CHANNEL_MASK) | channel;
}

/**
 * @ingroup bcm2837
 *
 * Read from the specified channel of the mailbox.  
 * @param channel	Mailbox channel to read from
 * @return		Value read from the mailbox
 */
uint
bcm2837_mailbox_read(uint channel)
{
    uint value;

    while (mailbox_regs[MAILBOX_STATUS] & MAILBOX_EMPTY)
    {
    }
    do
    {
        value = mailbox_regs[MAILBOX_READ];
    } while ((value & MAILBOX_CHANNEL_MASK) != channel);
    return (value & ~MAILBOX_CHANNEL_MASK);
}

/**
 * @ingroup bcm2837
 *
 * Retrieve the bitmask of power on/off state.  
 * @return Value held by the BCM2837's mailbox power management channel
 */
static uint bcm2837_get_power_mask(void)
{
    return (bcm2837_mailbox_read(MAILBOX_CHANNEL_POWER_MGMT) >> 4);
}

/**
 * @ingroup bcm2837
 *
 * Set the bitmask of power on/off state.  
 * @param mask	Bitmask to write to the channel
 */
static void
bcm2837_set_power_mask(uint mask)
{
    bcm2837_mailbox_write(MAILBOX_CHANNEL_POWER_MGMT, mask << 4);
}

/**
 * @ingroup bcm2837
 *
 * Bitmask that gives the current on/off state of the BCM2837 hardware.
 * This is a cached value.  
 */
static uint bcm2837_power_mask;

/**
 * @ingroup bcm2837
 *
 * Powers on or powers off BCM2837 hardware.
 *
 * @param feature	Device or hardware to power on or off.
 * @param on	::TRUE to power on; ::FALSE to power off.
 *
 * @return ::OK if successful; ::SYSERR otherwise.
 */
int bcm2837_setpower(enum board_power_feature feature, bool on)
{
    uint bit;
    uint newmask;
    bool is_on;

    bit = 1 << feature;
    is_on = (bcm2837_power_mask & bit) != 0;
    if (on != is_on)
    {
        newmask = bcm2837_power_mask ^ bit;
        bcm2837_set_power_mask(newmask);
        bcm2837_power_mask = bcm2837_get_power_mask();
        if (bcm2837_power_mask != newmask)
        {
            return SYSERR;
        }
    }
    return OK;
}

/**
 * @ingroup bcm2837
 *
 * Resets BCM2837 power to default state (all devices powered off).
 */
void bcm2837_power_init(void)
{
    bcm2837_set_power_mask(0);
    bcm2837_power_mask = 0;
}
