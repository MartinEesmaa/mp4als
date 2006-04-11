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

filename : decoder.h
project  : MPEG-4 Audio Lossless Coding
author   : Tilman Liebchen (Technical University of Berlin)
date     : June 23, 2003
contents : Header file for decoder.cpp

*************************************************************************/

/*************************************************************************
 * Modifications:
 *
 * 11/11/2003, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   added support for multichannel, 32-bit, and new file format:
 *     - substituted WaveFileSize by FileSize, WaveHeaderSize by HeaderSize
 *     - added new variables FileType, MSBfirst, TrailerSize, CPE, SCE,
 *       Unimode, ChanSort, ChPos[]
 *     - substituted DecodeHeader() by AnalyseInputFile() and WriteHeader()
 *     - substituted DecodeRemains() by WriteTrailer()
 *     - substituted DecodeFile() by DecodeAll()
 *
 * 03/17/2004, Koichi Sugiura <ksugiura@mitaka.ntt-at.co.jp>
 *   - supported floating-point PCM.
 *
 * 07/29/2004, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - added variable: CoefTable
 *
 * 11/05/2004, Yutaka Kamamoto <kamamoto@hil.t.u-tokyo.ac.jp>
 *   - added MCC, NeedPuchBit and MccBuf member variable to CLpacDecoder class
 *   - divided DecodeBlock() into DecodeBlockParameter()
 *                            and DecodeBlockReconstruct()
 *
 * 11/17/2004, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - replaced FLOAT_BUF structure with CFloat object.
 *
 * 02/06/2005, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - removed variables CPE, SCE, InsertSwitchBit, N1
 *   - added parameters in DecodeBlock*(..., long Nb, short ra)
 *   - changed type of N, N0 from short to long
 *
 * 03/03/2005, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - integration of CE10 and CE11
 *
 * 04/27/2005, Yutaka Kamamoto <kamamoto@theory.brl.ntt.co.jp>
 *   - changed type of Chan, Channel from short to long
 *
 * 06/08/2005, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - syntax changes according to N7134
 *   - added RAflag, ChanConfig, CRCenabled, RAUnits, RAUid, RAUsize, CRCorg
 *   - removed Version
 *	 - changed type of ChPos to unsigned short*
 *
 * 06/08/2005, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - added AUXenabled
 *
 ************************************************************************/


#include <stdio.h>

#include "wave.h"
#include "floating.h"
#include "mcc.h"
#include "lms.h"

class CLpacDecoder
{
protected:
	long N;					// Frame length
	short P;				// (Max.) Predictor order
	short Adapt;			// Adaptive order
	short Joint;			// Joint Stereo
	short RA;				// Random Access
	short Sub;				// Block switching mode
	short BGMC;				// BGMC coding for residual
	long  MCC;				// Multi-channel correlation
	long NeedPuchBit;		// #Bits for reference channel (MCC)
	long  NeedTdBit;		// #Bits for Time lag (MCC-ex)
	short PITCH;			// Pitch Coding
	short RLSLMS;			// RLS+LMS mode

	long FileSize;			// Length of wave file (in bytes)

	short FileType;			// File Type (raw, wave, aiff, ...)
	short MSBfirst;			// Byte Order (0 = LSB first, 1 = MSB first)
	long  Chan;				// Number of channels
	short Res;				// Actual resolution (in bits)
	short IntRes;			// Integer resolution (in bits)
	unsigned char SampleType;	// Sample type (0=int, 1=float)
	unsigned long Samples;	// Number of samples (per channel)
	long Freq;				// Sampling frequency
	unsigned long HeaderSize;	// Length of file header (in bytes)
	unsigned long TrailerSize;	// Number of trailing non-audio bytes

	long frames;			// Number of frames
	long fid;				// Current frame
	long N0;				// Length of last frame
	unsigned long CRC;		// 32-bit CRC
	short Q;				// Quantizer value
	long ChanSort;			// Rearrange Channels
	unsigned short *ChPos;	// Channel Positions

	unsigned char CoefTable;// Table for entropy coding of coefficients
	short SBpart;			// Subblock partition

	short RAflag;			// location of random access info
	short ChanConfig;		// channel configuration present
	short CRCenabled;		// CRC enabled
	long RAUnits;			// number of random access units
	long RAUid;				// current RAU
	unsigned long *RAUsize;	// sizes of RAUs
	unsigned long CRCorg;	// original (transmitted) CRC value
	short AUXenabled;		// AUX data present

	FILE *fpInput;			// Input file
	FILE *fpOutput;			// Output file

	unsigned char *bbuf, *tmpbuf, *buff;
	long **x, **xp, **xs, **xps, *d, *cofQ;

	CFloat			Float;		// Floating point class
	MCC_DEC_BUFFER	MccBuf;		// Buffer for multi-channel correlation

	// RLSLMS related variables
	short mono_frame;        // frame is mono
	char  RLSLMS_ext;
	rlslms_buf_ptr rlslms_ptr;

public:
	short MCCflag;				// Multi-channel correlation
	CLpacDecoder();				// Constructor
	~CLpacDecoder();			// Destructor
	short CloseFiles();
	short OpenInputFile(const char *name);
	short AnalyseInputFile(AUDIOINFO *ainfo, ENCINFO *encinfo);
	short OpenOutputFile(const char *name);
	long WriteHeader();
	long WriteTrailer();
	long DecodeAll();
	short DecodeFrame();		// Decode one frame
	unsigned long GetCRC();

protected:
	short DecodeBlock(long *x, long Nb, short ra);			// Decode one block
	void  DecodeBlockParameter(MCC_DEC_BUFFER *pBuffer, long Channel, long Nb, short ra);
	short DecodeBlockReconstruct(MCC_DEC_BUFFER *pBuffer, long Channel, long *x, long Nb, short ra);
	void  DecodeBlockParameterRLSLMS(MCC_DEC_BUFFER *pBuffer, long Channel);
	short DecodeBlockReconstructRLSLMS(MCC_DEC_BUFFER *pBuffer, long Channel, long *x);
};

