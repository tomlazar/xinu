/**
 * @file trig.c
 *
 * Provides sine and cosine for Taylor series.
 * Cannot handle values above 215 degrees--will cause overflow.
 * To correct this, only compute values between 0 and 90.
 * Answers are not exactly accurate, but accurate enough for our purposes.
 */
/* Embedded Xinu, Copyright (C) 2013.  All rights reserved. */

#include <stddef.h>
#include <framebuffer.h>

/**
 * @ingroup framebuffer
 *
 * Compute the double-precision result of a power operation
 * @param base	Double-precision base value of operation
 * @param exp	Integer exponent to which the base is raised
 * @return Double-precision result of the power operation
 */
double power(double base, int exp)
{
    double result = 1.0;
    int i;
    for (i = 0; i < exp; i++)
    {
        result *= base;
    }
    return result;
}

/**
 * @ingroup framebuffer
 *
 * Compute the factorial of an integer
 * @param num	Integer to compute factorial operation on
 * @return	Long integer result of the factorial operation
 */
long factorial(int num)
{
    if (num < 2)
        return 1;
    return num*factorial(num-1);
}

/**
 * @ingroup framebuffer
 *
 * Compute the Taylor series approximation of cosine using power and factorial
 * @oaram x	Base value
 * @param terms	Terms of the sum
 * @return Double-precision result of the cosine approximation
 */
double cosine_taylor(double x, int terms)
{
    int i;
    double result = 0.0;
    for (i = 0; i < terms; i++)
    {
        result += power(-1.0,i)*power(x,i*2)/factorial(i*2);
    }
    return result;
}

/**
 * @ingroup framebuffer
 *
 * Compute the cosine using the Taylor approximation as a helper function
 * @param x	Angle, in degrees
 * @return	Double-precision result of the computation
 */
double cos(int x)
{
    double radx;
    while (x < 0) x += 360;
    x = x % 360;
    //handle basic angles.
    if (x == 0) return 1;
    else if ((x % 180) == 0) return -1;
    else if ((x % 90) == 0) return 0;
    //quadrant 1
    else if ( ((x > -360) && (x < -270)) || ((x > 0) && (x < 90)) ) {
        radx = PI * x / 180;
        return cosine_taylor(radx, 10);
    }
    //quadrant 2
    else if ( ((x > -270) && (x < -180)) || ((x > 90) && (x < 180)) ) {
        radx = PI * (180 - x) / 180;
        return -1 * cosine_taylor(radx, 10);
    } //quadrant 3
    else if ( ((x > -180) && (x < -90)) || ((x > 180) && (x < 270)) ) {
        radx = PI * (x - 180) / 180;
        return -1 * cosine_taylor(radx, 10);
    } //quadrant 4
    else {
        radx = PI * (360 - x) / 180;
        return cosine_taylor(radx, 10);
    }
}

/**
 * @ingroup framebuffer
 *
 * Compute the Taylor series approximation of sine
 * @param x	Base value
 * @param terms	Terms of the sum
 * @return Double-precision result of the sine approximation
 */
double sine_taylor(double x, int terms)
{
    int i;
    double result = 0.0;
    for (i = 0; i < terms; i++)
    {
        result += power(-1,i)*power(x,i*2+1)/factorial(i*2+1);
    }
    return result;
}

/**
 * @ingroup framebuffers
 *
 * Compute the sine using the Taylor approximation as a helper function
 * Note: For graphics purposes, the sine must always be turned negative because
 * a monitor is technically in quadrant 4 instead of quadrant 1. 
 * @param x	Angle, in degrees
 * @return Double-precision result of the computation
 */
double sin(int x)
{
    double radx;
    while (x < 0) x += 360;
    x = x % 360;
    //handle basic angles.
    if ((x % 180) == 0) return 0;
    else if ((x == 90) || (x == -270)) return -1;
    else if ((x == -90) || (x == 270)) return 1;
    //quadrant 1
    else if ( ((x > -360) && (x < -270)) || ((x > 0) && (x < 90)) ) {
        radx = PI * x / 180;
        return -1 * sine_taylor(radx, 10);
    } //quadrant 2
    else if ( ((x > -270) && (x < -180)) || ((x > 90) && (x < 180)) ) {
        radx = PI * (180 - x) / 180;
        return -1 * sine_taylor(radx, 10);
    } //quadrant 3
    else if ( ((x > -180) && (x < -90)) || ((x > 180) && (x < 270)) ) {
        radx = PI * (x - 180) / 180;
        return sine_taylor(radx, 10);
    } //quadrant 4
    else {
        radx = PI * (360 - x) / 180;
        return sine_taylor(radx, 10);
    }
}
