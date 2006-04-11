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

filename : mp4als.cpp
project  : MPEG-4 Audio Lossless Coding
author   : Tilman Liebchen (Technical University of Berlin)
date     : June 25, 2003
contents : Main file for MPEG-4 Audio Lossless Coding framework

*************************************************************************/

/*************************************************************************
 *
 * Modifications:
 *
 * 08/31/2003, Yuriy A. Reznik <yreznik@real.com>
 *  - added -b option enabling advanced encoding of the residual.
 *
 * 11/26/2003, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   some additions to support multichannel and new file format
 *   - added option -m for channel rearrangement
 *   - added option -u for universal mode
 *   - added options -R -C -W -F -M -H -T for arbitrary audio formats
 *
 * 12/16/2003, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   some bug fixes, resulting in updates of the following files:
 *   - cmdline.cpp: definition of GetOptionValueL()
 *   - cmdline.h: declaration of GetOptionValueL()
 *   - encoder.cpp: new sizes of tmpbuf<1,2,3>[] arrays
 *   - decoder.cpp: new size of tmpbuf[] array
 *   - mp4als.cpp: uses GetOptionValueL(), set version to CE2 rev 3
 *
 * 12/17/2003, Koichi Sugiura <ksugiura@mitaka.ntt-at.co.jp>
 *   - changed short main() to int main().
 *   - supported floating point PCM.
 *   - added option -S0 for integer PCM and -S1 for float PCM for raw format.
 *
 * 03/24/2004, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - changed version to CE4
 *   - progressive prediction for random access frames
 *   - removed support for file format version < 8
 *
 * 5/17/2004, Yuriy Reznik <yreznik@real.com>
 *   - changed version to CE5
 *   - removed support for file format versions < 9
 *
 * 7/30/2004, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - changed version to CE6
 *   - enabled options for adaptive prediction order
 *
 * 10/26/2004, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - changed version to CE7
 *
 * 11/05/2004, Yutaka Kamamoto <kamamoto@hil.t.u-tokyo.ac.jp>
 *   - changed version to RM8
 *   - added option -s# for multi-channel correlation method
 *
 * 11/18/2004, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - added option -noACF to skip ACF search.
 *
 * 02/07/2005, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - changed version to CE10
 *   - renamed option -noACF into -t (to avoid conflict with -n)
 *   - adjusted screen output (verbose mode decoding)
 *
 * 02/11/2005, Takehiro Moriya (NTT) <t.moriya@ieee.org>
 *   - add LTP modules for CE11
 *
 * 03/08/2005, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - integration of CE10 and CE11
 *   - changed version to RM11
 *   - added option -p# (pitch prediction)
 *   - added option -g# (block switching)
 *
 * 03/24/2005, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - added option -f# (ACF/MLZ mode)
 *   - integration of CE12
 *
 * 04/27/2005, Yutaka Kamamoto <kamamoto@theory.brl.ntt.co.jp>
 *   - added option -t# (Joint Stereo-MCC switching mode)
 *   - changed MCC(RM8) to multi-tap MCC
 *   - changed option -p# to -p (LTP)
 *
 * 06/08/2005, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - syntax changes according to N7134
 *
 * 07/07/2005, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - changed version to RM15v2
 *
 * 07/21/2005, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - changed version to RM15v3
 *   - fixed a bug in order to make the -c option work
 *
 * 07/21/2005, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - changed version to RM15x
 *
 * 08/22/2005, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *   - changed version to RM16
 *
 * 03/29/2006, Tilman Liebchen <liebchen@nue.tu-berlin.de>
 *             Noboru Harada <n-harada@theory.brl.ntt.co.jp>
 *   - changed version to RM17
 *   - made code consistant with ALS corrigendum
 *   - allow prediction order zero (-o0)
 *   - some minor bug fixes
 *
 ************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "wave.h"
#include "encoder.h"
#include "decoder.h"
#include "cmdline.h"
#include "audiorw.h"

#ifdef WIN32
	#define SYSTEM_STR "Win32"
#else
	#define SYSTEM_STR "Linux"
#endif

#define CODEC_STR "mp4alsRM17"

void ShowUsage(void);
void ShowHelp(void);

int main(short argc, char **argv)
{
	clock_t start, finish;
	short verbose, decode, info, autoname = 0, output, len, crc = 0, result = 0, input, wrongext;
	char *infile, *outfile, tmp[255], *tmp2;
	double playtime;
	AUDIOINFO ainfo;
	ENCINFO encinfo;

	// Check parameters ///////////////////////////////////////////////////////////////////////////
	if (argc == 1)								// "codec" without parameters
	{
		ShowUsage();
		return(1);
	}

	if (CheckOption(argc, argv, "-h"))			// "codec ... -h ..."
	{
		ShowHelp();
		return(0);
	}

	output = !strcmp(argv[argc-1], "-");		// "codec ... -", output to stdout

	if (argc < 3)
	{
		if (output)								// "codec -", not allowed
		{
			ShowUsage();
			return(1);
		}
		else									// "codec <infile>"
			autoname = 1;
	}

	input = !strcmp(argv[argc-2], "-");			// "codec ... - ...", input from stdin
	if (input && output)						// "codec ... - -", not allowed
	{
		ShowUsage();
		return(1);
	}

	if ((!output) && (argv[argc-1][0] == '-'))	// "codec ... -<...>", not allowed
	{
		ShowUsage();
		return(1);
	}

	if ((!input) && (argv[argc-2][0] == '-'))	// "codec -<...> <file/->
	{
		if (output)								// "codec -<...> -", not allowed
		{
			ShowUsage();
			return(1);
		}
		else									// "codec -<...> <file>"
			autoname = 1;
	}

	verbose = CheckOption(argc, argv, "-v");
	decode = CheckOption(argc, argv, "-x");
	info = CheckOption(argc, argv, "-I");

	if (info)
	{
		//decode = 1;
		verbose = 1;
	}

	if (output)
		verbose = 0;

	start = clock();

	// Encoder mode ///////////////////////////////////////////////////////////////////////////////
	if (!decode)
	{
		CLpacEncoder encoder;

		if (autoname)		// Automatic generation of output file name
		{
			infile = argv[argc-1];
			strcpy(tmp, infile);
			len = strlen(tmp);

			wrongext = 0;
			if (len < 5)
				wrongext = 1;
			else
			{
				tmp2 = strrchr(tmp, '.');
				if (tmp2 == NULL)
					wrongext = 1;
				else if ((tmp2 - tmp + 1) > (len - 3))
					wrongext = 1;
			}

			if (!wrongext)
				wrongext = strcmp(tmp2, ".wav") && strcmp(tmp2, ".WAV") &&
							strcmp(tmp2, ".aif") && strcmp(tmp2, ".AIF") &&
							strcmp(tmp2, ".aiff") && strcmp(tmp2, ".AIFF") &&
							strcmp(tmp2, ".raw") && strcmp(tmp2, ".RAW") &&
							strcmp(tmp2, ".pcm") && strcmp(tmp2, ".PCM");

			if (wrongext)
			{
				fprintf(stderr, "\nFile %s doesn't seem to be a supported sound file!\n", infile);
				exit(2);
			}

			strcpy(tmp2 + 1, "als");
			outfile = tmp;
		}
		else if (output)	// Output to stdout
		{
			infile = argv[argc-2];
			strcpy(tmp, " ");
			outfile = tmp;
		}
		else if (input)		// Input from stdin
		{
			strcpy(tmp, " ");
			infile = tmp;
			outfile = argv[argc-1];
		}
		else
		{
			infile = argv[argc-2];
			outfile = argv[argc-1];
		}

		// Open Input File
		if (result = encoder.OpenInputFile(infile))
		{
			fprintf(stderr, "\nUnable to open file %s for reading!\n", infile);
			exit(1);
		}

		// Raw input file
		if (encoder.SetRawAudio(CheckOption(argc, argv, "-R")))
		{
			encoder.SetChannels(GetOptionValue(argc, argv, "-C"));
			encoder.SetSampleType(GetOptionValue(argc, argv, "-S"));
			encoder.SetWordlength(GetOptionValue(argc, argv, "-W"));
			encoder.SetFrequency(GetOptionValue(argc, argv, "-F"));
			encoder.SetMSBfirst(CheckOption(argc, argv, "-M"));
			encoder.SetHeaderSize(GetOptionValue(argc, argv, "-H"));
			encoder.SetTrailerSize(GetOptionValue(argc, argv, "-T"));
		}

		// Analyse Input File
		if (result = encoder.AnalyseInputFile(&ainfo))
		{
			fprintf(stderr, "\nFile type of %s is not supported!\n", infile);
			exit(2);
		}

		short res = ainfo.Res;
		short intres = ainfo.IntRes;
		unsigned char samptype = ainfo.SampleType;
		long freq = ainfo.Freq;
		long chan = ainfo.Chan;
		long samp = ainfo.Samples;
		short type = ainfo.FileType;
		short msb = ainfo.MSBfirst;
		unsigned long head = ainfo.HeaderSize;
		unsigned long trail = ainfo.TrailerSize;

		if (info)
		{
			printf("\nInfo on %s\n", infile);

			// Screen output
			printf("\nAudio Properties");
			printf("\n  Sample type   : %s", ( samptype == 0 ) ? "int" : "float");
			printf("\n  Resolution    : %d bit", res);
			if ( samptype == 1 ) printf( " (int %d bit)", intres );
			printf("\n  Sample Rate   : %d Hz", freq);
			printf("\n  Channels      : %d ch", chan);
			printf("\n  Samples/Chan. : %d", samp);
			printf("\n  Duration      : %.1f sec\n", playtime = (double)samp / freq);

			char typestr[16];
			if (type == 0) strcpy(typestr, "RAW");
			else if (type == 1) strcpy(typestr, "WAVE");
			else if (type == 2) strcpy(typestr, "AIFF");
			else strcpy(typestr, "UNKNOWN");

			printf("\nFormat Properties");
			printf("\n  File Type    : %s", typestr);
			printf("\n  Byte Order   : %s", msb ? "MSB first" : "LSB first");
			printf("\n  Header Size  : %d bytes", head);
			printf("\n  Trailer Size : %d bytes\n", trail);

			exit(0);
		}

		// Set encoder options ////////////////////////////////////////////////////////////////////
		encoder.SetAdapt(CheckOption(argc, argv, "-a"));
		if (CheckOption(argc, argv, "-i"))
			encoder.SetJoint(-1);									// Independent Coding
		else
			encoder.SetJoint(0);									// Joint Coding of all CPEs
		encoder.SetLSBcheck(CheckOption(argc, argv, "-l"));
		long N = encoder.SetFrameLength(GetOptionValue(argc, argv, "-n"));
		short ord = GetOptionValue(argc, argv, "-o", -1);
		encoder.SetOrder(ord);
		if(ord == 0) encoder.SetAdapt(0);							// force fixed order
		short ra = encoder.SetRA(GetOptionValue(argc, argv, "-r"));
		encoder.SetRAmode(GetOptionValue(argc, argv, "-u"));
		encoder.SetBGMC(CheckOption(argc, argv, "-b"));				// BGMC mode
		encoder.SetMCC(GetOptionValue(argc, argv, "-t"));			// Two methods mode (Joint Stereo and Multi-channel correlation)
		encoder.SetPITCH(CheckOption(argc, argv, "-p"));			// PITCH mode (LTP)
		encoder.SetSub(GetOptionValue(argc, argv, "-g"));			// block switching level
		encoder.SetCRC(!CheckOption(argc, argv, "-e"));				// disable CRC
		
		long mccnojs = GetOptionValue(argc, argv, "-s");
		if (mccnojs)
		{
			if (encoder.SetMCC(mccnojs))
				encoder.SetMCCnoJS(1);
			encoder.SetJoint(-1);
		}

		encoder.SetHEMode(GetOptionValue(argc, argv, "-z"));

		// Optimum compression
		if (CheckOption(argc, argv, "-7"))
		{
			// Common options
			encoder.SetAdapt(1);
			encoder.SetBGMC(1);
			encoder.SetSub(5);
			encoder.SetLSBcheck(1);

			// Order and frame length
			if (freq <= 48000)
			{
				if (ord < 0)
					encoder.SetOrder(1023);
				if (N == 0)
					N = encoder.SetFrameLength(20480);
			}
			else if (freq <= 96000)
			{
				if ((ord < 0) || (ord > 511))
					encoder.SetOrder(511);		//
				if (N == 0)
					N = encoder.SetFrameLength(20480);
			}
			else
			{
				if ((ord < 0) || (ord > 127))
					encoder.SetOrder(127);
				if (N == 0)
					N = encoder.SetFrameLength(30720);
			}

			// Adjust frame length for random access
			if ((ra > 0) && (ra <= ((10 * N - 1) / freq)))
			{
				N = ra * freq / 10;			// maximum possible frame length
				N = (N / 256) * 256;		// multiple of 256
				encoder.SetFrameLength(N);
			}
		}

		// Parse -f option
		short	AcfMode, MlzMode;
		float	AcfGain = 0.f;
		if ( !GetFOption( argc, argv, AcfMode, AcfGain, MlzMode ) ) {
			fprintf( stderr, "\nInvalid -f option.\n" );
			exit( 1 );
		}
		encoder.SetAcf( AcfMode, AcfGain );
		encoder.SetMlz( MlzMode );

		// reordering of channels
		if (chan > 2)
		{
			unsigned short *pos = new unsigned short[chan];
			encoder.SetChanSort(GetOptionValues(argc, argv, "-m", chan, pos), pos);
			delete [] pos;
		}

		// Open Output File
		if (result = encoder.OpenOutputFile(outfile))
		{
			fprintf(stderr, "\nUnable to open file %s for writing!\n", outfile);
			exit(1);
		}

		// Screen output (1)
		if (verbose)
		{
			printf("\nPCM file: %s", strcmp(infile, " ") ? infile : "-");
			printf("\nALS file: %s", outfile);
			printf("\n\nEncoding...   0%%");
			fflush(stdout);
		}

		// Encoding ///////////////////////////////////////////////////////////////////////////////
		if (!verbose)
		{
			result = encoder.EncodeAll();
		}
		else
		{
			long frames, f, fpro, fpro_alt = 0, step = 1;

			// Generate and write header
			if ((frames = encoder.WriteHeader(&encinfo)) < 0)
				result = (short)frames;

			if (frames < 1000)
				step = 10;
			else if (frames < 2000)
				step = 5;
			else if (frames < 5000)
				step = 2;

			// Main loop for all frames
			for (f = 0; f < frames; f++)
			{
				// Encode frame
				if (encoder.EncodeFrame())
				{
					result = -2;
					break;
				}
				if (verbose)
				{
					fpro = (f + 1) * 100 / frames;
					if ((fpro >= fpro_alt + step) || (fpro == 100))
					{
						printf("\b\b\b\b%3d%%", fpro_alt = fpro);
						fflush(stdout);
					}
				}
			}

			// Encode non-audio bytes (if there are any)
			if (!result)
			{
				if (encoder.WriteTrailer() < 0)
					result = -2;
			}
		}
		// End of encoding ////////////////////////////////////////////////////////////////////////

		if (result == -1)
		{
			fprintf(stderr, "\nERROR: %s is not a supported sound file!\n", infile);
			exit(2);
		}
		else if (result == -2)
		{
			fprintf(stderr, "\nERROR: Unable to write to %s - disk full?\n", outfile);
			exit(1);
		}

		// Screen output (2)
		if (verbose)
		{
			short res = ainfo.Res;
			short intres = ainfo.IntRes;
			unsigned char samptype = ainfo.SampleType;
			long freq = ainfo.Freq;
			long chan = ainfo.Chan;
			long samp = ainfo.Samples;
			long pcmsize, alssize;
			encoder.GetFilePositions(&pcmsize, &alssize);
			double ratio = (double)pcmsize / alssize;
			playtime = (double)samp / freq;

			printf(" done\n");
			printf("\nAudio format : %s / %d bit ", ( samptype == 0 ) ? "int" : "float", res );
			if ( samptype == 1 ) printf( "(int %d bit) ", intres );
			printf( "/ %ld Hz / %d ch", freq, chan);
			printf("\nBit rate     : %.1f kbit/s", freq * chan * res / 1000.0);
			printf("\nPlaying time : %.1f sec", playtime);
			printf("\nPCM file size: %ld bytes", pcmsize);
			printf("\nALS file size: %ld bytes", alssize);
			printf("\nCompr. ratio : %.3f (%.2f %%)", ratio, 100 / ratio);
			printf("\nAverage bps  : %.3f", res / ratio);
			printf("\nAverage rate : %.1f kbit/s\n", freq * chan * res / (ratio * 1000));
			fflush(stdout);
		}

		// Check for accurate decoding
		if (CheckOption(argc, argv, "-c"))
		{
			if (encoder.CloseFiles())
			{
				fprintf(stderr, "\nUnable to close files. Check not possible\n");
				exit(1);
			}

			if (verbose)
			{
				printf("\nCheck for accurate decoding...");
				fflush(stdout);
			}

			CLpacDecoder decoder;

			if (result = decoder.OpenInputFile(outfile))
			{
				fprintf(stderr, "\nUnable to open file %s for reading!\n", infile);
				exit(1);
			}

			if (result = decoder.AnalyseInputFile(&ainfo, &encinfo))
			{
				fprintf(stderr, "\nERROR: %s is not a valid ALS file!\n", infile);
				exit(2);
			}

			decoder.OpenOutputFile("");

			crc = decoder.DecodeAll();

			if (crc == -1)
				fprintf(stderr, "\nERROR: %s is not a valid ALS file!\n", outfile);
			else if (crc < -1)
				fprintf(stderr, "\nUNKNOWN DECODING ERROR!\n");
			else if (crc > 0)
				fprintf(stderr, "\nDECODING ERROR: CRC failed!\n");
			else if (verbose)
			{
				printf(" Ok!\n");
				fflush(stdout);
			}
		}
	}
	// Decoder mode ///////////////////////////////////////////////////////////////////////////////
	else
	{
		CLpacDecoder decoder;

		if (autoname)		// Automatic generation of output file name
		{
			infile = argv[argc-1];
			strcpy(tmp, infile);
			len = strlen(tmp);

			wrongext = 0;
			if (len < 5)
				wrongext = 1;
			else
			{
				tmp2 = strrchr(tmp, '.');
				if (tmp2 == NULL)
					wrongext = 1;
				else if ((tmp2 - tmp + 1) > (len - 3))
					wrongext = 1;
			}

			if (!wrongext)
				wrongext = strcmp(tmp2, ".als") && strcmp(tmp2, ".ALS") &&
							strcmp(tmp2, ".pac") && strcmp(tmp2, ".PAC");
			if (wrongext)
			{
				fprintf(stderr, "\nFile %s does't seem to be an ALS file (.als/.pac)!\n", infile);
				exit(2);
			}
			// Output file name depends on original file type, see below
		}
		else if (output)	// Output to stdout
		{
			infile = argv[argc-2];
			strcpy(tmp, " ");
			outfile = tmp;
		}
		else if (input)		// Input from stdin
		{
			strcpy(tmp, " ");
			infile = tmp;
			outfile = argv[argc-1];
		}
		else
		{
			infile = argv[argc-2];
			outfile = argv[argc-1];
		}

		// Open Input File
		if (result = decoder.OpenInputFile(infile))
		{
			fprintf(stderr, "\nUnable to open file %s for reading!\n", infile);
			exit(1);
		}

		// Analyse Input File
		if (result = decoder.AnalyseInputFile(&ainfo, &encinfo))
		{
			fprintf(stderr, "\nERROR: %s is not a valid ALS file!\n", infile);
			exit(2);
		}

		if (verbose)
		{
			if (info)
				printf("\nInfo on %s\n", infile);

			// Screen output
			short res = ainfo.Res;
			short intres = ainfo.IntRes;
			unsigned char samptype = ainfo.SampleType;
			long freq = ainfo.Freq;
			long chan = ainfo.Chan;
			long samp = ainfo.Samples;
			short type = ainfo.FileType;
			short msb = ainfo.MSBfirst;
			unsigned long head = ainfo.HeaderSize;
			unsigned long trail = ainfo.TrailerSize;

			printf("\nAudio Properties");
			printf("\n  Sample type   : %s", ( samptype == 0 ) ? "int" : "float");
			printf("\n  Resolution    : %d bit", res);
			if ( samptype == 1 ) printf( " (int %d bit)", intres );
			printf("\n  Sample Rate   : %d Hz", freq);
			printf("\n  Channels      : %d ch", chan);
			printf("\n  Samples/Chan. : %d", samp);
			printf("\n  Duration      : %.1f sec\n", playtime = (double)samp / freq);

			long fl = encinfo.FrameLength;
			short ap = encinfo.AdaptOrder;
			short mo = encinfo.MaxOrder;
			short ra = encinfo.RandomAccess;
			short js = encinfo.JointCoding;
			short sb = encinfo.SubBlocks;
			short bg = encinfo.BGMC;
			short mc = encinfo.MCC;
			short pi = encinfo.PITCH;

			printf("\nCodec Properties");
			printf("\n  Frame Length  : %d", fl);
			printf("\n  BS Level      : %d", sb);
			printf("\n  Pred. Order   : %d%s", mo, ap == 0 ? "" : " (max)");
			printf("\n  Random Access : %s (%d frames)", ra == 0 ? "no" : "yes", ra);
			printf("\n  BGMC Coding   : %s", bg == 0 ? "no" : "yes");
			printf("\n  Joint Stereo  : %s", js == 0 ? "no" : "yes");
			printf("\n  MCC enabled   : %s", mc == 0 ? "no" : "yes");
			printf("\n  LTP enabled   : %s\n", pi == 0 ? "no" : "yes");

			char typestr[16];
			if (type == 0) strcpy(typestr, "RAW");
			else if (type == 1) strcpy(typestr, "WAVE");
			else if (type == 2) strcpy(typestr, "AIFF");
			else strcpy(typestr, "UNKNOWN");

			printf("\nFormat Properties");
			printf("\n  Orig. Format  : %s", typestr);
			printf("\n  Byte Order    : %s", msb ? "MSB first" : "LSB first");
			printf("\n  Header Size   : %d bytes", head);
			printf("\n  Trailer Size  : %d bytes\n", trail);

			if (info)
				exit(0);
		}

		if (autoname)
		{
			// Append original file extension (if known)
			if (ainfo.FileType == 1)
				strcpy(tmp2 + 1, "wav");
			else if (ainfo.FileType == 2)
				strcpy(tmp2 + 1, "aif");
			else
				strcpy(tmp2 + 1, "raw");

			outfile = tmp;
		}

		// Open Output File
		if (result = decoder.OpenOutputFile(outfile))
		{
			fprintf(stderr, "\nUnable to open file %s for writing!\n", outfile);
			exit(1);
		}

		// Decoding ///////////////////////////////////////////////////////////////////////////////
		if (!verbose)
			crc = decoder.DecodeAll();
		else
		{
			long frames, f, fpro, fpro_alt = 0, step = 1;

			// Analyse and decode header
			if ((frames = decoder.WriteHeader()) < 1)
				crc = (short)frames;
			else
			{
				printf("\nALS file: %s", strcmp(infile, " ") ? infile : "-");
				printf("\nPCM file: %s\n", outfile);

				printf("\nDecoding...   0%%");
				fflush(stdout);

				if (frames < 1000)
					step = 10;
				else if (frames < 2000)
					step = 5;
				else if (frames < 5000)
					step = 2;

				// Main loop for all frames
				for (f = 0; f < frames; f++)
				{
					// Decode frame
					if (decoder.DecodeFrame())
					{
						crc = -2;
						break;
					}
					if (verbose)
					{
						fpro = (f + 1) * 100 / frames;
						if ((fpro >= fpro_alt + step) || (fpro == 100))
						{
							printf("\b\b\b\b%3d%%", fpro_alt = fpro);
							fflush(stdout);
						}
					}
				}

				// Decode non-audio bytes (if there are any)
				if (!crc)
				{
					if (decoder.WriteTrailer() < 0)
						crc = -2;
					else
						crc = (decoder.GetCRC() != 0);
				}
			}
		}
		// End of decoding ////////////////////////////////////////////////////////////////////////

		// Screen output
		if (crc == -1)
		{
			fprintf(stderr, "\nERROR: %s is not a valid ALS file!\n", infile);
			exit(2);
		}
		else if (crc == -2)
		{
			fprintf(stderr, "\nERROR: Unable to write to %s - disk full?\n", outfile);
			exit(1);
		}
		else if (verbose)
		{
			printf(" done\n");
			printf("\nCRC status: %s\n", encinfo.CRCenabled ? (crc ? "FAILED!" : "ok") : "n/a");
			fflush(stdout);
		}
		else if ((encinfo.CRCenabled) && (crc > 0))
			fprintf(stderr, "\nDECODING ERROR: CRC failed for %s\n", infile);
	}

	// Delete input file?
	if (!crc && CheckOption(argc, argv, "-d"))
		remove(infile);

	// Show elapsed time //////////////////////////////////////////////////////////////////////////
	finish = clock();
	if (verbose)
	{
		double cputime = (double)(finish - start) / CLOCKS_PER_SEC;
		if (verbose)
			printf("\nProcessing took %.2f sec (%.1f x real-time)\n", cputime, playtime / cputime);
		else
			printf("%6.2f | %4.1f\n", cputime, playtime / cputime);
	}

	fflush(stdout);

	if (crc)		// Decoding failed
		exit(2);

	return(0);
}

// Show usage message
void ShowUsage()
{
	fprintf(stderr, "Usage: %s [options] infile [outfile]\n", CODEC_STR);
	fprintf(stderr, "For help, type: %s -h\n", CODEC_STR);
}

// Show help screen
void ShowHelp()
{
	printf("\n%s - MPEG-4 Audio Lossless Coding (ALS), Reference Model Codec", CODEC_STR);
    printf("\n  Version 17 for %s", SYSTEM_STR);
	printf("\n  (c) 2003-2006 Tilman Liebchen, Technical University of Berlin");
	printf("\n    E-mail: liebchen@nue.tu-berlin.de");
    printf("\n  Portions by Yuriy A. Reznik, RealNetworks, Inc.");
    printf("\n    E-mail: yreznik@real.com");
    printf("\n  Portions by Koichi Sugiura, NTT Advanced Technology corporation");
	printf("\n    E-mail: ksugiura@mitaka.ntt-at.co.jp");
    printf("\n  Portions by Takehiro Moriya, Noboru Harada and Yutaka Kamamoto, NTT");
	printf("\n    E-mail: t.moriya@ieee.org, {n-harada,kamamoto}@theory.brl.ntt.co.jp");
    printf("\n");
	printf("\nUsage: %s [options] infile [outfile]\n", CODEC_STR);
	printf("\n  In compression mode, infile must be a PCM file (wav, aif, or raw format)");
	printf("\n  or a 32-bit floating point file (normalized, wav format type 3).");
	printf("\n  Mono, stereo, and multichannel files with up to 65536 channels and up to");
	printf("\n  32-bit resolution are supported at any sampling frequency.");
	printf("\n  In decompression mode (-x), infile is the compressed file (.als).");
	printf("\n  If outfile is not specified, the name of the output file will be generated");
	printf("\n  by replacing the extension of the input file (wav <-> als).");
	printf("\n  If outfile is '-', the output will be written to stdout. If infile is '-',");
	printf("\n  the input will be read from stdin, and outfile has to be specified.\n");
	printf("\nGeneral Options:");
	printf("\n  -c : Check accuracy by decoding the whole file after encoding.");
	printf("\n  -d : Delete input file after completion.");
	printf("\n  -h : Help (this message)");
	printf("\n  -v : Verbose mode (file info, processing time)");
	printf("\n  -x : Extract (all options except -v are ignored)");
	printf("\nEncoding Options:");
	printf("\n  -7 : Set parameters for optimum compression (except LTP, MCC, RLSLMS)");
	printf("\n  -a : Adaptive prediction order");
	printf("\n  -b : Use BGMC codes for prediction residual (default: use Rice codes)");
	printf("\n  -e : Exclude CRC calculation");
	printf("\n  -f#: ACF/MLZ mode: # = 0-7, -f6/-f7 requires ACF gain value");
	printf("\n  -g#: Block switching level: 0 = off (default), 5 = maximum");
	printf("\n  -i : Independent stereo coding (turn off joint stereo coding)");
	printf("\n  -l : Check for empty LSBs (e.g. 20-bit files)");
	printf("\n  -m#: Rearrange channel configuration (example: -m1,2,4,5,3)");
	printf("\n  -n#: Frame length: 0 = auto (default), max = 65536");
	printf("\n  -o#: Prediction order (default = 10), max = 1023");
	printf("\n  -p : Use long-term prediction");
	printf("\n  -r#: Random access (multiples of 0.1 sec), -1 = each frame, 0 = off (default)");
	printf("\n  -s#: Multi-channel correlation (#=1-65536, jointly code every # channels)");
	printf("\n       # must be a divisor of number of channels, otherwise -s is ignored");
	printf("\n  -t#: Two methods mode (Joint Stereo and Multi-channel correlation)");
	printf("\n       # must be a divisor of number of channels");
	printf("\n  -u#: Random access info location, 0 = frames (default), 1 = header, 2 = none");
	printf("\n  -z#: RLSLMS mode (default = 0: no RLSLMS mode,  1-quick, 2-medium 3-best )");
	printf("\nAudio file support:");
	printf("\n  -R : Raw audio file (use -C, -W, -F and -M to specifiy format)");
	printf("\n  -S#: Sample type: 0 = integer (default), 1 = float");
	printf("\n  -C#: Number of Channels (default = 2)");
	printf("\n  -W#: Word length in bits per sample (default = 16)");
	printf("\n  -F#: Sampling frequency in Hz (default = 44100)");
	printf("\n  -M : 'MSByte first' byte order (otherwise 'LSByte first')");
	printf("\n  -H#: Header size in bytes (default = 0)");
	printf("\n  -T#: Trailer size in bytes (default = 0)");
	printf("\n  -I : Show info only, no (de)compression (add -x for compressed files)");
	printf("\n");
	printf("\nExamples:");
	printf("\n  %s -v sound.wav", CODEC_STR);
	printf("\n  %s -n1024 -i -o20 sound.wav", CODEC_STR);
	printf("\n  %s - sound.als < sound.wav", CODEC_STR);
	printf("\n  %s -x sound.als", CODEC_STR);
	printf("\n  %s -x sound.als - > sound.wav", CODEC_STR);
	printf("\n  %s -I -x sound.als", CODEC_STR);
	printf("\n");
	return;
}
