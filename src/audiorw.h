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

filename : audiorw.h
project  : MPEG-4 Audio Lossless Coding
author   : Tilman Liebchen (Technical University of Berlin)
date     : June 23, 2003
contents : Header file for audiorw.cpp

*************************************************************************/

/*************************************************************************
 *
 * Modifications:
 *
 * 11/11/2003, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - changed function prototypes according to changes in audiorw.cpp
 *     (see audiorw.cpp for a complete list of modifications)
 *
 * 03/17/2004, Koichi Sugiura <ksugiura@mitaka.ntt-at.co.jp>
 *   -  added ReadFloatNM function for floating-point PCM.
 *
 * 11/22/2004, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - removed unnecessary parameters from ReadFloatNM().
 *
 *************************************************************************/

long Read8BitOffsetNM(long **x, long M, long N, unsigned char *b, FILE *fp);
long Write8BitOffsetNM(long **x, long M, long N, unsigned char *b, FILE *fp);
long Read16BitNM(long **x, long M, long N, short msbfirst, unsigned char *b, FILE *fp);
long Write16BitNM(long **x, long M, long N, short msbfirst, unsigned char *b, FILE *fp);
long Read24BitNM(long **x, long M, long N, short msbfirst, unsigned char *b, FILE *fp);
long Write24BitNM(long **x, long M, long N, short msbfirst, unsigned char *b, FILE *fp);
long Read32BitNM(long **x, long M, long N, short msbfirst, unsigned char *b, FILE *fp);
long Write32BitNM(long **x, long M, long N, short msbfirst, unsigned char *b, FILE *fp);
long ReadFloatNM( long** ppLongBuf, long M, long N, short msbfirst, unsigned char* b, FILE* fp, float** ppFloatBuf );
