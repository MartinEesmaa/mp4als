/******************* MPEG-4 Audio Lossless Coding ******************
 ******************* MPEG-A Audio Archival MAF    ******************

This software module was originally developed by

NTT (Nippon Telegraph and Telephone Corporation), Japan

in the course of development of the MPEG-4 Audio standard ISO/IEC 
14496-3, associated amendments and the ISO/IEC 23000-6: Audio 
Archival Multimedia Application Format standard.
This software module is an implementation of a part of one or more 
MPEG-4 Audio lossless coding tools as specified by the MPEG-4 Audio 
standard and ISO/IEC 23000-6: Audio Archival Multimedia Application 
Format tools  as specified by the MPEG-A Requirements.
ISO/IEC gives users of the MPEG-4 Audio standards and of ISO/IEC 
23000-6: Audio Archival Multimedia Application Format free license 
to this software module or modifications thereof for use in hardware 
or software products claiming conformance to MPEG-4 Audio and MPEG-A.
Those intending to use this software module in hardware or software 
products are advised that its use may infringe existing patents. 
The original developer of this software module and his/her company, 
the subsequent editors and their companies, and ISO/IEC have no 
liability for use of this software module or modifications thereof 
in an implementation.
Copyright is not released for non MPEG-4 / MPEG-A conforming 
products. The organizations named above retain full rights to use 
the code for their own purpose, assign or donate the code to a third 
party and inhibit third parties from using the code for non MPEG-4 / 
MPEG-A conforming products.

Copyright (c) 2006.

This notice must be included in all copies or derivative works.

Filename : ImfFileStream.h
Project  : MPEG-A Audio Archival Multimedia Application Format
Author   : Koichi Sugiura (NTT Advanced Technology Corporation)
           Noboru Harada  (NTT)
Date     : August 31st, 2006
Contents : File stream classes

*******************************************************************/

#if !defined( IMFFILESTREAM_INCLUDED )
#define	IMFFILESTREAM_INCLUDED

#include	<iostream>
#include	<cstdio>
#include	<cstring>
#include	"ImfType.h"
#include	"ImfStream.h"

namespace NAlsImf {

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                        CFileReader class                         //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	class	CFileReader : public CBaseStream {
	public:
		CFileReader( void ) : m_fp( NULL ), m_Offset( 0 ) {}
		virtual	~CFileReader( void ) { Close(); }
		IMF_UINT32	Read( void* pBuffer, IMF_UINT32 Size );
		IMF_UINT32	Write( const void* pBuffer, IMF_UINT32 Size ) { SetLastError( E_READONLY ); return 0; }
		IMF_INT64	Tell( void );
		bool		Seek( IMF_INT64 Offset, SEEK_ORIGIN Origin );
		bool		Open( const char* pFilename, IMF_INT64 Offset = 0 );
		bool		Close( void );
	protected:
		FILE*		m_fp;			// File pointer
		IMF_INT64	m_Offset;		// Offset position
	};

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                        CFileWriter class                         //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	class	CFileWriter : public CBaseStream {
	public:
		enum { FW_OPEN_EXISTING = 1, FW_NO_TRUNCATE = 2 };
		CFileWriter( void ) : m_fp( NULL ), m_Offset( 0 ), m_Mode( 0 ) {}
		virtual	~CFileWriter( void ) { Close(); }
		IMF_UINT32	Read( void* pBuffer, IMF_UINT32 Size ) { SetLastError( E_WRITEONLY ); return 0; }
		IMF_UINT32	Write( const void* pBuffer, IMF_UINT32 Size );
		IMF_INT64	Tell( void );
		bool		Seek( IMF_INT64 Offset, SEEK_ORIGIN Origin );
		bool		Open( const char* pFilename, IMF_INT64 Offset = 0, IMF_UINT32 Mode = 0 );
		bool		Close( void );
	protected:
		FILE*		m_fp;			// File pointer
		IMF_INT64	m_Offset;		// Offset position
		IMF_UINT32	m_Mode;			// Stream mode
	};

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                       CHexDumpStream class                       //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	class	CHexDumpStream : public CBaseStream {
	public:
		CHexDumpStream( void ) : m_pStream( NULL ), m_Indent( 0 ), m_Pos( 0 ) { memset( m_Data, 0, sizeof(m_Data) ); }
		virtual	~CHexDumpStream( void ) {}
		IMF_UINT32	Read( void* pBuffer, IMF_UINT32 Size ) { SetLastError( E_WRITEONLY ); return 0; }
		IMF_UINT32	Write( const void* pBuffer, IMF_UINT32 Size );
		IMF_INT64	Tell( void ) { SetLastError( E_TELL_STREAM ); return -1; }
		bool		Seek( IMF_INT64 Offset, SEEK_ORIGIN Origin ) { SetLastError( E_SEEK_STREAM ); return false; }
		bool		Open( std::ostream& OutStream, IMF_UINT16 Indent = 0 );
		bool		Close( void );
	protected:
		void		FlushLine( void );
	protected:
		std::ostream*	m_pStream;	// File pointer
		IMF_UINT16		m_Indent;	// Indent size
		IMF_UINT8		m_Data[16];	// Data buffer
		IMF_UINT8		m_Pos;		// Data position
	};
}

#endif	// IMFFILESTREAM_INCLUDED

// End of ImfFileStream.h
