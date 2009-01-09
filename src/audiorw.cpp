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

filename : audiorw.cpp
project  : MPEG-4 Audio Lossless Coding
author   : Tilman Liebchen (Technical University of Berlin)
date     : June 23, 2003
contents : Audio data I/O

*************************************************************************/

/*************************************************************************
 *
 * Modifications:
 *
 * 11/11/2003, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   -  added support for multichannel and different byte orders
 *   -  all new functions are of type ReadXBitNM(), WriteXBitNM()
 *
 * 03/17/2004, Koichi Sugiura <ksugiura@mitaka.ntt-at.co.jp>
 *   -  added ReadFloatNM function for floating-point PCM.
 *
 * 11/17/2004, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - removed float-to-integer conversion from ReadFloatNM().
 *
 * 11/22/2004, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - removed unnecessary parameters from ReadFloatNM().
 *
 * 05/19/2005, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - modified ReadFloatNM() to set values to float buffer
 *     in form of unsigned long.
 *
 * 05/23/2007, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - replaced FILE* with HALSSTREAM.
 *
 *************************************************************************/

// I/O of PCM audio data
// ---------------------
// - Supports the following resolutions: 8-bit (unsigned), 16-bit, 24-bit (3 bytes), 32-bit
// - Reads/writes multichannel data with an arbitrary number of channels (interleaved)
// - Internal data is stored using long variables
// Parameters fo all functions:
// - x is a two-dimensional array containing the audio samples for all channels
// - M is the number of channels
// - N is the number of samples to read/write (per channel)
// - msbfirst indicates the original byte order of the audio samples
// - b is an I/O buffer. Its minimum length is N*Channels*BytesPerSample
//   (e.g. N*4 for 16-bit stereo)
// - fp is the file pointer
// Notes:
// - 8-bit : [0;255] <-> [-128;127]
// - 24-bit: Packet format (3 bytes per sample)
//   The MSByte of the long variable is 0x00 for positive values (0x00000000...0x007FFFFF)
//   and 0xFF for negative values (0xFF800000...0xFFFFFFFF). It is removed before writing and
//   added after reading.

#include <stdio.h>
#include <math.h>
#include "floating.h"
#include "stream.h"

long Read8BitOffsetNM(int **x, long M, long N, unsigned char *b, HALSSTREAM fp)
{
	unsigned char *bt;
	long n, m;

	bt = b;

	fread(b, 1, N * M, fp);

	for (n = 0; n < N; n++)
	{
		for (m = 0; m < M; m++)
			x[m][n] = int(bt[m]) - 128;
		
		bt += M;
	}

	return(0);
}

long Write8BitOffsetNM(int **x, long M, long N, unsigned char *b, HALSSTREAM fp)
{
	unsigned char *bt;
	long n, m;

	bt = b;

	for (n = 0; n < N; n++)
	{
		for (m = 0; m < M; m++)
			bt[m] = (unsigned char)(x[m][n] + 128);

		bt += M;
	}

	if (fp != NULL)
		return(fwrite(b, 1, N * M, fp));
	else
		return(0);
}

long Read16BitNM(int **x, long M, long N, short msbfirst, unsigned char *b, HALSSTREAM fp)
{
	short c0, c1;
	long r, n, m;

	if (msbfirst)
	{
		c0 = 1;
		c1 = 0;
	}
	else
	{
		c0 = 0;
		c1 = 1;
	}

	r = fread(b, 1, 2 * N * M, fp);

	for (n = 0; n < N; n++)
	{
		for (m = 0; m < M; m++)
			x[m][n] = short(b[2*m+c0] | (static_cast<unsigned short>(b[2*m+c1]) << 8));

		b += 2 * M;
	}

	return(r);
}

long Write16BitNM(int **x, long M, long N, short msbfirst, unsigned char *b, HALSSTREAM fp)
{
	unsigned char *bt;
	short c0, c1;
	long n, m;

	bt = b;

	if (msbfirst)
	{
		c0 = 1;
		c1 = 0;
	}
	else
	{
		c0 = 0;
		c1 = 1;
	}

	for (n = 0; n < N; n++)
	{
		for (m = 0; m < M; m++)
		{
			bt[2*m+c0] = x[m][n] & 0xFF;
			bt[2*m+c1] = (x[m][n] >> 8) & 0xFF;
		}

		bt += 2 * M;
	}

	if (fp != NULL)
		return(fwrite(b, 1, 2 * N * M, fp));
	else
		return(0);

}

long Read24BitNM(int **x, long M, long N, short msbfirst, unsigned char *b, HALSSTREAM fp)
{
	short c0, c1, c2;
	long r, n, m;

	if (msbfirst)
	{
		c0 = 2;
		c1 = 1;
		c2 = 0;
	}
	else
	{
		c0 = 0;
		c1 = 1;
		c2 = 2;
	}

	r = fread(b, 1, 3 * N * M, fp);

	for (n = 0; n < N; n++)
	{
		for (m = 0; m < M; m++)
		{
			x[m][n] = b[3*m+c0] | (static_cast<unsigned int>(b[3*m+c1]) << 8) | (static_cast<unsigned int>(b[3*m+c2]) << 16);
			if (x[m][n] & 0x00800000)
				x[m][n] = (x[m][n] | 0xFF000000);
		}

		b += 3 * M;
	}

	return(r);
}

long Write24BitNM(int **x, long M, long N, short msbfirst, unsigned char *b, HALSSTREAM fp)
{
	unsigned char *bt;
	short c0, c1, c2;
	long n, m;

	bt = b;

	if (msbfirst)
	{
		c0 = 2;
		c1 = 1;
		c2 = 0;
	}
	else
	{
		c0 = 0;
		c1 = 1;
		c2 = 2;
	}

	for (n = 0; n < N; n++)
	{
		for (m = 0; m < M; m++)
		{
			bt[3*m+c0] = x[m][n] & 0xFF;
			bt[3*m+c1] = (x[m][n] >> 8) & 0xFF;
			bt[3*m+c2] = (x[m][n] >> 16) & 0xFF;
		}

		bt += 3 * M;
	}

	if (fp != NULL)
		return(fwrite(b, 1, 3 * N * M, fp));
	else
		return(0);
}

long Read32BitNM(int **x, long M, long N, short msbfirst, unsigned char *b, HALSSTREAM fp)
{
	short c0, c1, c2, c3;
	long r, n, m;

	if (msbfirst)
	{
		c0 = 3;
		c1 = 2;
		c2 = 1;
		c3 = 0;
	}
	else
	{
		c0 = 0;
		c1 = 1;
		c2 = 2;
		c3 = 3;
	}

	r = fread(b, 1, 4 * N * M, fp);

	for (n = 0; n < N; n++)
	{
		for (m = 0; m < M; m++)
			x[m][n] = b[4*m+c0] | (static_cast<unsigned int>(b[4*m+c1]) << 8) | (static_cast<unsigned int>(b[4*m+c2]) << 16) | (static_cast<unsigned int>(b[4*m+c3]) << 24);

		b += 4 * M;
	}

	return(r);
}

long Write32BitNM(int **x, long M, long N, short msbfirst, unsigned char *b, HALSSTREAM fp)
{
	unsigned char *bt;
	short c0, c1, c2, c3;
	long n, m;

	bt = b;

	if (msbfirst)
	{
		c0 = 3;
		c1 = 2;
		c2 = 1;
		c3 = 0;
	}
	else
	{
		c0 = 0;
		c1 = 1;
		c2 = 2;
		c3 = 3;
	}

	for (n = 0; n < N; n++)
	{
		for (m = 0; m < M; m++)
		{
			bt[4*m+c0] = x[m][n] & 0xFF;
			bt[4*m+c1] = (x[m][n] >> 8) & 0xFF;
			bt[4*m+c2] = (x[m][n] >> 16) & 0xFF;
			bt[4*m+c3] = (x[m][n] >> 24) & 0xFF;
		}

		bt += 4 * M;
	}

	if (fp != NULL)
		return(fwrite(b, 1, 4 * N * M, fp));
	else
		return(0);
}

long	ReadFloatNM( int** ppLongBuf, long M, long N, short msbfirst, unsigned char* b, HALSSTREAM fp, float** ppFloatBuf )
{
	long	iSample, iChannel;
	unsigned long	ul;
	short	c0, c1, c2, c3;

	if ( msbfirst ) {
		c0 = 3;
		c1 = 2;
		c2 = 1;
		c3 = 0;
	} else {
		c0 = 0;
		c1 = 1;
		c2 = 2;
		c3 = 3;
	}

	// Read samples from pStream
	if ( fread( b, 1, M * N * sizeof(float), fp ) != M * N * sizeof(float) ) {
		// Read error
		return 0;
	}

	// Convert raw data to long/float array
	for( iSample=0; iSample<N; iSample++ ) {
		for( iChannel=0; iChannel<M; iChannel++ ) {
			ul = b[4*iChannel+c0] | ( static_cast<unsigned int>(b[4*iChannel+c1]) << 8 ) | ( static_cast<unsigned int>(b[4*iChannel+c2]) << 16 ) | ( static_cast<unsigned int>(b[4*iChannel+c3]) << 24 );
			// ul shold be copied as unsigned int. (it may lost some bits when copied as float.)
			*reinterpret_cast<unsigned int*>( &ppFloatBuf[iChannel][iSample] ) = ul;
		}
		b += sizeof(float) * M;
	}
	return 0;
}
