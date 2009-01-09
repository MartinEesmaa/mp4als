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

Copyright (c) 2006.

filename : als2mp4.cpp
project  : MPEG-4 Audio Lossless Coding
author   : Tilman Liebchen (Technical University of Berlin)
date     : May 19, 2006
contents : Conversion of plain ALS files to MP4 files

*************************************************************************/

/*************************************************************************
 *
 * Modifications:
 *
 * May 22, 2006, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *  - added to show error messages.
 *  - renamed mp4LibWin.lib to mp4v2.lib, which is built from mpeg4ip 
 *    ver1.5 source codes with modification to integrate mp4helper.
 *    Now, mp4v2.lib is built for both Release(libc) and Debug(libcd).
 *  - renewed the VC++6 project.
 *  - replaced MP4GetTrackAudioType with MP4GetTrackEsdsObjectTypeId, 
 *    because MP4GetTrackAudioType is not available anymore.
 *
 * Jun 1, 2006, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *  - integrated need_for_win32.c into als2mp4.cpp.
 *  - modified GetOptionValue() to support long option name.
 *  - split main() into AlsToMp4() and Mp4ToAls().
 *  - added A2MERR enumeration and MP4OPT structure.
 *  - added Makefile to support Linux and Mac OS X.
 *  - created als2mp4.h.
 *  - updated MP4 header spec in N8166.
 *  - updated version number to "0.9".
 *
 * Jul 3, 2006, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *  - removed -c# option to link original mp4v2.lib.
 *  - added channel sort field handling.
 *
 * Jul 10, 2006, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *  - rewrote most of the codes.
 *
 * Aug 31, 2006, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *  - removed -e, -f, -F, -h, -i#, -m, -o options.
 *  - revised error codes.
 *  - replaced libisomediafile with AlsImf library.
 *
 * Sep 16, 2006, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *  - updated AlsImf library.
 *
 * May 10, 2007, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *  - updated version number to "0.91".
 *  - debugged the location of fillBits added by N8166.
 *
 * May 23, 2007, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *  - updated version number to "0.92".
 *  - updated AlsImf library.
 *  - updated to meet N9071.
 *  - supported 64-bit data size.
 *  - supported huge header and trailer.
 *
 * Jun 4, 2007, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *  - modified CCopyData constructer to accept a negative offset value.
 *  - debugged trailer handling.
 *
 * Jun 8, 2007, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *  - updated version number to "0.93".
 *  - debugged Als2Mp4() to open CFileWriter with proper use64bit flag.
 *  - debugged Als2Mp4() to extend pRauBuf automatically for RAflag==1.
 *
 * Jun 11, 2007, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *  - changed const MP4INFO& to MP4INFO& in AlsToMp4() and 
 *    ReadAlsHeaderFromStream().
 *  - added initialization of Mp4Info.m_FileType.
 *  - modified to set Mp4Info.m_FileType in ReadAlsHeaderFromStream().
 *
 * Jul 11, 2007, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *  - debugged Mp4ToAls(). Loop count of the MP4 frames had been wrong.
 *
 * Jul 15, 2007, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *  - updated version number to "0.94".
 *  - added -OAFI option to force to use meta box with oafi record.
 *  - supported ALSSpecificConfig with header and trailer embedded.
 *  - added CCopyData::CCopyData() with 1 parameter.
 *  - modified CCopyData::Read() to support fill-in mode.
 *
 * Jul 23, 2007, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *  - updated version number to "0.95".
 *  - updated AlsImf library.
 *
 * Aug 10, 2007, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *  - updated version number to "0.96".
 *  - debugged Mp4ToAls() to detect original file type when oafi box is 
 *    not present.
 *
 ************************************************************************/

#include "ImfFileStream.h"
#include "Mp4aFile.h"
#include "als2mp4.h"
#include "cmdline.h"
#include "wave.h"

#if defined( _WIN32 )
#include	<winsock2.h>
#include	<sys/timeb.h>
#endif

#define	ALS2MP4_VERSION		"0.96"		// Version number

#define	READ_UINT( x )		( ( (unsigned int)(x)[0] << 24 ) | ( (unsigned int)(x)[1] << 16 ) | ( (unsigned int)(x)[2] << 8 ) | ( (unsigned int)(x)[3] ) )
#define	READ_USHORT( x )	( ( (unsigned short)(x)[0] << 8 ) | ( (unsigned short)(x)[1] ) )
#define	PROCEED( x )		{ if ( (x) > DataSize ) throw A2MERR_INVALID_CONFIG; pData += (x); DataSize -= (x); }

using namespace NAlsImf;

// main function //////////////////////////////////////////////////////////////////////////////////
#if defined( ALS2MP4_MAIN )
int main(int argc, char **argv)
{
	if (argc < 3)
	{
		printf("ALS2MP4 version %s\n", ALS2MP4_VERSION);
		printf("Usage: als2mp4 [options] infile outfile\n");
		printf("Convert ALS file into MP4 file (default), and vice versa (-x)\n");
		printf("\n");
		printf("  -s    do not strip original random access information\n");
		printf("  -u#   RAU size location (-x only): 0 = frames (default), 1 = header, 2 = none\n");
		printf("  -x    convert MP4 into ALS file (options -u# only)\n");
		printf("  -OAFI force to create meta box with oafi record\n");
		printf("\n");
		printf("The ALS file to be converted must be encoded in random access mode. There\n");
		printf("are several options to deal with the random access information.\n");
		printf("In an MP4 file, random access information, i.e. the size of each random\n");
		printf("access unit (RAU), is stored in a unified way, and the original information\n");
		printf("is stripped from the encoded stream (use -s to keep it).\n");
		printf("In an ALS file to be generated from an MP4 file (-x), the RAU size location\n");
		printf("can be freely chosen (option -u#), except when the original information was\n");
		printf("kept (-s, see above).\n");
		exit(1);
	}

	// Parse options into MP4INFO structure.
	MP4INFO	Mp4Info;
	A2MERR	ErrCode;

	Mp4Info.m_pInFile = argv[argc-2];		// must exist
	Mp4Info.m_pOutFile = argv[argc-1];		// is created
	Mp4Info.m_pOriginalFile = NULL;

	Mp4Info.m_StripRaInfo = ( CheckOption( argc, argv, "-s" ) == 0 );	// Negated!
	Mp4Info.m_RaLocation = static_cast<short>( GetOptionValue( argc, argv, "-u" ) );
	Mp4Info.m_Samples = 0;
	Mp4Info.m_HeaderSize = Mp4Info.m_TrailerSize = Mp4Info.m_AuxDataSize = 0;
	Mp4Info.m_HeaderOffset = Mp4Info.m_TrailerOffset = Mp4Info.m_AuxDataOffset = 0;
	Mp4Info.m_FileType = 0xff;				// File type is determined by ALS header.
	Mp4Info.m_RMflag = false;
	Mp4Info.m_UseMeta = ( CheckOption( argc, argv, "-OAFI" ) != 0 );

	ErrCode = CheckOption( argc, argv, "-x" ) ? Mp4ToAls( Mp4Info ) : AlsToMp4( Mp4Info );
	if ( ErrCode != A2MERR_NONE ) {
		printf( "Failed. %s\n", ToErrorString( ErrCode ) );
	} else {
		printf( "Succeeded.\n" );
	}
	return static_cast<int>( ErrCode );
}
#endif	// ALS2MP4_MAIN

////////////////////////////////////////
//                                    //
//         Convert ALS to MP4         //
//                                    //
////////////////////////////////////////
// Mp4Info = MP4 information
// Return value = Error code
A2MERR	AlsToMp4( MP4INFO& Mp4Info )
{
	A2MERR		ErrCode = A2MERR_NONE;
	CFileReader	InFile;
	CFileWriter	OutFile;
	CMp4aWriter	Writer;
	ALS_HEADER	AlsHeader;
	IMF_UINT64	RauBufSize;
	IMF_UINT8*	pRauBuf = NULL;
	IMF_UINT32	RauSize;
	IMF_UINT64	SampleDuration, LastSampleDuration;
	IMF_UINT32	i;
	IMF_UINT32	CopySize;
	bool		Use64bit;

	// Initialize ALS_HEADER structure.
	memset( &AlsHeader, 0, sizeof(AlsHeader) );

	try {
		// open input (ALS) file
		if ( !InFile.Open( Mp4Info.m_pInFile ) ) throw A2MERR_OPEN_FILE;

		// Read ALS header information.
		ReadAlsHeaderFromStream( InFile, &AlsHeader, Mp4Info );

		// Create output (MP4) file ///////////////////////////////////////////////////////////////////

		// Create MP4 file.
		if ( !OutFile.Open( Mp4Info.m_pOutFile ) ) throw A2MERR_CREATE_FILE;

		// Open MP4 writer.
		Use64bit = ( Mp4Info.m_HeaderSize + Mp4Info.m_TrailerSize + AlsHeader.m_FileSize > 0xffffffff );
		if ( !Writer.Open( OutFile, AlsHeader.m_Freq, AlsHeader.m_Chan, AlsHeader.m_Res, Mp4Info.m_FileType, AlsHeader.m_pALSSpecificConfig, AlsHeader.m_ALSSpecificConfigSize, Use64bit, Mp4Info.m_UseMeta ) ) throw A2MERR_INIT_MP4WRITER;

		// Calculate sample duration and last sample duration.
		SampleDuration = AlsHeader.m_N * AlsHeader.m_RA;
		LastSampleDuration = AlsHeader.m_Samples - ( ( AlsHeader.m_RAUnits - 1 ) * SampleDuration );

		// Estimate maximum RAU size.
		if ( AlsHeader.m_RA == 0 ) {
			// The whole audio data is regarded as one RA unit.
			RauBufSize = AlsHeader.m_FileSize - InFile.Tell();
		} else if ( AlsHeader.m_RAflag == 1 ) {
			// Assumes uncompressed size.
			RauBufSize = AlsHeader.m_N * AlsHeader.m_RA * AlsHeader.m_Chan * ( AlsHeader.m_Res / 8 ) + sizeof(IMF_UINT32);
		} else if ( AlsHeader.m_RAflag == 2 ) {
			// Search the maximum size.
			RauBufSize = 0;
			for( i=0; i<AlsHeader.m_RAUnits; i++ ) if ( AlsHeader.m_RAUsize[i] > RauBufSize ) RauBufSize = AlsHeader.m_RAUsize[i];
		} else {
			// RA is enabled, but no RAU size information found.
			throw A2MERR_NO_RAU_SIZE;
		}
		if ( RauBufSize >> 32 ) throw A2MERR_RAU_TOO_BIG;

		// Allocate buffer for RAU.
		pRauBuf = new IMF_UINT8 [ static_cast<IMF_UINT32>( RauBufSize ) ];
		if ( pRauBuf == NULL ) throw A2MERR_NO_MEMORY;

		// Write header data.
		if ( Mp4Info.m_HeaderSize > 0 ) {
			CCopyData	CopyData( Mp4Info.m_pOriginalFile, 0, Mp4Info.m_HeaderSize );
			while( CopyData.Read( CopySize ) ) if ( !Writer.WriteHeader( CopyData, CopySize ) ) throw A2MERR_WRITE_HEADER;
		}

		// Copy all frames.
		if ( AlsHeader.m_RA == 0 ) {
			// Write all audio data as a single RAU.
			if ( AlsHeader.m_Samples >> 32 ) throw A2MERR_RAU_TOO_BIG;
			InFile.Read( pRauBuf, static_cast<IMF_UINT32>( RauBufSize ) );
			if ( !Writer.WriteFrame( pRauBuf, static_cast<IMF_UINT32>( RauBufSize ), static_cast<IMF_UINT32>( AlsHeader.m_Samples ), true ) ) throw A2MERR_WRITE_FRAME;

		} else {
			// RAU loop
			for( i=0; i<AlsHeader.m_RAUnits; i++ ) {

				// Get RAU size.
				if ( AlsHeader.m_RAflag == 1 ) {
					InFile.Read32( RauSize );
					if ( RauSize > RauBufSize ) {
						// Extend pRauBuf size.
						delete[] pRauBuf;
						RauBufSize = RauSize;
						pRauBuf = new IMF_UINT8 [ static_cast<IMF_UINT32>( RauBufSize ) ];
						if ( pRauBuf == NULL ) throw A2MERR_NO_MEMORY;
					}
					if ( !Mp4Info.m_StripRaInfo ) {
						// Regard RAU size as a part of a frame.
						RauSize += 4;
						InFile.Seek( -4, CBaseStream::S_CURRENT );
					}
				} else if ( AlsHeader.m_RAflag == 2 ) {
					RauSize = AlsHeader.m_RAUsize[i];
				}

				// Read RAU from ALS file.
				InFile.Read( pRauBuf, RauSize );

				// Adjust sample duration.
				if ( i == AlsHeader.m_RAUnits - 1 ) SampleDuration = LastSampleDuration;

				// Write RAU to MP4 file.
				if ( SampleDuration >> 32 ) throw A2MERR_RAU_TOO_BIG;
				if ( !Writer.WriteFrame( pRauBuf, RauSize, static_cast<IMF_UINT32>( SampleDuration ), true ) ) throw A2MERR_WRITE_FRAME;
			}
		}

		// Write trailer data.
		if ( Mp4Info.m_TrailerSize > 0 ) {
			CCopyData	CopyData( Mp4Info.m_pOriginalFile, -static_cast<IMF_INT64>( Mp4Info.m_TrailerSize ), Mp4Info.m_TrailerSize );
			while( CopyData.Read( CopySize ) ) if ( !Writer.WriteTrailer( CopyData, CopySize ) ) throw A2MERR_WRITE_TRAILER;
		}

		// Write auxiliary data.
		if ( AlsHeader.m_AuxSize > 0 ) {
			if ( !Writer.WriteAuxData( AlsHeader.m_pAuxData, AlsHeader.m_AuxSize ) ) throw A2MERR_WRITE_AUXDATA;
		}

		// Complete MP4 file.
		if ( !Writer.Close() ) throw A2MERR_WRITE_MOOV;
	}
	catch( A2MERR e ) {
		ErrCode = e;
	}

	// Clean up files and buffers.
	Writer.Close();
	OutFile.Close();
	InFile.Close();
	if ( pRauBuf ) delete[] pRauBuf;
	ClearAlsHeader( &AlsHeader );

	return ErrCode;
}

////////////////////////////////////////
//                                    //
//     Read ALS header from file      //
//                                    //
////////////////////////////////////////
// fp = Input file
// pHeader = ALS_HEADER structure to receive the result
// Mp4Info = Const reference to MP4INFO structure
// * This function may throw A2MERR.
void	ReadAlsHeaderFromStream( CBaseStream& Stream, ALS_HEADER* pHeader, MP4INFO& Mp4Info )
{
	IMF_UINT8	tmp8;
	IMF_UINT16	tmp16;
	IMF_UINT32	tmp32;
	IMF_UINT64	tmp64;
	IMF_UINT32	i;
	IMF_INT32	NeedPuchBit;
	IMF_INT32	size;
	ALS_HEADER*	p = pHeader;
	IMF_INT64	AudioTop;

	// Get file size.
	Stream.Seek( 0, CBaseStream::S_END );
	p->m_FileSize = Stream.Tell();
	Stream.Seek( 0, CBaseStream::S_BEGIN );

	// read ALS header information (section basically copied from decoder.cpp of RM17) ////////////
	Stream.Read32( p->m_als_id );					// ALS identifier: 'ALS' + 0x00 (= 0x414C5300)
	if ( p->m_als_id != 0x414C5300UL) throw A2MERR_INVALID_ALSID;

	Stream.Read32( p->m_Freq );						// sampling frequency
	Stream.Read32( tmp32 );							// samples
	if ( tmp32 == 0xffffffff ) {
		if ( !Mp4Info.m_RMflag ) throw A2MERR_INVALID_SAMPLES;
		p->m_Samples = Mp4Info.m_Samples;
	} else {
		p->m_Samples = tmp32;
	}
	Stream.Read16( tmp16 );
	p->m_Chan = tmp16 + 1;							// channels

	Stream.Read8( tmp8 );
	p->m_FileType = tmp8 >> 5;						// file type			(XXXx xxxx)
	p->m_Res =  8 * (((tmp8 >> 2) & 0x07) + 1);		// resolution			(xxxX XXxx)
	p->m_SampleType = (tmp8 >> 1) & 0x01;			// floating-point		(xxxx xxXx)
	p->m_MSBfirst = tmp8 & 0x01;					// MSB/LSB first		(xxxx xxxX)

	Stream.Read16( tmp16 );
	p->m_N = tmp16 + 1;								// frame length

	Stream.Read8( tmp8 );
	p->m_RA = tmp8;									// random access

	Stream.Read8( tmp8 );
	p->m_RAflag = (tmp8 >> 6) & 0x03;				// RA location			(XXxx xxxx)
	p->m_Adapt = (tmp8 >> 5) & 0x01;				// adaptive order		(xxXx xxxx)
	p->m_CoefTable = (tmp8 >> 3) & 0x03;			// entropy coding table	(xxxX Xxxx)
	p->m_PITCH = (tmp8 >> 2) & 0x01;				// pitch Coding	(LTP)	(xxxx xXxx)
	p->m_P = tmp8 & 0x03;							// pred. order (MSBs)	(xxxx xxXX)

	Stream.Read8( tmp8 );							// pred. order (LSBs)	(XXXX XXXX)
	p->m_P = (p->m_P << 8) | tmp8;

	Stream.Read8( tmp8 );
	p->m_Sub = (tmp8 >> 6) & 0x03;					// block switching		(XXxx xxxx)
	if (p->m_Sub) p->m_Sub += 2;
	p->m_BGMC = (tmp8 >> 5) & 0x01;					// entropy coding		(xxXx xxxx)
	p->m_SBpart = (tmp8 >> 4) & 0x01;				// subblock partition	(xxxX xxxx)
	p->m_Joint = (tmp8 >> 3) & 0x01;				// joint stereo			(xxxx Xxxx)
	p->m_MCC = (tmp8 >> 2) & 0x01;					// multi-channel coding	(xxxx xXxx)
	p->m_ChanConfig = (tmp8 >> 1) & 0x01;			// channel config.		(xxxx xxXx)
	p->m_ChanSort = tmp8 & 0x01;					// new channel sorting	(xxxx xxxX)

	Stream.Read8( tmp8 );
	p->m_CRCenabled = (tmp8 >> 7) & 0x01;			// CRC enabled			(Xxxx xxxx)
	p->m_RLSLMS = (tmp8 >> 6) & 0x01;				// RLSLMS mode			(xXxx xxxx)
	p->m_AUXenabled = tmp8 & 0x01;					// AUX data present		(xxxx xxxX)

	if ( p->m_ChanConfig ) Stream.Read16( tmp16 );	// channel configuration data (not used)

	if ( p->m_ChanSort ) {
		// Calculate bit width of a chan_pos field.
		i = ( p->m_Chan > 1 ) ? ( p->m_Chan - 1 ) : 1;
		NeedPuchBit = 0;
		while( i ) { i /= 2; NeedPuchBit++; }

		// Skip number of bytes for chan_pos[] (byte-aligned).
		size = ( NeedPuchBit * p->m_Chan ) / 8 + ( ( ( NeedPuchBit * p->m_Chan ) & 0x7 ) ? 1 : 0 );
		Stream.Seek( size, CBaseStream::S_CURRENT );
	}
	p->m_AlsHeaderSize = static_cast<IMF_UINT32>( Stream.Tell() );

	Stream.Read32( p->m_HeaderSize );				// header size
	Stream.Read32( p->m_TrailerSize );				// trailer size

	if ( p->m_HeaderSize == 0xffffffff ) {
		if ( !Mp4Info.m_RMflag ) throw A2MERR_INVALID_CONFIG;
	} else {
		if ( p->m_pHeaderData ) delete[] p->m_pHeaderData;
		p->m_pHeaderData = new IMF_UINT8 [ p->m_HeaderSize ];
		if ( p->m_pHeaderData == NULL ) throw A2MERR_NO_MEMORY;
		Stream.Read( p->m_pHeaderData, p->m_HeaderSize );
	}

	if ( p->m_TrailerSize == 0xffffffff ) {
		if ( !Mp4Info.m_RMflag ) throw A2MERR_INVALID_CONFIG;
	} else {
		if ( p->m_pTrailerData ) delete[] p->m_pTrailerData;
		p->m_pTrailerData = new IMF_UINT8 [ p->m_TrailerSize ];
		if ( p->m_pTrailerData == NULL ) throw A2MERR_NO_MEMORY;
		Stream.Read( p->m_pTrailerData, p->m_TrailerSize );
	}

	if ( p->m_CRCenabled ) Stream.Read32( p->m_CRCorg );

	// Calculate number of frames.
	tmp64 = ( p->m_Samples / p->m_N ) + ( ( p->m_Samples % p->m_N ) ? 1 : 0 );
	if ( tmp64 >> 32 ) throw A2MERR_INVALID_SAMPLES;
	p->m_frames = static_cast<IMF_UINT32>( tmp64 );

	if ( p->m_RA == 0 ) {
		p->m_RAUnits = 1;
	} else {
		// Calculate number of RAU size.
		p->m_RAUnits = ( p->m_frames / p->m_RA ) + ( ( p->m_frames % p->m_RA ) ? 1 : 0 );

		if ( p->m_RAflag == 2 ) {
			if ( p->m_RAUsize ) delete[] p->m_RAUsize;
			p->m_RAUsize = new IMF_UINT32 [ p->m_RAUnits ];
			if ( p->m_RAUsize == NULL ) throw A2MERR_NO_MEMORY;
			for( i=0; i<p->m_RAUnits; i++ ) Stream.Read32( p->m_RAUsize[i] );
		}
	}

	if ( p->m_AUXenabled ) {
		// aux data present
		Stream.Read32( p->m_AuxSize );
		if ( p->m_AuxSize == 0xFFFFFFFF ) throw A2MERR_INVALID_CONFIG;	// AuxData must be present in ALS header.
		if ( p->m_pAuxData ) delete[] p->m_pAuxData;
		p->m_pAuxData = new IMF_UINT8 [ p->m_AuxSize ];
		if ( p->m_pAuxData == NULL ) throw A2MERR_NO_MEMORY;
		Stream.Read( p->m_pAuxData, p->m_AuxSize );
	}

	// Save the top position of the audio data.
	AudioTop = Stream.Tell();

	// Create ALSSpecificConfig to be embedded in MP4 file.
	IMF_UINT8*	pDst;
	IMF_INT64	ConfigSize;

	// Allocate memory for ALS header with possible maximum size.
	ConfigSize = 6 + p->m_AlsHeaderSize + 12 + p->m_RAUnits * sizeof(IMF_UINT32) + 4;
	if ( p->m_HeaderSize != 0xffffffff ) ConfigSize += p->m_HeaderSize;
	if ( p->m_TrailerSize != 0xffffffff ) ConfigSize += p->m_TrailerSize;
	if ( p->m_AuxSize != 0xffffffff ) ConfigSize += p->m_AuxSize;
	if ( ConfigSize >> 32 ) throw A2MERR_NO_MEMORY;	// ConfigSize must be less than 4GB.
	p->m_ALSSpecificConfigSize = static_cast<IMF_UINT32>( ConfigSize );
	if ( p->m_pALSSpecificConfig ) delete[] p->m_pALSSpecificConfig;
	pDst = p->m_pALSSpecificConfig = new IMF_UINT8 [ p->m_ALSSpecificConfigSize ];
	if ( pDst == NULL ) throw A2MERR_NO_MEMORY;

	// Generate AudioSpecificConfig data in front of ALSSpecificConfig data.
	pDst[0] = static_cast<IMF_UINT8>( 0xF8 + ( ( ( 36 - 32 ) >> 3 ) & 0x07 ) );							// 1111 1xxx
	pDst[1] = static_cast<IMF_UINT8>( ( ( 36 - 32 ) << 5 ) | 0x1E | ( ( p->m_Freq >> 23 ) & 0x01 ) );	// xxx1 111x
	pDst[2] = static_cast<IMF_UINT8>( ( p->m_Freq >> 15 ) & 0xFF );										// xxxx xxxx
	pDst[3] = static_cast<IMF_UINT8>( ( p->m_Freq >> 7 ) & 0xFF );										// xxxx xxxx
	pDst[4] = static_cast<IMF_UINT8>( ( p->m_Freq << 1 ) & 0xFE );										// xxxx xxx0
	pDst[5] = 0;																						// 000- ---- <- 5 bits added in N8166
	pDst += 6;

	// Read from the top of ALS file.
	Stream.Seek( 0, CBaseStream::S_BEGIN );

	// Read ALS header through byte_align after chan_pos[].
	pDst += Stream.Read( pDst, p->m_AlsHeaderSize );

	// Set header size.
	Set32( pDst, p->m_HeaderSize );
	pDst += sizeof(IMF_UINT32);

	// Set trailer size.
	Set32( pDst, p->m_TrailerSize );
	pDst += sizeof(IMF_UINT32);

	// Set header data.
	if ( p->m_HeaderSize != 0xffffffff ) {
		memcpy( pDst, p->m_pHeaderData, p->m_HeaderSize );
		pDst += p->m_HeaderSize;
	}

	// Set trailer data.
	if ( p->m_TrailerSize != 0xffffffff ) {
		memcpy( pDst, p->m_pTrailerData, p->m_TrailerSize );
		pDst += p->m_TrailerSize;
	}

	// Set crc.
	if ( p->m_CRCenabled ) {
		Set32( pDst, p->m_CRCorg );
		pDst += sizeof(IMF_UINT32);
	}

	// Read RA_unit_size[], if needed.
	if ( ( p->m_RA != 0 ) && ( p->m_RAflag == 2 ) ) {
		// Skip RAU size.
		Stream.Seek( p->m_RAUnits * sizeof(IMF_UINT32), CBaseStream::S_CURRENT );
	}
	if ( p->m_RA != 0 ) {
		p->m_pALSSpecificConfig[18+6] &= 0x3F;	// 00XX XXXX: set RAflag to 00 in bitstream (RAUsize not stored)
	}

	// Set aux_size.
	if ( p->m_AUXenabled ) {
		Set32( pDst, p->m_AuxSize );
		pDst += sizeof(IMF_UINT32);
		if ( p->m_AUXenabled != 0xffffffff ) {
			memcpy( pDst, p->m_pAuxData, p->m_AuxSize );
			pDst += p->m_AuxSize;
		}
	}

	// Adjust ConfigSize.
	p->m_ALSSpecificConfigSize = static_cast<IMF_UINT32>( pDst - p->m_pALSSpecificConfig );

	// Seek to the top of the audio data.
	Stream.Seek( AudioTop, CBaseStream::S_BEGIN );

	// Save file type.
	if ( Mp4Info.m_FileType == 0xff ) {
		Mp4Info.m_FileType = p->m_FileType;
	}
}

////////////////////////////////////////
//                                    //
//         Convert MP4 to ALS         //
//                                    //
////////////////////////////////////////
// Mp4Info = Reference to MP4 information
// Return value = Error code
A2MERR	Mp4ToAls( MP4INFO& Mp4Info )
{
	A2MERR		ErrCode = A2MERR_NONE;
	CFileReader	InFile;
	CFileWriter	OutFile;
	CMp4aReader	Reader;
	ALS_HEADER	AlsHeader;
	IMF_UINT8*	pConfigData = NULL;
	IMF_UINT32	ConfigSize;
	IMF_UINT8*	pRauBuf = NULL;
	IMF_UINT32	MaxRauSize;
	IMF_UINT32	RauSize;
	IMF_UINT8	Aot;
	IMF_UINT32	i;
	IMF_UINT32	NumFrames;
	IMF_UINT64	TotalSamples;
	CMp4aReader::CFrameInfo	FrameInfo;
	bool		AddFrameInfo = false;
	IMF_UINT32	CopySize;

	// Initialize ALS_HEADER structure.
	memset( &AlsHeader, 0, sizeof(AlsHeader) );

	try {
		// Open MP4 file.
		if ( !InFile.Open( Mp4Info.m_pInFile ) ) throw A2MERR_OPEN_FILE;

		// Open MP4A reader.
		if ( !Reader.Open( InFile ) ) throw A2MERR_INIT_MP4READER;

		// Get decoder specific config.
		pConfigData = Reader.GetDecSpecInfo( ConfigSize );
		if ( ( pConfigData == NULL ) || ( ConfigSize < 6 ) ) throw A2MERR_INVALID_CONFIG;

		// Analyse AudioSpecificConfig information.
		Aot = pConfigData[0] >> 3;																	// XXXX Xxxx
		if ( Aot == 0x1F ) Aot = 32 + ( ( pConfigData[0] & 0x07 ) << 3 ) | ( pConfigData[1] >> 5 );	// xxx- ----
		if ( Aot != 36 ) throw A2MERR_NOT_ALS;

		// Calculate total number of samples.
		TotalSamples = 0;
		NumFrames = Reader.GetFrameCount();
		for( i=0; i<NumFrames; i++ ) {
			if ( Reader.GetFrameInfo( i, FrameInfo ) ) TotalSamples += FrameInfo.m_NumSamples;
		}

		// Fill in MP4INFO structure.
		Mp4Info.m_Samples = TotalSamples;
		Reader.GetHeader( Mp4Info.m_HeaderOffset, Mp4Info.m_HeaderSize );
		Reader.GetTrailer( Mp4Info.m_TrailerOffset, Mp4Info.m_TrailerSize );
		Reader.GetAuxData( Mp4Info.m_AuxDataOffset, Mp4Info.m_AuxDataSize );
		Mp4Info.m_pOriginalFile = Mp4Info.m_pInFile;
		Reader.GetFileType( Mp4Info.m_FileType, Mp4Info.m_FileTypeName );

		// Read ALS header information.
		ReadAlsHeaderFromMemory( pConfigData + 6, ConfigSize - 6, &AlsHeader, Mp4Info );

		// When CMp4aReader cannot detect the file type, take it from ALS header.
		if ( Mp4Info.m_FileType == 0xff ) {
			Mp4Info.m_FileType = AlsHeader.m_FileType;
			Mp4Info.m_FileTypeName.erase();
		}

		// open output (ALS) file /////////////////////////////////////////////////////////////////

		// Create ALS file.
		if ( !OutFile.Open( Mp4Info.m_pOutFile ) ) throw A2MERR_CREATE_FILE;

		// Set samples.
		if ( AlsHeader.m_Samples == 0xffffffff ) {
			if ( !Mp4Info.m_RMflag ) {
				if ( Mp4Info.m_Samples >> 32 ) throw A2MERR_INVALID_SAMPLES;
				Set32( pConfigData + 6 + 8, static_cast<IMF_UINT32>( Mp4Info.m_Samples ) );
			}
		}

		// Write ALS header.
		if ( ( AlsHeader.m_RA != 0 ) && ( AlsHeader.m_RAflag == 0 ) ) {
			// RAU size has been stripped in encoding.
			if ( Mp4Info.m_RaLocation == 0 ) {
				pConfigData[18+6] |= 0x40;	// 01xx xxxx: set RAflag to 01 in bitstream (RAU size in frame)
				if ( OutFile.Write( pConfigData + 6, AlsHeader.m_AlsHeaderSize ) != AlsHeader.m_AlsHeaderSize ) throw A2MERR_WRITE_ALSHEADER;

			} else if ( Mp4Info.m_RaLocation == 1 ) {
				pConfigData[18+6] |= 0x80;	// 10xx xxxx: set RAflag to 10 in bitstream (RAU size in header)
				// Write ALS header, inserting RAU size information.
				if ( OutFile.Write( pConfigData + 6, AlsHeader.m_AlsHeaderSize ) != AlsHeader.m_AlsHeaderSize ) throw A2MERR_WRITE_ALSHEADER;
				AddFrameInfo = true;

			} else if ( Mp4Info.m_RaLocation == 2 ) {
				// RA is enabled, but RAU size location has not been specified.
				printf( "***** WARNING: Random access is enabled, but RAU size information has been stripped. *****\n" );
				if ( OutFile.Write( pConfigData + 6, AlsHeader.m_AlsHeaderSize ) != AlsHeader.m_AlsHeaderSize ) throw A2MERR_WRITE_ALSHEADER;

			} else {
				// Invalid Mp4Info.m_RaLocation value.
				throw A2MERR_INVALID_OPTION;
			}
		} else {	// ( RA == 0 ) || ( AlsHeader.m_RAflag != 0 )
			// Just copy ALS header as is.
			if ( OutFile.Write( pConfigData + 6, AlsHeader.m_AlsHeaderSize ) != AlsHeader.m_AlsHeaderSize ) throw A2MERR_WRITE_ALSHEADER;
		}

		// Write header size.
		if ( AlsHeader.m_HeaderSize == 0xffffffff ) {
			if ( Mp4Info.m_RMflag ) {
				// Keep 0xffffffff for header size.
				if ( !OutFile.Write32( AlsHeader.m_HeaderSize ) ) throw A2MERR_WRITE_HEADER;
			} else {
				// Embed header size.
				if ( Mp4Info.m_HeaderSize >> 32 ) throw A2MERR_HEADER_TOO_BIG;
				if ( !OutFile.Write32( static_cast<IMF_UINT32>( Mp4Info.m_HeaderSize ) ) ) throw A2MERR_WRITE_HEADER;
			}
		} else {
			if ( !OutFile.Write32( AlsHeader.m_HeaderSize ) ) throw A2MERR_WRITE_HEADER;
		}

		// Write trailer size.
		if ( AlsHeader.m_TrailerSize == 0xffffffff ) {
			if ( Mp4Info.m_RMflag ) {
				// Keep 0xffffffff for trailer size.
				if ( !OutFile.Write32( AlsHeader.m_TrailerSize ) ) throw A2MERR_WRITE_TRAILER;
			} else {
				// Embed trailer size.
				if ( Mp4Info.m_TrailerSize >> 32 ) throw A2MERR_TRAILER_TOO_BIG;
				if ( !OutFile.Write32( static_cast<IMF_UINT32>( Mp4Info.m_TrailerSize ) ) ) throw A2MERR_WRITE_TRAILER;
			}
		} else {
			if ( !OutFile.Write32( AlsHeader.m_TrailerSize ) ) throw A2MERR_WRITE_TRAILER;
		}

		// Write header data.
		if ( AlsHeader.m_HeaderSize == 0xffffffff ) {
			if ( !Mp4Info.m_RMflag ) {
				// Embed header data.
				CCopyData	CopyData( Mp4Info.m_pOriginalFile, Mp4Info.m_HeaderOffset, Mp4Info.m_HeaderSize );
				while( CopyData.Read( CopySize ) ) if ( OutFile.Write( CopyData, CopySize ) != CopySize ) throw A2MERR_WRITE_HEADER;
			}
		} else {
			if ( OutFile.Write( AlsHeader.m_pHeaderData, AlsHeader.m_HeaderSize ) != AlsHeader.m_HeaderSize ) throw A2MERR_WRITE_HEADER;
		}

		// Write trailer data.
		if ( AlsHeader.m_TrailerSize == 0xffffffff ) {
			if ( !Mp4Info.m_RMflag ) {
				// Embed trailer data.
				CCopyData	CopyData( Mp4Info.m_pOriginalFile, Mp4Info.m_TrailerOffset, Mp4Info.m_TrailerSize );
				while( CopyData.Read( CopySize ) ) if ( OutFile.Write( CopyData, CopySize ) != CopySize ) throw A2MERR_WRITE_TRAILER;
			}
		} else {
			if ( OutFile.Write( AlsHeader.m_pTrailerData, AlsHeader.m_TrailerSize ) != AlsHeader.m_TrailerSize ) throw A2MERR_WRITE_TRAILER;
		}

		// CRC
		if ( AlsHeader.m_CRCenabled ) {
			if ( !OutFile.Write32( AlsHeader.m_CRCorg ) ) throw A2MERR_WRITE_ALSHEADER;
		}

		// ra_unit_size
		if ( ( AlsHeader.m_RA != 0 ) && ( AlsHeader.m_RAflag == 2 ) ) {
			if ( OutFile.Write( AlsHeader.m_RAUsize, AlsHeader.m_RAUnits * sizeof(IMF_UINT32) ) != AlsHeader.m_RAUnits * sizeof(IMF_UINT32) ) throw A2MERR_WRITE_ALSHEADER;
		} else if ( AddFrameInfo ) {
			for( i=0; i<AlsHeader.m_RAUnits; i++ ) {
				if ( !Reader.GetFrameInfo( i, FrameInfo ) ) throw A2MERR_NO_FRAMEINFO;
				if ( !OutFile.Write32( FrameInfo.m_EncSize ) ) throw A2MERR_WRITE_ALSHEADER;
			}
		}

		// Write auxiliary data.
		if ( AlsHeader.m_AUXenabled ) {
			if ( AlsHeader.m_AuxSize == 0xffffffff ) {
				if ( Mp4Info.m_AuxDataSize >> 32 ) throw A2MERR_AUXDATA_TOO_BIG;
				if ( !OutFile.Write32( static_cast<IMF_UINT32>( Mp4Info.m_AuxDataSize ) ) ) throw A2MERR_WRITE_AUXDATA;
				CCopyData	CopyData( Mp4Info.m_pOriginalFile, Mp4Info.m_AuxDataOffset, Mp4Info.m_AuxDataSize );
				while( CopyData.Read( CopySize ) ) if ( OutFile.Write( CopyData, CopySize ) != CopySize ) throw A2MERR_WRITE_AUXDATA;
			} else {
				if ( !OutFile.Write32( AlsHeader.m_AuxSize ) ) throw A2MERR_WRITE_AUXDATA;
				if ( OutFile.Write( AlsHeader.m_pAuxData, AlsHeader.m_AuxSize ) != AlsHeader.m_AuxSize ) throw A2MERR_WRITE_AUXDATA;
			}
		}

		// Allocate buffer for RAU.
		MaxRauSize = Reader.GetMaxFrameSize();
		if ( MaxRauSize == 0 ) throw A2MERR_MAX_SIZE;
		pRauBuf = new IMF_UINT8 [ MaxRauSize ];
		if ( pRauBuf == NULL ) throw A2MERR_NO_MEMORY;

		// RAU loop
		for( i=0; i<Reader.GetFrameCount(); i++ ) {
			RauSize = Reader.ReadFrame( i, pRauBuf, MaxRauSize );
			if ( RauSize == 0 ) throw A2MERR_READ_FRAME;

			if ( ( AlsHeader.m_RA != 0 ) && ( AlsHeader.m_RAflag == 0 ) && ( Mp4Info.m_RaLocation == 0 ) ) {
				// RAU size should be inserted here.
				OutFile.Write32( RauSize );
			}
			OutFile.Write( pRauBuf, RauSize );
		}
	}
	catch( A2MERR e ) {
		ErrCode = e;
	}

	// Clean up files and buffers.
	Reader.Close();
	OutFile.Close();
	InFile.Close();
	if ( pRauBuf ) delete[] pRauBuf;
	ClearAlsHeader( &AlsHeader );

	return ErrCode;
}

////////////////////////////////////////
//                                    //
//    Read ALS header from memory     //
//                                    //
////////////////////////////////////////
// pData = Pointer to ALS header image
// DataSize = Number of bytes in pData
// pHeader = ALS_HEADER structure to receive the result
// Mp4Info = Const reference to MP4 information structure
// * This function may throw A2MERR.
void	ReadAlsHeaderFromMemory( const IMF_UINT8* pData, IMF_UINT32 DataSize, ALS_HEADER* pHeader, const MP4INFO& Mp4Info )
{
	IMF_UINT64			tmp64;
	IMF_UINT32			i;
	long				NeedPuchBit;
	IMF_UINT32			size;
	ALS_HEADER*			p = pHeader;
	const IMF_UINT8*	pTop = pData;
	IMF_UINT64			Samples;

	p->m_FileSize = 0;

	// read ALS header information (section basically copied from decoder.cpp of RM17) ////////////
	p->m_als_id = READ_UINT( pData );
	if ( p->m_als_id != 0x414C5300UL) throw A2MERR_INVALID_ALSID;
	PROCEED( 4 );

	p->m_Freq = READ_UINT( pData );
	PROCEED( 4 );

	p->m_Samples = READ_UINT( pData );
	if ( p->m_Samples == 0xFFFFFFFF ) {
		Samples = Mp4Info.m_Samples;
	} else {
		Samples = p->m_Samples;
	}
	PROCEED( 4 );

	p->m_Chan = READ_USHORT( pData ) + 1;
	PROCEED( 2 );

	p->m_FileType = pData[0] >> 5;						// file type			(XXXx xxxx)
	p->m_Res = 8 * (((pData[0] >> 2) & 0x07) + 1);		// resolution			(xxxX XXxx)
	p->m_SampleType = (pData[0] >> 1) & 0x01;			// floating-point		(xxxx xxXx)
	p->m_MSBfirst = pData[0] & 0x01;					// MSB/LSB first		(xxxx xxxX)
	PROCEED( 1 );

	p->m_N = READ_USHORT( pData ) + 1;					// frame length
	PROCEED( 2 );

	p->m_RA = pData[0];									// random access
	PROCEED( 1 );

	p->m_RAflag = (pData[0] >> 6) & 0x03;				// RA location			(XXxx xxxx)
	p->m_Adapt = (pData[0] >> 5) & 0x01;				// adaptive order		(xxXx xxxx)
	p->m_CoefTable = (pData[0] >> 3) & 0x03;			// entropy coding table	(xxxX Xxxx)
	p->m_PITCH = (pData[0] >> 2) & 0x01;				// pitch Coding	(LTP)	(xxxx xXxx)
	p->m_P = pData[0] & 0x03;							// pred. order (MSBs)	(xxxx xxXX)
	PROCEED( 1 );

	p->m_P = (p->m_P << 8) | pData[0];					// pred. order (LSBs)	(XXXX XXXX)
	PROCEED( 1 );

	p->m_Sub = (pData[0] >> 6) & 0x03;					// block switching		(XXxx xxxx)
	if (p->m_Sub) p->m_Sub += 2;
	p->m_BGMC = (pData[0] >> 5) & 0x01;					// entropy coding		(xxXx xxxx)
	p->m_SBpart = (pData[0] >> 4) & 0x01;				// subblock partition	(xxxX xxxx)
	p->m_Joint = (pData[0] >> 3) & 0x01;				// joint stereo			(xxxx Xxxx)
	p->m_MCC = (pData[0] >> 2) & 0x01;					// multi-channel coding	(xxxx xXxx)
	p->m_ChanConfig = (pData[0] >> 1) & 0x01;			// channel config.		(xxxx xxXx)
	p->m_ChanSort = pData[0] & 0x01;					// new channel sorting	(xxxx xxxX)
	PROCEED( 1 );

	p->m_CRCenabled = (pData[0] >> 7) & 0x01;			// CRC enabled			(Xxxx xxxx)
	p->m_RLSLMS = (pData[0] >> 6) & 0x01;				// RLSLMS mode			(xXxx xxxx)
	p->m_AUXenabled = pData[0] & 0x01;					// AUX data present		(xxxx xxxX)
	PROCEED( 1 );

	if ( p->m_ChanConfig ) PROCEED( 2 );				// channel configuration data (not used)

	if ( p->m_ChanSort ) {
		// Calculate bit width of a chan_pos field.
		i = ( p->m_Chan > 1 ) ? ( p->m_Chan - 1 ) : 1;
		NeedPuchBit = 0;
		while( i ) { i /= 2; NeedPuchBit++; }

		// Skip number of bytes for chan_pos[] (byte-aligned).
		size = ( NeedPuchBit * p->m_Chan ) / 8 + ( ( ( NeedPuchBit * p->m_Chan ) & 0x7 ) ? 1 : 0 );
		PROCEED( size );
	}
	p->m_AlsHeaderSize = static_cast<IMF_UINT32>( pData - pTop );

	p->m_HeaderSize = READ_UINT( pData );		// header size
	PROCEED( 4 );

	p->m_TrailerSize = READ_UINT( pData );		// trailer size
	PROCEED( 4 );

	if ( p->m_pHeaderData ) { delete[] p->m_pHeaderData; p->m_pHeaderData = NULL; }
	if ( ( p->m_HeaderSize != 0xffffffff ) && ( p->m_HeaderSize != 0 ) ) {
		p->m_pHeaderData = new IMF_UINT8 [ p->m_HeaderSize ];
		if ( p->m_pHeaderData == NULL ) throw A2MERR_NO_MEMORY;
		memcpy( p->m_pHeaderData, pData, p->m_HeaderSize );
		PROCEED( p->m_HeaderSize );
	}

	if ( p->m_pTrailerData ) { delete[] p->m_pTrailerData; p->m_pTrailerData = NULL; }
	if ( ( p->m_TrailerSize != 0xffffffff ) && ( p->m_TrailerSize != 0 ) ) {
		p->m_pTrailerData = new IMF_UINT8 [ p->m_TrailerSize ];
		if ( p->m_pTrailerData == NULL ) throw A2MERR_NO_MEMORY;
		memcpy( p->m_pTrailerData, pData, p->m_TrailerSize );
		PROCEED( p->m_TrailerSize );
	}

	if ( p->m_CRCenabled ) {
		p->m_CRCorg = READ_UINT( pData );
		PROCEED( 4 );
	}

	// Calculate number of frames.
	tmp64 = ( Samples / p->m_N ) + ( ( Samples % p->m_N ) ? 1 : 0 );
	if ( tmp64 >> 32 ) throw A2MERR_INVALID_SAMPLES;
	p->m_frames = static_cast<IMF_UINT32>( tmp64 );

	if ( p->m_RA == 0 ) {
		p->m_RAUnits = 1;
	} else {
		// Calculate number of RAU size.
		p->m_RAUnits = ( p->m_frames / p->m_RA ) + ( ( p->m_frames % p->m_RA ) ? 1 : 0 );

		if ( p->m_RAflag == 2 ) {
			if ( p->m_RAUsize ) delete[] p->m_RAUsize;
			p->m_RAUsize = new IMF_UINT32 [ p->m_RAUnits ];
			if ( p->m_RAUsize == NULL ) throw A2MERR_NO_MEMORY;
			for( i=0; i<p->m_RAUnits; i++ ) {
				p->m_RAUsize[i] = READ_UINT( pData );
				PROCEED( 4 );
			}
		}
	}

	if ( p->m_AUXenabled ) {
		// aux data present
		p->m_AuxSize = READ_UINT( pData );
		PROCEED( 4 );
		if ( p->m_pAuxData ) { delete[] p->m_pAuxData; p->m_pAuxData = NULL; }
		if ( ( p->m_AuxSize != 0xffffffff ) && ( p->m_AuxSize != 0 ) ) {
			p->m_pAuxData = new IMF_UINT8 [ p->m_AuxSize ];
			if ( p->m_pAuxData == NULL ) throw A2MERR_NO_MEMORY;
			memcpy( p->m_pAuxData, pData, p->m_AuxSize );
			PROCEED( p->m_AuxSize );
		}
	}
}

////////////////////////////////////////
//                                    //
//          Clear ALS header          //
//                                    //
////////////////////////////////////////
// pHeader = Pointer to ALS_HEADER structure to be cleared
void	ClearAlsHeader( ALS_HEADER* pHeader )
{
	if ( pHeader->m_pHeaderData ) delete[] pHeader->m_pHeaderData;
	if ( pHeader->m_pTrailerData ) delete[] pHeader->m_pTrailerData;
	if ( pHeader->m_RAUsize ) delete[] pHeader->m_RAUsize;
	if ( pHeader->m_pAuxData ) delete[] pHeader->m_pAuxData;
	if ( pHeader->m_pALSSpecificConfig ) delete[] pHeader->m_pALSSpecificConfig;
	memset( pHeader, 0, sizeof(ALS_HEADER) );
}

////////////////////////////////////////
//                                    //
//     Set ULONG value to memory      //
//                                    //
////////////////////////////////////////
// pDst = Pointer to destination buffer
// Value = ULONG value
void	Set32( void* pDst, IMF_UINT32 Value )
{
	IMF_UINT8*	p = static_cast<IMF_UINT8*>( pDst );
	p[0] = static_cast<IMF_UINT8>( Value >> 24 );
	p[1] = static_cast<IMF_UINT8>( Value >> 16 );
	p[2] = static_cast<IMF_UINT8>( Value >>  8 );
	p[3] = static_cast<IMF_UINT8>( Value       );
}

////////////////////////////////////////
//                                    //
//    Convert error code to string    //
//                                    //
////////////////////////////////////////
// ErrCode = Error code
// Return value = Pointer to error code string
#define	RETURN_ERRSTR( x )	if ( ErrCode == x ) return #x
const char*	ToErrorString( A2MERR ErrCode )
{
	RETURN_ERRSTR( A2MERR_NONE );
	RETURN_ERRSTR( A2MERR_NO_MEMORY );
	RETURN_ERRSTR( A2MERR_OPEN_FILE );
	RETURN_ERRSTR( A2MERR_CREATE_FILE );
	RETURN_ERRSTR( A2MERR_INIT_MP4READER );
	RETURN_ERRSTR( A2MERR_READ_FRAME );
	RETURN_ERRSTR( A2MERR_INIT_MP4WRITER );
	RETURN_ERRSTR( A2MERR_WRITE_MOOV );
	RETURN_ERRSTR( A2MERR_WRITE_FRAME );
	RETURN_ERRSTR( A2MERR_NOT_ALS );
	RETURN_ERRSTR( A2MERR_INVALID_ALSID );
	RETURN_ERRSTR( A2MERR_INVALID_CONFIG );
	RETURN_ERRSTR( A2MERR_INVALID_SAMPLES );
	RETURN_ERRSTR( A2MERR_INVALID_OPTION );
	RETURN_ERRSTR( A2MERR_MAX_SIZE );
	RETURN_ERRSTR( A2MERR_NO_FRAMEINFO );
	RETURN_ERRSTR( A2MERR_NO_RAU_SIZE );
	RETURN_ERRSTR( A2MERR_WRITE_ALSHEADER );
	RETURN_ERRSTR( A2MERR_WRITE_HEADER );
	RETURN_ERRSTR( A2MERR_WRITE_TRAILER );
	RETURN_ERRSTR( A2MERR_WRITE_AUXDATA );
	RETURN_ERRSTR( A2MERR_HEADER_TOO_BIG );
	RETURN_ERRSTR( A2MERR_TRAILER_TOO_BIG );
	RETURN_ERRSTR( A2MERR_AUXDATA_TOO_BIG );
	RETURN_ERRSTR( A2MERR_RAU_TOO_BIG );
	return "(unknown error)";
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                         CCopyData class                          //
//                                                                  //
//////////////////////////////////////////////////////////////////////

const IMF_UINT32	CCopyData::FILECOPY_BUFFER_SIZE = 1048576;	// Buffer size

////////////////////////////////////////
//                                    //
//            Constructor             //
//                                    //
////////////////////////////////////////
// InFile = Input stream
// Offset = File offset in InFile (positive: from the start / negative: from the bottom)
// Size = Total size to be copied
CCopyData::CCopyData( CBaseStream& InFile, IMF_INT64 Offset, IMF_UINT64 Size )
{
	m_pInFile = &InFile;
	m_pInFile->Seek( Offset, ( Offset >= 0 ) ? CBaseStream::S_BEGIN : CBaseStream::S_END );
	m_TotalSize = Size;
	m_pBuffer = new IMF_UINT8 [ FILECOPY_BUFFER_SIZE ];
}

////////////////////////////////////////
//                                    //
//            Constructor             //
//                                    //
////////////////////////////////////////
// pFilename = Filename to open
// Offset = File offset in InFile (positive: from the start / negative: from the bottom)
// Size = Total size to be copied
CCopyData::CCopyData( const char* pFilename, IMF_INT64 Offset, IMF_UINT64 Size )
{
	m_pInFile = &m_Reader;
	m_TotalSize = Size;
	m_pBuffer = new IMF_UINT8 [ FILECOPY_BUFFER_SIZE ];
	if ( !m_Reader.Open( pFilename ) ) throw A2MERR_OPEN_FILE;
	m_pInFile->Seek( Offset, ( Offset >= 0 ) ? CBaseStream::S_BEGIN : CBaseStream::S_END );
}

////////////////////////////////////////
//                                    //
//            Constructor             //
//                                    //
////////////////////////////////////////
// Size = Total size to be written
CCopyData::CCopyData( IMF_UINT64 Size )
{
	m_pInFile = NULL;
	m_TotalSize = Size;
	m_pBuffer = new IMF_UINT8 [ FILECOPY_BUFFER_SIZE ];
	memset( m_pBuffer, 0, FILECOPY_BUFFER_SIZE );
}

////////////////////////////////////////
//                                    //
//             Read data              //
//                                    //
////////////////////////////////////////
// Size = Reference to variable which received data size
// Return value = true:Success / false:Error or end of data
bool	CCopyData::Read( IMF_UINT32& Size )
{
	IMF_UINT32	ReadSize;
	ReadSize = ( m_TotalSize < FILECOPY_BUFFER_SIZE ) ? static_cast<IMF_UINT32>( m_TotalSize ) : FILECOPY_BUFFER_SIZE;
	if ( m_pInFile ) Size = m_pInFile->Read( m_pBuffer, ReadSize );
	else Size = ReadSize;
	m_TotalSize -= Size;
	return ( Size > 0 );
}

// End of als2mp4.cpp
