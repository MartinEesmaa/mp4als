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

filename : crc.cpp
project  : MPEG-4 Audio Lossless Coding
author   : Tilman Liebchen (Technical University of Berlin)
date     : June 16, 2003
contents : 32-bit CRC (based on Nelson/Gailly "The Data Compression Book")

*************************************************************************/

#include "crc.h"

// Table used to calculate the CRC values
unsigned int Ccitt32Table[256];

// Build the table
void BuildCRCTable()
{
	short i, j;
	unsigned int value;

	for (i = 0; i <= 255; i++)
	{
		value = i;
		for (j = 8; j > 0; j--)
		{
			if (value & 1)
				value = (value >> 1) ^ CRC32_POLYNOMIAL;
			else
				value >>= 1;
		}
		Ccitt32Table[i] = value;
    }
}

// Calculate the CRC of a block of data
unsigned int CalculateBlockCRC32(unsigned int count, unsigned int crc, void *buffer)
{
	unsigned char *p = (unsigned char *) buffer;
	unsigned int temp1, temp2;

	while (count-- != 0)
	{
		temp1 = (crc >> 8) & 0x00FFFFFFL;
		temp2 = Ccitt32Table[((int)crc ^ *p++) & 0xff];
		crc = temp1 ^ temp2;
    }
    return(crc);
}
