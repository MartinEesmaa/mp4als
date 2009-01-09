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

filename : bitio.h
project  : MPEG-4 Audio Lossless Coding
author   : Tilman Liebchen (Technical University of Berlin)
date     : June 16, 2003
contents : Header file for bitio.cpp

*************************************************************************/

/*************************************************************************
 *
 * Modifications:
 *
 * 11/11/2003, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - added functions WriteBits() and ReadBits()
 *
 * 03/17/2004, Koichi Sugiura <ksugiura@mitaka.ntt-at.co.jp>
 *   - added BITIO_INCLUDED symbol to avoid multiple inclusion
 *
 *************************************************************************/

#ifndef	BITIO_INCLUDED
#define	BITIO_INCLUDED

#if 0

class CBitIO
{
protected:
	unsigned char *pb, mask;
	long total;
public:
	void InitBitWrite(unsigned char *buffer);
	long EndBitWrite();
	long WriteByteAlign(unsigned char wert);
	long WriteBits(unsigned int value, short bits);
	long WriteRice(int *wert, char s, long N);

	void InitBitRead(unsigned char *buffer);
	long EndBitRead();
	long ReadBits(unsigned int *value, short bits);
	long ReadRice(long *wert, char s, long N);
};

#else

/*
 * rn_bitio.h/rn_bitio.c already provides generic bit-level IO routines
 * allowing much simpler and compact implementation of Golomb-Rice codes,
 * so we use this library. Function-wise it is absolutely identical to
 * Tilman's original implementation.
 */
#include "rn_bitio.h"

/* C++ wrapper: */
struct CBitIO
{
    BITIO bio;

    void InitBitWrite(unsigned char *buffer)		{bitio_init (buffer, 1, &bio);}
    long EndBitWrite()								{return bitio_term (&bio);}
    long WriteByteAlign(unsigned char wert)			{put_bits (wert, 8, &bio); return 8;}
    long WriteRice(int *wert, char s, long N)		{return rice_encode_block (wert, s, N, &bio);}
	long WriteBits(unsigned int value, short bits)	{put_bits (value, bits, &bio); return bits;}

    void InitBitRead(unsigned char *buffer)			{bitio_init (buffer, 0, &bio);}
    long EndBitRead()								{return bitio_term (&bio);}
    long ReadRice(int *wert, char s, long N)		{return rice_decode_block (wert, s, N, &bio);}
	long ReadBits(unsigned int *value, short bits)	{*value = get_bits(bits, &bio); return bits;}
};

#endif

#endif	// BITIO_INCLUDED
