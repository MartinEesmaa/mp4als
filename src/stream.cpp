/***************** MPEG-4 Audio Lossless Coding **************************

This software module was originally developed by

Koichi Sugiura (NTT Advanced Technology Corporation)

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

Copyright (c) 2007.

filename : stream.cpp
project  : MPEG-4 Audio Lossless Coding
author   : Koichi Sugiura (NTT Advanced Technology Corporation)
date     : May 23, 2007
contents : Bridge functions to stream classes in AlsImf

*************************************************************************/

#include	"stream.h"
#include	"ImfFileStream.h"

using namespace NAlsImf;

// Stream mode
typedef enum tagALSSTREAM_MODE {
	ALSSTRMODE_READER,		// File reader mode
	ALSSTRMODE_WRITER,		// File writer mode
} ALSSTREAM_MODE;

// Stream information
typedef	struct tagALSSTREAM {
	ALSSTREAM_MODE			m_Mode;		// Stream mode
	NAlsImf::CFileReader	m_Reader;	// File reader object
	NAlsImf::CFileWriter	m_Writer;	// File writer object
} ALSSTREAM;

////////////////////////////////////////
//                                    //
//         Close file stream          //
//                                    //
////////////////////////////////////////
// fp = File handle
// Return value = 0:Success / EOF:Error
int	fclose( HALSSTREAM fp )
{
	// Check parameter.
	if ( fp == NULL ) return EOF;

	// Close files.
	ALSSTREAM*	pStream = reinterpret_cast<ALSSTREAM*>( fp );
	pStream->m_Reader.Close();
	pStream->m_Writer.Close();
	delete pStream;
	return 0;
}

////////////////////////////////////////
//                                    //
//        Get current position        //
//                                    //
////////////////////////////////////////
// fp = File handle
// Return value = Current position
ALS_INT64	ftell( HALSSTREAM fp )
{
	// Check parameter.
	if ( fp == NULL ) return -1;

	ALSSTREAM*	pStream = reinterpret_cast<ALSSTREAM*>( fp );
	return ( pStream->m_Mode == ALSSTRMODE_READER ) ? pStream->m_Reader.Tell() : pStream->m_Writer.Tell();
}

////////////////////////////////////////
//                                    //
//               Rewind               //
//                                    //
////////////////////////////////////////
// fp = File handle
void	rewind( HALSSTREAM fp )
{
	// Check parameter.
	if ( fp == NULL ) return;

	ALSSTREAM*	pStream = reinterpret_cast<ALSSTREAM*>( fp );
	if ( pStream->m_Mode == ALSSTRMODE_READER ) pStream->m_Reader.Seek( 0, CBaseStream::S_BEGIN );
	else pStream->m_Writer.Seek( 0, CBaseStream::S_BEGIN );
}

////////////////////////////////////////
//                                    //
//                Seek                //
//                                    //
////////////////////////////////////////
// fp = File handle
// offset = Offset position from origin
// origin = Base point (SEEK_SET or SEEK_CUR or SEEK_END)
// Return value = 0:Success / -1:Error
int	fseek( HALSSTREAM fp, ALS_INT64 offset, int origin )
{
	// Check parameter.
	if ( fp == NULL ) return -1;

	ALSSTREAM*	pStream = reinterpret_cast<ALSSTREAM*>( fp );
	if ( pStream->m_Mode == ALSSTRMODE_READER ) {
		return pStream->m_Reader.Seek( offset, static_cast<CBaseStream::SEEK_ORIGIN>( origin ) ) ? 0 : -1;
	} else {
		return pStream->m_Writer.Seek( offset, static_cast<CBaseStream::SEEK_ORIGIN>( origin ) ) ? 0 : -1;
	}
}

////////////////////////////////////////
//                                    //
//             Write data             //
//                                    //
////////////////////////////////////////
// buffer = Data to write
// size = Data unit size in bytes
// count = Number of data units
// fp = File handle
// Return value = Number of actually written data units
ALS_UINT32	fwrite( const void* buffer, ALS_UINT32 size, ALS_UINT32 count, HALSSTREAM fp )
{
	// Check parameter.
	if ( fp == NULL ) return 0;

	ALSSTREAM*	pStream = reinterpret_cast<ALSSTREAM*>( fp );
	if ( ( pStream->m_Mode != ALSSTRMODE_WRITER ) || ( size == 0 ) || ( count == 0 ) ) return 0;

	ALS_UINT64	TotalSize = static_cast<ALS_UINT64>( size ) * static_cast<ALS_UINT64>( count );
	if ( TotalSize > 0xffffffff ) TotalSize = 0xffffffff;
	return pStream->m_Writer.Write( buffer, static_cast<IMF_UINT32>( TotalSize ) ) / size;
}

////////////////////////////////////////
//                                    //
//             Read data              //
//                                    //
////////////////////////////////////////
// buffer = Read data buffer
// size = Data unit size in bytes
// count = Number of data untis
// fp = File handle
// Return value = Number of actually read data units
ALS_UINT32	fread( void* buffer, ALS_UINT32 size, ALS_UINT32 count, HALSSTREAM fp )
{
	// Check parameter.
	if ( fp == NULL ) return 0;

	ALSSTREAM*	pStream = reinterpret_cast<ALSSTREAM*>( fp );
	if ( ( pStream->m_Mode != ALSSTRMODE_READER ) || ( size == 0 ) || ( count == 0 ) ) return 0;

	ALS_UINT64	TotalSize = static_cast<ALS_UINT64>( size ) * static_cast<ALS_UINT64>( count );
	if ( TotalSize > 0xffffffff ) TotalSize = 0xffffffff;
	return pStream->m_Reader.Read( buffer, static_cast<IMF_UINT32>( TotalSize ) ) / size;
}

////////////////////////////////////////
//                                    //
//      Open a file for reading       //
//                                    //
////////////////////////////////////////
// pFilename = Filename to open
// phStream = Pointer to variable which receives stream handle
// Return value = Error code (0 means no error)
int	OpenFileReader( const char* pFilename, HALSSTREAM* phStream )
{
	ALSSTREAM*	pStream = NULL;
	int			RetCode = 0;

	try {
		// Check parameters.
		if ( ( pFilename == NULL ) || ( *pFilename == '\0' ) || ( phStream == NULL ) ) throw -1;

		// Create ALSSTREAM structure.
		pStream = new ALSSTREAM;
		if ( pStream == NULL ) throw -2;
		pStream->m_Mode = ALSSTRMODE_READER;

		// Open a file.
		if ( !pStream->m_Reader.Open( pFilename ) ) throw -3;

		// Save pStream as HALSSTREAM.
		*phStream = reinterpret_cast<HALSSTREAM>( pStream );
	}
	catch( int e ) {
		if ( pStream != NULL ) {
			pStream->m_Reader.Close();
			delete pStream;
		}
		RetCode = e;
	}
	return RetCode;
}

////////////////////////////////////////
//                                    //
//     Create a file for writing      //
//                                    //
////////////////////////////////////////
// pFilename = Filename to create
// phStream = Pointer to variable which receives stream handle
// Return value = Error code (0 means no error)
int	OpenFileWriter( const char* pFilename, HALSSTREAM* phStream )
{
	ALSSTREAM*	pStream = NULL;
	int			RetCode = 0;

	try {
		// Check parameters.
		if ( ( pFilename == NULL ) || ( *pFilename == '\0' ) || ( phStream == NULL ) ) throw -1;

		// Create ALSSTREAM structure.
		pStream = new ALSSTREAM;
		if ( pStream == NULL ) throw -2;
		pStream->m_Mode = ALSSTRMODE_WRITER;

		// Create a file.
		if ( !pStream->m_Writer.Open( pFilename, 0, CFileWriter::FW_NO_TRUNCATE ) ) throw -3;

		// Save pStream as HALSSTREAM.
		*phStream = reinterpret_cast<HALSSTREAM>( pStream );
	}
	catch( int e ) {
		if ( pStream != NULL ) {
			pStream->m_Writer.Close();
			delete pStream;
		}
		RetCode = e;
	}
	return RetCode;
}

// End of stream.cpp
