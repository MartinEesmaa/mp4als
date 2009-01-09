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

Filename : ImfFileStream.h
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
 *   - moved CHexDumpStream class to ImfPrintStream.h.
 *
 ******************************************************************/

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
}

#endif	// IMFFILESTREAM_INCLUDED

// End of ImfFileStream.h
