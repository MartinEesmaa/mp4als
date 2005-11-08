/***************** MPEG-4 Audio Lossless Coding **************************

This software module was originally developed by

Tilman Liebchen (Technical University of Berlin)

in the course of development of the MPEG-4 Audio standard ISO/IEC 14496-3
and associated amendments. This software module is an implementation of
a part of one or more MPEG-4 Audio lossless coding tools as specified
by the MPEG-4 Audio standard. ISO/IEC gives users of the MPEG-4 Audio
standards free license to this software module or modifications
thereof for use in hardware or software products claiming conformance
to the MPEG-4 Audio standards. Those intending to use this software
module in hardware or software products are advised that this use may
infringe existing patents. The original developer of this software
module, the subsequent editors and their companies, and ISO/IEC have
no liability for use of this software module or modifications thereof
in an implementation. Copyright is not released for non MPEG-4 Audio
conforming products. The original developer retains full right to use
the code for the developer's own purpose, assign or donate the code to
a third party and to inhibit third party from using the code for non
MPEG-4 Audio conforming products. This copyright notice must be included
in all copies or derivative works.

Copyright (c) 2003.

filename : ec.cpp
project  : MPEG-4 Audio Lossless Coding
author   : Tilman Liebchen (Technical University of Berlin)
date     : June 23, 2003
contents : Estimation of entropy codes

*************************************************************************/

/*************************************************************************
 *
 * Modifications:
 *
 * 8/31/2003, Yuriy A. Reznik <yreznik@real.com>
 *   - added parameter sx in GetRicePara()
 *
 * 12/14/2003, Koichi Sugiura <ksugiura@mitaka.ntt-at.co.jp>
 *   - added #include <cstdlib>.
 *
 *************************************************************************/

#include <math.h>
#include <cstdlib>

#define LN2 0.69314718055994529

// Calculate code parameter for Rice coding (returns s >= 0)
short GetRicePara(long *x, long N, short *sx)
{
	short s;
	long n;
	double mean;

	mean = 0.0;
	for (n = 0; n < N; n++)
		mean += labs(x[n]);
	mean /= N;

#if 0
    /* Tilman's code: */
	if (mean < 1.02)	// mean < 1.02 leads to s < 1 (according to the formula below)
		s = 0;
	else
		s = (short)floor(log(1.386*mean)/LN2 + 0.5);
#else
    /**********
     * 8/31/2003 3:30PM, Yuriy A. Reznik <yreznik@real.com>
     ***/

    /* get a high-precision version first: */
    if (mean <= .5288048725)
        s = 0;
    else
        s = (short) floor (log(1.386*mean)/LN2 * 16. + 8.);
    /* store LSBs and quantize s: */
    *sx = s & 0x0F;
    s >>= 4;
#endif
	return(s);
}

// Calculate number of bits for Rice coding of one block x[0...N-1]
// The code parameter must be s >= 0 
long GetRiceBits(long *x, long N, short s)
{
	long n, tmp, pre, sum = 0;

	s--;	// Makes calculation more convenient
	if (s == -1)	// Special case: Unary code, alternating for positive and negative numbers
	{
		for (n = 0; n < N; n++)
			sum += (2 * labs(x[n]) + (x[n] >= 0));		// Positive numbers need 1 bit more
	}
	else
	{
		for (n = 0; n < N; n++)
		{
			tmp = x[n];
			if (tmp < 0L)
				tmp = ~tmp;				// tmp = -tmp - 1, mapping [-2^31...-1] to [0...2^31-1]
			pre = (tmp >> s) + 1;		// This is equivalent to (tmp / (2^s)) + 1
			sum += (pre + s + 1);		// +1 for sign
		}
	}
	return(sum);
}
