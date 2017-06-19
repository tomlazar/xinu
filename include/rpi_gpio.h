#ifndef _RPI_GPIO_H_
#define _RPI_GPIO_H_

struct rpi_gpio_regs
{
	volatile unsigned int gpfsel[6];	/* GPIO Funciton Select */
	volatile unsigned int do_not_use_a;
	volatile unsigned int gpset[2];		/* GPIO Pin Output Set */
	volatile unsigned int do_not_use_b;
	volatile unsigned int gpclr[2];		/* GPIO Pin Output Clear */
	volatile unsigned int do_not_use_c;
	volatile unsigned int gplev[2];		/* GPIO Pin Level */
	volatile unsigned int do_not_use_d;
	volatile unsigned int gpeds[2];		/* GPIO Pin Event Detect Status */
	volatile unsigned int do_not_use_e;
	volatile unsigned int gpren[2];		/* GPIO Pin Rising Edge Detect Enable */
	volatile unsigned int do_not_use_f;
	volatile unsigned int gpfen[2];		/* GPIO Pin Falling Edge Detect Enable */
	volatile unsigned int do_not_use_g;
	volatile unsigned int gphen[2];		/* GPIO Pin High Detect Enable */
	volatile unsigned int do_not_use_h;
	volatile unsigned int gplen[2];		/* GPIO Pin Low Detect Enable */
	volatile unsigned int do_not_use_i;
	volatile unsigned int gparen[2];	/* GPIO Pin Async. Rising Edge Detect */
	volatile unsigned int do_not_use_j;
	volatile unsigned int gpafen[2];	/* GPIO Pin Async. Falling Edge Detect */
	volatile unsigned int do_not_use_k;
	volatile unsigned int gppud;		/* GPIO Pin Pull-up/down Enable */
	volatile unsigned int gppudclk[2];	/* GPIO Pin Pull-up/down Enable Clock */
};

#endif /* _RPI_GPIO_H_ */
