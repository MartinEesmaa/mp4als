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

filename : lpc.h
project  : MPEG-4 Audio Lossless Coding
author   : Tilman Liebchen (Technical University of Berlin)
date     : June 25, 2003
contents : Header file for lpc.cpp

*************************************************************************/

/*************************************************************************
 *
 * Modifications:
 *
 * 03/24/2004, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - added GetResidualRA() and GetSignalRA()
 *
 * 02/06/2005, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - changed some parameters from short to long (e.g. block length N)
 *
 *************************************************************************/

void acf(double *x, long N, long k, short norm, double *rxx);
void hamming(long *x, double *xd, long N);
void hanning(long *x, double *xd, long N);
void blackman(long *x, double *xd, long N);
void rect(long *x, double *xd, long N);
short durbin(short ord, double *rxx, double *par);
short par2cof(long *cof, long *par, short ord, short Q);

short GetCof(long *x, long N, short P, short win, double *par);
void GetResidual(long *x, long N, short P, short Q, long *cof, long *d);
void GetSignal(long *x, long N, short P, short Q, long *cof, long *d);
short GetResidualRA(long *x, long N, short P, short Q, long *par, long *cof, long *d);
short GetSignalRA(long *x, long N, short P, short Q, long *par, long *cof, long *d);

short BlockIsZero(long *x, long N);
long BlockIsConstant(long *x, long N);
short ShiftOutEmptyLSBs(long *x, long N);
