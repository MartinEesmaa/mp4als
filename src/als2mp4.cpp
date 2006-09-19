// als2mp4.cpp - conversion of plain ALS files to MP4 files
// (c) Tilman Liebchen, Technical University of Berlin
// 19 May 2006

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

#define	ALS2MP4_VERSION		"0.9"		// Version number

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
		printf("  -s  do not strip original random access information\n");
		printf("  -u# RAU size location (-x only): 0 = frames (default), 1 = header, 2 = none\n");
		printf("  -x  convert MP4 into ALS file (options -u# only)\n");
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

	// Parse options into MP4OPT structure.
	MP4OPT	Opt;
	A2MERR	ErrCode;

	Opt.m_pInFile = argv[argc-2];		// must exist
	Opt.m_pOutFile = argv[argc-1];		// is created

	Opt.m_StripRaInfo = ( CheckOption( argc, argv, "-s" ) == 0 );	// Negated!
	Opt.m_RaLocation = static_cast<short>( GetOptionValue( argc, argv, "-u" ) );

	ErrCode = CheckOption( argc, argv, "-x" ) ? Mp4ToAls( Opt ) : AlsToMp4( Opt );
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
	return "(unknown error)";
}

////////////////////////////////////////
//                                    //
//     Read ALS header from file      //
//                                    //
////////////////////////////////////////
// fp = Input file
// pHeader = ALS_HEADER structure to receive the result
// * This function may throw A2MERR.
void	ReadAlsHeaderFromStream( CBaseStream& Stream, ALS_HEADER* pHeader )
{
	IMF_UINT8	tmp8;
	IMF_UINT16	tmp16;
	IMF_UINT32	i;
	IMF_INT32	NeedPuchBit;
	IMF_INT32	size;
	ALS_HEADER*	p = pHeader;

	// Get file size.
	Stream.Seek( 0, CBaseStream::S_END );
	p->m_FileSize = static_cast<IMF_UINT32>( Stream.Tell() );
	Stream.Seek( 0, CBaseStream::S_BEGIN );

	// read ALS header information (section basically copied from decoder.cpp of RM17) ////////////
	Stream.Read32( p->m_als_id );					// ALS identifier: 'ALS' + 0x00 (= 0x414C5300)
	if ( p->m_als_id != 0x414C5300UL) throw A2MERR_INVALID_ALSID;

	Stream.Read32( p->m_Freq );						// sampling frequency
	Stream.Read32( p->m_Samples );					// samples
	if ( p->m_Samples == 0xFFFFFFFF ) throw A2MERR_INVALID_SAMPLES;
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

	Stream.Read32( p->m_HeaderSize );				// header size
	if ( p->m_HeaderSize == 0xFFFFFFFF ) p->m_HeaderSize = 0;
	Stream.Read32( p->m_TrailerSize );				// trailer size
	if ( p->m_TrailerSize == 0xFFFFFFFF ) p->m_TrailerSize = 0;

	if ( p->m_buff ) delete[] p->m_buff;
	p->m_buff = new IMF_UINT8 [ p->m_HeaderSize + p->m_TrailerSize ];
	if ( p->m_buff == NULL ) throw A2MERR_NO_MEMORY;
	Stream.Read( p->m_buff, p->m_HeaderSize + p->m_TrailerSize );

	if ( p->m_CRCenabled ) Stream.Read32( p->m_CRCorg );
	p->m_AlsHeaderSize = static_cast<IMF_UINT32>( Stream.Tell() );

	// Calculate number of frames.
	p->m_frames = ( p->m_Samples / p->m_N ) + ( ( p->m_Samples % p->m_N ) ? 1 : 0 );

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
		if ( p->m_AuxSize == 0xFFFFFFFF ) p->m_AuxSize = 0;
		if ( p->m_AuxData ) delete[] p->m_AuxData;
		p->m_AuxData = new IMF_UINT8 [ p->m_AuxSize ];
		if ( p->m_AuxData == NULL ) throw A2MERR_NO_MEMORY;
		Stream.Read( p->m_AuxData, p->m_AuxSize );
	}
}

////////////////////////////////////////
//                                    //
//    Read ALS header from memory     //
//                                    //
////////////////////////////////////////
// pData = Pointer to ALS header image
// DataSize = Number of bytes in pData
// pHeader = ALS_HEADER structure to receive the result
// * This function may throw A2MERR.
#define	READ_ULONG( x )		( ( (unsigned long)x[0] << 24 ) | ( (unsigned long)x[1] << 16 ) | ( (unsigned long)x[2] << 8 ) | ( (unsigned long)x[3] ) )
#define	READ_USHORT( x )	( ( (unsigned short)x[0] << 8 ) | ( (unsigned short)x[1] ) )
#define	PROCEED( x )		{ if ( (x) > DataSize ) throw A2MERR_INVALID_CONFIG; pData += (x); DataSize -= (x); }
void	ReadAlsHeaderFromMemory( const IMF_UINT8* pData, IMF_UINT32 DataSize, ALS_HEADER* pHeader )
{
	IMF_UINT32			i;
	long				NeedPuchBit;
	IMF_UINT32			size;
	ALS_HEADER*			p = pHeader;
	const IMF_UINT8*	pTop = pData;

	p->m_FileSize = 0;

	// read ALS header information (section basically copied from decoder.cpp of RM17) ////////////
	p->m_als_id = READ_ULONG( pData );
	if ( p->m_als_id != 0x414C5300UL) throw A2MERR_INVALID_ALSID;
	PROCEED( 4 );

	p->m_Freq = READ_ULONG( pData );
	PROCEED( 4 );

	p->m_Samples = READ_ULONG( pData );
	if ( p->m_Samples == 0xFFFFFFFF ) throw A2MERR_INVALID_SAMPLES;
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

	p->m_HeaderSize = READ_ULONG( pData );		// header size
	if ( p->m_HeaderSize == 0xFFFFFFFF ) p->m_HeaderSize = 0;
	PROCEED( 4 );

	p->m_TrailerSize = READ_ULONG( pData );	// trailer size
	if ( p->m_TrailerSize == 0xFFFFFFFF ) p->m_TrailerSize = 0;
	PROCEED( 4 );

	if ( p->m_buff ) delete[] p->m_buff;
	p->m_buff = new IMF_UINT8 [ p->m_HeaderSize + p->m_TrailerSize ];
	if ( p->m_buff == NULL ) throw A2MERR_NO_MEMORY;
	memcpy( p->m_buff, pData, p->m_HeaderSize + p->m_TrailerSize );
	PROCEED( p->m_HeaderSize + p->m_TrailerSize );

	if ( p->m_CRCenabled ) {
		p->m_CRCorg = READ_ULONG( pData );
		PROCEED( 4 );
	}
	p->m_AlsHeaderSize = static_cast<IMF_UINT32>( pData - pTop );

	// Calculate number of frames.
	p->m_frames = ( p->m_Samples / p->m_N ) + ( ( p->m_Samples % p->m_N ) ? 1 : 0 );

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
				p->m_RAUsize[i] = READ_ULONG( pData );
				PROCEED( 4 );
			}
		}
	}

	if ( p->m_AUXenabled ) {
		// aux data present
		p->m_AuxSize = READ_ULONG( pData );
		PROCEED( 4 );
		if ( p->m_AuxSize == 0xFFFFFFFF ) p->m_AuxSize = 0;
		if ( p->m_AuxData ) delete[] p->m_AuxData;
		p->m_AuxData = new IMF_UINT8 [ p->m_AuxSize ];
		if ( p->m_AuxData == NULL ) throw A2MERR_NO_MEMORY;
		memcpy( p->m_AuxData, pData, p->m_AuxSize );
		PROCEED( p->m_AuxSize );
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
	if ( pHeader->m_buff ) delete[] pHeader->m_buff;
	if ( pHeader->m_RAUsize ) delete[] pHeader->m_RAUsize;
	if ( pHeader->m_AuxData ) delete[] pHeader->m_AuxData;
	memset( pHeader, 0, sizeof(ALS_HEADER) );
}

////////////////////////////////////////
//                                    //
//         Convert ALS to MP4         //
//                                    //
////////////////////////////////////////
// Opt = Options
// Return value = Error code
A2MERR	AlsToMp4( const MP4OPT& Opt )
{
	A2MERR		ErrCode = A2MERR_NONE;
	CFileReader	InFile;
	CFileWriter	OutFile;
	CMp4aWriter	Writer;
	ALS_HEADER	AlsHeader;
	IMF_UINT32	ConfigSize;
	IMF_UINT8*	pConfigData = NULL;
	IMF_UINT8*	pDst;
	IMF_UINT32	RauBufSize;
	IMF_UINT8*	pRauBuf = NULL;
	IMF_UINT32	RauSize;
	IMF_UINT32	SampleDuration, LastSampleDuration;
	IMF_UINT32	i;

	// Initialize ALS_HEADER structure.
	memset( &AlsHeader, 0, sizeof(AlsHeader) );

	try {
		// open input (ALS) file
		if ( !InFile.Open( Opt.m_pInFile ) ) throw A2MERR_OPEN_FILE;

		// Read ALS header information.
		ReadAlsHeaderFromStream( InFile, &AlsHeader );

		// Allocate memory for ALS header with possible maximum size.
		ConfigSize = 6 + AlsHeader.m_AlsHeaderSize + AlsHeader.m_RAUnits * sizeof(unsigned long) + AlsHeader.m_AuxSize;
		pDst = pConfigData = new IMF_UINT8 [ ConfigSize ];
		if ( pConfigData == NULL ) throw A2MERR_NO_MEMORY;

		// Generate AudioSpecificConfig data in front of ALSSpecificConfig data.
		pDst[0] = static_cast<unsigned char>( 0xF8 + ( ( ( 36 - 32 ) >> 3 ) & 0x07 ) );			// 1111 1xxx
		pDst[1] = static_cast<unsigned char>( ( ( 36 - 32 ) << 5 ) & 0xE0 );					// xxx0 0000 <- 5 bits added in N8166
		pDst[2] = static_cast<unsigned char>( 0xF0 + ( ( AlsHeader.m_Freq >> 20 ) & 0x0F ) );	// 1111 xxxx
		pDst[3] = static_cast<unsigned char>( ( AlsHeader.m_Freq >> 12 ) & 0xFF );				// xxxx xxxx
		pDst[4] = static_cast<unsigned char>( ( AlsHeader.m_Freq >> 4 ) & 0xFF );				// xxxx xxxx
		pDst[5] = static_cast<unsigned char>( ( AlsHeader.m_Freq << 4 ) & 0xF0 );				// xxxx 0000
		pDst += 6;

		// Read from the top of ALS file.
		InFile.Seek( 0, CBaseStream::S_BEGIN );

		// Read ALS header through CRC field.
		pDst += InFile.Read( pDst, AlsHeader.m_AlsHeaderSize );

		// Read RA_unit_size[], if needed.
		if ( ( AlsHeader.m_RA != 0 ) && ( AlsHeader.m_RAflag == 2 ) ) {
			if ( Opt.m_StripRaInfo ) {
				// Skip RAU size.
				InFile.Seek( AlsHeader.m_RAUnits * sizeof(unsigned long), CBaseStream::S_CURRENT );
			} else {
				// Read RAU size.
				pDst += InFile.Read( pDst, AlsHeader.m_RAUnits * sizeof(unsigned long) );
			}
		}
		if ( ( AlsHeader.m_RA != 0 ) && Opt.m_StripRaInfo ) {
			pConfigData[18+6] &= 0x3F;	// 00XX XXXX: set RAflag to 00 in bitstream (RAUsize not stored)
		}

		// Read aux_data[], if it exists.
		if ( AlsHeader.m_AUXenabled ) pDst += InFile.Read( pDst, AlsHeader.m_AuxSize );

		// Adjust ConfigSize.
		ConfigSize = static_cast<IMF_UINT32>( pDst - pConfigData );

		// Create output (MP4) file ///////////////////////////////////////////////////////////////////

		// Create MP4 file.
		if ( !OutFile.Open( Opt.m_pOutFile ) ) throw A2MERR_CREATE_FILE;

		// Open MP4 writer.
		if ( !Writer.Open( OutFile, AlsHeader.m_Freq, AlsHeader.m_Chan, AlsHeader.m_Res, pConfigData, ConfigSize, false ) ) throw A2MERR_INIT_MP4WRITER;

		// Calculate sample duration and last sample duration.
		SampleDuration = AlsHeader.m_N * AlsHeader.m_RA;
		LastSampleDuration = AlsHeader.m_Samples - ( ( AlsHeader.m_RAUnits - 1 ) * SampleDuration );

		// Estimate maximum RAU size.
		if ( AlsHeader.m_RA == 0 ) {
			// The whole audio data is regarded as one RA unit.
			RauBufSize = AlsHeader.m_FileSize - static_cast<IMF_INT32>( InFile.Tell() );
		} else if ( AlsHeader.m_RAflag == 1 ) {
			// Assumes uncompressed size.
			RauBufSize = AlsHeader.m_N * AlsHeader.m_RA * AlsHeader.m_Chan * ( AlsHeader.m_Res / 8 ) + sizeof(unsigned long);
		} else if ( AlsHeader.m_RAflag == 2 ) {
			// Search the maximum size.
			RauBufSize = 0;
			for( i=0; i<AlsHeader.m_RAUnits; i++ ) if ( AlsHeader.m_RAUsize[i] > RauBufSize ) RauBufSize = AlsHeader.m_RAUsize[i];
		} else {
			// RA is enabled, but no RAU size information found.
			throw A2MERR_NO_RAU_SIZE;
		}

		// Allocate buffer for RAU.
		pRauBuf = new IMF_UINT8 [ RauBufSize ];
		if ( pRauBuf == NULL ) throw A2MERR_NO_MEMORY;

		// Copy all frames.
		if ( AlsHeader.m_RA == 0 ) {
			// Write all audio data as a single RAU.
			InFile.Read( pRauBuf, RauBufSize );
			if ( !Writer.WriteFrame( pRauBuf, RauBufSize, AlsHeader.m_Samples, true ) ) throw A2MERR_WRITE_FRAME;

		} else {
			// RAU loop
			for( i=0; i<AlsHeader.m_RAUnits; i++ ) {

				// Get RAU size.
				if ( AlsHeader.m_RAflag == 1 ) {
					InFile.Read32( RauSize );
					if ( !Opt.m_StripRaInfo ) {
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
				if ( !Writer.WriteFrame( pRauBuf, RauSize, SampleDuration, true ) ) throw A2MERR_WRITE_FRAME;
			}
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
	if ( pConfigData ) delete[] pConfigData;
	if ( pRauBuf ) delete[] pRauBuf;
	ClearAlsHeader( &AlsHeader );

	return ErrCode;
}

////////////////////////////////////////
//                                    //
//         Convert MP4 to ALS         //
//                                    //
////////////////////////////////////////
A2MERR	Mp4ToAls( const MP4OPT& Opt )
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
	CMp4aReader::CFrameInfo	FrameInfo;

	// Initialize ALS_HEADER structure.
	memset( &AlsHeader, 0, sizeof(AlsHeader) );

	try {

		// open MP4 file //////////////////////////////////////////////////////////////////////////

		// Open MP4 file.
		if ( !InFile.Open( Opt.m_pInFile ) ) throw A2MERR_OPEN_FILE;

		// Open MP4A reader.
		if ( !Reader.Open( InFile ) ) throw A2MERR_INIT_MP4READER;

		// Get decoder specific config.
		pConfigData = Reader.GetDecSpecInfo( ConfigSize );
		if ( pConfigData == NULL ) throw A2MERR_INVALID_CONFIG;

		// Analyse AudioSpecificConfig information.
		Aot = pConfigData[0] >> 3;																	// XXXX Xxxx
		if ( Aot == 0x1F ) Aot = 32 + ( ( pConfigData[0] & 0x07 ) << 3 ) | ( pConfigData[1] >> 5 );	// xxxx xXXX XXXx xxxx
		if ( Aot != 36 ) throw A2MERR_NOT_ALS;

		// Read ALS header information.
		ReadAlsHeaderFromMemory( pConfigData + 6, ConfigSize - 6, &AlsHeader );

		// open output (ALS) file /////////////////////////////////////////////////////////////////

		// Create ALS file.
		if ( !OutFile.Open( Opt.m_pOutFile ) ) throw A2MERR_CREATE_FILE;

		// Write ALS header.
		if ( ( AlsHeader.m_RA != 0 ) && ( AlsHeader.m_RAflag == 0 ) ) {
			// RAU size has been stripped in encoding.
			if ( Opt.m_RaLocation == 0 ) {
				pConfigData[18+6] |= 0x40;	// 01xx xxxx: set RAflag to 01 in bitstream (RAU size in frame)
				OutFile.Write( pConfigData + 6, ConfigSize - 6 );

			} else if ( Opt.m_RaLocation == 1 ) {
				pConfigData[18+6] |= 0x80;	// 10xx xxxx: set RAflag to 10 in bitstream (RAU size in header)
				// Write ALS header, inserting RAU size information.
				OutFile.Write( pConfigData + 6, AlsHeader.m_AlsHeaderSize );
				for( i=0; i<AlsHeader.m_RAUnits; i++ ) {
					if ( !Reader.GetFrameInfo( i, FrameInfo ) ) throw A2MERR_NO_FRAMEINFO;
					OutFile.Write32( FrameInfo.m_EncSize );
				}
				if ( AlsHeader.m_AUXenabled ) OutFile.Write( pConfigData + 6 + AlsHeader.m_AlsHeaderSize, AlsHeader.m_AuxSize );

			} else if ( Opt.m_RaLocation == 2 ) {
				// RA is enabled, but RAU size location has not been specified.
				printf( "***** WARNING: Random access is enabled, but RAU size information has been stripped. *****\n" );
				OutFile.Write( pConfigData + 6, ConfigSize - 6 );

			} else {
				// Invalid Opt.m_RaLocation value.
				throw A2MERR_INVALID_OPTION;
			}
		} else {	// ( RA == 0 ) || ( AlsHeader.m_RAflag != 0 )
			// Just copy ALS header as is.
			OutFile.Write( pConfigData + 6, ConfigSize - 6 );
		}

		// Allocate buffer for RAU.
		MaxRauSize = Reader.GetMaxFrameSize();
		if ( MaxRauSize == 0 ) throw A2MERR_MAX_SIZE;
		pRauBuf = new IMF_UINT8 [ MaxRauSize ];
		if ( pRauBuf == NULL ) throw A2MERR_NO_MEMORY;

		// RAU loop
		for( i=0; i<AlsHeader.m_RAUnits; i++ ) {
			RauSize = Reader.ReadFrame( i, pRauBuf, MaxRauSize );
			if ( RauSize == 0 ) throw A2MERR_READ_FRAME;

			if ( ( AlsHeader.m_RA != 0 ) && ( AlsHeader.m_RAflag == 0 ) && ( Opt.m_RaLocation == 0 ) ) {
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

// End of als2mp4.cpp
