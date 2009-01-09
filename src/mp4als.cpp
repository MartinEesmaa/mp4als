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
 * 07/10/2006,  Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 * -09/19/2006, Noboru Harada <n-harada@theory.brl.ntt.co.jp>
 *   - added MP4 file format support
 *   - changed version to RM18
 *
 * 05/10/2007, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - changed version to RM18b
 *   - fixed a bug in AudioSpecificConfig
 *
 * 05/23/2007, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - changed version to RM19
 *   - supported 64-bit data size
 *   - supported Sony Wave64 and BWF with RF64 formats
 *
 * 05/25/2007, Nobory Harada <n-harada@theory.brl.ntt.co.jp>
 *   - debugged.
 *   - modified -7 option.
 *
 * 06/08/2007, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - added error messages for encoding failure.
 *   - removed SetMp4Info().
 *
 * 07/02/2007, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - added error message for encoding failure (memory error).
 *
 * 07/15/2007, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - added -OAFI command line option.
 *   - supported ALSSpecificConfig with header and trailer embedded.
 *
 * 07/23/2007, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - updated AlsImf library.
 *
 * 08/10/2007, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - debugged autoname in decoder.
 *
 * 10/20/2008, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - changed version to RM22.
 *   - updated AlsImf library.
 *
 * 10/22/2008, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - modified process exit code.
 *
 * 11/28/2008, Csaba Kos <csaba.kos@ex.ssh.ntt-at.co.jp>
 *   - fixed the signature of main().
 *   - fixed some printf format strings
 *   - changed version to RM22rev1.
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
#include "als2mp4.h"

#if defined( WIN32 )
	#define SYSTEM_STR "Win32"
#elif defined( WIN64 )
	#define SYSTEM_STR "Win64"
#elif defined( __linux__ )
	#define SYSTEM_STR "Linux"
#elif defined( __APPLE__ )
	#define SYSTEM_STR "Mac OS X"
#else
	#define SYSTEM_STR "(unknown)"
#endif

#if defined( _MSC_VER )
#define	PRINTF_LL	"%I64d"
#else
#define	PRINTF_LL	"%lld"
#endif

#define CODEC_STR "mp4alsRM22rev1"
#define	ALS_TMP_FILENAME	CODEC_STR ".$$$"

void ShowUsage(void);
void ShowHelp(void);

int main(int argc, char **argv)
{
	clock_t start, finish;
	short verbose, decode, info, autoname = 0, output, len, crc = 0, result = 0, input, wrongext;
	char *infile, *outfile, tmp[255], *tmp2;
	double playtime;
	AUDIOINFO ainfo;
	ENCINFO encinfo;
	bool	mp4file;
	bool	oafi_flag;
	MP4INFO	mp4info;

	// Initialize MP4INFO structure.
	mp4info.m_StripRaInfo = false;
	mp4info.m_RaLocation = 0;
	mp4info.m_pInFile = mp4info.m_pOutFile = mp4info.m_pOriginalFile = NULL;
	mp4info.m_Samples = 0;
	mp4info.m_HeaderSize = mp4info.m_TrailerSize = mp4info.m_AuxDataSize = 0;
	mp4info.m_HeaderOffset = mp4info.m_TrailerOffset = mp4info.m_AuxDataOffset = 0;
	mp4info.m_FileType = 0;
	mp4info.m_RMflag = false;
	mp4info.m_UseMeta = false;

	// Check parameters ///////////////////////////////////////////////////////////////////////////
	if (argc == 1)								// "codec" without parameters
	{
		ShowUsage();
		return(3);
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
			return(3);
		}
		else									// "codec <infile>"
			autoname = 1;
	}

	input = !strcmp(argv[argc-2], "-");			// "codec ... - ...", input from stdin
	if (input && output)						// "codec ... - -", not allowed
	{
		ShowUsage();
		return(3);
	}

	if ((!output) && (argv[argc-1][0] == '-'))	// "codec ... -<...>", not allowed
	{
		ShowUsage();
		return(3);
	}

	if ((!input) && (argv[argc-2][0] == '-'))	// "codec -<...> <file/->
	{
		if (output)								// "codec -<...> -", not allowed
		{
			ShowUsage();
			return(3);
		}
		else									// "codec -<...> <file>"
			autoname = 1;
	}

	verbose = CheckOption(argc, argv, "-v");
	decode = CheckOption(argc, argv, "-x");
	info = CheckOption(argc, argv, "-I");
	mp4file = ( CheckOption( argc, argv, "-MP4" ) != 0 );
	oafi_flag = ( CheckOption( argc, argv, "-OAFI" ) != 0 );

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
							strcmp(tmp2, ".pcm") && strcmp(tmp2, ".PCM") &&
							strcmp(tmp2, ".bwf") && strcmp(tmp2, ".BWF") &&
							strcmp(tmp2, ".w64") && strcmp(tmp2, ".W64");

			if (wrongext)
			{
				fprintf(stderr, "\nFile %s doesn't seem to be a supported sound file!\n", infile);
				exit(2);
			}

			strcpy(tmp2 + 1, mp4file ? "mp4" : "als");
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
			// If outfile ends with ".mp4", then -MP4 option is assumed.
			tmp2 = strrchr( outfile, '.' );
			if ( ( tmp2 != NULL ) && ( ( strcmp( tmp2, ".mp4" ) == 0 ) || ( strcmp( tmp2, ".MP4" ) == 0 ) ) ) mp4file = true;
		}

		// Open Input File
		if (result = encoder.OpenInputFile(infile))
		{
			fprintf(stderr, "\nUnable to open file %s for reading!\n", infile);
			exit(3);
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
		ALS_INT64 samp = ainfo.Samples;
		short type = ainfo.FileType;
		short msb = ainfo.MSBfirst;
		ALS_INT64 head = ainfo.HeaderSize;
		ALS_INT64 trail = ainfo.TrailerSize;

		if (info)
		{
			printf("\nInfo on %s\n", infile);

			// Screen output
			printf("\nAudio Properties");
			printf("\n  Sample type   : %s", ( samptype == 0 ) ? "int" : "float");
			printf("\n  Resolution    : %d bit", res);
			if ( samptype == 1 ) printf( " (int %d bit)", intres );
			printf("\n  Sample Rate   : %ld Hz", freq);
			printf("\n  Channels      : %ld ch", chan);
			printf("\n  Samples/Chan. : " PRINTF_LL, samp);
			printf("\n  Duration      : %.1f sec\n", playtime = (double)samp / freq);

			char typestr[16];
			if (type == 0) strcpy(typestr, "RAW");
			else if (type == 1) strcpy(typestr, "WAVE");
			else if (type == 2) strcpy(typestr, "AIFF");
			else if (type == 3) strcpy(typestr, "BWF");
			else if (type == 4) strcpy(typestr, "Sony Wave64");
			else if (type == 5) strcpy(typestr, "BWF with RF64");
			else strcpy(typestr, "UNKNOWN");

			printf("\nFormat Properties");
			printf("\n  File Type    : %s", typestr);
			printf("\n  Byte Order   : %s", msb ? "MSB first" : "LSB first");
			printf("\n  Header Size  : " PRINTF_LL " bytes", head);
			printf("\n  Trailer Size : " PRINTF_LL " bytes\n", trail);

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
		short bs = GetOptionValue(argc, argv, "-g");				// block switching level
		encoder.SetSub(bs);
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
			if ((bs<=0) || (bs>5)) encoder.SetSub(5);
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
			exit( 3 );
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

		// Use oafi when either samples, header size, trailer size exceeds 4GB.
		if ( mp4file && ( ( type & ~0x3 ) || ( ainfo.Samples >= 0xffffffff ) || ( ainfo.HeaderSize >= 0xffffffff ) || ( ainfo.TrailerSize >= 0xffffffff ) ) ) oafi_flag = true;

		// Parse -MP4x options
		if ( mp4file ) {
			// Check options.
			if ( strcmp( outfile, " " ) == 0 ) { fprintf( stderr, "\nstdout is not available for MP4 file format.\n" ); exit( 3 ); }
			if ( CheckOption( argc, argv, "-c" ) ) { fprintf( stderr, "\n-c option is not available for MP4 file format.\n" ); exit( 3 ); }
			if ( GetOptionValue( argc, argv, "-u" ) == 2 ) { fprintf( stderr, "\n-u2 option is not available for MP4 file format.\n" ); exit( 3 ); }

			// Build up MP4INFO structure.
			mp4info.m_pInFile = ALS_TMP_FILENAME;
			mp4info.m_pOutFile = outfile;
			mp4info.m_pOriginalFile = infile;
			mp4info.m_Samples = ainfo.Samples;
			mp4info.m_HeaderSize = oafi_flag ? ainfo.HeaderSize : 0;
			mp4info.m_TrailerSize = oafi_flag ? ainfo.TrailerSize : 0;
			mp4info.m_AuxDataSize = 0;	// Not used.
			mp4info.m_HeaderOffset = mp4info.m_TrailerOffset = mp4info.m_AuxDataOffset = 0;	// Not used in encoding.
			mp4info.m_FileType = static_cast<NAlsImf::IMF_UINT8>( type );
			mp4info.m_FileTypeName.erase();
			mp4info.m_RMflag = true;
			mp4info.m_StripRaInfo = true;	// true:Strip RA info / false:Do not strip RA info
			mp4info.m_RaLocation = 0;		// RAU size location: 0=frames, 1=header, 2=none
			mp4info.m_UseMeta = oafi_flag;
			outfile = (char*)ALS_TMP_FILENAME;
		}

		// Open Output File
		if (result = encoder.OpenOutputFile(outfile, mp4file, oafi_flag ))
		{
			fprintf(stderr, "\nUnable to open file %s for writing!\n", outfile);
			exit(1);
		}

		// Screen output (1)
		if (verbose)
		{
			printf("\nPCM file: %s", strcmp(infile, " ") ? infile : "-");
			if ( mp4file ) printf( "\nMP4 file: %s", mp4info.m_pOutFile );
			else printf("\nALS file: %s", outfile);
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
			long fpro, fpro_alt = 0, step = 1;
			ALS_INT64 frames, f;

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
					fpro = static_cast<long>( (f + 1) * 100 / frames );
					if ((fpro >= fpro_alt + step) || (fpro == 100))
					{
						printf("\b\b\b\b%3ld%%", fpro_alt = fpro);
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

		// Converting ALS to MP4 //////////////////////////////////////////////////////////////////
		if ( mp4file ) {
			A2MERR	A2mErr = AlsToMp4( mp4info );
			if ( A2mErr != A2MERR_NONE ) {
				fprintf( stderr, "\nERROR: Unable to convert ALS to MP4: %s\n", ToErrorString( A2mErr ) );
				encoder.CloseFiles();
				remove( ALS_TMP_FILENAME );
				exit( 1 );
			}
			outfile = const_cast<char*>( mp4info.m_pOutFile );
		}

		// End of encoding ////////////////////////////////////////////////////////////////////////

		if ( result < 0 ) {
			encoder.CloseFiles();
			if ( mp4file ) remove( ALS_TMP_FILENAME );
			switch( result ) {
			case -1:
				fprintf(stderr, "\nERROR: %s is not a supported sound file!\n", infile);
				exit(2);
			case -2:
				fprintf(stderr, "\nERROR: Unable to write to %s - disk full?\n", outfile);
				exit(1);
			case -3:
				fprintf(stderr, "\nERROR: Number of RAUs exceeds the limit of 0x7fffffff.\n");
				exit(3);
			case -4:
				fprintf(stderr, "\nERROR: Number of samples exceeds the limit of 0xffffffff in ALS format.\n");
				exit(3);
			case -5:
				fprintf(stderr, "\nERROR: File header is too big for ALS format!\n");
				exit(3);
			case -6:
				fprintf(stderr, "\nERROR: File trailer is too big for ALS format!\n");
				exit(3);
			case -7:
				fprintf(stderr, "\nERROR: Memory error!\n");
				exit(3);
			default:
				fprintf(stderr, "\nERROR: Unexpected error.\n");
				exit(3);
			}
		}

		// Screen output (2)
		if (verbose)
		{
			short res = ainfo.Res;
			short intres = ainfo.IntRes;
			unsigned char samptype = ainfo.SampleType;
			long freq = ainfo.Freq;
			long chan = ainfo.Chan;
			ALS_INT64 samp = ainfo.Samples;
			ALS_INT64 pcmsize, alssize, mp4size;
			encoder.GetFilePositions(&pcmsize, &alssize);
			mp4size = 0;
			if ( mp4file ) {
				HALSSTREAM hMp4Stream = NULL;
				if ( OpenFileReader( mp4info.m_pOutFile, &hMp4Stream ) == 0 ) {
					fseek( hMp4Stream, 0, SEEK_END );
					mp4size = ftell( hMp4Stream );
					fclose( hMp4Stream );
				}
			}
			double ratio = (double)pcmsize / alssize;
			playtime = (double)samp / freq;

			printf(" done\n");
			printf("\nAudio format : %s / %d bit ", ( samptype == 0 ) ? "int" : "float", res );
			if ( samptype == 1 ) printf( "(int %d bit) ", intres );
			printf( "/ %ld Hz / %ld ch", freq, chan);
			printf("\nBit rate     : %.1f kbit/s", freq * chan * res / 1000.0);
			printf("\nPlaying time : %.1f sec", playtime);
			printf("\nPCM file size: " PRINTF_LL " bytes", pcmsize);
			if ( mp4file ) printf("\nMP4 file size: " PRINTF_LL " bytes", mp4size);
			else printf("\nALS file size: " PRINTF_LL " bytes", alssize);
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
				exit(3);
			}

			if (verbose)
			{
				printf("\nCheck for accurate decoding...");
				fflush(stdout);
			}

			CLpacDecoder decoder;

			if (result = decoder.OpenInputFile(outfile, mp4file))
			{
				fprintf(stderr, "\nUnable to open file %s for reading!\n", outfile);
				exit(3);
			}

			if (result = decoder.AnalyseInputFile(&ainfo, &encinfo, mp4info))
			{
				fprintf(stderr, "\nERROR: %s is not a valid ALS file!\n", outfile);
				decoder.CloseFiles();
				exit(2);
			}

			decoder.OpenOutputFile("");

			crc = decoder.DecodeAll( mp4info );

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
			decoder.CloseFiles();
		} else {
			encoder.CloseFiles();
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
							strcmp(tmp2, ".mp4") && strcmp(tmp2, ".MP4");
			if (wrongext)
			{
				fprintf(stderr, "\nFile %s does't seem to be an ALS file (.als or .mp4)!\n", infile);
				exit(2);
			}
			tmp2 = strrchr( tmp, '.' );
			// Output file name depends on original file type, see below
		}
		else if (output)	// Output to stdout
		{
			infile = argv[argc-2];
			strcpy(tmp, " ");
			outfile = tmp;
			tmp2 = strrchr( infile, '.' );
		}
		else if (input)		// Input from stdin
		{
			strcpy(tmp, " ");
			infile = tmp;
			outfile = argv[argc-1];
			tmp2 = strrchr( tmp, '.' );
		}
		else
		{
			infile = argv[argc-2];
			outfile = argv[argc-1];
			tmp2 = strrchr( infile, '.' );
		}

		// Turn on -MP4 option, when input file ends with ".mp4".
		if ( ( tmp2 != NULL ) && ( ( strcmp( tmp2, ".mp4" ) == 0 ) || ( strcmp( tmp2, ".MP4" ) == 0 ) ) ) mp4file = true;

		if ( mp4file ) {
			// Check options.
			if ( strcmp( infile, " " ) == 0 ) { fprintf( stderr, "\nstdin is not available for MP4 file format.\n" ); exit( 3 ); }

			// Build up MP4INFO structure.
			mp4info.m_pInFile = infile;
			mp4info.m_pOutFile = ALS_TMP_FILENAME;
			mp4info.m_RMflag = true;
			mp4info.m_StripRaInfo = true;	// true:Strip RA info / false:Do not strip RA info
			mp4info.m_RaLocation = 0;		// RAU size location: 0=frames, 1=header, 2=none

			// Convert MP4 to ALS.
			A2MERR	A2mErr = Mp4ToAls( mp4info );
			if ( A2mErr != A2MERR_NONE ) {
				fprintf( stderr, "\nERROR: Unable to convert MP4 to ALS: %s\n", ToErrorString( A2mErr ) );
				remove( ALS_TMP_FILENAME );
				exit( 1 );
			}
		}

		// Open Input File
		if (result = decoder.OpenInputFile(mp4file ? mp4info.m_pOutFile : infile, mp4file))
		{
			fprintf(stderr, "\nUnable to open file %s for reading!\n", mp4file ? mp4info.m_pOutFile : infile);
			if ( mp4file ) remove( ALS_TMP_FILENAME );
			exit(3);
		}

		// Analyse Input File
		if (result = decoder.AnalyseInputFile(&ainfo, &encinfo, mp4info))
		{
			fprintf(stderr, "\nERROR: %s is not a valid ALS file!\n", mp4file ? mp4info.m_pOutFile : infile);
			decoder.CloseFiles();
			if ( mp4file ) remove( ALS_TMP_FILENAME );
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
			ALS_INT64 samp = ainfo.Samples;
			short type = mp4file ? mp4info.m_FileType : ainfo.FileType;
			short msb = ainfo.MSBfirst;
			ALS_INT64 head = ainfo.HeaderSize;
			ALS_INT64 trail = ainfo.TrailerSize;

			printf("\nAudio Properties");
			printf("\n  Sample type   : %s", ( samptype == 0 ) ? "int" : "float");
			printf("\n  Resolution    : %d bit", res);
			if ( samptype == 1 ) printf( " (int %d bit)", intres );
			printf("\n  Sample Rate   : %ld Hz", freq);
			printf("\n  Channels      : %ld ch", chan);
			printf("\n  Samples/Chan. : " PRINTF_LL, samp);
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
			printf("\n  Frame Length  : %ld", fl);
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
			else if (type == 3) strcpy(typestr, "BWF");
			else if (type == 4) strcpy(typestr, "Sony Wave64");
			else if (type == 5) strcpy(typestr, "BWF with RF64");
			else if ( ( type == 15 ) && mp4file && !mp4info.m_FileTypeName.empty() ) {
				// Show original_MIME_type (ASCII char only)
				const char*	pName = mp4info.m_FileTypeName.c_str();
				while( *pName != '\0' ) {
					if ( ( *pName >= 0x20 ) && ( *pName <= 0x7e ) ) putchar( *pName );
					else putchar( ' ' );
				}
			} else strcpy(typestr, "UNKNOWN");

			printf("\nFormat Properties");
			printf("\n  Orig. Format  : %s", typestr);
			printf("\n  Byte Order    : %s", msb ? "MSB first" : "LSB first");
			printf("\n  Header Size   : " PRINTF_LL " bytes", head);
			printf("\n  Trailer Size  : " PRINTF_LL " bytes\n", trail);

			if (info) {
				decoder.CloseFiles();
				if ( mp4file ) remove( ALS_TMP_FILENAME );
				exit(0);
			}
		}

		if (autoname)
		{
			// Append original file extension (if known)
			static const char* KnownExt[5] = { "wav", "aif", "bwf", "w64", "bwf" };
			if ( ( ainfo.FileType >= 1 ) && ( ainfo.FileType <= 5 ) ) strcpy( tmp2+1, KnownExt[ ainfo.FileType-1 ] );
			else strcpy( tmp2+1, "raw" );
			outfile = tmp;
		}

		// Open Output File
		if (result = decoder.OpenOutputFile(outfile))
		{
			fprintf(stderr, "\nUnable to open file %s for writing!\n", outfile);
			decoder.CloseFiles();
			if ( mp4file ) remove( ALS_TMP_FILENAME );
			exit(1);
		}

		// Decoding ///////////////////////////////////////////////////////////////////////////////
		if (!verbose)
			crc = decoder.DecodeAll( mp4info );
		else
		{
			long fpro, fpro_alt = 0, step = 1;
			ALS_INT64 frames, f;

			// Analyse and decode header
			if ((frames = decoder.WriteHeader( mp4info )) < 1)
				crc = (short)frames;
			else
			{
				printf("\n%s file: %s", mp4file ? "MP4" : "ALS", strcmp(infile, " ") ? infile : "-");
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
						fpro = static_cast<long>( (f + 1) * 100 / frames );
						if ((fpro >= fpro_alt + step) || (fpro == 100))
						{
							printf("\b\b\b\b%3ld%%", fpro_alt = fpro);
							fflush(stdout);
						}
					}
				}

				// Decode non-audio bytes (if there are any)
				if (!crc)
				{
					if (decoder.WriteTrailer( mp4info ) < 0)
						crc = -2;
					else
						crc = (decoder.GetCRC() != 0);
				}
			}
		}
		decoder.CloseFiles();
		// End of decoding ////////////////////////////////////////////////////////////////////////

		// Screen output
		if (crc == -1)
		{
			fprintf(stderr, "\nERROR: %s is not a valid %s file!\n", infile, mp4file ? "MP4" : "ALS");
			if ( mp4file ) remove( ALS_TMP_FILENAME );
			exit(2);
		}
		else if (crc == -2)
		{
			fprintf(stderr, "\nERROR: Unable to write to %s - disk full?\n", outfile);
			if ( mp4file ) remove( ALS_TMP_FILENAME );
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

	// Removing temporary file.
	if ( mp4file ) remove( ALS_TMP_FILENAME );

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
    printf("\n  Version 22 for %s", SYSTEM_STR);
	printf("\n  (c) 2003-2008 Tilman Liebchen, TU Berlin / LG Electronics");
	printf("\n    E-mail: liebchen@nue.tu-berlin.de");
    printf("\n  Portions by Yuriy A. Reznik, RealNetworks, Inc.");
    printf("\n    E-mail: yreznik@real.com");
    printf("\n  Portions by Koichi Sugiura, NTT Advanced Technology corporation");
	printf("\n    E-mail: koichi.sugiura@ntt-at.co.jp");
    printf("\n  Portions by Takehiro Moriya, Noboru Harada and Yutaka Kamamoto, NTT");
	printf("\n    E-mail: {moriya.takehiro,harada.noboru,kamamoto.yutaka}@lab.ntt.co.jp");
    printf("\n");
	printf("\nUsage: %s [options] infile [outfile]\n", CODEC_STR);
	printf("\n  In compression mode, infile must be a PCM file (wav, aif, or raw format)");
	printf("\n  or a 32-bit floating point file (normalized, wav format type 3).");
	printf("\n  Mono, stereo, and multichannel files with up to 65536 channels and up to");
	printf("\n  32-bit resolution are supported at any sampling frequency.");
	printf("\n  In decompression mode (-x), infile is the compressed file (.als or .mp4).");
	printf("\n  If outfile is not specified, the name of the output file will be generated");
	printf("\n  by replacing the extension of the input file (wav <-> als).");
	printf("\n  If outfile is '-', the output will be written to stdout. If infile is '-',");
	printf("\n  the input will be read from stdin, and outfile has to be specified.\n");
	printf("\nGeneral Options:");
	printf("\n  -c  : Check accuracy by decoding the whole file after encoding.");
	printf("\n  -d  : Delete input file after completion.");
	printf("\n  -h  : Help (this message)");
	printf("\n  -v  : Verbose mode (file info, processing time)");
	printf("\n  -x  : Extract (all options except -v and -MP4 are ignored)");
	printf("\nEncoding Options:");
	printf("\n  -7  : Set parameters for optimum compression (except LTP, MCC, RLSLMS)");
	printf("\n  -a  : Adaptive prediction order");
	printf("\n  -b  : Use BGMC codes for prediction residual (default: use Rice codes)");
	printf("\n  -e  : Exclude CRC calculation");
	printf("\n  -f# : ACF/MLZ mode: # = 0-7, -f6/-f7 requires ACF gain value");
	printf("\n  -g# : Block switching level: 0 = off (default), 5 = maximum");
	printf("\n  -i  : Independent stereo coding (turn off joint stereo coding)");
	printf("\n  -l  : Check for empty LSBs (e.g. 20-bit files)");
	printf("\n  -m# : Rearrange channel configuration (example: -m1,2,4,5,3)");
	printf("\n  -n# : Frame length: 0 = auto (default), max = 65536");
	printf("\n  -o# : Prediction order (default = 10), max = 1023");
	printf("\n  -p  : Use long-term prediction");
	printf("\n  -r# : Random access (multiples of 0.1 sec), -1 = each frame, 0 = off (default)");
	printf("\n  -s# : Multi-channel correlation (#=1-65536, jointly code every # channels)");
	printf("\n        # must be a divisor of number of channels, otherwise -s is ignored");
	printf("\n  -t# : Two methods mode (Joint Stereo and Multi-channel correlation)");
	printf("\n        # must be a divisor of number of channels");
	printf("\n  -u# : Random access info location, 0 = frames (default), 1 = header, 2 = none");
	printf("\n  -z# : RLSLMS mode (default = 0: no RLSLMS mode,  1-quick, 2-medium 3-best )");
	printf("\nMP4 File Format Support:");
	printf("\n  -MP4: Use MP4 file format for compressed file (default if extension is .mp4)");
	printf("\n  -OAFI:Force to embed meta box with oafi record");
	printf("\nAudio file support:");
	printf("\n  -R  : Raw audio file (use -C, -W, -F and -M to specify format)");
	printf("\n  -S# : Sample type: 0 = integer (default), 1 = float");
	printf("\n  -C# : Number of Channels (default = 2)");
	printf("\n  -W# : Word length in bits per sample (default = 16)");
	printf("\n  -F# : Sampling frequency in Hz (default = 44100)");
	printf("\n  -M  : 'MSByte first' byte order (otherwise 'LSByte first')");
	printf("\n  -H# : Header size in bytes (default = 0)");
	printf("\n  -T# : Trailer size in bytes (default = 0)");
	printf("\n  -I  : Show info only, no (de)compression (add -x for compressed files)");
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
