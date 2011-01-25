/***************** MPEG-4 Audio Lossless Coding *********************

This software module was originally developed by

NTT (Nippon Telegraph and Telephone Corporation), Japan

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

Filename : ImfFileStream.cpp
Project  : MPEG-4 Audio Lossless Coding
Author   : Koichi Sugiura (NTT Advanced Technology Corporation)
           Noboru Harada  (NTT)
Date     : August 31st, 2006
Contents : File stream classes

*******************************************************************/

/******************************************************************
 *
 * Modifications:
 *
 * 2007/05/23, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - moved CHexDumpStream class to ImfPrintStream.cpp.
 *
 * 2011/01/24, Csaba Kos <csaba.kos@as.ntt-at.co.jp>
 *   - added detection of FreeBSD.
 *
 ******************************************************************/

#include	<string>
#include	"ImfFileStream.h"

using namespace NAlsImf;

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                      OS dependent function                       //
//                                                                  //
//////////////////////////////////////////////////////////////////////

#if defined( _MSC_VER )

//----------------------------------------
// For Win32 / VC++6 and VC++.net 2003
//----------------------------------------
#include	<windows.h>
#include	<io.h>

// 64-bit functions
#define	FOPEN64( a, b )		fopen( a, b )
#define	FSEEK64( a, b, c )	fseek64_win( a, b, c )
#define	FTELL64( a )		ftell64_win( a )
#define	FTRUNCATE64( a )	SetEndOfFile( reinterpret_cast<HANDLE>( _get_osfhandle( _fileno( a ) ) ) )

__int64	ftell64_win( FILE* fp )
{
	fpos_t	Pos = 0;
	return ( fgetpos( fp, &Pos ) == 0 ) ? Pos : -1i64;
}

int	fseek64_win( FILE* fp, __int64 offset, int origin )
{
	fpos_t	Pos;
	switch( origin ) {
	case	SEEK_SET:
		Pos = offset;
		break;
	case	SEEK_CUR:
		if ( fgetpos( fp, &Pos ) != 0 ) return -1;
		Pos += offset;
		break;
	case	SEEK_END:
		if ( ( fflush( fp ) != 0 ) || ( ( Pos = _filelengthi64( _fileno( fp ) ) ) == -1i64 ) ) return -1;
		Pos += offset;
		break;
	default:
		return -1;
	}
	return fsetpos( fp, &Pos );
}

#elif defined( __linux__ ) && defined( __GNUG__ )

//----------------------------------------
// For Linux / g++
//----------------------------------------
#include	<unistd.h>
#include	<sys/types.h>

// 64-bit functions
#define	FOPEN64( a, b )		fopen64( a, b )
#define	FSEEK64( a, b, c )	fseeko64( a, b, c )
#define	FTELL64( a )		ftello64( a )
#define	FTRUNCATE64( a )	ftruncate64( fileno( a ), ftello64( a ) )

#elif (defined( __APPLE__ ) || defined( __FreeBSD__ )) && defined( __GNUG__ )

//----------------------------------------
// For Mac OS X / g++
//----------------------------------------
#include	<unistd.h>

// 64-bit functions
#define	FOPEN64( a, b )		fopen( a, b )
#define	FSEEK64( a, b, c )	fseeko( a, b, c )
#define	FTELL64( a )		ftello( a )
#define	FTRUNCATE64( a )	ftruncate( fileno( a ), ftello( a ) )

#else
#error Unknown compiler.
#endif

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                        CFileReader class                         //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Open                //
//                                    //
////////////////////////////////////////
// pFilename = File name to open.
// Offset = Offset position
// Return value = true:Success / false:Error
bool	CFileReader::Open( const char* pFilename, IMF_INT64 Offset )
{
	bool	Result = true;

	// Check double open.
	if ( m_fp != NULL ) {
		SetLastError( E_ALREADY_OPENED );
		return false;
	}

	try {
		// Open a file.
		m_fp = FOPEN64( pFilename, "rb" );
		if ( m_fp == NULL ) throw E_OPEN_STREAM;

		// Seek to Offset.
		if ( Offset != 0 ) {
			if ( FSEEK64( m_fp, Offset, S_BEGIN ) != 0 ) throw E_SEEK_STREAM;
			if ( FTELL64( m_fp ) != Offset ) throw E_SEEK_STREAM;
		}
		m_Offset = Offset;
	}
	catch( IMF_UINT32 ErrCode ) {
		// Clean up.
		if ( m_fp ) {
			fclose( m_fp );
			m_fp = NULL;
		}
		m_Offset = 0;
		SetLastError( ErrCode );
		Result = false;
	}
	return Result;
}

////////////////////////////////////////
//                                    //
//               Close                //
//                                    //
////////////////////////////////////////
// Return value = true:Success / false:Error
bool	CFileReader::Close( void )
{
	bool	Result = true;

	if ( m_fp != NULL ) {
		// Close a stream.
		if ( fclose( m_fp ) != 0 ) {
			SetLastError( E_CLOSE_STREAM );
			Result = false;
		}
		m_fp = NULL;
	}
	m_Offset = 0;
	return Result;
}

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// pBuffer = Buffer to store read data
// Size = Number of bytes to read
// Return value = Actual read byte count
IMF_UINT32	CFileReader::Read( void* pBuffer, IMF_UINT32 Size )
{
	// Make sure that the stream is opened.
	if ( m_fp == NULL ) {
		SetLastError( E_NOT_OPENED );
		return 0;
	}

	// Read data.
	return static_cast<IMF_UINT32>( fread( pBuffer, 1, Size, m_fp ) );
}

////////////////////////////////////////
//                                    //
//                Tell                //
//                                    //
////////////////////////////////////////
// Return value = Current file position
IMF_INT64	CFileReader::Tell( void )
{
	IMF_INT64	Result;

	// Make sure that the stream is opened.
	if ( m_fp == NULL ) {
		SetLastError( E_NOT_OPENED );
		return -1;
	}

	// Get file position.
	Result = static_cast<IMF_INT64>( FTELL64( m_fp ) );
	if ( Result == -1 ) {
		SetLastError( E_TELL_STREAM );
		return -1;
	}

	// Return adjusted offset.
	return Result - m_Offset;
}

////////////////////////////////////////
//                                    //
//                Seek                //
//                                    //
////////////////////////////////////////
// Offset = Offset
// Origin = Starting point
// Return value = true:Success / false:Error
bool	CFileReader::Seek( IMF_INT64 Offset, SEEK_ORIGIN Origin )
{
	// Make sure that the stream is opened.
	if ( m_fp == NULL ) {
		SetLastError( E_NOT_OPENED );
		return false;
	}

	// Adjust offset position.
	if ( Origin == S_BEGIN ) Offset += m_Offset;

	// Set file position.
	if ( FSEEK64( m_fp, Offset, static_cast<int>( Origin ) ) != 0 ) {
		SetLastError( E_SEEK_STREAM );
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                        CFileWriter class                         //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Open                //
//                                    //
////////////////////////////////////////
// pFilename = File name to open.
// Offset = Offset position
// Mode = Stream mode
// Return value = true:Success / false:Error
bool	CFileWriter::Open( const char* pFilename, IMF_INT64 Offset, IMF_UINT32 Mode )
{
	bool	Result = true;

	// Check double open.
	if ( m_fp != NULL ) {
		SetLastError( E_ALREADY_OPENED );
		return false;
	}

	try {
		// Open a file.
		if ( Mode & FW_OPEN_EXISTING ) {
			// Try to open an existing file.
			m_fp = FOPEN64( pFilename, "r+b" );
		}
		if ( m_fp == NULL ) {
			// Create a new file.
			m_fp = FOPEN64( pFilename, "wb" );
			if ( m_fp == NULL ) throw E_OPEN_STREAM;
		}

		// Seek to Offset.
		if ( Offset != 0 ) {
			if ( FSEEK64( m_fp, Offset, S_BEGIN ) != 0 ) throw E_SEEK_STREAM;
			if ( FTELL64( m_fp ) != Offset ) throw E_SEEK_STREAM;
		}
		m_Offset = Offset;
		m_Mode = Mode;
	}
	catch( IMF_UINT32 ErrCode ) {
		// Clean up.
		if ( m_fp ) {
			fclose( m_fp );
			m_fp = NULL;
		}
		m_Offset = 0;
		m_Mode = 0;
		SetLastError( ErrCode );
		Result = false;
	}
	return Result;
}

////////////////////////////////////////
//                                    //
//               Close                //
//                                    //
////////////////////////////////////////
// Return value = true:Success / false:Error
bool	CFileWriter::Close( void )
{
	bool	Result = true;

	if ( m_fp != NULL ) {
		if ( ( m_Mode & FW_NO_TRUNCATE ) == 0 ) {
			// Cut off data after the current write position.
			if ( fflush( m_fp ) != 0 ) {
				SetLastError( E_CLOSE_STREAM );
				Result = false;
			}
			FTRUNCATE64( m_fp );
		}

		// Close a stream.
		if ( fclose( m_fp ) != 0 ) {
			SetLastError( E_CLOSE_STREAM );
			Result = false;
		}
		m_fp = NULL;
	}
	m_Offset = 0;
	m_Mode = 0;
	return Result;
}

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// pData = Data to write
// Size = Number of bytes to write
// Return value = Actual written byte count
IMF_UINT32	CFileWriter::Write( const void* pData, IMF_UINT32 Size )
{
	// Make sure that the stream is opened.
	if ( m_fp == NULL ) {
		SetLastError( E_NOT_OPENED );
		return 0;
	}

	// Write data.
	return static_cast<IMF_UINT32>( fwrite( pData, 1, Size, m_fp ) );
}

////////////////////////////////////////
//                                    //
//                Tell                //
//                                    //
////////////////////////////////////////
// Return value = Current file position
IMF_INT64	CFileWriter::Tell( void )
{
	IMF_INT64	Result;

	// Make sure that the stream is opened.
	if ( m_fp == NULL ) {
		SetLastError( E_NOT_OPENED );
		return -1;
	}

	// Get file position.
	Result = static_cast<IMF_INT64>( FTELL64( m_fp ) );
	if ( Result == -1 ) {
		SetLastError( E_TELL_STREAM );
		return -1;
	}

	// Return adjusted offset.
	return Result - m_Offset;
}

////////////////////////////////////////
//                                    //
//                Seek                //
//                                    //
////////////////////////////////////////
// Offset = Offset
// Origin = Starting point
// Return value = true:Success / false:Error
bool	CFileWriter::Seek( IMF_INT64 Offset, SEEK_ORIGIN Origin )
{
	// Make sure that the stream is opened.
	if ( m_fp == NULL ) {
		SetLastError( E_NOT_OPENED );
		return false;
	}

	// Adjust offset position.
	if ( Origin == S_BEGIN ) Offset += m_Offset;

	// Set file position.
	if ( FSEEK64( m_fp, Offset, static_cast<int>( Origin ) ) != 0 ) {
		SetLastError( E_SEEK_STREAM );
		return false;
	}
	return true;
}

// End of ImfFileStream.cpp
