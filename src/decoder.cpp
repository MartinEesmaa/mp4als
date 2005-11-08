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

filename : decoder.cpp
project  : MPEG-4 Audio Lossless Coding
author   : Tilman Liebchen (Technical University of Berlin)
date     : June 23, 2003
contents : Implementation of the CLpacDecoder class

*************************************************************************/

/*************************************************************************
 * Modifications:
 *
 * 8/31/2003, Yuriy A. Reznik <yreznik@real.com>
 *  made set of changes to support BGMC coding mode:
 *      - added CLpacDecoder::GetBGMC() function;
 *      - added logic for using 0x10 bit (BGMC) in the "parameters" field
 *        and 0x40 bit in the version number in CLpacDecoder::DecodeHeader()
 *      - added calls to BGMC decoding routines in CLpacDecoder::DecodeBlock()
 *
 * 11/26/2003, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   added support for multichannel, 32-bit, and new file format:
 *     - substituted DecodeHeader() by AnalyseInputFile() and WriteHeader()
 *     - substituted DecodeRemains() by WriteTrailer()
 *     - substituted DecodeFile() by DecodeAll()
 *     - added decoding of new file format header in AnalyseInputFile()
 *     - added decoding of multichannel and 32-bit in DecodeFrame() and
 *       DecodeBlock()
 *
 * 12/16/2003, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - new size of tmpbuf[] array
 *
 * 03/22/2004, Koichi Sugiura <ksugiura@mitaka.ntt-at.co.jp>
 *   - supported floating point PCM.
 *
 * 03/24/2004, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - progressive prediction for random access frames
 *   - removed support for file format version < 8
 *
 * 5/17/2004, Yuriy Reznik <yreznik@real.com>
 *   changes to support improved coding of LPC coefficients:
 *   - modified CLpacDecoder::DecodeBlock(() to use new scheme
 *   - incremented version # and removed support for older file formats
 *
 * 07/29/2004, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - added support for higher orders (up to 255)
 *   - further improved entropy coding of coefficients (3 tables)
 *   - streamlined block header syntax
 *   - removed support for file format version < 11
 *
 * 10/26/2004, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - added use of bgmc_decode_blocks()
 *   - added support for up to 8 subblocks
 *   - added support for improved coding of code parameters
 *   - incremented version # and removed support for older file formats
 *   - removed block switching flag where not necessary
 *
 * 11/05/2004, Yutaka Kamamoto <kamamoto@hil.t.u-tokyo.ac.jp>
 *   - added multi-channel correlation method
 *   - divided DecodeBlock() into DecodeBlockParameter()
 *                            and DecodeBlockReconstruct()
 *
 * 11/17/2004, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - modified floating point operations using CFloat class.
 *
 * 11/22/2004, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - supported channel sort feature for floating point data.
 *
 * 11/25/2004, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - added RandomAccess parameter to DecodeDiff().
 *
 * 02/06/2005, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - added support for extended block switching and higher orders
 *
 * 02/11/2005, Takehiro Moriya (NTT) <t.moriya.ieee.org>
 *   - add LTP modules for CE11
 *
 * 03/08/2005, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - integration of CE10 and CE11
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
 *   - support for auxiliary data
 *
 * 10/17/2005, Noboru Harada <n-harada@theory.brl.ntt.co.jp>
 *   - Merged bug fixed codes for RLS-LMS prepared from I2R.
 *
 ************************************************************************/

#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <assert.h>

#ifdef WIN32
	#include <io.h>
	#include <fcntl.h>
#endif

#include "decoder.h"
#include "bitio.h"
#include "lpc.h"
#include "audiorw.h"
#include "crc.h"
#include "wave.h"
#include "floating.h"
#include "mcc.h"
#include "lms.h"

#define min(a, b)  (((a) < (b)) ? (a) : (b))
#define max(a, b)  (((a) > (b)) ? (a) : (b))

// Constructor
CLpacDecoder::CLpacDecoder()
{
	fpInput = NULL;
	fpOutput = NULL;

	frames = 0;		// Number of frames
	fid = 0;		// No frame decoded yet
	CRC = 0;		// CRC initialization
	Q = 20;			// Quantizer value
}

CLpacDecoder::~CLpacDecoder()
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

		delete [] tmpbuf;

		if (Joint)
		{
			for (i = 0; i < Chan/2; i++)
				delete [] xps[i];
			delete [] xps;
			delete [] xs;
		}

		delete [] bbuf;
		delete [] d;
		delete [] cofQ;
		delete [] buff;

		if (RA && (RAflag == 2))
			delete [] RAUsize;
	}

	if (ChanSort)
		delete [] ChPos;

	// Deallocate float buffer
	if ( SampleType == SAMPLE_TYPE_FLOAT ) Float.FreeBuffer();
	// Deallocate MCC buffer
	FreeMccDecBuffer( &MccBuf );

	// Close files
	CloseFiles();
}

short CLpacDecoder::CloseFiles()
{
	if (fpInput != NULL)
		if (fclose(fpInput))
			return(1);
	fpInput = NULL;

	if (fpOutput != NULL)
		if (fclose(fpOutput))
			return(2);
	fpOutput = NULL;

	return(0);
}

/*void CLpacDecoder::GetFilePositions(long *SizeIn, long *SizeOut)
{
	*SizeIn = ftell(fpInput);
	*SizeOut = ftell(fpOutput);
}*/

short CLpacDecoder::OpenInputFile(const char *name)
{
	if (name[0] == ' ')
	{
		fpInput = stdin;
#ifdef WIN32
		if (_setmode(_fileno(stdin), _O_BINARY) == -1)
		{
			fprintf(stderr, "Cannot set 'stdin' mode\n");
			return(1);
		}
#endif
	}
	else if (!(fpInput = fopen(name, "rb")))
		return(2);

	return(0);
}

short CLpacDecoder::AnalyseInputFile(AUDIOINFO *ainfo, ENCINFO *encinfo)
{
	BYTE tmp;
	long i;

	// Read ALS header information ////////////////////////////////////////////////////////////////
	Freq = ReadULongMSBfirst(fpInput);			// sampling frequency
	Samples = ReadULongMSBfirst(fpInput);		// samples
	Chan = ReadUShortMSBfirst(fpInput) + 1;		// channels
	
	fread(&tmp, 1, 1, fpInput);
	FileType = tmp >> 5;						// file type			(XXXx xxxx)
	Res =  8 * (((tmp >> 2) & 0x07) + 1);		// resolution			(xxxX XXxx)
	SampleType = (tmp >> 1) & 0x01;				// floating-point		(xxxx xxXx)
	if (SampleType == 1)
		IntRes = IEEE754_PCM_RESOLUTION;
	else
		IntRes = Res;
	MSBfirst = tmp & 0x01;						// MSB/LSB first		(xxxx xxxX)

	N = ReadUShortMSBfirst(fpInput) + 1;		// frame length

	fread(&tmp, 1, 1, fpInput);
	RA = tmp;									// random access

	fread(&tmp, 1, 1, fpInput);
	RAflag = (tmp >> 6) & 0x03;					// RA location			(XXxx xxxx)
	Adapt = (tmp >> 5) & 0x01;					// adaptive order		(xxXx xxxx)
	CoefTable = (tmp >> 3) & 0x03;				// entropy coding table	(xxxX Xxxx)
	PITCH = (tmp >> 2) & 0x01;					// pitch Coding	(LTP)	(xxxx xXxx)
	P = tmp & 0x03;								// pred. order (MSBs)	(xxxx xxXX)

	fread(&tmp, 1, 1, fpInput);					// pred. order (LSBs)	(XXXX XXXX)
	P = (P << 8) | tmp;

	fread(&tmp, 1, 1, fpInput);
	Sub = (tmp >> 6) & 0x03;					// block switching		(XXxx xxxx)
	if (Sub)
		Sub += 2;
	BGMC = (tmp >> 5) & 0x01;					// entropy coding		(xxXx xxxx)
	SBpart = (tmp >> 4) & 0x01;					// subblock partition	(xxxX xxxx)
	Joint = (tmp >> 3) & 0x01;					// joint stereo			(xxxx Xxxx)
	MCC = (tmp >> 2) & 0x01;					// multi-channel coding	(xxxx xXxx)
	ChanConfig = (tmp >> 1) & 0x01;				// channel config.		(xxxx xxXx)
	ChanSort = tmp & 0x01;						// new channel sorting	(xxxx xxxX)

	fread(&tmp, 1, 1, fpInput);
	CRCenabled = (tmp >> 7) & 0x01;				// CRC enabled			(Xxxx xxxx)
	RLSLMS = (tmp >> 6) & 0x01;					// RLSLMS mode			(xXxx xxxx)
	AUXenabled = tmp & 0x01;					// AUX data present		(xxxx xxxX)

	if (ChanConfig)
		ReadUShortMSBfirst(fpInput);			// channel configuration data (not used)

	if (ChanSort)
	{
		CBitIO in;
		unsigned long u;
		ChPos = new unsigned short[Chan];

		i = (Chan > 1) ? (Chan-1) : 1;
		NeedPuchBit = 0;
		while(i) {i /= 2; NeedPuchBit++;}

		long size = (NeedPuchBit*Chan)/8+1;
		buff = new unsigned char[size];
		fread(buff, 1, size, fpInput);
		in.InitBitRead(buff);

		for (i = 0; i < Chan; i++)
		{
			in.ReadBits(&u, NeedPuchBit);
			ChPos[i] = u;
		}
		in.EndBitRead();
		delete [] buff;
	}

	HeaderSize = ReadUShortMSBfirst(fpInput);	// header size
	TrailerSize = ReadUShortMSBfirst(fpInput);	// trailer size
	// End of header information //////////////////////////////////////////////////////////////////

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

	encinfo->FrameLength = N;
	encinfo->AdaptOrder = Adapt;
	encinfo->JointCoding = Joint;
	encinfo->SubBlocks = Sub;
	encinfo->RandomAccess = RA;
	encinfo->BGMC = BGMC;
	encinfo->MaxOrder = P;
	encinfo->MCC = MCC;
	encinfo->PITCH = PITCH;
	encinfo->RAflag = RAflag;
	encinfo->CRCenabled = CRCenabled;

	return(0);
}

short CLpacDecoder::OpenOutputFile(const char *name)
{
	if (name[0] == '\0')
		fpOutput = NULL;
	else if (name[0] == ' ')
	{
		fpOutput = stdout;
#ifdef WIN32
		if (_setmode(_fileno(stdout), _O_BINARY) == -1)
		{
			fprintf(stderr, "Cannot set 'stdout' mode\n");
			return(1);
		}
#endif
	}
	else if (!(fpOutput = fopen(name, "wb")))
	{
		fclose(fpInput);
		return(2);
	}

	return(0);
}

long CLpacDecoder::WriteHeader()
{
	long i, j, rest;

	// Copy header
	buff = new unsigned char[max(HeaderSize, TrailerSize)];
	fread(buff, 1, HeaderSize, fpInput);
	if (fpOutput != NULL)
		if (fwrite(buff, 1, HeaderSize, fpOutput) != HeaderSize) return(frames = -2);

	// Read trailer
	fread(buff, 1, TrailerSize, fpInput);

	if (CRCenabled)
	{
		CRCorg = ReadULongMSBfirst(fpInput);
		// Initialize CRC
		BuildCRCTable();
		CRC = CRC_MASK;
	}

	// Number of frames
	frames = Samples / N;
	rest = Samples % N;
	if (rest)
		frames++;

	if (RA)
	{
		// number of random acess units
		RAUnits = frames / RA;
		if (frames % RA)
			RAUnits++;
		RAUid = 0;			// RAU index (0..RAUnits-1)

		if (RAflag == 2)		// read random access info from header
		{
			RAUsize = new unsigned long[RAUnits];
			for (long r = 0; r < RAUnits; r++)
				RAUsize[r] = ReadULongMSBfirst(fpInput);
		}
	}

	if (AUXenabled)	// aux data present
	{
		long size = ReadUShortMSBfirst(fpInput);	// size of aux data
		fseek(fpInput, size, SEEK_CUR);				// skip aux data
	}

	if (Chan == 1)
		Joint = 0;

	// Allocate memory
	xp = new long*[Chan];
	x = new long*[Chan];
	for (i = 0; i < Chan; i++)
	{
		xp[i] = new long[N+P];
		x[i] = xp[i] + P;
		memset(xp[i], 0, 4*P);
	}
	
	// following buffer size is enough if only forward predictor is used.
	//	tmpbuf = new unsigned char[((long)((Res+7)/8)+1)*N + (4*P + 128)*(1+(1<<Sub))];
	// for RLS-LMS
	tmpbuf = new unsigned char[long(Res/8+10)*N + MAXODR * 4];

	if (RLSLMS)
	{
		rlslms_ptr.pbuf = new BUF_TYPE*[Chan];
		rlslms_ptr.weight  = new W_TYPE*[Chan];
		rlslms_ptr.Pmatrix  = new P_TYPE*[Chan];
		for (i = 0; i < Chan; i++)
		{
			rlslms_ptr.pbuf[i] = new BUF_TYPE[TOTAL_LMS_LEN];
			rlslms_ptr.weight[i]=new W_TYPE[TOTAL_LMS_LEN];
			rlslms_ptr.Pmatrix[i] = new P_TYPE[JS_LEN*JS_LEN];
			for(j=0;j<TOTAL_LMS_LEN;j++) rlslms_ptr.pbuf[i][j]=0;
		}
	}

	if (Joint)
	{
		xps = new long*[Chan/2];
		xs = new long*[Chan/2];

		for (i = 0; i < Chan/2; i++)
		{
			xps[i] = new long[N+P];
			xs[i] = xps[i] + P;
			memset(xps[i], 0, 4*P);
		}
	}

	// following buffer size is enough if only forward predictor is used.
	//	bbuf = new unsigned char[(((long)((Res+7)/8)+1)*N + (4*P + 128)*(1+(1<<Sub))) * Chan];	// Input buffer
	// for RLS-LMS
	bbuf = new unsigned char[long(Res/8+10)*Chan*N];	// Input buffer

	// Allocate float buffer
	if ( SampleType == SAMPLE_TYPE_FLOAT ) Float.AllocateBuffer( Chan, N, IntRes );
	// Allocate MCC buffer
	if (Freq > 96000L)
		NeedTdBit = 7;
	else if (Freq > 48000L)
		NeedTdBit = 6;
	else
		NeedTdBit = 5;

	AllocateMccDecBuffer( &MccBuf, Chan, N);

	// #bits/channel: NeedPuchBit = max(1,ceil(log2(Chan)))
	i = (Chan > 1) ? (Chan-1) : 1;
	NeedPuchBit = 0;
	while(i){
		i /= 2;
		NeedPuchBit++;
	}

	d = new long[N];								// Prediction residual
	cofQ = new long[P];								// Quantized coefficients

	// Length of last frame
	if (rest)
		N0 = rest;
	else
		N0 = N;

	return(frames);
}

short CLpacDecoder::WriteTrailer()
{
	if (fpOutput != NULL)
		if (fwrite(buff, 1, TrailerSize, fpOutput) != TrailerSize) return(-1);

	CRC ^= CRC_MASK;
	CRC -= CRCorg;		// CRC = 0 if decoding was successful

	return(TrailerSize);
}

short CLpacDecoder::DecodeAll()
{
	long f;

	if ((frames = WriteHeader()) < 1)
		return((short)frames);

	// Main loop for all frames
	for (f = 0; f < frames; f++)
	{
		// Decode one frame
		if (DecodeFrame())
			return(-2);
	}

	if (WriteTrailer() < 0)
		return(-2);

	return(CRC != 0);	// Return CRC status
}

unsigned long CLpacDecoder::GetCRC()
{
	return(CRC);
}

/*short CLpacDecoder::GetFrameSize()
{
	return(N * Chan * (Res / 8));
}

long CLpacDecoder::GetOrgFileSize()
{
	return(FileSize);
}

long CLpacDecoder::GetPacFileSize()
{
	long pos, size;

	pos = ftell(fpInput);
	fseek(fpInput, 0, SEEK_END);
	size = ftell(fpInput);
	fseek(fpInput, pos, SEEK_SET);

	return(size);
}*/

short GetBlockSequence(unsigned long BSflags, long N, long *Nb)
{
	short B = 0;

	if (!(BSflags & 0x40000000))
		Nb[B++] = N;
	else						// N/2
	{
		for (short b = 0; b < 2; b++)
		{
			if (!(BSflags & (0x20000000 >> b)))
				Nb[B++] = N >> 1;			
			else						// N/4
			{
				for (short bb = 0; bb < 2; bb++)
				{
					if (!(BSflags & (0x08000000 >> ((b<<1)+bb) )))
						Nb[B++] = N >> 2;
					else						// N/8
					{
						for (short bbb = 0; bbb < 2; bbb++)
						{
							if (!(BSflags & (0x00800000 >> ((b<<2)+(bb<<1)+bbb) )))
								Nb[B++] = N >> 3;
							else						// N/16
							{
								for (short bbbb = 0; bbbb < 2; bbbb++)
								{
									if (!(BSflags & (0x00008000 >> ((b<<3)+(bb<<2)+(bbb<<1)+bbbb) )))
										Nb[B++] = N >> 4;
									else						// N/32
									{
										Nb[B++] = N >> 5;
										Nb[B++] = N >> 5;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return(B);
}

// Decode frame
short CLpacDecoder::DecodeFrame()
{
	short RAframe;
	BYTE h1;
	long c, c1, c2, b, B, oaa;
	long i, Nb[32], NN, Nsum, mtp;
	ULONG BSflags;
	short CBS;
	BYTE h, typ, flag;

	long **xsave, **xssave, **xtmp;
	xsave = new long*[Chan];
	xssave = new long*[(long)(Chan/2)];
	xtmp = new long*[Chan];
	
	fid++;						// Number of current frame

	NN = N;

	// Random Access
	RAframe = RA;
	if (RA)
	{
		if (((fid - 1) % RA))	// Not first frame of RA unit
			RAframe = 0;		// Turn off RA for current frame
		else if (RAflag == 1)
			ReadULongMSBfirst(fpInput);		// read size of RAU
	}

	MCCflag=0;
	if(MCC)
	{
		MCCflag=1;
		if(Joint)
		{
			unsigned char uu;
			fread(&uu, 1, 1, fpInput);
			if(uu) MCCflag=1;
			else MCCflag=0;
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Normal coding (no multi-channel correlation method) ////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if(!MCCflag && !RLSLMS)
	{
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
			CBS = (Joint && (c < Chan - 1) && (c % 2 == 0));	// Joint coding, and not the last channel

			if (Sub)	// block switching enabled
			{
				// read block switching info
				fread(&h1, 1, 1, fpInput);
				if (h1 & 0x80)	// if independent block switching is indicated...
					CBS = 0;	// ...turn off channel coupling
				BSflags = h1 << 24;
				if (Sub > 3)
				{
					fread(&h1, 1, 1, fpInput);
					BSflags |= h1 << 16;
				}
				if (Sub > 4)
				{
					fread(&h1, 1, 1, fpInput);
					BSflags |= h1 << 8;
					fread(&h1, 1, 1, fpInput);
					BSflags |= h1;
				}

				// get #blocks B and block lengths Nb[]
				B = GetBlockSequence(BSflags, NN, Nb);
			}
			else		// fixed block length (= frame length)
			{
				B = 1;
				Nb[0] = N;
			}

			if (fid == frames)	// last frame needs some recalculation
			{
				Nsum = 0;
				B = 0;
				while (Nsum < N0)
					Nsum += Nb[B++];
				Nb[B-1] -= (Nsum - N0);
				N = N0;

				/*fprintf(stderr, "\nBSflags = %X\n", BSflags);
				for (b = 0; b < B; b++)
					fprintf(stderr, "\nNb[%d] = %d\n", b, Nb[b]);*/
			}

			if (CBS)	// Decode two coupled channels
			{
				c1 = c + 1;		// index of second channel
				c2 = c >> 1;	// index of difference channel

				for (b = 0; b < B; b++)
				{
					// Difference method
					fread(&h, 1, 1, fpInput);
					typ = h >> 6;
					flag = h & 0x20;
					fseek(fpInput, -1, SEEK_CUR);

					if ((typ == 0x03) || ((typ < 0x02) && flag))	// Channel 1 = difference signal
					{
						DecodeBlock(xs[c2], Nb[b], RAframe && (b == 0));
						DecodeBlock(x[c1], Nb[b], RAframe && (b == 0));

						for (i = 0; i < Nb[b]; i++)
							x[c][i] = x[c1][i] - xs[c2][i];
					}
					else											// Channel 1 = normal signal
					{
						DecodeBlock(x[c], Nb[b], RAframe && (b == 0));

						fread(&h, 1, 1, fpInput);
						typ = h >> 6;
						flag = h & 0x20;
						fseek(fpInput, -1, SEEK_CUR);

						if ((typ == 0x03) || ((typ < 0x02) && flag))	// Channel 2 = difference signal
						{
							DecodeBlock(xs[c2], Nb[b], RAframe && (b == 0));

							for (i = 0; i < Nb[b]; i++)
								x[c1][i] = x[c][i] + xs[c2][i];
						}
						else											// Channel 2 = normal signal
						{
							DecodeBlock(x[c1], Nb[b], RAframe && (b == 0));

							// Generate values for xs
							for (i = 0; i < Nb[b]; i++)
								xs[c2][i] = x[c1][i] - x[c][i];
						}
					}
					// Increment pointers (except for last subblock)
					if (b < B - 1)
					{
						x[c] = x[c] + Nb[b];
						x[c1] = x[c1] + Nb[b];
						if (Joint)
							xs[c2] = xs[c2] + Nb[b];
					}
				}
				// increment channel index
				c++;
			}
			else	// Decode one independent channel
			{
				for (b = 0; b < B; b++)
				{
					DecodeBlock(x[c], Nb[b], RAframe && (b == 0));

					// Increment pointers (except for last subblock)
					if (b < B - 1)
						x[c] = x[c] + Nb[b];
				}
				// Generate difference signal for subsequent frame
				if (Joint && (c % 2))	// c = 1, 3, 5, ...
				{
					c1 = c - 1;
					c2 = c1 >> 1;
					// Generate values for xs
					for (i = 0; i < N; i++)
						xs[c2][i] = xsave[c][i] - xsave[c1][i];
				}
			}
		}
		// End of Channels ////////////////////////////////////////////////////////////////////////////

		// Restore original pointers
		for (c = 0; c < Chan; c++){
			
			x[c] = xsave[c];
			//if (fid == frames) fprintf(stderr,"%d=%d\n",c,x[c][0]);
		}
		if (Joint)
		{
			for (c = 0; c < Chan/2; c++)
				xs[c] = xssave[c];
		}

		
	}
	else if (RLSLMS)//   RLSLMS -mode
	{
			short cpe,c0,c1,sce;
			//MCCflag = 0;
			if (fid == frames)	// last frame needs some recalculation
			{
				N = N0;
			}
			// Channel Pair Elements

			for (cpe = 0; cpe < Chan/2; cpe++)
			{
				// Channel numbers for this CPE
				c0 = 2 * cpe;
				c1 = c0 + 1;	

				if (!Joint)			// Independent Stereo
				{
					if (RLSLMS)
					{
						rlslms_ptr.channel=c0;
						DecodeBlockParameter( &MccBuf, c0, N, RAframe);
						DecodeBlockReconstructRLSLMS( &MccBuf, c0, x[c0]);
						synthesize(x[c0], N, &rlslms_ptr, RAframe || RLSLMS_ext==7, &MccBuf);

						rlslms_ptr.channel=c1;
						DecodeBlockParameter( &MccBuf, c1, N, RAframe);
						DecodeBlockReconstructRLSLMS( &MccBuf,  c1, x[c1]);

						synthesize(x[c1], N, &rlslms_ptr, RAframe || RLSLMS_ext==7, &MccBuf);
					}
				}
				else	// Joint Stereo
				{
					if (RLSLMS)
					{
						// Difference method
						char *xpr0=&MccBuf.m_xpara[c0];
						char *xpr1=&MccBuf.m_xpara[c1];
						
						DecodeBlockParameter( &MccBuf, c0, N, RAframe);
						DecodeBlockReconstructRLSLMS( &MccBuf, c0, x[c0]);
						DecodeBlockParameter( &MccBuf, c1, N, RAframe);
						DecodeBlockReconstructRLSLMS( &MccBuf, c1, x[c1]);
						rlslms_ptr.channel=c0;
						synthesize_joint(x[c0],x[c1], N, &rlslms_ptr, RAframe || RLSLMS_ext==7, mono_frame, &MccBuf);
					}
				}
			}

			// Single Channel Elements
			for (sce = 0; sce < Chan%2; sce++)
			{
				c0 = sce+2*(Chan/2);
				DecodeBlockParameter( &MccBuf, c0, N, RAframe);
				DecodeBlockReconstructRLSLMS( &MccBuf, c0, x[c0]);
				rlslms_ptr.channel=c0;
				synthesize(x[c0], N, &rlslms_ptr, RAframe || RLSLMS_ext==7, &MccBuf);
			}
			RLSLMS_ext=0; // reset for next frame
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// End of normal coding - Multi-channel correlation method ////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	else
	{

		// Save original pointers
		for (c = 0; c < Chan; c++)
			xsave[c] = x[c];
		if (Joint)
		{
			for (c = 0; c < Chan/2; c++)
				xssave[c] = xs[c];
		}

		if (Sub)	// block switching enabled
		{
			// read block switching info
			fread(&h1, 1, 1, fpInput);
			BSflags = h1 << 24;
			if (Sub > 3)
			{
				fread(&h1, 1, 1, fpInput);
				BSflags |= h1 << 16;
			}
			if (Sub > 4)
			{
				fread(&h1, 1, 1, fpInput);
				BSflags |= h1 << 8;
				fread(&h1, 1, 1, fpInput);
				BSflags |= h1;
			}

			// get #blocks B and block lengths Nb[]
			B = GetBlockSequence(BSflags, NN, Nb);
		}
		else		// fixed block length (= frame length)
		{
			B = 1;
			Nb[0] = N;
		}

		if (fid == frames)	// last frame needs some recalculation
		{
			Nsum = 0;
			B = 0;
			while (Nsum < N0)
				Nsum += Nb[B++];
			Nb[B-1] -= (Nsum - N0);
			N = N0;
		}
			
		for (b = 0; b < B; b++)
		{
			InitMccDecBuffer( &MccBuf );

			for(c = 0; c < Chan; c++)
				DecodeBlockParameter( &MccBuf, c, Nb[b], RAframe && (b == 0)); // Get residual and parameter

			for(oaa = 0; oaa < OAA; oaa++)
			{
				for(c = 0; c < Chan; c++)
				{
					MccBuf.m_tmppuchan[c] = MccBuf.m_puchan[c][OAA-1-oaa];
					MccBuf.m_tmptdtau[c] = MccBuf.m_tdtau[c][OAA-1-oaa];
					MccBuf.m_tmpMM[c] = MccBuf.m_MccMode[c][OAA-1-oaa];
					for(mtp = 0; mtp < Mtap; mtp++) MccBuf.m_mtgmm[c][mtp]=MccBuf.m_cubgmm[c][OAA-1-oaa][mtp];
				}
				ReconstructResidualTD( &MccBuf, Chan, Nb[b] ); // d[] = (subtracted d[]) + gamma*(reference d[])
			}

			for(c = 0; c < Chan; c++)
			{
				DecodeBlockReconstruct( &MccBuf, c, x[c], Nb[b], RAframe && (b == 0)); // Residual --> Original signal

				// Increment pointers (except for last subblock)
				if (b < B - 1)
					x[c] = x[c] + Nb[b];

				if (Joint && (c % 2))	// c = 1, 3, 5, ...
				{
					c1 = c - 1;
					c2 = c1 >> 1;
					// Generate values for xs
					for (i = 0; i < N; i++)
						xs[c2][i] = xsave[c][i] - xsave[c1][i];
				}
			}
		}

		// Restore original pointers
		for (c = 0; c < Chan; c++)
			x[c] = xsave[c];

		if (Joint)
		{
			for (c = 0; c < Chan/2; c++)
				xs[c] = xssave[c];
		}

	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// End of MCC /////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Copy last P samples of each frame in front of it
	for (c = 0; c < Chan; c++)
		memcpy(x[c] - P, x[c] + (N - P), P * 4);
	if (Joint)
	{
		for (c = 0; c < Chan/2; c++)
			memcpy(xs[c] - P, xs[c] + (N - P), P * 4);
	}

	// Write PCM audio data
	if ( SampleType == SAMPLE_TYPE_INT )
	{
		if (ChanSort)
		{
			// Restore original channel pointers
			for (c = 0; c < Chan; c++)
				xtmp[c] = x[c];
			for (c = 0; c < Chan; c++)
				x[ChPos[c]] = xtmp[c];
		}

		if (Res == 16)
		{
			if ((Write16BitNM(x, Chan, N, MSBfirst, bbuf, fpOutput) != 2L*Chan*N) && (fpOutput != NULL)) return(-1);
			CRC = CalculateBlockCRC32(2L*Chan*N, CRC, (void*)bbuf);
		}
		else if (Res == 8)
		{
			if ((Write8BitOffsetNM(x, Chan, N, bbuf, fpOutput) != (long)Chan*N) && (fpOutput != NULL)) return(-1);
			CRC = CalculateBlockCRC32((long)Chan*N, CRC, (void*)bbuf);
		}
		else if (Res == 24)
		{
			if ((Write24BitNM(x, Chan, N, MSBfirst, bbuf, fpOutput) != 3L*Chan*N) && (fpOutput != NULL)) return(-1);
			CRC = CalculateBlockCRC32(3L*Chan*N, CRC, (void*)bbuf);
		}
		else	// Res == 32
		{
			if ((Write32BitNM(x, Chan, N, MSBfirst, bbuf, fpOutput) != 4L*Chan*N) && (fpOutput != NULL)) return(-1);
			CRC = CalculateBlockCRC32(4L*Chan*N, CRC, (void*)bbuf);
		}

		if (ChanSort)
		{
			// Rearrange channel pointers
			for (c = 0; c < Chan; c++)
				xtmp[c] = x[c];
			for (c = 0; c < Chan; c++)
				x[c] = xtmp[ChPos[c]];
		}

	}
	else	// Floating-point PCM
		Float.ReformatData( x, N );

	if ( SampleType == SAMPLE_TYPE_FLOAT ) {
		if ( !Float.DecodeDiff( fpInput, N, RAframe != 0 ) ) return -1;
		if ( !Float.AddIEEEDiff( N ) ) return -1;
		if ( ChanSort ) Float.ChannelSort( ChPos, false );
		Float.ConvertFloatToRBuff( bbuf, N );
		if ( ChanSort ) Float.ChannelSort( ChPos, true );

		// Write floating point data into output file
		if ( fwrite( bbuf, 1, N * Chan * IEEE754_BYTES_PER_SAMPLE, fpOutput ) != N * Chan * IEEE754_BYTES_PER_SAMPLE ) {
			// Write error
			return -1;
		}
		CRC = CalculateBlockCRC32( Chan * N * IEEE754_BYTES_PER_SAMPLE, CRC, static_cast<void*>( bbuf ) );
	}

	delete [] xsave;
	delete [] xssave;
	delete [] xtmp;

	return(0);
}

// Decode block (Normal)
short CLpacDecoder::DecodeBlock(long *x, long Nb, short ra)
{
	DecodeBlockParameter(&MccBuf, 0, Nb, ra);
	DecodeBlockReconstruct(&MccBuf, 0, x, Nb, ra);
	return (0);
}

// Decode block parameter
void CLpacDecoder::DecodeBlockParameter(MCC_DEC_BUFFER *pBuffer, long Channel, long Nb, short ra)
{
	char *xpra = &pBuffer->m_xpara[Channel];
	long *d = pBuffer->m_dmat[Channel];
	long *puch = pBuffer->m_puchan[Channel];
	long *mccparq = pBuffer->m_parqmat[Channel];
	short *sft = &pBuffer->m_shift[Channel];
	short *oP = &pBuffer->m_optP[Channel];
	long oaa;
	//MCC-ex
	long *tdtauval = pBuffer->m_tdtau[Channel];
	short *MccMode = pBuffer->m_MccMode[Channel];
	long **mtgmm=pBuffer->m_cubgmm[Channel];

	BYTE h, hl[4];
	short BlockType, optP, shift = 0;
	long c, count, bytes, i, Ns;
    long asi[1023];
    long parq[1023];
	ULONG u;
	short s[16], sx[16], sub;
    // Bit-oriented input
    CBitIO in;
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

    /* rice code parameters for each coeff: */
	struct pv {int m,s;} *parcor_vars;

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

    optP = 10;

	// Read block header
	fread(&h, 1, 1, fpInput);
	BlockType = h >> 6;			// Type of block

	// ZERO BLOCK
	if (BlockType == 0)
	{
		*xpra=1;
	}
	// CONSTANT BLOCK
	else if (BlockType == 1)
	{
		fread( hl, 1, IntRes / 8, fpInput );
		if ( IntRes == 8 ) {
			c = static_cast<long>( hl[0] ) - 128;
		}
		else if (IntRes == 16)
		{
			c = short((hl[0] << 8) + hl[1]);
		}
		else if (IntRes == 24)
		{
			c = hl[2] | (hl[1] << 8) | (hl[0] << 16);
			if (c & 0x00800000)
				c = c | 0xFF000000;
		}
		else	// IntRes == 32
		{
			c = hl[3] | (hl[2] << 8) | (hl[1] << 16) | (hl[0] << 24);
		}
		
		d[0] = c;
		*xpra=2;
	}
	// NORMAL BLOCK
	else if (BlockType > 1)
	{
		fseek(fpInput, -1, SEEK_CUR);	// Go back one byte which was already read
		// following buffer size is enough if only forward predictor is used.
		//		count = fread(tmpbuf, 1, (((long)((Res+7)/8)+1)*Nb + 4*P + 128), fpInput);   // Fill input buffer
        // for RLS-LMS
		count = fread(tmpbuf, 1, long(Res/8+10) * Nb + P + 16 + 255*4, fpInput);   // Fill input buffer

        in.InitBitRead(tmpbuf);
		
		in.ReadBits(&u, 2);		// 1J

		if (!BGMC)
		{
			if (SBpart)
			{
				in.ReadBits(&u, 1);
				if (u)
					sub = 4;
				else
					sub = 1;
			}
			else
				sub = 1;
		}
		else
		{
			if (SBpart)
			{
				in.ReadBits(&u, 2);
				sub = 1 << u;
			}
			else
			{
				in.ReadBits(&u, 1);
				if (u)
					sub = 4;
				else
					sub = 1;
			}
		}

		// Code parameters for EC blocks
		if (!BGMC)
		{
			short sbits = Res <= 16 ? 4 : 5;
			in.ReadBits(&u, sbits);
			s[0] = u;
			for (i = 1; i < sub; i++)
			{
				in.ReadRice(&c, 0, 1);
				s[i] = s[i-1] + c;
			}
		}
		else
		{
			short sbits = Res <= 16 ? 8 : 9;
			ULONG S[16];
			in.ReadBits(&S[0], sbits);
			for (i = 1; i < sub; i++)
			{
				in.ReadRice(&c, 2, 1);
				S[i] = S[i-1] + c;
			}
			// Separate BGMC code parameters
			for (i = 0; i < sub; i++)
			{
				s[i] = S[i] >> 4;
				sx[i] = S[i] & 0x0F;
			}
		}

		// LSB shift
		shift = 0;
		in.ReadBits(&u, 1);
		if (u)
		{
			in.ReadBits(&u, 4);
			shift = short(u + 1);
		}
		if (!RLSLMS)
		{
			if (Adapt)
			{
				// Limit maximum predictor order to 1/8 of the block length
				short Pmax = P;
				if (P >= (Nb >> 3))
					Pmax = (Nb >> 3) - 1;
				if (Pmax < 1)
					Pmax = 1;	// -> h >= 0

				for (h = 9; Pmax < (1 << h); h--);
				in.ReadBits(&u, h+1);
				optP = (short)u;
			}
			else
				optP = P;

			/* decode quantized coefficients: */
			if (CoefTable == 3)
			{
				for (i = 0; i < optP; i++)
				{
					in.ReadBits(&u, 7);
					asi[i] = (long)u - 64;
				}
			}
			else
			{
				for (i = 0; i < min(optP,20); i++)
					asi[i] = parcor_vars[i].m + rice_decode (parcor_vars[i].s, &in.bio);
				for (; i < min(optP,127); i++)
					asi[i] = (i&1) + rice_decode (2, &in.bio);
				for (; i < optP; i++)
					asi[i] = rice_decode (1, &in.bio);
			}
			
			/* reconstruct parcor coefficients: */
			if (optP > 0)
				parq[0] = pc12_tbl[asi[0] + 64];
			if (optP > 1)
				parq[1] = -pc12_tbl[asi[1] + 64];
			for (i = 2; i < optP; i++)
				parq[i] = (asi[i] << (Q -6)) + (1 << (Q-7));
		}

		if(PITCH) pBuffer->m_Ltp.Decode( Channel, Nb, Freq, &in );

		Ns = Nb / sub;
		if (!ra)	// No random access
		{
			if (!BGMC)	// Use Rice codes
			{
				for (i = 0; i < sub; i++)
					in.ReadRice(d+(i*Ns), (char)s[i], Ns);
			}
			else		// Use block Gilbert-Moore codes for central region
				bgmc_decode_blocks(d, 0, s, sx, Nb, sub, &in.bio);
		}
		else		// Random access
		{
			// Progressive prediction: Separate entropy coding of the first 3 samples
			if (optP > 0)
				in.ReadRice(d, Res-4, 1);
			if (optP > 1)
				in.ReadRice(d+1, min(s[0]+3, 31), 1);
			if (optP > 2)
				in.ReadRice(d+2, min(s[0]+1, 31), 1);

			// Number of separately coded samples
			short num = optP < 3 ? optP : 3;

	        if (!BGMC)
			{
				in.ReadRice(d + num, (char)s[0], Ns - num);	// First EC block is 'num' samples shorter
				for (i = 1; i < sub; i++)						// Remaining 3 EC blocks
					in.ReadRice(d + i*Ns, (char)s[i], Ns);
			}
			else
			{
				bgmc_decode_blocks(d, num, s, sx, Nb, sub, &in.bio);
			}
		}

		for(i = 0; i < optP; i++)
			mccparq[i] = parq[i];
		*sft = shift;
		*oP = optP;
		*xpra = 0;

		if (!RLSLMS && !MCCflag)
		{
			bytes = in.EndBitRead();					// Number of bytes read
			fseek(fpInput, bytes - count, SEEK_CUR);	// Set working pointer to current position
		}
	}

	if (RLSLMS)
	{
		if (BlockType<=1)
		{
			count = fread(tmpbuf, 1, 400, fpInput);   // Fill input buffer
			in.InitBitRead(tmpbuf);
		}
		in.ReadBits(&u,1);
		mono_frame = u;
		in.ReadBits(&u,1);
		if (u!=0)
		{
			in.ReadBits(&u,3);
			RLSLMS_ext = u;
			//printf("%d %d ",RLSLMS_ext,optP);
			if (RLSLMS_ext&0x01) 
			{
				c_mode_table.filter_len[0]=1;
				in.ReadBits(&u,4);
				c_mode_table.filter_len[1]=(u)<<1;
				in.ReadBits(&u,3);
				c_mode_table.nstage=u+2;
				for(i=2;i<c_mode_table.nstage;i++)
				{
					in.ReadBits(&u,5);
					c_mode_table.filter_len[i]=lms_order_table[u];						
				}
			}
			if (RLSLMS_ext&0x02)
			{
				if (c_mode_table.filter_len[1]){
					in.ReadBits(&u,10);
					c_mode_table.lambda[0]=u;
					in.ReadBits(&u,10);
					c_mode_table.lambda[1]=u;
				}
			}
			if (RLSLMS_ext&0x04)
			{
				for(i=2;i<c_mode_table.nstage;i++)
				{
					in.ReadBits(&u,5);
					c_mode_table.opt_mu[i]=mu_table[u];
				}
				in.ReadBits(&u,3);
				c_mode_table.step_size=u*LMS_MU_INT;
			}
		}
		if(!MCCflag)
		{
			bytes = in.EndBitRead();					// Number of bytes read
			fseek(fpInput, bytes - count, SEEK_CUR);	// Set working pointer to current position
		}
	}

	// MCC Coefficients (Get referene channels and weighting factor)
	if(MCCflag)
	{
		if ( (BlockType<=1) && !RLSLMS)
		{
			count = fread(tmpbuf, 1, 256, fpInput);   // Fill input buffer
			in.InitBitRead(tmpbuf);
		}

		for (oaa = 0; oaa < OAA+1; oaa++)
		{
			in.ReadBits(&u,1); // Endflag
			if(u)
			{
				break;
			}
		
			in.ReadBits(&u,NeedPuchBit); // Reference Channel
			puch[oaa]=(long) u;
		
			if(puch[oaa] == Channel)
			{
				// not need weighting factor
			}
			else
			{
				in.ReadBits(&u,1);

				if(!u)
				{
					MccMode[oaa]=1;

					mtgmm[oaa][0]=rice_decode (1, &in.bio); // -1
					mtgmm[oaa][1]=rice_decode (2, &in.bio); //  0
					mtgmm[oaa][2]=rice_decode (1, &in.bio); // +1
					mtgmm[oaa][1] -= 2;
				}
				else
				{
					MccMode[oaa]=2;

					mtgmm[oaa][0]=rice_decode (1, &in.bio); // -1
					mtgmm[oaa][1]=rice_decode (2, &in.bio); //  0
					mtgmm[oaa][2]=rice_decode (1, &in.bio); // +1
					mtgmm[oaa][3]=rice_decode (1, &in.bio); // Tau-1
					mtgmm[oaa][4]=rice_decode (1, &in.bio); // Tau
					mtgmm[oaa][5]=rice_decode (1, &in.bio); // Tau+1
					mtgmm[oaa][1] -= 2;

					ULONG v;
					in.ReadBits(&v,1);
					in.ReadBits(&u,NeedTdBit);
					tdtauval[oaa]=(long)u+3;
					if(v) tdtauval[oaa] *=-1;
				}
				
				for(i = 0; i < Mtap; i++)
						mtgmm[oaa][i] += 16;
			}
		}
		bytes = in.EndBitRead();					// Number of bytes read
		fseek(fpInput, bytes - count, SEEK_CUR);	// Set working pointer to current position
	}
}

short	CLpacDecoder::DecodeBlockReconstruct( MCC_DEC_BUFFER* pBuffer, long Channel, long* x, long Nb, short ra )
{
	CLtpBuffer*	pLtpBuf = pBuffer->m_Ltp.m_pBuffer + Channel;
	long*	parq = pBuffer->m_parqmat[Channel];
	long*	d = pBuffer->m_dmat[Channel];
	short	shift = pBuffer->m_shift[Channel];
	short	optP = pBuffer->m_optP[Channel];
	short	xpara = pBuffer->m_xpara[Channel];
	long	i;
	long*	xtmp;

	if ( xpara == 1 ) {
		memset( x, 0, 4 * Nb );

	} else if ( xpara == 2 ) {
		for( i=0; i<Nb; i++ ) x[i] = d[0];

	} else {
		if ( shift ) {
			xtmp = new long [optP];
			// "Shift" last P samples of previous block
			for( i=-optP; i<0; i++ ) {
				xtmp[optP+i] = x[i];		// buffer original values
				x[i] >>= shift;
			}
		}

		if ( PITCH ) {
			long*	dd = pLtpBuf->m_ltpmat + 2048;
			if ( pLtpBuf->m_ltp ) {
				memcpy( dd, d, Nb * sizeof(long) );
				memset( dd-2048, 0, 2048 * sizeof(long) );
				pBuffer->m_Ltp.PitchReconstruct( dd, Nb, optP, Channel, Freq );
				memcpy( dd-2048, dd+Nb-2048, 2048 * sizeof(long) );
				memcpy( d, dd, Nb * sizeof(long) );
			} else {
				memcpy( dd, d, Nb * sizeof(long) );
				memcpy( dd-2048, dd+Nb-2048, 2048 * sizeof(long) );
			}
		}

		if ( !ra ) {
			// Conversion from parcor to direct form coefficients
			par2cof( cofQ, parq, optP, Q );
			// Calculate original signal
			GetSignal( x, Nb, optP, Q, cofQ, d );
		} else {
			// Progressive prediction (internal conversion)
			GetSignalRA( x, Nb, optP, Q, parq, cofQ, d );
		}

		if ( shift ) {
			// Undo "shift" of whole block (and restore optP samples of the previous block)
			for( i=-optP; i<0; i++ ) x[i] = xtmp[optP+i];
			for( i=0; i<Nb; i++ ) x[i] <<= shift;
			delete[] xtmp;
		}
	}
	return 0;
}

// Decode block reconstruct
short CLpacDecoder::DecodeBlockReconstructRLSLMS(MCC_DEC_BUFFER *pBuffer, long Channel, long *x)
{
	CLtpBuffer*	pLtpBuf = pBuffer->m_Ltp.m_pBuffer + Channel;
	long *parq = pBuffer->m_parqmat[Channel];
	long *d = pBuffer->m_dmat[Channel];
	short shift = pBuffer->m_shift[Channel];
	short optP = pBuffer->m_optP[Channel];
	short xpara = pBuffer->m_xpara[Channel];

	short i;

	if(xpara == 1)
		memset(x, 0, 4 * N);
	else if(xpara == 2)
		for(i = 0; i < N; i++) x[i]=d[0];
	else
	{
		if (shift)
		{
			// "Shift" last P samples of previous block
			for (i = -optP; i < 0; i++)
				x[i] >>= shift;
		}
		// rescontruct the pitch information
		if ( PITCH ) {
			long*	dd = pLtpBuf->m_ltpmat + 2048;
			if ( pLtpBuf->m_ltp ) {
				memcpy( dd, d, N * sizeof(long) );
				memset( dd-2048, 0, 2048 * sizeof(long) );
				pBuffer->m_Ltp.PitchReconstruct( dd, N, 10, Channel, Freq );
				memcpy( dd-2048, dd+N-2048, 2048 * sizeof(long) );
				memcpy( d, dd, N * sizeof(long) );
			} else {
				memcpy( dd, d, N * sizeof(long) );
				memcpy( dd-2048, dd+N-2048, 2048 * sizeof(long) );
			}
		}
		// restore the sample from d array
		for(i=0;i<N;i++) x[i]=d[i];
		if (shift)
		{
			// Undo "shift" of whole block (and P samples of the previous block)
			for (i = -optP; i < N; i++)
				x[i] <<= shift;
		}
	}

	// Copy last P samples of this block in front of it
	memcpy(x - P, x + (N - P), P * 4);

	return(0);
}
