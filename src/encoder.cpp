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

filename : encoder.cpp
project  : MPEG-4 Audio Lossless Coding
author   : Tilman Liebchen (Technical University of Berlin)
date     : June 25, 2003
contents : Implementation of the CLpacEncoder class

*************************************************************************/

/*************************************************************************
 * Modifications:
 *
 * 8/31/2003, Yuriy A. Reznik <yreznik@real.com>
 *  made set of changes to support BGMC coding mode:
 *      - added CLpacEncoder::SetBGMC() function;
 *      - added logic for using 0x10 bit (BGMC) in the "parameters" field
 *        and 0x40 bit in the version number in CLpacEncoder::EncodeHeader()
 *      - added calls to BGMC coding routines in CLpacEncoder::EncodeBlock()
 *
 * 11/26/2003, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   added support for multichannel, 32-bit, and new file format:
 *     - substituted DecodeHeader() by AnalyseInputFile() and WriteHeader()
 *     - substituted DecodeRemains() by WriteTrailer()
 *     - substituted DecodeFile() by DecodeAll()
 *     - added SetUniMode(), SetRawAudioAudio(), SetChannels(),
 *       SetWordlength(), SetFrequency(), SetMSBfirst(),
 *       SetHeaderSize(), SetTrailerSize(), SetChanSort(),
 *       SpecifyAudioInfo()
 *     - added encoding of new file format header in WriteHeader()
 *     - added encoding of multichannel and 32-bit in EncodeFrame() and
 *       EncodeBlock()
 *   fixed bug in specification of 24/32-bit zero/const blocks
 *
 * 12/16/2003, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - new sizes of tmpbuf<1,2,3>[] arrays
 *
 * 03/17/2004, Koichi Sugiura <ksugiura@mitaka.ntt-at.co.jp>
 *   - added (double) cast to parameter of log().
 *   - supported floating point PCM.
 *
 * 03/24/2004, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - progressive prediction for random access frames
 *   - removed support for file format version < 8
 *
 * 5/17/2004, Yuriy Reznik <yreznik@real.com>
 *  changes to support improved coding of LPC coefficients:
 *   - modified CLpacEncoder::EncodeBlock() to use new scheme
 *   - incremented version # and removed support for older file formats
 *
 * 07/29/2004, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - added support for higher orders (up to 255)
 *   - further improved entropy coding of coefficients (3 tables)
 *   - streamlined block header syntax
 *   - removed support for file format version < 11
 *   - enabled adaptive prediction order
 *
 * 10/26/2004, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - added use of bgmc_encode_blocks()
 *   - added support for up to 8 subblocks
 *   - improved coding of code parameters
 *   - incremented version # and removed support for older file formats
 *   - removed block switching flag where not necessary
 *
 * 11/05/2004, Yutaka Kamamoto <kamamoto@hil.t.u-tokyo.ac.jp>
 *   - added Multi-channel correlation method
 *   - divided EncodeBlock() into EncodeBlockAnalysis() 
 *                            and EncodeBlockCoding()
 *
 * 11/19/2004, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - modified floating point operations using CFloat class.
 *   - added SetNoAcf().
 *   - disabled adaptive order temporarily, because lpc_adapt.obj
 *     has been supplied for Windows only. Even with -l option, adaptive 
 *     order is turned off.
 *
 * 11/22/2004, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - supported channel sort feature for floating point data.
 *   - lpc_adapt.obj is required only when LPC_ADAPT symbol is defined.
 *
 * 02/06/2005, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - added support for extended block switching and higher orders
 *
 * 02/11/2005, Takehiro Moriya (NTT) <t.moriya@ieee.org>
 *   - add LTP modules for CE11
 *
 * 03/08/2005, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - integration of CE10 and CE11
 *   - added block switching in EncodeFrame()
 *
 * 03/23/2005,  Yutaka Kamamoto (NTT/The University of Tokyo)
 *   - integration of multi-channel correlation and block switching (MCConBS)
 *
 * 3/25/2005, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - removed SetNoAcf().
 *   - added SetAcf() and SetMlz().
 *
 * 03/28/2005, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - support for orders up to 1023
 *
 * 04/27/2005, Yutaka Kamamoto <kamamoto@theory.brl.ntt.co.jp>
 *   - added MCCex(multi-tap MCC) modules for CE14
 *   - changed ALS header information of #Channel 1byte(256) to 2 bytes(65536)
 *
 * 04/27/2005, Takehiro Moriya (NTT) <t.moriya@ieee.org>
 *   - added Joint Stereo and MCC switching in EncodeFrame()
 *
 * 05/16/2005, Choo Wee Boon (I2R) <wbchoo@a-star.i2r.edu.sg>
 *	 - added RLSLMS mode 
 *
 * 06/08/2005, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - syntax changes according to N7134
 *
 * 07/07/2005, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - some bugfixes (RM15v2)
 *
 * 07/15/2005, Yutaka Kamamoto <kamamoto@theory.brl.ntt.co.jp>
 * 07/21/2005, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - some more bugfixes (RM15v3)
 *
 * 07/22/2005, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - revised format of ChanSort/ChanPos
 *   - removed restriction of s values (Rice codes)
 *   - support for auxiliary data
 *
 * 08/22/2005, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - some minor revisions
 *
 * 10/08/2005, Noboru Harada <n-harada@theory.brl.ntt.co.jp>
 *   - fixed an endian related bug on floating-point codes.
 *
 * 10/17/2005, Noboru Harada <n-harada@theory.brl.ntt.co.jp>
 *   - Merged bug fixed codes for RLS-LMS prepared from I2R.
 *
 * 07/10/2006, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - debugged ALSSpecificConfig generation. (CRC position)
 *
 * 07/14/2006, Noboru Harada <n-harada@theory.brl.ntt.co.jp>
 *   - added safty measures for some memcopy oprerations.
 *
 * 05/23/2007, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - supported 64-bit data size.
 *   - replaced FILE* with HALSSTREAM.
 *   - supported Sony Wave64 and BWF with RF64 formats.
 *
 * 05/25/2007, Nobory Harada <n-harada@theory.brl.ntt.co.jp>
 *   - debugged.
 *
 * 06/08/2007, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - added return values of CLpacEncoder::WriteHeader() to distinguish 
 *     error causes.
 *
 * 06/18/2007, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - debugged file_type in ALS header.
 *   - debugged maximum value of samples, header size and trailer size
 *     in ALS header.
 *
 * 07/02/2007, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - debugged WriteHeader() in case that header/trailer size is less 
 *     than 4GB, but too big to allocate.
 *
 * 07/15/2007, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - added oafi_flag and supported ALSSpecificConfig with header and 
 *     trailer embedded.
 *
 * 05/14/2008, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - supported WAV files with WAVEFORMATEXTENSIBLE.
 *
 * 10/20/2008, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - debugged the bit location of js_switch in frame_data.
 *   - debugged the limitation of 2nd and 3rd rice parameters to 15 
 *     in case of Res<=16.
 *   - debugged EncodeBlockCoding() to handle IntRes=20 properly.
 *   - debugged maximum prediction order calculation.
 *
 * 10/22/2008, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - moved LPC_ADAPT symbol definition to project file/Makefile.
 *   - modified process exit code.
 *
 * 10/23/2008, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - replaced ceil( double ) with ceil( float ) to avoid g++ bug.
 *
 * 11/28/2008, Csaba Kos <csaba.kos@ex.ssh.ntt-at.co.jp>
 *   - rewritten the formula for calculating the bit-width of the prediction
 *     order using integer operations (instead of floating-point).
 *   - fixed some printf format strings
 *
 * 09/04/2009, Csaba Kos <csaba.kos@ex.ssh.ntt-at.co.jp>
 *   - added support for enforcing profile levels
 *   - added support for checking conformant profile levels
 *
 * 12/07/2009, Csaba Kos <csaba.kos@as.ntt-at.co.jp>
 *   - clip CoefTable to 3 for higher sampling rates instead of
 *     wrapping-around
 *   - add missing direct coding of coefficients when CoefTable is 3
 *
 * 12/25/2009, Csaba Kos <csaba.kos@as.ntt-at.co.jp>
 *   - do not use the const block coding tool when the constant value
 *     cannot be represented on IntRes bits
 *
 * 12/28/2009, Csaba Kos <csaba.kos@as.ntt-at.co.jp>
 *   - aligned NeedTdBit calculation with the ALS specification
 *   - fixed progressive coding of the residual for short frames
 *   - use IntRes consistently in the integer encoder
 *
 * 02/03/2009, Csaba Kos <csaba.kos@as.ntt-at.co.jp>
 *   - correctly set independent_bs=1 in bs_info for the second channel
 *     of independently coded channel pairs
 *
 * 02/04/2009, Csaba Kos <csaba.kos@as.ntt-at.co.jp>
 *   - do not allow shorter block length than the fixed prediction order
 *
 ************************************************************************/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>

#if defined(WIN32) || defined(WIN64)
	#include <io.h>
	#include <fcntl.h>
#endif

#include "encoder.h"
#include "lpc.h"
#include "lms.h"
#include "ec.h"
#include "bitio.h"
#include "audiorw.h"
#include "crc.h"
#include "wave.h"
#include "floating.h"
#include "lpc_adapt.h"
#include "mcc.h"
#include "stream.h"

#define PI 3.14159265359

#define min(a, b)  (((a) < (b)) ? (a) : (b))
#define max(a, b)  (((a) > (b)) ? (a) : (b))

namespace {

int ilog2_ceil(unsigned int x) {
	if (x == 0) return -1;

	int l = 0;
	while ((l < sizeof(x)*CHAR_BIT) && ((1u << l) < x)) ++l;
	return l;
}

} /* namespace */

///////////////////////////////////////////////////////////////////////////////////////////////////
// Constructor
CLpacEncoder::CLpacEncoder()
{
	fpInput = NULL;
	fpOutput = NULL;

	N = 0;			// Automatic choice of frame length
	P = 10;			// 10th order filter
	Adapt = 0;		// Adaptive order = off
	Win = 0;		// Hanning window
	Joint = -1;		// Joint Stereo = off
	RA = 0;			// Random Access = off
	LSBcheck = 0;	// LSB check = off
	MSBfirst = 0;	// Read LSByte first
	RawAudio = 0;	// Raw Audio Data = false
	Chan = 2;		// Default: stereo
	Res = 16;		// Default: 16-bit
	IntRes = 16;	// Default: 16-bit
	SampleType = 0;	// Default: 0=int
	Freq = 44100;	// Default: 44100 Hz
	HeaderSize = 0;	// Default: No header
	TrailerSize = 0;// Default: No trailer
	ChanSort = 0;	// Default: Don't rearrange channels
	BGMC = 0;		// Default: Rice coding
	fid = 0;		// No frame decoded yet
	CRC = 0;		// CRC initialization
	Q = 20;			// Quantizer value (fixed)
	CoefTable = 0;	// Entropy table for coefficients
	SBpart = 1;		// Subblock partition
	MCC = 0;		// Multi-channel correlation = off
	MCCnoJS = 0;	// MCC without Joint Stereo = off
	PITCH = 0;		// Pitch Coding
	mono_frame = 0; // mono_block 0
	Sub = 0;		// Block switching mode = off
	RAflag = 1;		// Location of random access info (default: in frames)
	ChanConfig = 0;	// Channel configuration = off
	CRCenabled = 1;	// CRC = on
	ChPos = NULL;	// No channel sorting table defined
	CloseInput = CloseOutput = false;
	frames = 0;
	mp4file = false;
	oafi_flag = false;

	ALSProfFillSet( ConformantProfiles );
	ALSProfEmptySet( EnforcedProfiles );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Destructor
CLpacEncoder::~CLpacEncoder()
{
	long i;

	if (frames > 0)
	{
		// Deallocate memory
		for (i = 0; i < Chan; i++)
			delete [] xp[i];
		delete [] xp;
		delete [] x;

		if (RLSLMS)
		{
			for (i = 0; i < Chan; i++)
			{
				delete [] rlslms_ptr.pbuf[i];
				delete [] rlslms_ptr.weight[i];
			}
			delete [] rlslms_ptr.pbuf;
			delete [] rlslms_ptr.weight;
		}

		delete [] tmpbuf1;

		if (CPE)
		{
			delete [] tmpbuf2;

			if (Joint)
			{
				for (i = 0; i < CPE; i++)
					delete [] xps[i];
				delete [] xps;
				delete [] xs;

				delete [] tmpbuf3;
			}
		}

		delete [] bbuf;
		delete [] d;
		delete [] par;
		delete [] cof;
		delete [] buff;

		for (short s = 0; s <= Sub; s++)
			delete [] buffer[s];

		if (RA && (RAflag == 2))
			delete [] RAUsize;

		if (ChanSort)
			delete [] ChPos;

		// Deallocate float buffer
		if ( SampleType == SAMPLE_TYPE_FLOAT ) Float.FreeBuffer();
		// Deallocate MCC buffer
		FreeMccEncBuffer ( &MccBuf );
		for (i = 0; i < Chan; i++)
			delete [] tmpbuf_MCC[i];
		delete [] tmpbuf_MCC;
		delete [] buffer_m;
	}

	// Close files
	CloseFiles();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Close input and output file
short CLpacEncoder::CloseFiles()
{
	if ( fpInput ) {
		if ( CloseInput ) fclose( fpInput );
		fpInput = NULL;
		CloseInput = false;
	}

	if ( fpOutput ) {
		if ( CloseOutput ) fclose( fpOutput );
		fpOutput = NULL;
		CloseOutput = false;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Get positions of file pointers
void CLpacEncoder::GetFilePositions(ALS_INT64 *SizeIn, ALS_INT64 *SizeOut)
{
	*SizeIn = ftell(fpInput);
	*SizeOut = ftell(fpOutput);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Retrieve information from audio file
short CLpacEncoder::AnalyseInputFile(AUDIOINFO *ainfo)
{
	ALS_INT64 DataLen, FileLen;
	WAVEFORMATPCM wf;
	AIFFCOMMON ac;
	UINT samplerate;
	ALS_INT64 Offset, BlockSize;
	USHORT	BitWidth;

	try {
		if ( RawAudio ) throw 0;	// RAW
		
		// Try wave format
		HeaderSize = GetWaveFormatPCM( fpInput, &wf, &Samples );
		if ( HeaderSize >= 0 ) throw 1;	// WAVE
		
		// Try aiff format
		rewind( fpInput );
		HeaderSize = GetAiffFormat( fpInput, &ac, &Offset, &BlockSize, &samplerate );
		if ( HeaderSize >= 0 ) { MSBfirst = 1; throw 2; }	// AIFF
		
		// Try Sony Wave64 format
		rewind( fpInput );
		HeaderSize = GetWave64FormatPCM( fpInput, &wf, &Samples );
		if ( HeaderSize >= 0 ) throw 4;	// Sony Wave64
		
		// Try bwf with RF64 format
		rewind( fpInput );
		HeaderSize = GetRF64FormatPCM( fpInput, &wf, &Samples );
		if ( HeaderSize >= 0 ) throw 5;	// BWF with RF64
		
		return -1;	// Unknown file type
	}
	catch( int ft ) {
		FileType = static_cast<short>( ft );
	}

	fseek(fpInput, 0L, SEEK_END);
	FileLen = ftell(fpInput);
	rewind(fpInput);

	if ( ( FileType == 1 ) || ( FileType == 3 ) || ( FileType == 4 ) || ( FileType == 5 ) )
	{
		Chan = wf.Channels;
		Freq = wf.SamplesPerSec;
		Res = wf.ValidBitsPerSample;
		DataLen = Samples * wf.BlockAlign;
		BitWidth = wf.BitsPerSample;

		// Check format
		if ( wf.FormatTag == 1 ) {				//WAVE_FORMAT_PCM
			// Integer PCM
			SampleType = 0;
			IntRes = Res;
		} else if ( wf.FormatTag == 3 && Res == 32 ) {		//WAVE_FORMAT_IEEE_FLOAT
			// Floating point PCM
			SampleType = 1;
			IntRes = IEEE754_PCM_RESOLUTION;
		} else {
			// Invalid FormatTag
			return -1;
		}
	}
	else if (FileType == 2)
	{
		Samples = ac.SampleFrames;
		Chan = ac.Channels;
		BitWidth = IntRes = Res = ac.SampleSize;
		SampleType = 0;
		Freq = samplerate;
		DataLen = Samples * Chan * (Res / 8);
	}
	else	// FileType == 0
	{
		if (SampleType == 1) /* Force 32-bit resolution */
			Res = 32;        /* for floating point      */
		if ((FileLen - HeaderSize) % (Chan * (Res / 8)))
			return(-1);
		Samples = (FileLen - HeaderSize) / (Chan * (Res / 8));
		BitWidth = Res;
	}

	if (FileType != 0)
	{
		// Check lengths
		if (FileLen - HeaderSize < DataLen)				// File is shorter than specified
			return(-1);
		//if (FileLen - DataLen - HeaderSize > 32767)		// More than 32767 remaining bytes
		//	return(-1);
		else
			TrailerSize = FileLen - DataLen - HeaderSize;
	}

	// Check parameters
	if ((BitWidth > 32) || (BitWidth < 1) || (BitWidth < Res))
		return(-1);
	if ( BitWidth > Res ) LSBcheck = 1;
	Res = BitWidth;
	if ((Res < 32) && (Res > 24))
	{
		Res = 32;			// Interpret data as 32-bit
		LSBcheck = 1;		// Check LSBs
	}
	else if ((Res < 24) && (Res > 16))
	{
		Res = 24;			// Interpret data as 24-bit
		LSBcheck = 1;		// Check LSBs
	}
	else if ((Res < 16) && (Res > 8))
	{
		Res = 16;			// Interpret data as 16-bit
		LSBcheck = 1;		// Check LSBs
	}
	else if (Res < 8)
	{
		Res = 8;			// Interpret data as 8-bit
		LSBcheck = 1;		// Check LSBs
	}

	if ((Chan > MAXCHAN) || (Chan < 1))
		return(-1);

	// Set parameter structure
	ainfo->FileType = (unsigned char)FileType;
	ainfo->MSBfirst = (unsigned char)MSBfirst;
	ainfo->Chan = Chan;
	ainfo->Res = Res;
	ainfo->IntRes = IntRes;
	ainfo->SampleType = SampleType;
	ainfo->Samples = Samples;
	ainfo->Freq = Freq;
	ainfo->HeaderSize = HeaderSize;
	ainfo->TrailerSize = TrailerSize;

	if (!EnforceProfiles())
		return -1;

	return(0);
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Specify audio format for raw file
short CLpacEncoder::SpecifyAudioInfo(AUDIOINFO *ainfo)
{
	FileType = ainfo->FileType;
	MSBfirst = ainfo->MSBfirst;
	Chan = ainfo->Chan;
	Res = ainfo->Res;
	IntRes = ainfo->IntRes;
	SampleType = ainfo->SampleType;
	Samples = ainfo->Samples;
	Freq = ainfo->Freq;
	HeaderSize = ainfo->HeaderSize;
	TrailerSize = ainfo->TrailerSize;

	if (!EnforceProfiles())
		return -1;

	return(0);
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Generate and write header
ALS_INT64 CLpacEncoder::WriteHeader(ENCINFO *encinfo)
{
	BYTE audiotyp, tmp = 0;
	long i, j, rest;

#ifdef ALS_VERSION_INFO
	short AUXenabled = 1;	// set AUXenabled = 1 to generate aux data (see below)
#else
	short AUXenabled = 0;	// set AUXenabled = 0 not to generate aux data (see below)
#endif

#ifndef	LPC_ADAPT
	Adapt = 0;
#endif

	// Enfore profiles for all parameters that are finalized
	if (!EnforceProfiles())
		return -1;

	if (P == 0)
		Adapt = 0;			// force fixed order

	// Channel configuration
	CPE = Chan / 2;		// Channel Pair Elements
	SCE = Chan % 2;		// Single Channel Elements

	if ((Chan == 1) || (Joint == -1))			// Independent Coding of all channels
		Joint = 0;
	else if ((Joint == 0) || (Joint > CPE))		// Joint Coding of all CPEs
		Joint = 1;
	else
	{
		CPE = Joint;							// Joint Coding of <Joint> CPEs
		SCE = Chan - 2*CPE;						// Independent Coding of remaining channels
		Joint = 1;
	}

	if((Chan < 2) && MCC)						// MCC shold be used in multi-channel data
	{					
		MCC = 0;
		if(MCCnoJS) MCCnoJS=0;
	}

	if (ChanSort != Chan)						// Turn on only if all channels positions are given
		ChanSort = 0;

	// Adjust frame length automatically
	if ((N == 0) || (N % (1<<Sub)))
	{
		if (Freq > 96000L)
			N = 8192;
		else if (Freq > 48000L)
			N = 4096;
		else if (Freq > 32000L)
			N = 2048;
		else
			N = 1024;

		// Adjust length if block switching is used
		N *= (1<<(Sub/2));		// Sub=0/1:N, Sub=2/3:2N, Sub=4/5:4N
	}
	Q = 20;		// Quantizer value (for conversion of coefficients)

	// Enfore profiles for possibly calculated parameters (frame length, etc.)
	if (!EnforceProfiles())
		return -1;

	// If fixed prediction is used, P <= N must be true
	if (!Adapt && P > N)
		return -1;

	// Number of frames
	frames = Samples / N;
	rest = static_cast<long>( Samples % N );
	if (rest)
	{
		frames++;
		N0 = rest;
	}
	else
		N0 = N;

	// Random Access
	if (RA)
	{
		if (RA > 0)
		{
			// Conversion of RA frame distance: 1/10 sec -> number of frames
			// Example: RA = 5 (0.5 s) -> RA = 5*44100/(10*2048) = 10 frames
			RA = RA * Freq / (10 * N);
			if (RA == 0)
				RA = 1;
		}
		else		// RA == -1
			RA = 1; // Make each frame decodable
		if (RA > 255)
			RA = 255;

		// number of random acess units
		if ( frames / RA > 0x7fffffff ) return ( frames = -3 );
		RAUnits = static_cast<long>( frames / RA );
		if (frames % RA)
			RAUnits++;
		RAUid = 0;			// RAU index (0..RAUnits-1)
	}

	// Allocate memory (will be deallocated by the destructor)
	xp = new int*[Chan];
	x = new int*[Chan];
	for (i = 0; i < Chan; i++ )
	{
		xp[i] = new int[N+P];
		x[i] = xp[i] + P;
		memset(xp[i], 0, sizeof(int)*P);
	}

	if (RLSLMS)
	{
		rlslms_ptr.pbuf = new BUF_TYPE*[Chan];
		rlslms_ptr.weight = new W_TYPE*[Chan];
		rlslms_ptr.Pmatrix = new P_TYPE*[Chan];
		for (i = 0; i < Chan; i++ )
		{
			rlslms_ptr.pbuf[i] = new BUF_TYPE[TOTAL_LMS_LEN];
			rlslms_ptr.weight[i] = new W_TYPE[TOTAL_LMS_LEN];
			rlslms_ptr.Pmatrix[i] = new P_TYPE[JS_LEN*JS_LEN];
		}
	}

	// following buffer size is enough if only forward predictor is used.
	// //(long)((IntRes+7)/8) = ceil(IntRes/8)
	//	long BufSize = ((long)((IntRes+7)/8)+1)*N + (4*P + 128)*(1+(1<<Sub));
	// for RLS-LMS
	long BufSize = long(IntRes/8+10)*N*2;

	tmpbuf1 = new unsigned char[BufSize];				// Buffer for one channel

	if (CPE)
	{
		tmpbuf2 = new unsigned char[BufSize];			// Buffer for another channel

		if (Joint)
		{
			xps = new int*[CPE];
			xs = new int*[CPE];

			for (i = 0; i < CPE; i++)
			{
				xps[i] = new int[N+P];
				xs[i] = xps[i] + P;
				memset(xps[i], 0, sizeof(int)*P);
			}

			tmpbuf3 = new unsigned char[BufSize];		// Buffer for a difference channel
		}
	}

	bbuf = new unsigned char[BufSize * Chan];	// Input buffer

	// Allocate float buffer
	if ( SampleType == SAMPLE_TYPE_FLOAT ) Float.AllocateBuffer( Chan, N, IntRes );
	
	// Allocate MCC buffer
	if (Freq >= 192000L)
		NeedTdBit = 7;
	else if (Freq >= 96000L)
		NeedTdBit = 6;
	else
		NeedTdBit = 5;

	AllocateMccEncBuffer( &MccBuf, Chan, N, IntRes,(1<<NeedTdBit));

	// #bits/channel: NeedPuchBit = max(1,ceil(log2(Chan)))
	i = (Chan > 1) ? (Chan-1) : 1;
	NeedPuchBit = 0;
	while(i){
		i /= 2;
		NeedPuchBit++;
	}
	
	tmpbuf_MCC = new unsigned char*[Chan];
	for(i = 0; i < Chan; i++)
		tmpbuf_MCC[i] = new unsigned char[BufSize];
	buffer_m = new unsigned char[4L*N*Chan + 4L*P + N*Chan*IEEE754_BYTES_PER_SAMPLE+100];

	d = new int[N];											// Difference signal (residual)
	par = new double[P];										// Coefficients (parcor)
	cof = new int[P];											// Coefficients (direct form, quantized)
	size_t buff_size = ( NeedPuchBit * Chan ) / 8 + 1;
	if ( buff_size < 4 ) buff_size = 4;
	if ( !mp4file ) {
		if ( HeaderSize >= 0xffffffff ) return ( frames = -5 );		// ALS can support header up to 4GB.
		if ( TrailerSize >= 0xffffffff ) return ( frames = -6 );	// ALS can support trailer up to 4GB.
		if ( buff_size < HeaderSize ) buff_size = static_cast<size_t>( HeaderSize );
		if ( buff_size < TrailerSize ) buff_size = static_cast<size_t>( TrailerSize );
	} else if ( !oafi_flag ) {
		if ( ( HeaderSize < 0xffffffff ) && ( buff_size < HeaderSize ) ) buff_size = static_cast<size_t>( HeaderSize );
		if ( ( TrailerSize < 0xffffffff ) && ( buff_size < TrailerSize ) ) buff_size = static_cast<size_t>( TrailerSize );
	}
	buff = new unsigned char[ buff_size ];		// Buffer for audio header/trailer and ChanPos[]
	if ( buff == NULL ) return ( frames = -7 );	// Memory error

	// Frame buffer for all channels
	buffer[0] = new unsigned char[4L*N*Chan + 4L*P + N*Chan*IEEE754_BYTES_PER_SAMPLE+100];					
	
	for (short s = 1; s <= Sub; s++)
		buffer[s] = new unsigned char[4L*N*Chan + 4L*P + N*Chan*IEEE754_BYTES_PER_SAMPLE+100]; // Frame buffer for all channel (subblock)

	short SubX = Sub;	// Index for block switching level
	if (Sub)
		SubX = (Sub < 3) ? 1 : Sub - 2;

	// Information about the audio signal
	switch( FileType ) {
	case 1:	audiotyp = 32;	break;		// 001x xxxx (original = wave file)
	case 2:	audiotyp = 64;	break;		// 010x xxxx (original = aiff file)
	case 3:	audiotyp = 96;	break;		// 011x xxxx (original = bwf file)
	case 4:	audiotyp = 0;	break;		// 000x xxxx (original = unknown) Sony Wave64
	case 5:	audiotyp = 0;	break;		// 000x xxxx (original = unknown) bwf with RF64
	default:audiotyp = 0;	break;
	}
	if (Res == 16) audiotyp += 4;		// xxx0 01xx
	else if (Res == 24) audiotyp += 8;	// xxx0 10xx
	else if (Res == 32) audiotyp += 12;	// xxx0 11xx
	if ( SampleType ) audiotyp += 2;	// xxxx xx1x
	if ( MSBfirst ) audiotyp += 1;		// xxxx xxx1

	// Select entropy coding table for coefficients via sampling rate, other criteria possible
	CoefTable = min((Freq / 48000L) >> 1, 0x03);				// 44k/48k->00, 96k->01, 192k->10, 288k or higher->11

	if (RLSLMS)
	{
		initCoefTable(RLSLMS, CoefTable);
		RLSLMS_ext = 7;
		for(i=0;i<Chan;i++)
		{
			for(j=0;j<TOTAL_LMS_LEN;j++) rlslms_ptr.pbuf[i][j]=0;
			rlslms_ptr.channel=i;
			predict_init(&rlslms_ptr); 
		}
	}

	// Write ALS header information ///////////////////////////////////////////////////////////////
	UINT als_id;
	als_id = 0x414C5300UL;
	WriteUIntMSBfirst(als_id, fpOutput);							// 'ALS' + 0x00
	WriteUIntMSBfirst((UINT)Freq, fpOutput);						// sampling frequency
	if ( mp4file ) {
		if ( Samples >= 0xffffffff ) WriteUIntMSBfirst( 0xffffffff, fpOutput );	// samples
		else WriteUIntMSBfirst( static_cast<ALS_UINT32>( Samples ), fpOutput );
	} else {
		if ( Samples >= 0xffffffff ) return ( frames = -4 );		// ALS can support samples up to 4GB.
		WriteUIntMSBfirst( static_cast<ALS_UINT32>( Samples ), fpOutput );
	}
	WriteUShortMSBfirst(USHORT(Chan - 1), fpOutput);				// channels

	if (fwrite(&audiotyp, 1, 1, fpOutput) != 1) return(frames = -2);// FFFRRRSM

	WriteUShortMSBfirst(USHORT(N - 1), fpOutput);					// frame length

	tmp = (BYTE)RA;
	if (fwrite(&tmp, 1, 1, fpOutput) != 1) return(frames = -2);		// random access

	tmp = (BYTE)((RAflag << 6) | (Adapt ? 0x20 : 0) | (CoefTable << 3) | (PITCH ? 0x04 : 0) | ((P >> 8) & 0x03));
	if (fwrite(&tmp, 1, 1, fpOutput) != 1) return(frames = -2);		// RRACCLPP (L: LTP/PITCH)

	tmp = (BYTE)(P & 0xFF);
	if (fwrite(&tmp, 1, 1, fpOutput) != 1) return(frames = -2);		// prediction order (8 LSBs)

	tmp = (BYTE)((SubX << 6) | (BGMC ? 0x20 : 0) | (SBpart ? 0x10 : 0) | (Joint ? 0x08 : 0) | (MCC ? 0x04 : 0) | (ChanConfig ? 0x02 : 0) | (ChanSort ? 0x01 : 0));
	if (fwrite(&tmp, 1, 1, fpOutput) != 1) return(frames = -2);		// SSBSJMCS

	tmp = (BYTE)((CRCenabled ? 0x80 : 0) | (RLSLMS ? 0x40 : 0) | (AUXenabled ? 0x01 : 0));
	if (fwrite(&tmp, 1, 1, fpOutput) != 1) return(frames = -2);		// CRxxxxxA

	//if (ChanConfig)
	//	WriteUShortMSBfirst(ChanConfigInfo, fpOutput);				// channel configuration data

	if (ChanSort)
	{
		CBitIO out;
		out.InitBitWrite(buff);

		for (i = 0; i < Chan; i++)
			out.WriteBits(UINT(ChPos[i]), NeedPuchBit);

		long size = out.EndBitWrite();
		if (fwrite(buff, 1, size, fpOutput) != size) return(frames = -2);
	}

	// Header and trailer sizes
	if ( oafi_flag ) {
		WriteUIntMSBfirst( ( HeaderSize > 0 ) ? 0xffffffff : 0, fpOutput );
		WriteUIntMSBfirst( ( TrailerSize > 0 ) ? 0xffffffff : 0, fpOutput );
	} else {
		WriteUIntMSBfirst( static_cast<ALS_UINT32>( HeaderSize ), fpOutput );
		WriteUIntMSBfirst( static_cast<ALS_UINT32>( TrailerSize ), fpOutput );
	}

	if ( oafi_flag ) {
		// Skip file header
		fseek( fpInput, HeaderSize, SEEK_CUR );
	} else {
		// copy audio file header (if present)
		fread( buff, 1, static_cast<ALS_UINT32>( HeaderSize ), fpInput );
		if ( fwrite( buff, 1, static_cast<ALS_UINT32>( HeaderSize ), fpOutput ) != static_cast<ALS_UINT32>( HeaderSize ) ) return ( frames = -2 );
	}

	// get current position and write dummy bytes for trailer (if present)
	FilePos = ftell( fpOutput );
	if ( !oafi_flag ) {
		if ( fwrite( buff, 1, static_cast<ALS_UINT32>( TrailerSize ), fpOutput ) != static_cast<ALS_UINT32>( TrailerSize ) ) return ( frames = -2 );
	}

	if (CRCenabled)
	{
		// CRC initialization
		BuildCRCTable();
		CRC = CRC_MASK;
		// write dummy bytes
		if (fwrite(buff, 1, 4, fpOutput) != 4) return(frames = -2);
	}

	if (RA && (RAflag == 2))		// save random access info in header
	{
		RAUsize = new unsigned int[RAUnits];
		// write dummy bytes
		if (fwrite(RAUsize, 1, RAUnits * 4, fpOutput) != (RAUnits * 4)) return(frames = -2);
	}

	// aud data test
	if (AUXenabled)
	{
#if defined( ALS_VERSION_INFO )
		WriteUIntMSBfirst( sizeof(ALS_VERSION_INFO), fpOutput );
		if ( fwrite( ALS_VERSION_INFO, 1, sizeof(ALS_VERSION_INFO), fpOutput ) != sizeof(ALS_VERSION_INFO) ) return ( frames = -2 );
#else
		WriteUIntMSBfirst( 0, fpOutput );
#endif
	}

	// End of header information //////////////////////////////////////////////////////////////////

	AUDIOINFO ainfo;
	ainfo.Chan = Chan;
	ainfo.FileType = FileType;
	ainfo.MSBfirst = MSBfirst;
	ainfo.Res = Res;
	ainfo.IntRes = IntRes;
	ainfo.SampleType = SampleType;
	ainfo.Samples = Samples;
	ainfo.Freq = Freq;
	ainfo.HeaderSize = HeaderSize;
	ainfo.TrailerSize = TrailerSize;

	encinfo->FrameLength = N;
	encinfo->AdaptOrder = Adapt;
	encinfo->JointCoding = Joint;
	encinfo->SubBlocks = Sub;
	encinfo->RandomAccess = RA;
	encinfo->BGMC = BGMC;
	encinfo->MaxOrder = P;
	encinfo->MCC = MCC;
	encinfo->PITCH = PITCH;
	encinfo->RLSLMS = RLSLMS;
	encinfo->RAflag = RAflag;
	encinfo->CRCenabled = CRCenabled;

	CheckAlsProfiles_Header( ConformantProfiles, &ainfo, encinfo );

	return(frames);
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Write trailing non-audio data
ALS_INT64 CLpacEncoder::WriteTrailer()
{
	long r;

	fseek(fpOutput, FilePos, SEEK_SET);
	
	if ( oafi_flag ) {
		fseek( fpInput, TrailerSize, SEEK_CUR );
	} else {
		fread( buff, 1, static_cast<ALS_UINT32>( TrailerSize ), fpInput );
		if ( fwrite( buff, 1, static_cast<ALS_UINT32>( TrailerSize ), fpOutput ) != static_cast<ALS_UINT32>( TrailerSize ) ) return ( frames = -1 );
	}

	if (CRCenabled)
	{
		CRC ^= CRC_MASK;
		WriteUIntMSBfirst(CRC, fpOutput);
	}

	if (RA && (RAflag == 2))		// save random access info in header
	{
		for (r = 0; r < RAUnits; r++)
			WriteUIntMSBfirst(RAUsize[r], fpOutput);
	}

	fseek(fpOutput, 0, SEEK_END);

	return TrailerSize;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Generate and write header, encoded audio, and trailing non-audio data
short CLpacEncoder::EncodeAll()
{
	long f;
	ENCINFO encinfo;

	if ((frames = WriteHeader(&encinfo)) < 0)
		return static_cast<short>( frames );

	for (f = 0; f < frames; f++)
	{
		if (EncodeFrame())
			return(-2);
	}

	if (WriteTrailer() < 0)
		return(-2);

	return(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// SetXXX(): Set encoder options
long CLpacEncoder::SetFrameLength(long N_x)
{
	if ((N_x < 32) || (N_x > 65536))
		return(N = 0);
	else
		return(N = N_x);
}

short CLpacEncoder::SetJoint(short Joint_x)
{
	if (Joint_x < -1)
		return(Joint = -1);
	else
		return(Joint = Joint_x);
}

short CLpacEncoder::SetOrder(short P_x)
{
	if ((P_x < 0) || (P_x > 1023))
		return(P = 10);
	else
		return(P = P_x);
}

short CLpacEncoder::SetAdapt(short Adapt_x)
{
	if (Adapt_x != 1)
		return(Adapt = 0);
	else
		return(Adapt = 1);
}

short CLpacEncoder::SetRA(short RA_x)
{
	if (RA_x < -1)
		return(RA = 0);
	else
		return(RA = RA_x);
}

short CLpacEncoder::SetRAmode(short RAmode)
{
	if ((RAmode < 0) || (RAmode > 2))
		RAmode = 0;
	if (RAmode == 0)			// store random access info...
		return(RAflag = 1);		// ...in frames
	else if (RAmode == 1)
		return(RAflag = 2);		// ...in header
	else
		return(RAflag = 0);		// ...not at all
}

short CLpacEncoder::SetWin(short Win_x)
{
	if ((Win_x < 0) || (Win_x > 13))
		return(Win = 0);
	else
		return(Win = Win_x);
}

short CLpacEncoder::SetLSBcheck(short LSBcheck_x)
{
	if (LSBcheck_x != 1)
		return(LSBcheck = 0);
	else
		return(LSBcheck = 1);
}

short CLpacEncoder::SetRawAudio(short RawAudio_x)
{
	if (RawAudio_x != 1)
		return(RawAudio = 0);
	else
		return(RawAudio = 1);
};

long CLpacEncoder::SetChannels(long Chan_x)
{
	if ((Chan_x >= 1) && (Chan_x <= MAXCHAN)) //256
		return(Chan = Chan_x);
	else
		return(Chan);
};

unsigned char CLpacEncoder::SetSampleType(unsigned char SampleType_x)
{
	if ( SampleType_x <= 1 ) {
		if ( SampleType == 1 ) IntRes = IEEE754_PCM_RESOLUTION;
		else IntRes = Res;
		return ( SampleType = SampleType_x );
	} else {
		return SampleType;
	}
}

short CLpacEncoder::SetWordlength(short Res_x)
{
	if ((Res_x >= 1) && (Res_x <= 32)) {
		if ( SampleType == 1 ) IntRes = IEEE754_PCM_RESOLUTION;
		else IntRes = Res_x;
		return(Res = Res_x);
	} else {
		return(Res);
	}
};

long CLpacEncoder::SetFrequency(long Freq_x)
{
	if ((Freq_x >= 1) && (Freq_x <= 192000))
		return(Freq = Freq_x);
	else
		return(Freq);
};

short CLpacEncoder::SetMSBfirst(short MSBfirst_x)
{
	if (MSBfirst_x != 1)
		return(MSBfirst = 0);
	else
		return(MSBfirst = 1);
};

ALS_INT64 CLpacEncoder::SetHeaderSize(ALS_INT64 HeaderSize_x)
{
	return(HeaderSize = HeaderSize_x);
};

ALS_INT64 CLpacEncoder::SetTrailerSize(ALS_INT64 TrailerSize_x)
{
	return(TrailerSize = TrailerSize_x);
};

long CLpacEncoder::SetChanSort(long ChanSort_x, unsigned short *ChPos_x)
{
	long i;

	if (ChanSort_x <= 2)
		return(ChanSort = 0);
	else
	{
		ChPos = new unsigned short[ChanSort_x];
		for (i = 0; i < ChanSort_x; i++)
			ChPos[i] = ChPos_x[i] - 1;
		return(ChanSort = ChanSort_x);
	}
}

short CLpacEncoder::SetBGMC(short BGMC_x)
{
    if (BGMC_x != 1)
        return(BGMC = 0);
    else
        return(BGMC = 1);
}

long CLpacEncoder::SetMCC(long MCC_x)
{
	if (MCC_x > 0)
	{
		if ( (Chan%MCC_x) == 0 )
			return(MCC = (Chan/MCC_x));
		else if( Chan < MCC_x )
			return(MCC = 1);
		else
			return(MCC = 0);
	}
	else
		return(MCC = 0);
}

short CLpacEncoder::SetHEMode(short RLSLMS_x)
{
    if (RLSLMS_x == 0)
        return(RLSLMS = 0);
    else
	{
		{
			RLSLMS_ext=0x7;  //for 1st frame
		}
        return(RLSLMS = RLSLMS_x);
	}
}

short CLpacEncoder::SetMCCnoJS(short MCCnoJS_x)
{
    if (MCCnoJS_x != 1)
        return(MCCnoJS = 0);
    else
        return(MCCnoJS = 1);
}

short CLpacEncoder::SetPITCH(short PITCH_x)
{
	if(PITCH_x != 1)
		return (PITCH = 0);
	else
		return(PITCH = 1);
}


short CLpacEncoder::SetSub(short Sub_x)
{
	if ((Sub_x >= 0) && (Sub_x <= 5))
		return(Sub = Sub_x);
	else
		return(Sub = 0);
};

short CLpacEncoder::SetAcf( short AcfMode_x, float AcfGain_x )
{
	AcfGain = AcfGain_x;
	if ( ( AcfMode_x >= 0 ) && ( AcfMode_x <= 3 ) ) return ( AcfMode = AcfMode_x );
	else return ( AcfMode = 0 );
}

short CLpacEncoder::SetMlz( short MlzMode_x )
{
	if ( MlzMode_x != 1 ) return ( MlzMode = 0 );
	else return ( MlzMode = 1 );
}

short CLpacEncoder::SetCRC(short CRCenabled_x)
{
	if (CRCenabled_x)
		return (CRCenabled = 1);
	else
		return(CRCenabled = 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Encode one frame
short CLpacEncoder::EncodeFrame()
{
	long bytes_1, bytes_2 = 0, bytes_3, oaa=0;		// Bytes for blocks 1, 2, difference
	long bpf_total = 0;						// Bytes for frame
	long bpf_total_m = 0;						// Bytes for frame
	static unsigned long ra_bytes = 0;		// Bytes for all frames of a RA unit
	short RAsave, RAframe = 0;
	long cpe, sce, c0, c1, c, c2;
	unsigned long bytes_diff;

	long bpf[6];							// Bytes per frame [level]
	long bpb[6][32];						// Bytes per block [level][block]
	short b, Bsub, a, B, CBS;
	long i, NN, Nrem, Nb;

	int **xsave, **xssave, **xtmp;
	long *bytes_MCC;
	xsave = new int*[Chan];
	xssave = new int*[(long)Chan/2];
	xtmp = new int*[Chan];
	bytes_MCC = new long[Chan];

	long bpbi_total;						// Bytes per frame (total)
	long bpfi[2][6];						// Bytes per frame [channel][level], independent channel coding
	long bpbi[2][6][32];					// Bytes per block [channel][level][block], independent channel coding
	BYTE *bufferi[2][6];					// Buffer for independent channels (locally allocated and deleted)
	short CheckIC = 1;						// Check independent coding (including block switching) of channel pairs
    short RESET;
	long tmp;
	if (!Joint)
		CheckIC = 0;						// if Joint is off, independent coding is used anyhow

	BYTE *buffer0 = buffer[0];		// store original address of buffer[0]
	BYTE *buffer0_m= buffer_m;
	
	// Block switching level
	Bsub = Sub;
	if ( !Adapt ) {
		// Decrease the block switching level to satisfy: P <= (N >> Bsub)
		while ( P > (N >> Bsub) ) --Bsub;
	}

	NN = N;						// Original frame length (N might be changed below)

	// Set length of last frame
	fid++;						// Number of current frame
	if (fid == frames)			// Last frame
		N = N0;

	// Read audio data
	if ( SampleType == SAMPLE_TYPE_INT ) {
		if (Res == 16)
		{
			Read16BitNM(x, Chan, N, MSBfirst, bbuf, fpInput);
			CRC = CalculateBlockCRC32(2L*Chan*N, CRC, (void*)bbuf);
		}
		else if (Res == 8)
		{
			Read8BitOffsetNM(x, Chan, N, bbuf, fpInput);
			CRC = CalculateBlockCRC32((long)Chan*N, CRC, (void*)bbuf);
		}
		else if (Res == 24)
		{
			Read24BitNM(x, Chan, N, MSBfirst, bbuf, fpInput);
			CRC = CalculateBlockCRC32(3L*Chan*N, CRC, (void*)bbuf);
		}
		else	// Res == 32
		{
			Read32BitNM(x, Chan, N, MSBfirst, bbuf, fpInput);
			CRC = CalculateBlockCRC32(4L*Chan*N, CRC, (void*)bbuf);
		}
	} else {
		// floating-point
		ReadFloatNM( x, Chan, N, MSBfirst, bbuf, fpInput, Float.GetFloatBuffer() );
		CRC = CalculateBlockCRC32( sizeof(float) * Chan * N, CRC, (void*)bbuf );
	}

	if (ChanSort)
	{
		// Rearrange channel pointers
		for (c = 0; c < Chan; c++)
			xtmp[c] = x[c];
		for (c = 0; c < Chan; c++)
			x[c] = xtmp[ChPos[c]];
		if ( SampleType == SAMPLE_TYPE_FLOAT ) Float.ChannelSort( ChPos, true );
	}

	// Random Access
	RAsave = RA;
	if (RA)
	{
		if (((fid - 1) % RA) == 0)	// first frame of RA unit
		{
			if (RAflag == 1) 			// save random access info in frame
			{
				if (fid > 1)
				{
					// save size of RAU before its first frame
					fseek(fpOutput, -(long)ra_bytes - 4, SEEK_CUR);	// back to the last RAU
					WriteUIntMSBfirst(ra_bytes, fpOutput);			// write size
					fseek(fpOutput, ra_bytes, SEEK_CUR);			// forward to current frame
				}
				WriteUIntMSBfirst(ra_bytes, fpOutput);			// write 4 dummy bytes for current RAU
			}
			else if (RAflag == 2)		// save random access info in header
			{
				if (fid > 1)
				{
					RAUsize[RAUid] = ra_bytes;		
					RAUid++;
				}
			}

			ra_bytes = 0;									// start counting bytes of current RAU
			RAframe = 1;									// flag for RA usage in current frame
		}
		else
			RA = 0;		// turn off RA in current frame
	}

	if ( SampleType == SAMPLE_TYPE_FLOAT ) {
		Float.ConvertFloatToInteger( x, N, RA != 0, AcfMode, AcfGain, MlzMode );
		if ( !Float.FindDiffFloatPCM( x, N ) ) return 1;
	}

	if (RLSLMS)
	{
		if (RAframe) RLSLMS_ext=7;
		initCoefTable(RLSLMS, CoefTable);
		RESET = (RLSLMS_ext==7);
	}

	//-------------------No Multi-channel corr. mode-------------------
	if (!MCCnoJS && !RLSLMS)
	{
		MCCflag=0;
		if (CheckIC)
		{
			// allocate memory locally
			for (short s = 0; s <= Sub; s++)
			{
				//bufferi[0][s] = new unsigned char[4L*N];	// Buffer for short frames
				//bufferi[1][s] = new unsigned char[4L*N];	// Buffer for short frames
				bufferi[0][s] = new unsigned char[((long)((IntRes+7)/8)+1)*N + 4*P + 128];	// Buffer for short frames
				bufferi[1][s] = new unsigned char[((long)((IntRes+7)/8)+1)*N + 4*P + 128];	// Buffer for short frames
			}
		}

		// Save original pointers
		for (c = 0; c < Chan; c++)
			xsave[c] = x[c];
		if (Joint)
		{
			for (c = 0; c < Chan/2; c++)
				xssave[c] = xs[c];
		}

		// Channels ///////////////////////////////////////////////////////////////////////////////////
		for (c = 0; c < Chan; c++)
		{
			// Coupled block switching if joint coding, and not the last channel
			CBS = (Joint && (c < Chan - 1) && ((c % 2) == 0));

			// Block switching levels /////////////////////////////////////////////////////////////////
			for (a = 0; a <= Bsub; a++)
			{
				bpf[a] = 0;

				if (CheckIC)
				{
					bpfi[0][a] = 0;
					bpfi[1][a] = 0;
				}

				B = 1 << a;			// number of blocks = 2^a
				Nb = NN / B;		// basic block length for this level

				// Last frame
				if (fid == frames)
				{
					B = N0 / Nb;		// #blocks of (full) length Nb
					Nrem = N0 % Nb;		// one block of (remaining) length Nrem
					if (Nrem)
						B++;			// increase total #blocks
				}

				if (CBS)	// two coupled channels
				{
					c1 = c + 1;
					c2 = c >> 1;

					// Blocks /////////////////////////////////////////////////////////////////////////
					for (b = 0; b < B; b++)
					{
						bpb[a][b] = 0;

						// Last block of last frame may be shorter 
						if ((fid == frames) && (b == B - 1) && Nrem)
							Nb = Nrem;

						N = Nb;

						if (RAframe && (b > 0))		// turn off RA temporarily, except for the first block 
							RA = 0;

						bytes_1 = EncodeBlock(x[c], tmpbuf1);
						bytes_2 = EncodeBlock(x[c1], tmpbuf2);

						if (CheckIC)
						{
							// byte per block
							bpbi[0][a][b] = bytes_1;
							bpbi[1][a][b] = bytes_2;
							// copy block data into frame buffer
							memcpy(bufferi[0][a] + bpfi[0][a], tmpbuf1, bytes_1);
							memcpy(bufferi[1][a] + bpfi[1][a], tmpbuf2, bytes_2);
							// increase bytes per frame value
							bpfi[0][a] += bytes_1;
							bpfi[1][a] += bytes_2;
						}

						// Generate difference signal
						for (i = 0; i < Nb; i++)
							xs[c2][i] = x[c1][i] - x[c][i];

						if ((bytes_1 > 3) && (bytes_2 > 3))			// No channel is zero or constant
						{
							bytes_3 = EncodeBlock(xs[c2], tmpbuf3);		// Encode difference signal

							if ((bytes_3 < bytes_1) || (bytes_3 <= bytes_2))
							{
								BYTE h = tmpbuf3[0];
								if (h & 0x80)						// Difference signal is not zero/constant
									tmpbuf3[0] |= 0x40;					// h = 11xx xxxx
								else								// Difference signal is zero or constant
									tmpbuf3[0] |= 0x20;					// h = 0x1x xxxx

								if (bytes_1 <= bytes_2)
								{
									memcpy(tmpbuf2, tmpbuf3, bytes_3);		// Difference substitutes channel 2
									bytes_2 = bytes_3;
								}
								else
								{
									memcpy(tmpbuf1, tmpbuf3, bytes_3);		// Difference substitutes channel 1
									bytes_1 = bytes_3;
								}
							}
						}

						// Write data to buffer
						memcpy(buffer[a] + bpf[a], tmpbuf1, bytes_1);
						memcpy(buffer[a] + bpf[a] + bytes_1, tmpbuf2, bytes_2);
						bpf[a] += long(bytes_1) + bytes_2;
						bpb[a][b] += long(bytes_1) + bytes_2;

						// Increment pointers (except for last subblock)
						if (b < B - 1)
						{
							x[c] = x[c] + Nb;
							x[c1] = x[c1] + Nb;
							if (Joint)
								xs[c2] = xs[c2] + Nb;
						}

						N = NN;		// restore value
					}
					// End of blocks //////////////////////////////////////////////////////////////////

					// Restore original pointers
					x[c] = xsave[c];
					x[c1] = xsave[c1];
					if (Joint)
						xs[c2] = xssave[c2];
				}
				else	// one independent channel
				{
					// Blocks /////////////////////////////////////////////////////////////////////////
					for (b = 0; b < B; b++)
					{
						bpb[a][b] = 0;

						// Last block of last frame may be shorter 
						if ((fid == frames) && (b == B - 1) && Nrem)
							Nb = Nrem;

						N = Nb;

						if (RAframe && (b > 0))		// turn off RA temporarily, except for the first block 
							RA = 0;

						bytes_1 = EncodeBlock(x[c], tmpbuf1);

						// Write data to buffer
						memcpy(buffer[a] + bpf[a], tmpbuf1, bytes_1);
						bpf[a] += long(bytes_1);
						bpb[a][b] += long(bytes_1);

						// Increment pointers (except for last subblock)
						if (b < B - 1)
							x[c] = x[c] + Nb;

						N = NN;				// restore value
					}
					// End of blocks //////////////////////////////////////////////////////////////////

					// Restore original pointers
					x[c] = xsave[c];
				}

				if (RAframe)		// turn on RA again in RA frames
					RA = RAsave;
			}
			// End of block switching levels //////////////////////////////////////////////////////////

			// Chose best partition and assign frame buffer ///////////////////////////////////////////
			long tmp, tmp_dest, tmp_src, tmp_org, bits[16];
			UINT BSflags, BSflagsi[2];
			unsigned short bshift, B1, Nb1;

			BSflags = 0;

			for (a = Bsub; a > 0; a--)		// levels (shortest to longest blocks)
			{
				tmp_dest = 0;
				tmp_src = 0;
				tmp_org = 0;

				B = (1 << (a-1));
				bshift = B - 1;

				if (fid == frames)	// last frame
				{
					// adjust B value
					Nb = NN / B;		// basic block length for upper level (a-1)
					B = N0 / Nb;		// #blocks of (full) length Nb
					if (N0 % Nb)		// if remainder...
						B++;			// ...increase total #blocks
					// calculate #blocks for lower level (a)
					B1 = (1 << a);
					Nb1 = NN / B1;
					B1 = N0 / Nb1;
					if (N0 % Nb1)
						B1++;
				}

				for (b = 0; b < B; b++)		// blocks
				{
					tmp = 0;
					if ((fid == frames) && (b == (B-1)) && ((B<<1) > B1))	// last block of last frame
					{
						// copy last block from lower level (a) to upper level (a-1)
						bits[b] = bpb[a][B1-1];
						memcpy(buffer[a-1] + tmp_dest, buffer[a] + tmp_src, bpb[a][B1-1]);
						BSflags |= (0x40000000 >> (bshift + b));			// 01223333 44444444 55555555 55555555
					}
					// Compare levels a and a-1
					else if (bpb[a-1][b] > (tmp = bpb[a][2*b] + bpb[a][2*b+1]))	// two short blocks need less bits
					{
						bits[b] = tmp;
						memcpy(buffer[a-1] + tmp_dest, buffer[a] + tmp_src, tmp);	// copy two short blocks into superior block
						BSflags |= (0x40000000 >> (bshift + b));			// 01223333 44444444 55555555 55555555
					}
					else													// one long block needs less bits
					{
						bits[b] = bpb[a-1][b];
						if (tmp_dest != tmp_org)
							memmove(buffer[a-1] + tmp_dest, buffer[a-1] + tmp_org, bpb[a-1][b]);
					}
					tmp_dest += bits[b];									// increment position in destination buffer
					tmp_src += tmp;											// increment position in source buffer
					tmp_org += bpb[a-1][b];
				}
				for (b = 0; b < B; b++)
					bpb[a-1][b] = bits[b];
			}
			// end of partition choice ////////////////////////////////////////////////////////////////

			// check independent coding as well ///////////////////////////////////////////////////////
			if (CheckIC && CBS)
			{
				long bitsi[2][16];
				short ch;
				for (ch = 0; ch < 2; ch++)			// channels
				{
					BSflagsi[ch] = 0;

					for (a = Bsub; a > 0; a--)		// levels
					{
						tmp_dest = 0;
						tmp_src = 0;
						tmp_org = 0;

						B = (1 << (a-1));
						bshift = B - 1;

						if (fid == frames)	// last frame
						{
							// adjust B value
							Nb = NN / B;		// basic block length for upper level (a-1)
							B = N0 / Nb;		// #blocks of (full) length Nb
							if (N0 % Nb)		// if remainder...
								B++;			// ...increase total #blocks
							// calculate #blocks for lower level (a)
							B1 = (1 << a);
							Nb1 = NN / B1;
							B1 = N0 / Nb1;
							if (N0 % Nb1)
								B1++;
						}

						for (b = 0; b < B; b++)			// blocks
						{
							tmp = 0;
							if ((fid == frames) && (b == (B-1)) && ((B<<1) > B1))	// last block of last frame
							{
								// copy last block from lower level (a) to upper level (a-1)
								bitsi[ch][b] = bpbi[ch][a][B1-1];
								memcpy(bufferi[ch][a-1] + tmp_dest, bufferi[ch][a] + tmp_src, bpbi[ch][a][B1-1]);
								BSflagsi[ch] |= (0x40000000 >> (bshift + b));			// 01223333 44444444 55555555 55555555
							}
							// Compare levels a and a-1
							else if (bpbi[ch][a-1][b] > (tmp = bpbi[ch][a][2*b] + bpbi[ch][a][2*b+1]))	// two short blocks need less bits
							{
								bitsi[ch][b] = tmp;
								memcpy(bufferi[ch][a-1] + tmp_dest, bufferi[ch][a] + tmp_src, tmp);	// copy two short blocks into superior block
								BSflagsi[ch] |= (0x40000000 >> (bshift + b));			// 01223333 44444444 55555555 55555555
							}
							else
							{
								bitsi[ch][b] = bpbi[ch][a-1][b];
								if (tmp_dest != tmp_org)
									memmove(bufferi[ch][a-1] + tmp_dest, bufferi[ch][a-1] + tmp_org, bpbi[ch][a-1][b]);
							}
							tmp_dest += bitsi[ch][b];									// increment position in destination buffer
							tmp_src += tmp;											// increment position in source buffer
							tmp_org += bpbi[ch][a-1][b];
						}
						for (b = 0; b < B; b++)
							bpbi[ch][a-1][b] = bitsi[ch][b];
					}
				}
			}
			// end of independent coding check ////////////////////////////////////////////////////////

			// Compose frame data 
			if (Sub)
			{
				// Use buffer[1] to rearrange data
				short BSbits = 1;
				buffer[1][0] = BSflags >> 24;
				if (Sub > 3)
				{
					BSbits = 2;
					buffer[1][1] = (BSflags >> 16) & 0xFF;
				}
				if (Sub == 5)
				{
					BSbits = 4;
					buffer[1][2] = (BSflags >> 8) & 0xFF;
					buffer[1][3] = BSflags & 0xFF;
				}

				if (CheckIC && CBS)
				{
					bpbi_total = bpbi[0][0][0] + bpbi[1][0][0];
					
					if (bpbi_total + BSbits < bpb[0][0])	// if independent coding is benificial...
					{
						long off = BSbits + bpbi[0][0][0];		// offset between channels 0 and 1

						// copy encoded data for both channels
						memcpy(buffer[1] + BSbits, bufferi[0][0], bpbi[0][0][0]);
						memcpy(buffer[1] + BSbits + off, bufferi[1][0], bpbi[1][0][0]);

						// set flags for both channels
						BSflagsi[0] |= 0x80000000;			// set msb to indicate independent block switching
						BSflagsi[1] |= 0x80000000;			// set msb to indicate independent block switching
						buffer[1][0] = BSflagsi[0] >> 24;
						buffer[1][off] = BSflagsi[1] >> 24;
						if (Sub > 3)
						{
							buffer[1][1] = (BSflagsi[0] >> 16) & 0xFF;
							buffer[1][off+1] = (BSflagsi[1] >> 16) & 0xFF;
						}
						if (Sub == 5)
						{
							buffer[1][2] = (BSflagsi[0] >> 8) & 0xFF;
							buffer[1][3] = BSflagsi[0] & 0xFF;
							buffer[1][off+2] = (BSflagsi[1] >> 8) & 0xFF;
							buffer[1][off+3] = BSflagsi[1] & 0xFF;
						}
						bpb[0][0] = bpbi_total + BSbits;
					}
					else	// use coupled block switching
						memcpy(buffer[1] + BSbits, buffer[0], bpb[0][0]);	// copy encoded block data
				}
				else	// coupled block switching or single channel
					memcpy(buffer[1] + BSbits, buffer[0], bpb[0][0]);	// copy encoded block data

				memcpy(buffer[0], buffer[1], bpb[0][0] + BSbits);	// copy data in buffer[0] again
				buffer[0] += bpb[0][0] + BSbits;					// increment pointer
				bpf_total += bpb[0][0] + BSbits;					// frame size so far
			}
			else	// no block switching
			{
				buffer[0] += bpb[0][0];
				bpf_total += bpb[0][0];
			}
			// increment channel index if two channels have been processed
			if (CBS)
				c++;
		}
		// End of Channels ////////////////////////////////////////////////////////////////////////////

		// Restore original pointers
		buffer[0] = buffer0;

		for (c = 0; c < Chan; c++){
			
			x[c] = xsave[c];
		}
		if (Joint)
		{
			for (c = 0; c < Chan/2; c++)
				xs[c] = xssave[c];
		}

	}
    else if (RLSLMS)//------------RLSLMS mode --------------------------
	{
		MCCflag=0;
		if (CheckIC)
		{
			// allocate memory locally
			for (short s = 0; s <= Sub; s++)
			{
				bufferi[0][s] = new unsigned char[4L*N];	// Buffer for short frames
				bufferi[1][s] = new unsigned char[4L*N];	// Buffer for short frames
			}
		}
		// Channel Pair Elements
		for (cpe = 0; cpe < CPE; cpe++)
		{
			bpf[0]=0;
			c0 = 2 * cpe;
			c1 = c0 + 1;

			if (Joint)
			{
				// Generate difference signal
				for (i = 0; i < N; i++)
					xs[cpe][i] = x[c1][i] - x[c0][i];
			}
			if (!Joint)
			{

				if (RLSLMS)   // independent channel
				{
					for(i=0;i<N;i++) MccBuf.m_dmat[c0][i]=x[c0][i];		// Save input
					rlslms_ptr.channel = c0;
					analyze(x[c0], N, &rlslms_ptr ,RAframe || RESET, IntRes, &MccBuf);
					for(i=0;i<N;i++)									// Restore input
					{
						tmp = MccBuf.m_dmat[c0][i];
						MccBuf.m_dmat[c0][i]=x[c0][i];
						x[c0][i] = tmp;
					}
					if(PITCH) 
					{
						memset( MccBuf.m_Ltp.m_pBuffer[c0].m_ltpmat, 0, sizeof(int) * 2048 );
						LTPanalysis(&MccBuf, c0, N, 10, x[c0]);
					}					
					bytes_1 = EncodeBlockCoding( &MccBuf, c0, MccBuf.m_dmat[c0], tmpbuf1, 0);
					RLSLMS_ext=0;
					for(i=0;i<N;i++) MccBuf.m_dmat[c1][i]=x[c1][i];		// Save input
					rlslms_ptr.channel = c1; 
					analyze(x[c1], N, &rlslms_ptr, RAframe || RESET, IntRes, &MccBuf);
					for(i=0;i<N;i++)									// Restore input
					{
						tmp = MccBuf.m_dmat[c1][i];
						MccBuf.m_dmat[c1][i]=x[c1][i];
						x[c1][i] = tmp;
					}
					if(PITCH) 
					{
						memset( MccBuf.m_Ltp.m_pBuffer[c1].m_ltpmat, 0, sizeof(int) * 2048 );
						LTPanalysis(&MccBuf, c1, N, 10, x[c1]);
					}
					bytes_2 = EncodeBlockCoding( &MccBuf, c1, MccBuf.m_dmat[c1], tmpbuf2, 0);

					if (bytes_1>N*IntRes/8 || bytes_2>N*IntRes/8)
					{
						// Copy safe_mode_table
						memcpy(&c_mode_table, &safe_mode_table, sizeof(mtable));
						RLSLMS_ext = 7;
						RESET = 1;

						// Redo everything
						rlslms_ptr.channel = c0;
						analyze(x[c0], N, &rlslms_ptr ,RAframe || RESET, IntRes, &MccBuf);
						for(i=0;i<N;i++) MccBuf.m_dmat[c0][i]=x[c0][i];
						if(PITCH) 
						{
							memset( MccBuf.m_Ltp.m_pBuffer[c0].m_ltpmat, 0, sizeof(int) * 2048 );
							LTPanalysis(&MccBuf, c0, N, 10, x[c0]);
						}					
						bytes_1 = EncodeBlockCoding( &MccBuf, c0, MccBuf.m_dmat[c0], tmpbuf1, 0);
						rlslms_ptr.channel = c1; 
						analyze(x[c1], N, &rlslms_ptr, RAframe || RESET, IntRes, &MccBuf);
						for(i=0;i<N;i++) MccBuf.m_dmat[c1][i]=x[c1][i];
						if(PITCH) 
						{
							memset( MccBuf.m_Ltp.m_pBuffer[c1].m_ltpmat, 0, sizeof(int) * 2048 );
							LTPanalysis(&MccBuf, c1, N, 10, x[c1]);
						}
						bytes_2 = EncodeBlockCoding( &MccBuf, c1, MccBuf.m_dmat[c1], tmpbuf2, 0);
					}
				}
			}
			if (Joint)						// Joint Stereo
			{
				if (RLSLMS)
				{
					char *xpr0=&MccBuf.m_xpara[c0];
					char *xpr1=&MccBuf.m_xpara[c1];

					for(i=0;i<N;i++)						// Save input
					{
						MccBuf.m_dmat[c0][i]=x[c0][i];
						MccBuf.m_dmat[c1][i]=x[c1][i];
					}
					rlslms_ptr.channel = c0;
					analyze_joint(x[c0],x[c1],N, &rlslms_ptr, RAframe || RESET, IntRes, &mono_frame, &MccBuf);
					for(i=0;i<N;i++)						// Restore input
					{
						tmp = MccBuf.m_dmat[c0][i];
						MccBuf.m_dmat[c0][i]=x[c0][i];
						x[c0][i] = tmp;

						tmp = MccBuf.m_dmat[c1][i];
						MccBuf.m_dmat[c1][i]=x[c1][i];
						x[c1][i] = tmp;
					}
					if(PITCH) 
					{
						memset( MccBuf.m_Ltp.m_pBuffer[c0].m_ltpmat, 0, sizeof(int) * 2048 );
						LTPanalysis(&MccBuf, c0, N, 10, x[c0]);
					}
					bytes_1 = EncodeBlockCoding( &MccBuf, c0, MccBuf.m_dmat[c0], tmpbuf1, 0);
					RLSLMS_ext=0;
					if(PITCH) 
					{
						memset( MccBuf.m_Ltp.m_pBuffer[c1].m_ltpmat, 0, sizeof(int) * 2048 );
						LTPanalysis(&MccBuf, c1, N, 10, x[c1]);
					}
					bytes_2 = EncodeBlockCoding( &MccBuf, c1, MccBuf.m_dmat[c1], tmpbuf2, 0);

					if (bytes_1>N*IntRes/8 || bytes_2>N*IntRes/8)
					{
						// Copy safe_mode_table
						memcpy(&c_mode_table, &safe_mode_table, sizeof(mtable));
						RLSLMS_ext = 7;
						RESET = 1;

						// Redo everthing
						xpr0=&MccBuf.m_xpara[c0];
						xpr1=&MccBuf.m_xpara[c1];

						rlslms_ptr.channel = c0;
						analyze_joint(x[c0],x[c1],N, &rlslms_ptr, RAframe || RESET, IntRes, &mono_frame, &MccBuf);
						for(i=0;i<N;i++) MccBuf.m_dmat[c0][i]=x[c0][i];
						if(PITCH) 
						{
							memset( MccBuf.m_Ltp.m_pBuffer[c0].m_ltpmat, 0, sizeof(int) * 2048 );
							LTPanalysis(&MccBuf, c0, N, 10, x[c0]);
						}
						bytes_1 = EncodeBlockCoding( &MccBuf, c0, MccBuf.m_dmat[c0], tmpbuf1, 0);
						for(i=0;i<N;i++) MccBuf.m_dmat[c1][i]=x[c1][i];
						if(PITCH) 
						{
							memset( MccBuf.m_Ltp.m_pBuffer[c1].m_ltpmat, 0, sizeof(int) * 2048 );
							LTPanalysis(&MccBuf, c1, N, 10, x[c1]);
						}
						bytes_2 = EncodeBlockCoding( &MccBuf, c1, MccBuf.m_dmat[c1], tmpbuf2, 0);
					}

					if (bytes_1<0 || bytes_2<0) 
					{
						printf("ALS file size error com %ld %ld \t",bytes_1,bytes_2);
						exit(3);
					}
				}
			}
			// Write data to buffer
			memcpy(buffer[0] + bpf_total, tmpbuf1, bytes_1);
			memcpy(buffer[0] + bpf_total + bytes_1, tmpbuf2, bytes_2);
			bpf_total += bytes_1 + bytes_2;
		}

		// Single Channel Elements
		for (sce = 0; sce < SCE; sce++)
		{
			c0 = sce+2*CPE;
			for(i=0;i<N;i++) MccBuf.m_dmat[c0][i]=x[c0][i];		// Save input
			rlslms_ptr.channel = c0;
			analyze(x[c0], N, &rlslms_ptr, RAframe || RESET, IntRes, &MccBuf);
			for(i=0;i<N;i++)									// Restore input
			{
				tmp = MccBuf.m_dmat[c0][i];
				MccBuf.m_dmat[c0][i]=x[c0][i];
				x[c0][i] = tmp;
			}
			if(PITCH) 
			{
				memset( MccBuf.m_Ltp.m_pBuffer[c0].m_ltpmat, 0, sizeof(int) * 2048 );
				LTPanalysis(&MccBuf, c0, N, 10, x[c0]);
			}
			bytes_1 = EncodeBlockCoding( &MccBuf, c0, MccBuf.m_dmat[c0], tmpbuf1, 0);
			RLSLMS_ext=0;

			if (bytes_1>N*IntRes/8)
			{
				// Copy safe_mode_table
				memcpy(&c_mode_table, &safe_mode_table, sizeof(mtable));
				RLSLMS_ext = 7;
				RESET = 1;

				// Redo everything
				c0 = sce+2*CPE;	
				rlslms_ptr.channel = c0;
				analyze(x[c0], N, &rlslms_ptr, RAframe || RESET, IntRes, &MccBuf);
				for(i=0;i<N;i++) MccBuf.m_dmat[c0][i]=x[c0][i];
				if(PITCH) 
				{
					memset( MccBuf.m_Ltp.m_pBuffer[c0].m_ltpmat, 0, sizeof(int) * 2048 );
					LTPanalysis(&MccBuf, c0, N, 10, x[c0]);
				}
				bytes_1 = EncodeBlockCoding( &MccBuf, c0, MccBuf.m_dmat[c0], tmpbuf1, 0);
			}

			// Write data to buffer
			memcpy(buffer[0] + bpf_total, tmpbuf1, bytes_1);
			bpf_total += bytes_1;
		}
		if (RAframe)		// turn on RA again in RA frames
			RA = RAsave;
	}
	
	//-------------End of Normal mode------------------------
	//-------Multi-channel correlation method----------

	//do both JS and MCC or only MCC
	if (MCC) 
	{
		if(!MCCnoJS)
		{
			memcpy(buffer_m, buffer[0], bpf_total);
			bpf_total_m=bpf_total;
			bpf_total=0;
		}

		MCCflag=MCC;

		// Save original pointers
		for (c = 0; c < Chan; c++)
			xsave[c] = x[c];
			
		
		// Block switching levels /////////////////////////////////////////////////////////////////
		for (a = 0; a <= Bsub; a++)
		{

			bpf[a] = 0;

			B = 1 << a;			// number of blocks = 2^a
			Nb = NN / B;		// basic block length for this level

			// Last frame
			if (fid == frames)
			{
				B = N0 / Nb;		// #blocks of (full) length Nb
				Nrem = N0 % Nb;		// one block of (remaining) length Nrem
				if (Nrem)
					B++;			// increase total #blocks
			}

			// Blocks /////////////////////////////////////////////////////////////////////////
			for (b = 0; b < B; b++)
			{
				bpb[a][b] = 0;
		
				// Last block of last frame may be shorter 
				if ((fid == frames) && (b == B - 1) && Nrem)
					Nb = Nrem;

				N = Nb;

				if (RAframe && (b > 0))		// turn off RA temporarily, except for the first block 
					RA = 0;


				// Initializing
				InitMccEncBuffer( &MccBuf );

				for(c = 0; c < Chan; c++)
				{
					EncodeBlockAnalysis( &MccBuf, c, x[c] );
					memcpy( MccBuf.m_stdmat[c], MccBuf.m_dmat[c], N * sizeof(int) );
					memcpy( MccBuf.m_orgdmat[c], MccBuf.m_dmat[c], N * sizeof(int) );
				}

				short	tt,TauTap=2,*ttOPT,mtp;
				long *ttMinBytes;
				ttOPT=new short[Chan];
				ttMinBytes=new long[Chan];
				int **stackmtgmm,*stdtau;
				stackmtgmm=new int*[Chan];
				for(c = 0; c < Chan; c++)
					stackmtgmm[c]=new int[Mtap];
				stdtau=new int[Chan];

				for(c = 0; c < Chan; c++)
				{
					ttOPT[c]=0;
					MccBuf.m_MccMode[c][oaa] = 0;
					ttMinBytes[c] = EncodeBlockCoding( &MccBuf, c, MccBuf.m_dmat[c], MccBuf.m_tmpbuf1, 0);
					for(mtp = 0; mtp < Mtap; mtp++) MccBuf.m_cubgmm[c][oaa][mtp]=16;
					MccBuf.m_tdtau[c][oaa]=0;
					MccBuf.m_puchan[c][oaa]=c;
				}

				CheckFrameDistanceTD( &MccBuf, Chan, N, MCCflag);	// Calculate channel correlation

				for(c = 0; c < Chan; c++)
					MccBuf.m_puchan[c][oaa]=MccBuf.m_tmppuchan[c];

				for(tt = 1; tt < TauTap+1; tt++)
				{
					for(c = 0; c < Chan; c++)
					{
						memcpy( MccBuf.m_stdmat[c], MccBuf.m_dmat[c], N * sizeof(int) );
						for(mtp = 0; mtp < Mtap; mtp++) stackmtgmm[c][mtp]=MccBuf.m_cubgmm[c][oaa][mtp];
						stdtau[c] = MccBuf.m_tdtau[c][oaa];
						memcpy( MccBuf.m_dmat[c], MccBuf.m_orgdmat[c], N * sizeof(int) );
					}

					SubtractResidualTD( &MccBuf, Chan, N ,tt);			// Slave channel - Master channel

					// Channel loop
					for(c = 0; c < Chan; c++)
					{
						MccBuf.m_MccMode[c][oaa] = tt;
						bytes_2 = EncodeBlockCoding( &MccBuf, c, MccBuf.m_dmat[c], MccBuf.m_tmpbuf2,1);
			
						if ( bytes_2 < ttMinBytes[c] )
						{
							MccBuf.m_gmmodr[c] = 1;
							for(mtp = 0; mtp < Mtap; mtp++) MccBuf.m_cubgmm[c][oaa][mtp]=MccBuf.m_mtgmm[c][mtp];
							MccBuf.m_tdtau[c][oaa] = MccBuf.m_tmptdtau[c];
							ttOPT[c]=tt;
							ttMinBytes[c]=bytes_2;
						}
						else
						{
							memcpy( MccBuf.m_dmat[c], MccBuf.m_stdmat[c], N * sizeof(int) );
							for(mtp = 0; mtp < Mtap; mtp++) MccBuf.m_cubgmm[c][oaa][mtp]=stackmtgmm[c][mtp];
							MccBuf.m_tdtau[c][oaa] = stdtau[c];
						}
					}//Channel Loop
				}//for(tt=1;tt<TauTap+1;tt++)

				for(c = 0; c < Chan; c++)
				{
					MccBuf.m_MccMode[c][oaa] = ttOPT[c];
					if(!ttOPT[c]) MccBuf.m_puchan[c][oaa]=c;
				}
				

				for(c = 0; c < Chan; c++)
				{						
					bytes_MCC[c] = EncodeBlockCoding( &MccBuf, c, MccBuf.m_dmat[c], tmpbuf_MCC[c], MccBuf.m_gmmodr[c]);

					// Write data to buffer
					memcpy(buffer[a] + bpf[a], tmpbuf_MCC[c], bytes_MCC[c]);
					bpf[a] += (long) bytes_MCC[c];
					bpb[a][b] += (long) bytes_MCC[c];

					// Increment pointers (except for last subblock)
					if (b < B - 1)
						x[c] = x[c] + Nb;
				}

				delete [] ttOPT;
				delete [] ttMinBytes;
				for(c = 0; c < Chan; c++) delete [] stackmtgmm[c];
				delete [] stackmtgmm;
				delete [] stdtau;

				N = NN;		// restore value
			}
			// End of blocks //////////////////////////////////////////////////////////////////
			
			// Restore original pointers
			for(c = 0; c < Chan; c++)
				x[c] = xsave[c];
					

			if (RAframe)		// turn on RA again in RA frames
				RA = RAsave;
		}
		// End of block switching levels //////////////////////////////////////////////////////////

		// Chose best partition and assign frame buffer ///////////////////////////////////////////
		long tmp, tmp_dest, tmp_src, tmp_org, bits[16];
		UINT BSflags;
		unsigned short bshift, B1, Nb1;

		BSflags = 0;

		for (a = Bsub; a > 0; a--)		// levels (shortest to longest blocks)
		{
			tmp_dest = 0;
			tmp_src = 0;
			tmp_org = 0;

			B = (1 << (a-1));
			bshift = B - 1;

			if (fid == frames)	// last frame
			{
				// adjust B value
				Nb = NN / B;		// basic block length for upper level (a-1)
				B = N0 / Nb;		// #blocks of (full) length Nb
				if (N0 % Nb)		// if remainder...
					B++;			// ...increase total #blocks
				// calculate #blocks for lower level (a)
				B1 = (1 << a);
				Nb1 = NN / B1;
				B1 = N0 / Nb1;
				if (N0 % Nb1)
					B1++;
			}

			for (b = 0; b < B; b++)		// blocks
			{
				tmp = 0;
				if ((fid == frames) && (b == (B-1)) && ((B<<1) > B1))	// last block of last frame
				{
					// copy last block from lower level (a) to upper level (a-1)
					bits[b] = bpb[a][B1-1];
					memcpy(buffer[a-1] + tmp_dest, buffer[a] + tmp_src, bpb[a][B1-1]);
					BSflags |= (0x40000000 >> (bshift + b));			// 01223333 44444444 55555555 55555555
				}
				// Compare levels a and a-1
				else if (bpb[a-1][b] > (tmp = bpb[a][2*b] + bpb[a][2*b+1]))	// two short blocks need less bits
				{
					bits[b] = tmp;
					memcpy(buffer[a-1] + tmp_dest, buffer[a] + tmp_src, tmp);	// copy two short blocks into superior block
					BSflags |= (0x40000000 >> (bshift + b));			// 01223333 44444444 55555555 55555555
				}
				else													// one long block needs less bits
				{
					bits[b] = bpb[a-1][b];
					if (tmp_dest != tmp_org)
						memmove(buffer[a-1] + tmp_dest, buffer[a-1] + tmp_org, bpb[a-1][b]);
				}
				tmp_dest += bits[b];									// increment position in destination buffer
				tmp_src += tmp;											// increment position in source buffer
				tmp_org += bpb[a-1][b];
			}
			for (b = 0; b < B; b++)
				bpb[a-1][b] = bits[b];
		}
		// end of partition choice ////////////////////////////////////////////////////////////////

		// Compose frame data 
		if (Sub)
		{
			// Use buffer[1] to rearrange data
			short BSbits = 1;
			buffer[1][0] = BSflags >> 24;
			if (Sub > 3)
			{
				BSbits = 2;
				buffer[1][1] = (BSflags >> 16) & 0xFF;
			}
			if (Sub == 5)
			{
				BSbits = 4;
				buffer[1][2] = (BSflags >> 8) & 0xFF;
				buffer[1][3] = BSflags & 0xFF;
			}
			memcpy(buffer[1] + BSbits, buffer[0], bpb[0][0]);	// copy encoded block data
			memcpy(buffer[0], buffer[1], bpb[0][0] + BSbits);	// copy data in buffer[0] again
			buffer[0] += bpb[0][0] + BSbits;					// increment pointer
			bpf_total += bpb[0][0] + BSbits;					// frame size so far
		}
		else	// no block switching
		{
			buffer[0] += bpb[0][0];
			bpf_total += bpb[0][0];
		}

		// Restore original pointers
		buffer[0] = buffer0;

		for (c = 0; c < Chan; c++)
			x[c] = xsave[c];

		if(!MCCnoJS)
		{
			unsigned char uu;
			if(bpf_total_m < bpf_total)
			{
				memcpy(buffer[0], buffer_m, bpf_total_m);
				bpf_total= bpf_total_m;
				uu=0x80;
				fwrite(&uu, 1, 1 , fpOutput);
			}	
			else
			{
				uu=0;
				fwrite(&uu, 1, 1, fpOutput);
			}
		}

	}
	//-------------End of MCC mode------------------------


	if ( SampleType == SAMPLE_TYPE_FLOAT ) {
		bytes_diff = Float.EncodeDiff( ( fid == frames ) ? N0 : N, RA != 0, MlzMode );
		bpf_total += bytes_diff + 4;
	}

	// Restore original RA value
	RA = RAsave;

	// Copy last P samples of each block in front of it
	for (c = 0; c < Chan; c++)
		memcpy(x[c] - P, x[c] + (N - P), P * sizeof(int));
	if (Joint)
	{
		for (cpe = 0; cpe < CPE; cpe++)
			memcpy(xs[cpe] - P, xs[cpe] + (N - P), P * sizeof(int));
	}

	// Write frame buffer
	if ( SampleType == SAMPLE_TYPE_FLOAT ) {
		// Floating point PCM
		if ( fwrite( buffer[0], 1, bpf_total - 4 - bytes_diff, fpOutput ) != bpf_total - 4 - bytes_diff ) return 1;
		WriteUIntMSBfirst(bytes_diff, fpOutput);
		if ( fwrite( Float.GetDiffBuffer(), 1, bytes_diff, fpOutput ) != bytes_diff ) return 1;
	} else {
		// Integer PCM
		if (fwrite(buffer[0], 1, bpf_total, fpOutput) != bpf_total) return(1);
	}
	if(MCC && !MCCnoJS)  bpf_total++; // switch for JS and MCC

	if (RA)
		ra_bytes += bpf_total;

	if (RA && (fid == frames))	// Last frame
	{
		if (RAflag == 1)
		{
			// save size last RAU before the first frame of last RAU
			fseek(fpOutput, -(long)ra_bytes - 4, SEEK_CUR);		// back to last RAU
			WriteUIntMSBfirst(ra_bytes, fpOutput);				// write size
			fseek(fpOutput, ra_bytes, SEEK_CUR);				// forward to current frame
		}
		else if (RAflag == 2)
			RAUsize[RAUid] = ra_bytes;
	}

	if (ChanSort)
	{
		// Restore original channel pointers
		for (c = 0; c < Chan; c++)
			xtmp[c] = x[c];
		for (c = 0; c < Chan; c++)
			x[ChPos[c]] = xtmp[c];
		if ( SampleType == SAMPLE_TYPE_FLOAT ) Float.ChannelSort( ChPos, false );
	}

	if (CheckIC)
	{

		for (short s = 0; s <= Sub; s++)
		{
			delete [] bufferi[0][s];
			delete [] bufferi[1][s];
		}
	}

	delete [] xsave;
	delete [] xssave;
	delete [] xtmp;
	delete [] bytes_MCC;

	return(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Encode a single block
long CLpacEncoder::EncodeBlock(int *x, unsigned char *bytebuf)
{
	EncodeBlockAnalysis( &MccBuf, 0, x );
	return EncodeBlockCoding( &MccBuf, 0, MccBuf.m_dmat[0], bytebuf, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Encode a single block analysis
void CLpacEncoder::EncodeBlockAnalysis(MCC_ENC_BUFFER *pBuffer, long Channel, int *x)
{
	int *d=pBuffer->m_dmat[Channel];
	int *mccasi=pBuffer->m_asimat[Channel];
	char *xpr=&pBuffer->m_xpara[Channel];
	short *sft=&pBuffer->m_shift[Channel];
	short *oP=&pBuffer->m_optP[Channel];

	static double d2mean;
	static long danz;
	short optP;
	long i, c;

	static long count = 1;

    /* reconstruction levels for 1st and 2nd coefficients: */
    static int pc12_tbl[128] = {
        -1048544, -1048288, -1047776, -1047008, -1045984, -1044704, -1043168, -1041376, -1039328,
        -1037024, -1034464, -1031648, -1028576, -1025248, -1021664, -1017824, -1013728, -1009376,
        -1004768, -999904, -994784, -989408, -983776, -977888, -971744, -965344, -958688, -951776,
        -944608, -937184, -929504, -921568, -913376, -904928, -896224, -887264, -878048, -868576,
        -858848, -848864, -838624, -828128, -817376, -806368, -795104, -783584, -771808, -759776,
        -747488, -734944, -722144, -709088, -695776, -682208, -668384, -654304, -639968, -625376,
        -610528, -595424, -580064, -564448, -548576, -532448, -516064, -499424, -482528, -465376,
        -447968, -430304, -412384, -394208, -375776, -357088, -338144, -318944, -299488, -279776,
        -259808, -239584, -219104, -198368, -177376, -156128, -134624, -112864, -90848, -68576,
        -46048, -23264, -224, 23072, 46624, 70432, 94496, 118816, 143392, 168224, 193312, 218656,
        244256, 270112, 296224, 322592, 349216, 376096, 403232, 430624, 458272, 486176, 514336,
        542752, 571424, 600352, 629536, 658976, 688672, 718624, 748832, 779296, 810016, 840992,
        872224, 903712, 935456, 967456, 999712, 1032224 };

	// ZERO BLOCK
	if (BlockIsZero(x, N))
	{
		for(i=0;i<N;i++)
			d[i]=0;
		*xpr=1;
	}
	// CONSTANT BLOCK
	else if (c = BlockIsConstant(x, N, IntRes))
	{
		for(i=0;i<N;i++)
			d[i]=c;
		*xpr=2;
	}
	// NORMAL BLOCK
	else
	{
		*xpr=0;
		int asi[1023], parq[1023], *xtmp;
		short shift = 0;

		optP = P;
		short Pmax = P;

		if (Adapt)
		{
			// Limit maximum predictor order
			const short h = (N < 8) ? 1 : min(ilog2_ceil(Pmax+1), max(ilog2_ceil(N >> 3), 1));
			if (Pmax >= (1 << h))
				Pmax = (1 << h) - 1;
		}

		if (LSBcheck)
		{
			// Empty LSBs?
			if (shift = ShiftOutEmptyLSBs(x, N))
			{
				xtmp = new int[Pmax];
				// "Shift" last Pmax samples of previous block
				for (i = -Pmax; i < 0; i++)
				{
					xtmp[Pmax+i] = x[i];		// buffer original values
					x[i] >>= shift;
				}
			}
		}

		// Calculate the LPC coefficients for a fixed order
		// To adapt the order as well, use a function which returns the optimal order (optP)
		// for this block and the corresponding set of parcor coefficients (par).
		if (!Adapt)
			GetCof(x, N, P, Win, par);						// Fixed order
#ifdef	LPC_ADAPT
		else
			//optP = GetCofAdaptOrder(x, N, Pmax, Win, par, Freq);		// Adaptive order
			optP = GetCofAdaptOrder(x, N, Pmax, Win, par, (Freq > 96000) && !Sub ? 0 : Freq);		// Adaptive order
#endif
		double q = PI / 256;	// Quantizer step size

		// Quantization of the coefficients

		/* first coefficient: */
		int a = (int) floor((-1 + sqrt(2.0) * sqrt(par[0]+1.0)) * 64);
		if (a > 63) a = 63; else if (a < -64) a = -64;
		asi[0] = a;
		parq[0] = pc12_tbl[a + 64];

		/* second coefficient: */
		if ( optP > 1) {
			a = (int) floor((-1 + sqrt(2.0) * sqrt(-par[1]+1.0)) * 64);
			if (a > 63) a = 63; else if (a < -64) a = -64;
			asi[1] = a;
			parq[1] = -pc12_tbl[a + 64];
		}

		/* the remaining coeffs: */
		for (i=2; i<optP; i++) {
			a = (int) floor(par[i]*64);
			if (a > 63) a = 63; else if (a < -64) a = -64;
			asi[i] = a;
			parq[i] = (a << (Q -6)) + (1 << (Q-7));
		}

		// Estimation of the residual
		if (!RA)
		{
			if (par2cof(cof, parq, optP, Q))	// Conversion from parcor to direct form coefficients
			{
				// If conversion fails, use first order predictor (par = -0.9)
				if(Adapt) optP = 1;
				if( optP > 0)
				{
					par[0] = -0.9;
					asi[0] = (int)floor((-1+sqrt(2.0)*sqrt(par[0]+1.0))*64);
					parq[0] = pc12_tbl[asi[0]+64];
				}
				if ( optP > 1)
				// set all coefficients 0 
				/* second coefficient: */
				{
					a = (int) floor((-1 + sqrt(2.0) ) * 64); 
					asi[1] = a; 
					parq[1] = -pc12_tbl[a + 64]; 
				}
				/* the remaining coeffs: */ 
				if(optP >2)
				{
					asi[2] = 0; 
					parq[2] =  (1 << (Q-7));
					for(i=3;i<optP;i++)
					{
						asi[i]=asi[2];
						parq[i]=parq[2];
					}
				}
				par2cof(cof, parq, optP, Q);	// Try again (always works)
			}
			GetResidual(x, N, optP, Q, cof, d);
		}
		else
		{
			if (GetResidualRA(x, N, optP, Q, parq, cof, d))		// Progressive prediction (internal conversion)
			{
				// If conversion fails, use first order predictor (par = -0.9)
				if(Adapt) optP = 1;
				if(optP>0)
				{
					par[0] = -0.9;
					asi[0] = (int)floor((-1+sqrt(2.0)*sqrt(par[0]+1.0))*64);
					parq[0] = pc12_tbl[asi[0]+64];
				}
				/* second coefficient: */ 
				if ( optP > 1)
				{
					a = (int) floor((-1 + sqrt(2.0) ) * 64);
					asi[1] = a;
					parq[1] = -pc12_tbl[a + 64];
				}
				/* the remaining coeffs: */
				if(optP >2)
				{
					asi[2] = 0;
					parq[2] =  (1 << (Q-7));
					for(i=3;i<optP;i++)
					{
						asi[i]=asi[2];
						parq[i]=parq[2];
					}
				}
				GetResidualRA(x, N, optP, Q, parq, cof, d);		// Try again (always works)
			}
		}

		for(i=0;i<optP;i++)
			mccasi[i]=asi[i];
		*sft=shift;
		*oP=optP;

		if(PITCH) 
		{
			memset( pBuffer->m_Ltp.m_pBuffer[Channel].m_ltpmat, 0, sizeof(int) * 2048 );
			LTPanalysis(pBuffer, Channel, N, optP, x);
		}

		if (shift)
		{
			// Undo "shift" of whole block (and restore Pmax samples of the previous block)
			for (i = -Pmax; i < 0; i++)
				x[i] = xtmp[Pmax+i];
			for (i = 0; i < N; i++)
				x[i] <<= shift;
			delete [] xtmp;
		}

	}	// End of NORMAL BLOCK
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Encode a single block coding
long CLpacEncoder:: EncodeBlockCoding(MCC_ENC_BUFFER *pBuffer, long Channel, int *d, unsigned char *bytebuf, long gmod)
{
	int *puch=pBuffer->m_puchan[Channel];
	int *asi=pBuffer->m_asimat[Channel];
	char xpra=pBuffer->m_xpara[Channel];
	short shift=pBuffer->m_shift[Channel];
	short optP=pBuffer->m_optP[Channel];
	long oaa;
	//MCC-ex
	int *tdtauval=pBuffer->m_tdtau[Channel];
	short *MccMode=pBuffer->m_MccMode[Channel];
	int **mtgmm=pBuffer->m_cubgmm[Channel];

	static double d2mean;
	static long danz;
	BYTE h, hl[4];
	short sub, s[8], sx[8], sfix, S[8];
	long Ns;
	long i, j;
	int c;
	/* rice code parameters for each coeff: */
	struct pv {int m,s;} *parcor_vars = 0;

	// 48kHz
	static struct pv parcor_vars_0[20] = {
        {-52, 4}, {-29, 5}, {-31, 4}, { 19, 4}, {-16, 4}, { 12, 3}, { -7, 3}, {  9, 3}, { -5, 3}, {  6, 3}, { -4, 3}, {  3, 3}, { -3, 2}, {  3, 2}, { -2, 2}, {  3, 2}, { -1, 2}, {  2, 2}, { -1, 2}, {  2, 2}  // i&1, 2
    };

    // 96kHz
	static struct pv parcor_vars_1[20] = {
        {-58, 3}, {-42, 4}, {-46, 4}, { 37, 5}, {-36, 4}, { 29, 4}, {-29, 4}, { 25, 4}, {-23, 4}, { 20, 4}, {-17, 4}, { 16, 4}, {-12, 4}, { 12, 3}, {-10, 4}, {  7, 3}, { -4, 4}, {  3, 3}, { -1, 3}, {  1, 3}  // i&1, 2
    };

    // 192 kHz
	static struct pv parcor_vars_2[20] = {
        {-59, 3}, {-45, 5}, {-50, 4}, { 38, 4}, {-39, 4}, { 32, 4}, {-30, 4}, { 25, 3}, {-23, 3}, { 20, 3}, {-20, 3}, { 16, 3}, {-13, 3}, { 10, 3}, { -7, 3}, {  3, 3}, {  0, 3}, { -1, 3}, {  2, 3}, { -1, 2}  // i&1, 2
    };

	if (CoefTable == 0)
		parcor_vars = parcor_vars_0;
	else if (CoefTable == 1)
		parcor_vars = parcor_vars_1;
	else if (CoefTable == 2)
		parcor_vars = parcor_vars_2;

	// Bit-oriented output (header + Rice codes)
	CBitIO out;
	out.InitBitWrite(bytebuf);

	// ZERO BLOCK
	if( xpra == 1 )
	{
		h = (BYTE)0;							// Block header h = 0000 0000
		out.WriteByteAlign(h);
		if(MCCflag) gmod=0; //Ignore MCC
	}
	// CONSTANT BLOCK
	else if ( xpra == 2 )
	{
		c = d[0];
		h = (BYTE)64;							// Block header h = 0100 0000
		out.WriteByteAlign(h);

		if ( IntRes <= 8 )
		{
			hl[0] = static_cast<BYTE>( c + 128 );
			out.WriteByteAlign( hl[0] );
		}
		else if ( IntRes <= 16 )
		{
			hl[0] = (short(c) & 0xFF00) >> 8;			// HiByte
			hl[1] = (short(c) & 0x00FF);				// LoByte
			out.WriteByteAlign(hl[0]);
			out.WriteByteAlign(hl[1]);
		}
		else if (IntRes <= 24)
		{
			hl[0] = (c >> 16) & 0xFF;
			hl[1] = (c >> 8) & 0xFF;
			hl[2] = c & 0xFF;
			out.WriteByteAlign(hl[0]);
			out.WriteByteAlign(hl[1]);
			out.WriteByteAlign(hl[2]);
		}
		else	// IntRes > 24
		{
			hl[0] = (c >> 24) & 0xFF;
			hl[1] = (c >> 16) & 0xFF;
			hl[2] = (c >> 8) & 0xFF;
			hl[3] = c & 0xFF;
			out.WriteByteAlign(hl[0]);
			out.WriteByteAlign(hl[1]);
			out.WriteByteAlign(hl[2]);
			out.WriteByteAlign(hl[3]);
		}
		if(MCCflag) gmod=0; //Ignore MCC
	}
	// NORMAL BLOCK
	else
	{
		short num = 0;
		if (RLSLMS) optP = 10;  // must assume optP = 10 as decoder default

		// Partition into entropy code (EC) blocks
		if ((N < 512) || (N % 8))		// Block length less than 512 or last block
			sub = 1;
		else if (!BGMC)
			sub = 4;
		else
			sub = 8;
		Ns = N / sub;

		// Number of separately coded samples in random access mode
		if (RA)
			num = min(optP, min(N, 3));

		s[0] = GetRicePara(d, num, Ns, sx);		// First subblock is num samples shorter in RA mode
		for (j = 1; j < sub; j++)						// Remaining subblocks
			s[j] = GetRicePara(d + j*Ns, 0, Ns, sx + j);

		if (IntRes <= 16)
		{
			for (j = 0; j < sub; j++)
			{
				if (s[j] > 15)
				{
					s[j] = 15;		// Limitation to 4-bit representation
					sx[j] = 15;
				}
			}
		}
		else	// IntRes == 24/32
		{
			for (j = 0; j < sub; j++)
			{
				if (s[j] > 31)
				{
					s[j] = 31;		// Limitation to 5-bit representation
                    sx[j] = 15;
				}
			}
		}

		if (!BGMC)
		{
			// Check if code parameters are the same for all EC blocks
			sfix = s[0];
			for (j = 1; j < sub; j++)
			{
				if (s[j] != sfix)
				{
					sfix = -1;
					break;
				}
			}
			if (sfix != -1)
			{
				sub = 1;
				Ns = N;
			}
		}
		else	// BGMC
		{
			// Calculate 'whole' BGMC code parameters
			for (j = 0; j < sub; j++)
				S[j] = (s[j] << 4) | sx[j];

			// Check if number of subblocks can be reduced
			long delta;
			short reduce = 1;
			short oldsub = sub;
			while ((sub > 1) && (reduce))
			{
				// Sum up differences of adjacent S values
				delta = 0;
				for (j = 0; j < sub; j += 2)
					delta += abs(S[j] - S[j+1]);
				if (delta > 3*(sub/2-1))			// threshold factor 
					reduce = 0;

				// If values are (nearly) equal...
				if (reduce)
				{
					sub = sub >> 1;				// Reduce number of s-blocks
					for (j = 0; j < sub; j++)
						S[j] = (S[2*j]+S[2*j+1]+1)/2;	// Merge S values (mean)
					Ns = Ns << 1;				// Double length of s-blocks
				}
			}
			if (sub != oldsub)
			{
				s[0] = GetRicePara(d, num, Ns, sx);
				S[0] = (s[0] << 4) | sx[0];
				// recalculate s and sx values
				for (j = 1; j < sub; j++)
				{
					s[j] = GetRicePara(d + (j*Ns), 0, Ns, sx + j);
					S[j] = (s[j] << 4) | sx[j];
				}
				// check values
				short maxS = IntRes <= 16 ? 255 : 511;	// 255=15*16+15, 511=31*16+15 (see above)
				for (j = 0; j < sub; j++)
				{
					if (S[j] > maxS)
					{
						S[j] = maxS;
						s[j] = S[j] >> 4;
						sx[j] = S[j] & 0x0F;
					}
				}
			}
		}

		// Write Data /////////////////////////////////////////////////////////////////////////////

		// Block header
		h = (BYTE)2;				// 1J (J is set in EncodeFrame)
		out.WriteBits((UINT)h, 2);

		if (!BGMC)
		{
			if (sub == 4)
				out.WriteBits((UINT)1, 1);
			else
				out.WriteBits((UINT)0, 1);
		}
		else
		{
			if (sub == 8)
				h = 3;			// 8 -> 11
			else
				h = sub >> 1;	// 4 -> 10, 2 -> 01, 1 -> 00
			out.WriteBits((UINT)h, 2);
		}

		// Code parameters for EC blocks
		if (!BGMC)
		{
			short sbits = IntRes <= 16 ? 4 : 5;
			out.WriteBits((UINT)s[0], sbits);	// direct coding of s[0]
			for (j = 1; j < sub; j++)
			{
				c = s[j] - s[j-1];				// delta coding of s[j]
				out.WriteRice(&c, 0, 1);
			}
		}
		else
		{
			short sbits = IntRes <= 16 ? 8 : 9;
			out.WriteBits((UINT)S[0], sbits);	// direct coding of S[0]
			for (j = 1; j < sub; j++)
			{
				c = S[j] - S[j-1];				// delta coding of S[j]
				out.WriteRice(&c, 2, 1);
			}
		}

		// LSB shift
		if (shift && !RLSLMS)
		{
			out.WriteBits(1, 1);
			out.WriteBits(UINT(shift - 1), 4);
		}
		else
			out.WriteBits(0, 1); // 0 if RLSLMS is used
		if (!RLSLMS)
		{
			// Predictor order (optP <= P, write only necessary bits)
			if (Adapt)
			{
				// Limit maximum predictor order
				h = (N < 8) ? 1 : min(ilog2_ceil(P+1), max(ilog2_ceil(N >> 3), 1));
				out.WriteBits(UINT(optP), h);	// max. 10 bits for 512 <= P <= 1023
			}

			/* encode coefs: */
			if (parcor_vars != 0) {
				for (i = 0; i < min(optP,20); i++)
					rice_encode (asi[i] - parcor_vars[i].m, parcor_vars[i].s, &out.bio);
				for (; i < min(optP,127); i++)
					rice_encode (asi[i] - (i & 1), 2, &out.bio);
				for (; i < optP; i++)
					rice_encode (asi[i], 1, &out.bio);
			}
			else
			{
				for (i = 0; i < optP; i++)
					out.WriteBits(asi[i] + 64, 7);
			}
		}

		if (PITCH) 	pBuffer->m_Ltp.Encode( Channel, d, bytebuf, N, Freq, &out );

		// Residual
		if (!RA)
		{
			if (!BGMC)
			{
				for (j = 0; j < sub; j++)			// EC blocks
					out.WriteRice(d + (j*Ns), (char)s[j], Ns);
			}
			else
			{
				bgmc_encode_blocks(d, 0, s, sx, N, sub, &out.bio);
			}
		}
		else
		{
			// Progressive prediction: Separate entropy coding of the first 3 samples
			short RiceLimit = ( IntRes <= 16 ) ? 15 : 31;
			if (num > 0)
				out.WriteRice(d, IntRes-4, 1);
			if (num > 1)
				out.WriteRice(d+1, min(s[0]+3, RiceLimit), 1);
			if (num > 2)
				out.WriteRice(d+2, min(s[0]+1, RiceLimit), 1);

			if (!BGMC)
			{
				out.WriteRice(d + num, (char)s[0], Ns - num);	// First EC block is 'num' samples shorter
				for (j = 1; j < sub; j++)				// Remaining 3 EC blocks
					out.WriteRice(d + j*Ns, (char)s[j], Ns);
			}
			else
			{
				bgmc_encode_blocks(d, num, s, sx, N, sub, &out.bio);
			}
		}
	
	}	// End of NORMAL BLOCK

	if (RLSLMS)
	{
		out.WriteBits((UINT) mono_frame,1);
		//printf("%d %d ",RLSLMS_ext,optP);
		if (RLSLMS_ext!=0)
		{
			out.WriteBits((UINT) 1,1);
			out.WriteBits((UINT) RLSLMS_ext,3);
			if (RLSLMS_ext&0x01) // change lambda only
			{
				out.WriteBits((c_mode_table.filter_len[1]>>1),4);
				out.WriteBits(c_mode_table.nstage-2,3);
				for(i=2;i<c_mode_table.nstage;i++)
				{
					out.WriteBits(lookup_table(lms_order_table,
							               c_mode_table.filter_len[i]
										   ),5);						
				}
			}
			if (RLSLMS_ext&0x02)
			{
				if (c_mode_table.filter_len[1]){
					out.WriteBits(c_mode_table.lambda[0],10);
					out.WriteBits(c_mode_table.lambda[1],10);
				}
			}
			if (RLSLMS_ext&0x04)
			{
				for(i=2;i<c_mode_table.nstage;i++)
				{
					out.WriteBits(lookup_mu(c_mode_table.opt_mu[i]),5);
				}
				out.WriteBits(c_mode_table.step_size/LMS_MU_INT,3);
			}
		}
		else
		{
			out.WriteBits(0,1);
		}
		//RLSLMS_ext=0;
	}

	// MCC Coefficients (Reference channels and weighting factor)
	if(MCCflag)
	{
		for(oaa=0;oaa<gmod;oaa++)
		{
			out.WriteBits(0,1); // End Flag OFF -> Continue

			out.WriteBits((puch[oaa]),NeedPuchBit); //Reference Channel

			if(puch[oaa]==Channel)
			{
				//We do not need weighting factor if this channel is reference channel
			}
			else 
			{
				out.WriteBits((UINT)(MccMode[oaa])-1,1); // MccEx

				for(j = 0; j < Mtap; j++)
					mtgmm[oaa][j] -= 16;

				if(MccMode[oaa] == 1)
				{
					mtgmm[oaa][1] += 2;
					rice_encode (mtgmm[oaa][0], 1, &out.bio); // -1
					rice_encode (mtgmm[oaa][1], 2, &out.bio); //  0
					rice_encode (mtgmm[oaa][2], 1, &out.bio); // +1
				}
				else
				{

					mtgmm[oaa][1] += 2;
					rice_encode (mtgmm[oaa][0], 1, &out.bio); // -1
					rice_encode (mtgmm[oaa][1], 2, &out.bio); //  0
					rice_encode (mtgmm[oaa][2], 1, &out.bio); // +1
					rice_encode (mtgmm[oaa][3], 1, &out.bio); // Tau-1
					rice_encode (mtgmm[oaa][4], 1, &out.bio); // Tau
					rice_encode (mtgmm[oaa][5], 1, &out.bio); // Tau+1

					if(tdtauval[oaa]>0)out.WriteBits(0,1);
					else out.WriteBits(1,1);
					out.WriteBits((UINT) (abs(tdtauval[oaa])-3),NeedTdBit);
				}
			}
		}
		out.WriteBits(1,1); // End Flag ON (Anyway terminate)
	}

	return(out.EndBitWrite());		// Return number of written bytes
}

////////////////////////////////////////
//                                    //
//            LTP analysis            //
//                                    //
////////////////////////////////////////
void	CLpacEncoder::LTPanalysis( MCC_ENC_BUFFER* pBuffer, long Channel, long N, short optP, int* x )
{
	CLtpBuffer*	pLtpBuf = pBuffer->m_Ltp.m_pBuffer + Channel;
	int*	dd = pLtpBuf->m_ltpmat + 2048;
	int*	dd0 = new int [ 4 * N ];
	long	tmpbytes0 = 0;
	long	minbytes;

	pBuffer->m_optP[Channel] = optP;
	
	memcpy( pLtpBuf->m_ltpmat + 2048, pBuffer->m_dmat[Channel], N * sizeof(int) );

	pLtpBuf->m_ltp = 0;
	tmpbytes0 = EncodeBlockCoding( pBuffer, Channel, dd, pBuffer->m_tmpbuf1, 0 );

	minbytes = tmpbytes0;

	// Pitch Coding
	CLtpBuffer	inpitch;
	pBuffer->m_Ltp.PitchSubtract( &inpitch, dd, dd0, N, optP, Freq, PITCH );
	memcpy( pLtpBuf->m_ltpmat + 2048, pBuffer->m_dmat[Channel], N * sizeof(int) );
	memcpy( pLtpBuf->m_ltpmat, pLtpBuf->m_ltpmat + N, 2048 * sizeof(int) );
	pLtpBuf->m_ltp = 0;
	tmpbytes0 = EncodeBlockCoding( pBuffer, Channel, dd0, pBuffer->m_tmpbuf1, 0 );

	if ( tmpbytes0 < minbytes ) {
		pLtpBuf->m_ltp = 1;
		minbytes = tmpbytes0;
		pLtpBuf->m_plag = inpitch.m_plag;
		memcpy( pBuffer->m_dmat[Channel], dd0, N * sizeof(int) );
		memcpy( pLtpBuf->m_pcoef_multi, inpitch.m_pcoef_multi, 5 * sizeof(short) );
	}
	delete[] dd0;
}

bool CLpacEncoder::EnforceProfiles()
{
	if (ALSProfIsMember(EnforcedProfiles, ALS_SIMPLE_PROFILE_L1)) {
		// Check non-adjustable parameters
		if (Chan > 2 || Freq > 48000 || Res > 16 || SampleType != 0)
			return false;

		// Limit the parameters
		if (N > 4096)
			N = 4096;
		if (P > 15)
			P = 15;
		if (Sub > 3)
			Sub = 3;
		BGMC = 0;
		RLSLMS = 0;
	}

	return true;
}
