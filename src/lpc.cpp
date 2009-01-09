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

filename : lpc.cpp
project  : MPEG-4 Audio Lossless Coding
author   : Tilman Liebchen (Technical University of Berlin)
date     : June 25, 2003
contents : Linear prediction and related functions

*************************************************************************/

/*************************************************************************
 *
 * Modifications:
 *
 * 03/24/2004, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - added GetResidualRA() and GetSignalRA()
 *
 * 07/29/2004, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - added support for higher orders (up to 255)
 *
 * 02/06/2005, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - added support for higher orders (<= 1023) and block lengths (<= 64k)
 *   - changed some variables from short to long (e.g. block length N)
 *
 * 07/04/2005, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - fixed bug in BlockIsZero()
 *
 *************************************************************************/

#if defined(WIN32) || defined(WIN64)
	typedef __int64 INT64;
#else
	#include <stdint.h>
	typedef int64_t INT64;
#endif

#include <math.h>
#include <memory.h>
#include <limits.h>

#define MIN(a, b)  (((a) < (b)) ? (a) : (b)) 
#define PI 3.14159265359
#define LN2 0.69314718056

// Autocorrelation function (ACF)
// x	: Samples
// N	: Number of samples
// k	: Calculate ACF up to rxx[k] (k+1 values)
// norm	: Normalization
// rxx	: ACF values
void acf(double *x, long N, long k, short norm, double *rxx)
{
	long i, n;

	for (i = 0; i <= k; i++)
	{
		rxx[i] = 0.0;
		for (n = i; n < N; n++)
			rxx[i] += x[n] * x[n-i];
	}

	if (norm)
	{
		for (i = 1; i <= k; i++)
			rxx[i] /= rxx[0];
		rxx[0] = 1.0;
	}
}

// Hanning window
void hanning(int *x, double *xd, long N)
{
	long n;

	for (n = 0; n < N; n++)
		xd[n] = (double)x[n] * (0.5 - 0.5 * cos(2.0*PI*n/(N-1)));
}

// Hamming window
void hamming(int *x, double *xd, long N)
{
	long n;

	for (n = 0; n < N; n++)
		xd[n] = (double)x[n] * (0.54 - 0.46 * cos(2.0*PI*n/(N-1)));
}

// Rect window
void rect(int *x, double *xd, long N)
{
	long n;

	for (n = 0; n < N; n++)
		xd[n] = (double)x[n];
}

// Blackman window
void blackman(int *x, double *xd, long N)
{
	long n;

	for (n = 0; n < N; n++)
		xd[n] = (double)x[n] * (0.42 - 0.5 * cos(2.0*PI*n/(N-1)) + 0.08 * cos(4.0*PI*n/(N-1)));
}

// Levinson-Durbin algorithm
// -> ord: Predictor order
// -> rxx: ACF values (rxx[0...ord])
// <- par: Parcor coefficients (par[0...ord-1])
short durbin(short ord, double *rxx, double *par)
{
	short i, j;
	double evar, temp, dir[1024];

	par--;

	evar = rxx[0];

	for (i = 1; i <= ord; i++)
	{
		par[i] = -rxx[i];
		for (j = 1; j < i; j++)
			par[i] -= dir[j] * rxx[i-j];
		par[i] /= evar;
		dir[i] = par[i];
		for (j = 1; j <= i/2; j++)
		{
			temp = dir[j] + par[i] * dir[i-j];
			dir[i-j] = dir[i-j] + par[i] * dir[j];
			dir[j] = temp;
		}
		evar *= (1.0 - par[i] * par[i]);
	}

	return(0);
}

// Calculate LPC coefficients for a block of samples
// -> x		: Samples
// -> N		: Number of samples
// -> P		: Predictor order
// -> win	: Window type
// <- par	: Parcor coefficients
short GetCof(int *x, long N, short P, short win, double *par)
{
	double *xd = new double[N];
	double *rxx = new double[P+1];

	// Windowing
	if (win == 1)
		hamming(x, xd, N);
	else if (win == 2)
		rect(x, xd, N);
	else if (win == 3)
		blackman(x, xd, N);
	else
		hanning(x, xd, N);

	// Calculate ACF
	acf(xd, N, P, 0, rxx);

	// Calculate LPC coefficients
	durbin(P, rxx, par);

	delete [] xd;
	delete [] rxx;

	return(0);
}

// Conversion from parcor to direct form coefficients
short par2cof(int *cof, int *par, short ord, short Q)
{
	short m, i;
	int korr;
	INT64 temp, temp2;

	korr = 1 << (Q - 1);

	par--;
	cof--;

	for (m = 1; m <= ord; m++)
	{
		for (i = 1; i <= m/2; i++)
		{
			temp = cof[i] + ((((INT64)par[m] * cof[m-i]) + korr) >> Q);
			if ((temp > INT_MAX) || (temp < INT_MIN))	// Overflow
				return(1);
			temp2 = cof[m-i] + ((((INT64)par[m] * cof[i]) + korr) >> Q);
			if ((temp2 > INT_MAX) || (temp2 < INT_MIN))	// Overflow
				return(1);
			cof[m-i] = (int)temp2;
			cof[i] = (int)temp;
		}
		cof[m] = par[m];
	}

	return(0);
}

// Caculate prediction residual
void GetResidual(int *x, long N, short P, short Q, int *cof, int *d)
{
	long n, i;
	int korr;
	INT64 y;

	korr = 1 << (Q - 1);	// Korrekturterm

	for (n = 0; n < N; n++)
	{
		// Initialisierung mit Korrekturterm
		y = korr;

		// Schätzwert berechnen
		for (i = 1; i <= P; i++)
			y += (INT64)cof[i-1] * x[n-i];

		// Schätzwert vom Signal abziehen
		d[n] = x[n] + (int)(y >> Q);				// Division y / 2^Q
	}
}

// Calculate original signal
void GetSignal(int *x, long N, short P, short Q, int *cof, int *d)
{
	long n, i;
	int korr;
	INT64 y;

	korr = 1 << (Q - 1);	// Korrekturterm

	for (n = 0; n < N; n++)
	{
		y = korr;

		// Schätzwert berechnen
		for (i = 1; i <= P; i++)
			y += (INT64)cof[i-1] * x[n-i];

		// Schätzwert zum Restfehlersignal addieren
		x[n] = d[n] - (int)(y >> Q);
	}
}

// Calculate prediction residual for random access block (internal par -> cof conversion)
short GetResidualRA(int *x, long N, short P, short Q, int *par, int *cof, int *d)
{
	long n, i, m;
	int korr;
	INT64 y, temp, temp2;

	if(N < P) P = (short)N;

	korr = 1 << (Q - 1);	// Correction term

	par--;
	cof--;

	// Progressive prediction for first P values
	for (n = 0; n < P; n++)
	{
		// Initialisation with correction term
		y = korr;

		// Calculate estimate
		for (i = 1; i <= n; i++)
			y += (INT64)cof[i] * x[n-i];			// cof[i] because of cof-- (see above)

		// Subtract estimate from signal
		d[n] = x[n] + (int)(y >> Q);				// Division y / 2^Q

		m = n + 1;	// Order

		// Calculate direct form m-th order predictor coefficients
		for (i = 1; i <= m/2; i++)
		{
			temp = cof[i] + ((((INT64)par[m] * cof[m-i]) + korr) >> Q);
			if ((temp > INT_MAX) || (temp < INT_MIN))	// Overflow
				return(1);
			temp2 = cof[m-i] + ((((INT64)par[m] * cof[i]) + korr) >> Q);
			if ((temp2 > INT_MAX) || (temp2 < INT_MIN))	// Overflow
				return(1);
			cof[m-i] = (int)temp2;
			cof[i] = (int)temp;
		}
		cof[m] = par[m];
	}

	for (n = P; n < N; n++)
	{
		// Initialisation with correction term
		y = korr;

		// Calculate estimate
		for (i = 1; i <= P; i++)
			y += (INT64)cof[i] * x[n-i];

		// Subtract estimate from signal
		d[n] = x[n] + (int)(y >> Q);				// Division y / 2^Q
	}

	return(0);
}

// Calculate original signal for random access block (internal par -> cof conversion)
short GetSignalRA(int *x, long N, short P, short Q, int *par, int *cof, int *d)
{
	long n, i, m;
	int korr;
	INT64 y, temp, temp2;

	if(N < P) P = (short)N;

	korr = 1 << (Q - 1);	// Correction term

	par--;
	cof--;

	// Progressive prediction for first P values
	for (n = 0; n < P; n++)
	{
		// Initialisation with correction term
		y = korr;

		// Calculate estimate
		for (i = 1; i <= n; i++)
			y += (INT64)cof[i] * x[n-i];			// cof[i] because of cof-- (see above)

		// Add estimate to residual
		x[n] = d[n] - (int)(y >> Q);				// Division y / 2^Q

		m = n + 1;	// Order

		// Calculate direct form m-th order predictor coefficients
		for (i = 1; i <= m/2; i++)
		{
			temp = cof[i] + ((((INT64)par[m] * cof[m-i]) + korr) >> Q);
			if ((temp > INT_MAX) || (temp < INT_MIN))	// Overflow
				return(1);
			temp2 = cof[m-i] + ((((INT64)par[m] * cof[i]) + korr) >> Q);
			if ((temp2 > INT_MAX) || (temp2 < INT_MIN))	// Overflow
				return(1);
			cof[m-i] = (int)temp2;
			cof[i] = (int)temp;
		}
		cof[m] = par[m];
	}

	for (n = P; n < N; n++)
	{
		// Initialisation with correction term
		y = korr;

		// Calculate estimate
		for (i = 1; i <= P; i++)
			y += (INT64)cof[i] * x[n-i];

		// Add estimate to residual
		x[n] = d[n] - (int)(y >> Q);				// Division y / 2^Q
	}

	return(0);
}

// Check if all samples are zero
short BlockIsZero(int *x, long N)
{
	short z;
	long n;

	z = 0;
	if (*x == 0)
	{
		z = 1;
		n = 1;
		x++;
		while (z && (n < N))
		{
			if (*x++)		
				z = 0;
			n++;
		}
	}
	return(z);
}

// Check if als samples have the same value
int BlockIsConstant(int *x, long N)
{
	int *x1, c, n;

	if (N == 1)
		return(x[0]);
	else
	{
		x1 = x + 1;
		c = *x;
		n = 0;
		do
		{
			if (*x != *x1)
				c = 0;
			x = x1++;
			n++;
		}
		while (c && (n < (N-1)));
	}
	
	return(c);
}

// Remove LSBs if they are zero
short ShiftOutEmptyLSBs(int *x, long N)
{
	short empty = 1, shift;
	int i, temp = 0;
		
	// Repeat OR operation while (at least the last) resulting LSB is empty
	for (i = 0; (i < N) & empty; i++)
	{
		temp |= x[i];
		empty = ((temp & 0x01) == 0);
	}

	// Get number of empty LSBs
	for (shift = 0; (temp & 0x01) == 0; shift++)
		temp >>= 1;

	if (shift > 16)
		shift = 16;

	// Shift out empty LSBs
	for (i = 0; i < N; i++)
		x[i] >>= shift;

	// Return number of shifted LSBs
	return(shift);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// ROUTINES WITHOUT CHECK

// Conversion from parcor to direct form coefficients (no check for overflow)
short par2cof_nocheck(int *cof, int *par, short ord, short Q)
{
	short m, i;
	int korr;
	INT64 temp, temp2;

	korr = 1 << (Q - 1);

	par--;
	cof--;

	for (m = 1; m <= ord; m++)
	{
		for (i = 1; i <= m>>1; i++)
		{
			temp = cof[i] + ((((INT64)par[m] * cof[m-i]) + korr) >> Q);
			temp2 = cof[m-i] + ((((INT64)par[m] * cof[i]) + korr) >> Q);
			cof[m-i] = (int)temp2;
			cof[i] = (int)temp;
		}
		cof[m] = par[m];
	}

	return(0);
}

// Calculate original signal for random access block (internal par -> cof conversion)
short GetSignalRA_nocheck(int *x, long N, short P, short Q, int *par, int *cof, int *d)
{
	long n, i, m;
	int korr;
	INT64 y, temp, temp2;

	korr = 1 << (Q - 1);	// Correction term

	par--;
	cof--;

	// Progressive prediction for first P values
	for (n = 0; n < P; n++)
	{
		// Initialisation with correction term
		y = korr;

		// Calculate estimate
		for (i = 1; i <= n; i++)
			y += (INT64)cof[i] * x[n-i];			// cof[i] because of cof-- (see above)

		// Add estimate to residual
		x[n] = d[n] - (int)(y >> Q);				// Division y / 2^Q

		m = n + 1;	// Order

		// Calculate direct form m-th order predictor coefficients
		for (i = 1; i <= m/2; i++)
		{
			temp = cof[i] + ((((INT64)par[m] * cof[m-i]) + korr) >> Q);
			temp2 = cof[m-i] + ((((INT64)par[m] * cof[i]) + korr) >> Q);
			cof[m-i] = (int)temp2;
			cof[i] = (int)temp;
		}
		cof[m] = par[m];
	}

	for (n = P; n < N; n++)
	{
		// Initialisation with correction term
		y = korr;

		// Calculate estimate
		for (i = 1; i <= P; i++)
			y += (INT64)cof[i] * x[n-i];

		// Add estimate to residual
		x[n] = d[n] - (int)(y >> Q);				// Division y / 2^Q
	}

	return(0);
}

