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

filename : encoder.h
project  : MPEG-4 Audio Lossless Coding
author   : Tilman Liebchen (Technical University of Berlin)
date     : June 23, 2003
contents : Header file for encoder.cpp

*************************************************************************/

/*************************************************************************
 * Modifications:
 *
 * 11/11/2003, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   added support for multichannel, 32-bit, and new file format:
 *     - substituted WaveHeaderSize by HeaderSize
 *     - added new variables FileType, MSBfirst, TrailerSize, CPE, SCE,
 *       Unimode, RawAudio, ChanSort, ChPos[]
 *     - substituted EncodeHeader() by AnalyseInputFile() and WriteHeader()
 *     - substituted EncodeRemains() by WriteTrailer()
 *     - substituted EncodeFile() by EncodeAll()
 *     - added SetUniMode(), SetRawAudioAudio(), SetChannels(),
 *       SetWordlength(), SetFrequency(), SetMSBfirst(),
 *       SetHeaderSize(), SetTrailerSize(), SetChanSort(),
 *       SpecifyAudioInfo()
 *
 * 03/17/2004, Koichi Sugiura <ksugiura@mitaka.ntt-at.co.jp>
 *   - supported floating-point PCM.
 *   - added SetSampleType() function.
 *
 * 03/24/2004, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - removed UniMode and SetUniMode()
 *
 * 07/29/2004, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - added variable: CoefTable
 *
 * 10/26/2004, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - added variable: SBpart
 *
 * 11/05/2004, Yutaka Kamamoto <kamamoto@hil.t.u-tokyo.ac.jp>
 *   - added MCC, NeedPuchBit and MccBuf member variable to CLpacEncoder class
 *   - added SetMCC function
 *   - divided EncodeBlock() into EncodeBlockAnalysis() 
 *                            and EncodeBlockCoding()
 *
 * 11/18/2004, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - replaced FLOAT_BUF structure with CFloat object.
 *   - added NoAcf and SetNoAcf().
 *
 * 02/06/2005, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - removed variable N1
 *   - changed type of N, N0, EncodeBlock() from short to long
 *
 * 03/08/2005, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - integration of CE10 and CE11
 *   - changed *buffer to buffer[6]
 *
 * 3/25/2005, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - removed SetNoAcf().
 *   - added SetAcf() and SetMlz().
 *   - added AcfMode, AcfGain and MlzMode member variable to CLpacEncoder class.
 *
 * 04/27/2005, Yutaka Kamamoto <kamamoto@theory.brl.ntt.co.jp>
 *   - changed type of Chan, Channel from short to long
 *
 * 06/08/2005, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - syntax changes according to N7134
 *   - changed arguments and results of SetHeaderSize() and
 *     SetTrailerSize() to unsigned short
 *   - added RAflag, ChanConfig, CRCenabled, RAUnits, RAUid, RAUsize, FilePos
 *   - removed Version
 *	 - changed type of ChPos to unsigned short*
 *   - added SetRAmode(), SetCRC()
 *
 ************************************************************************/

#include <stdio.h>

#include "wave.h"
#include "floating.h"
#include "mcc.h"
#include "lms.h"

// Type definition of 64-bit integer
#ifdef WIN32
	typedef	__int64				INT64;
	typedef	unsigned __int64	UINT64;
#else
	#include <stdint.h>
	typedef	int64_t				INT64;
	typedef	uint64_t			UINT64;
#endif

class CLpacEncoder
{
protected:
	long N;							// Frame length
	short P;						// (Max.) Predictor order
	short Adapt;					// Adaptive order
	short Win;						// Window function
	short Joint;					// Joint Stereo
	short RA;						// Random Access
	short LSBcheck;					// Check LSBs if they are zero
	short BGMC;						// BGMC coding for residual
	long  MCC;						// Multi-channel correlation method
	short MCCnoJS;					// MCC without Joint Stereo (default: with Joint)
	long NeedPuchBit;				// #Bits for reference channel (MCC)
	short RLSLMS;					// RLSLMS mode

	long  NeedTdBit;				// #Bits for Time lag (MCC-ex)
	short PITCH;					// Pitch Coding
	short Sub;						// Block switching mode
	short AcfMode;					// ACF mode (0-3)
	float AcfGain;					// ACF gain (valid when AcfMode==3)
	short MlzMode;					// MLZ mode (0-1)
	short FileType;					// File Type (raw, wave, aiff, ...)
	short MSBfirst;					// Byte Order (0 = LSB first, 1 = MSB first)
	long  Chan;						// Number of channels
	short Res;						// Actual resolution (in bits)
	short IntRes;					// Integer resolution (in bits)
	unsigned char SampleType;		// Sample type (0 = integer, 1 = float)
	unsigned long Samples;			// Number of samples (per channel)
	long Freq;						// Sampling frequency
	unsigned long HeaderSize;		// Length of file header (in bytes)
	unsigned long TrailerSize;		// Number of trailing non-audio bytes

	long frames;					// Number of frames
	long fid;						// Current frame
	long N0;						// Length of last frame
	unsigned long CRC;				// 32-bit CRC
	short Q;						// Quantizer value

	short CPE, SCE;
	short RawAudio;					// Raw Audio Data
	long  ChanSort;					// Rearrange Channels
	unsigned short *ChPos;			// Channel Positions

	unsigned char CoefTable;		// Table for entropy coding of coefficients
	short SBpart;					// Subblock partition

	short RAflag;					// location of random access info
	short ChanConfig;				// channel configuration present
	short CRCenabled;				// CRC enabled
	long RAUnits;					// number of random access units
	long RAUid;						// current RAU
	unsigned long *RAUsize;			// sizes of RAUs

	long FilePos;					// file position pointer

	FILE *fpInput;					// Input file
	FILE *fpOutput;					// Output file

	unsigned char *bbuf, *buff, *tmpbuf1, *tmpbuf2, *tmpbuf3, *buffer[6], **tmpbuf_MCC, *buffer_m;
	long **x, **xp, **xs, **xps, *d, *cof;
	double *par;

	CFloat Float;					// Floating point class
	MCC_ENC_BUFFER MccBuf;			// Buffer for multi-channel correlation method

	// RLSLMS related variables
	short mono_frame;				// frame is mono
	char RLSLMS_ext;
	rlslms_buf_ptr rlslms_ptr;

public:
	long MCCflag;					// Multi-channel correlation method	CLpacEncoder();	
	CLpacEncoder();					// Constructor
	~CLpacEncoder();				// Destructor

	short CloseFiles();
	short EncodeFrame();			// Encode one frame
	void GetFilePositions(long *SizeIn, long *SizeOut);	// Current file pointer positions

	short OpenInputFile(const char *name);
	short AnalyseInputFile(AUDIOINFO *ainfo);
	short SpecifyAudioInfo(AUDIOINFO *ainfo);
	short OpenOutputFile(const char *name);
	long WriteHeader(ENCINFO *encinfo);
	long WriteTrailer();
	long EncodeAll();

	long SetFrameLength(long N);
	short SetOrder(short P);
	short SetAdapt(short Adapt);
	short SetWin(short Win);
	short SetJoint(short Joint);
	short SetRA(short RA);
	short SetRAmode(short RAmode);
	short SetLSBcheck(short LSBcheck);
	short SetBGMC(short BGMC);
	long SetMCC(long MCC);
	short SetHEMode(short RLSLMS);
	short SetRawAudio(short RawAudio);
	long SetChannels(long Chan);
	unsigned char SetSampleType(unsigned char SampleType);
	short SetWordlength(short Res);
	long SetFrequency(long Freq);
	short SetMSBfirst(short MSBfirst);
	unsigned long SetHeaderSize(unsigned long HeaderSize);
	unsigned long SetTrailerSize(unsigned long TrailerSize);
	long SetChanSort(long ChanSort, unsigned short *ChPos);
	short SetPITCH(short PITCH);
	short SetSub(short Sub_x);
	short SetAcf(short AcfMode, float AcfGain);
	short SetMlz(short MlzMode);
	short SetMCCnoJS(short MCCnoJS);
	short SetCRC(short CRCenabled);

protected:
	long EncodeBlock(long *x, unsigned char *bytebuf);		// Encode block
	void EncodeBlockAnalysis(MCC_ENC_BUFFER *pBuffer, long Channel, long *d); //MCC
	long EncodeBlockCoding(MCC_ENC_BUFFER *pBuffer, long Channel, long *x, unsigned char *bytebuf, long gmod); //MCC
	void LTPanalysis(MCC_ENC_BUFFER *pBuffer, long Channel, long N, short optP, long *x);
};
