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

filename : cmdline.h
project  : MPEG-4 Audio Lossless Coding
author   : Tilman Liebchen (Technical University of Berlin)
date     : June 23, 2003
contents : Header file for cmdline.cpp

*************************************************************************/

/*************************************************************************
 *
 * Modifications:
 *
 * 11/11/2003, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - added function GetOptionValues()
 *
 * 12/16/2003, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - added function GetOptionValueL()
 *
 * 03/25/2005, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - added function GetFOption().
 *
 * 06/08/2005, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - changed GetOptionValue() to long, removed GetOptionValueL(), 
 *
 *************************************************************************/

short CheckOption(short argc, char **argv, char *opt);
long GetOptionValue(short argc, char **argv, char *opt, long default_value = 0);
long GetOptionValues(short argc, char **argv, char *opt, long N, unsigned short *val);
bool GetFOption( short argc, char** argv, short& AcfMode, float& AcfValue, short& MlzMode );
