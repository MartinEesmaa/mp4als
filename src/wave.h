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

filename : wave.h
project  : MPEG-4 Audio Lossless Coding
author   : Tilman Liebchen (Technical University of Berlin)
date     : June 16, 2003
contents : Header file for wave.cpp

*************************************************************************/

/*************************************************************************
 *
 * Modifications:
 *
 * 11/13/2003, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   added support for AIFF and other file formats
 *   - added AIFFCOMMON, AIFFFILEHEADER, AUDIOINFO, ENCINFO
 *   - added GetAiffFormat()
 *   - added void [Write/Read]U[Short/Long]MSBfirst() functions
 *
 * 12/15/2003, Koichi Sugiura <ksugiura@mitaka.ntt-at.co.jp>
 *   - added IntRes, SampleType member to AUDIOINFO structure.
 *     Now Res means the actual resolution for both int and float.
 *     IntRes means the resolution of core compression of integer PCM.
 *     In case of float PCM, Res = 32 and IntRes = 16. Otherwise, Res == IntRes.
 *     SampleType is 0 for integer PCM, and 1 for floating point PCM.
 *
 * 11/05/2004, Yutaka Kamamoto <kamamoto@hil.t.u-tokyo.ac.jp>
 *   - added MCC in ENCINFO
 *
 * 02/06/2005, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - changed type of FrameLength from u-short to u-long
 *
 * 06/08/2005, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - changed ENCINFO structure
 *
 * 05/23/2007, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - supported 64-bit data size.
 *   - replaced FILE* with HALSSTREAM to use functions in stream.cpp.
 *
 * 05/14/2008, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - added ValidBitsPerSample to WAVEFORMATPCM structure.
 *   - modified WAVEFILEHEADER definition.
 *   - added ReadWaveFormat().
 *
 *************************************************************************/

#ifndef _INC_WAVE_H
#define _INC_WAVE_H

#include <stdio.h>
#include "stream.h"

typedef unsigned char BYTE;
typedef unsigned short USHORT;
typedef unsigned int UINT;

#if defined( _MSC_VER )
#define PACKED
#pragma pack( push, 1 )
#elif defined( __GNUG__ )
#define PACKED __attribute__ ((packed))
#endif

typedef struct
{
	USHORT	FormatTag			PACKED;	// WAVE_FORMAT_PCM = 1, WAVE_FORMAT_IEEE_FLOAT = 3
	USHORT	Channels			PACKED;	// Mono = 1, Stereo = 2, Multichannel > 2 (undefined channel mapping)
	UINT	SamplesPerSec		PACKED;	// Sampling frequency (in Hz)
	UINT	AvgBytesPerSec		PACKED;	// Bytes per second (SampPerSec * BlockAlign)
	USHORT	BlockAlign			PACKED;	// Bytes per sample step (1...4 * Channels)
	USHORT	BitsPerSample		PACKED;	// Bits per sample (typically 8, 16, 24 or 32)
	USHORT	ValidBitsPerSample	PACKED;	// Valid bits per sample
} WAVEFORMATPCM;

typedef struct
{
	char	RiffChunk[4]	PACKED;		// "RIFF"
	UINT	RiffLen			PACKED;		// Length of following file data (file length - 8)
	char	WaveChunk[4]	PACKED;		// "WAVE"
	char	FormChunk[4]	PACKED;		// "fmt "
	UINT	FormLen			PACKED;		// Length of format chunk (typically 16)
	USHORT	FormatTag		PACKED;		// Format tag
	USHORT	Channels		PACKED;		// Number of channels
	UINT	SamplesPerSec	PACKED;		// Sampling frequency
	UINT	AvgBytesPerSec	PACKED;		// Bytes per second
	USHORT	BlockAlign		PACKED;		// Bytes per sample step
	USHORT	BitsPerSample	PACKED;		// Bits per sample
	char	DataChunk[4]	PACKED;		// "data"
	UINT	DataLen			PACKED;		// Length of data chunk
} WAVEFILEHEADER;

#if defined( _MSC_VER )
#pragma pack( pop )
#endif
#undef PACKED

typedef struct
{
	long		Channels;		// Number of channels
	UINT		SampleFrames;	// Samples per channel
	short		SampleSize;		// Bits per sample
	BYTE		SampleRate[10];	// Sampling frequency (in Hz)
} AIFFCOMMON;

typedef struct
{
	char	FormChunk[4];		// "FORM"
	UINT	FormLen;			// Length of following file data (file length - 8)
	char	AiffChunk[4];		// "AIFF"
	char	CommChunk[4];		// "COMM"
	UINT	CommLen;			// Length of common chunk (typically 16)
	AIFFCOMMON	ac;				// common chunk
	char	SsndChunk[4];		// "SSND"
	UINT	SsndLen;			// Length of sound data chunk
	UINT	Offset;
	UINT	BlockSize;
} AIFFFILEHEADER;

typedef struct
{
	unsigned char	FileType;		// File Type (raw, wave, aiff, ...)
	unsigned char	MSBfirst;		// Byte Order (0 = LSB first, 1 = MSB first)
	unsigned short	Chan;			// Number of Channels
	unsigned short	Res;			// Actual resolution in Bits (1...32)
	unsigned short	IntRes;			// Integer resolution in Bits
	unsigned char	SampleType;		// Sample type (0=int, 1=float)
	ALS_INT64		Samples;		// Number of Samples per Channel
	unsigned long	Freq;			// Sample Rate in Hz
	ALS_INT64		HeaderSize;		// Header Size in Bytes
	ALS_INT64		TrailerSize;	// Trailer Size in Bytes
} AUDIOINFO;

typedef struct
{
	unsigned long FrameLength;		// Frame Length in Samples
	unsigned char AdaptOrder;		// Adaptive Order (0 = off, 1 = on)
	unsigned char JointCoding;		// Joint Channel Coding (0 = off, 1 = on)
	unsigned char SubBlocks;		// Sbbblock Mode (0 = off, 1 = on)
	unsigned short RandomAccess;	// Random Access (Frame Distance: 0 = off, 1 = each, 2 = every 2nd, ...)
	unsigned char BGMC;				// BGMC coding for residual
	unsigned short MaxOrder;		// Maximum Order (Fixed Order if 'Adaptive Order' = off)
	unsigned char MCC;				// Multi-channel correlation (0 = off, 1 = on)
	unsigned char PITCH;			// Pitch Coding (0 = off, 1 = on)
	unsigned char RAflag;
	unsigned char CRCenabled;
} ENCINFO;

// Functions
ALS_INT64 GetWaveFormatPCM(HALSSTREAM wav,WAVEFORMATPCM *wf, ALS_INT64 *samples);
short WriteWaveHeaderPCM(HALSSTREAM wav, WAVEFORMATPCM *wf, UINT *samples);
ALS_INT64 GetAiffFormat(HALSSTREAM aif, AIFFCOMMON *ac, ALS_INT64 *Offset, ALS_INT64 *BlockSize, UINT *samplerate);
ALS_INT64 GetWave64FormatPCM(HALSSTREAM wav, WAVEFORMATPCM* wf, ALS_INT64* samples);
ALS_INT64 GetRF64FormatPCM(HALSSTREAM wav, WAVEFORMATPCM* wf, ALS_INT64* samples);
bool ReadWaveFormat( HALSSTREAM wav, WAVEFORMATPCM* pFmt, ALS_INT64 FmtLen );

void WriteUShortLSBfirst(unsigned short x, HALSSTREAM fp);
void WriteUShortMSBfirst(unsigned short x, HALSSTREAM fp);
unsigned short ReadUShortLSBfirst(HALSSTREAM fp);
unsigned short ReadUShortMSBfirst(HALSSTREAM fp);
void WriteUIntLSBfirst(unsigned int x, HALSSTREAM fp);
void WriteUIntMSBfirst(unsigned int x, HALSSTREAM fp);
unsigned int ReadUIntLSBfirst(HALSSTREAM fp);
unsigned int ReadUIntMSBfirst(HALSSTREAM fp);
void WriteUINT64LSBfirst(ALS_UINT64 x, HALSSTREAM fp);
void WriteUINT64MSBfirst(ALS_UINT64 x, HALSSTREAM fp);
ALS_UINT64 ReadUINT64LSBfirst(HALSSTREAM fp);
ALS_UINT64 ReadUINT64MSBfirst(HALSSTREAM fp);

#endif	//_INC_WAVE_H

