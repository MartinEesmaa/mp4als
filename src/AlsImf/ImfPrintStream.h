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

Filename : ImfPrintStream.h
Project  : MPEG-4 Audio Lossless Coding
Author   : Koichi Sugiura (NTT Advanced Technology Corporation)
           Noboru Harada  (NTT)
Date     : August 31st, 2006
Contents : Print stream class

*******************************************************************/

#if !defined( IMFPRINTSTREAM_INCLUDED )
#define	IMFPRINTSTREAM_INCLUDED

#include	<iostream>
#include	<cstdio>
#include	<cstring>
#include	"ImfType.h"
#include	"ImfStream.h"

// Print a parameter.
#define	IMF_PRINT( x )	Stream << #x " = " << m_##x << std::endl

namespace NAlsImf {

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

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                        CPrintStream class                        //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	class	CPrintStream : public CBaseStream {
	public:
		CPrintStream( void ) : m_pStream( NULL ), m_Indent( 0 ), m_MaxLen( 0 ) {}
		virtual	~CPrintStream( void ) {}
		IMF_UINT32	Read( void* pBuffer, IMF_UINT32 Size ) { SetLastError( E_WRITEONLY ); return 0; }
		IMF_UINT32	Write( const void* pBuffer, IMF_UINT32 Size );
		IMF_INT64	Tell( void ) { SetLastError( E_TELL_STREAM ); return -1; }
		bool		Seek( IMF_INT64 Offset, SEEK_ORIGIN Origin ) { SetLastError( E_SEEK_STREAM ); return false; }
		bool		Open( std::ostream& OutStream, IMF_UINT16 Indent = 0, IMF_UINT16 MaxLen = 0 );
		bool		Close( void );
		void		Indent( IMF_INT8 Count ) {
			IMF_INT32	Indent = m_Indent;
			Indent += Count;
			if ( Indent < 0 ) Indent = 0;
			else if ( Indent > 0xffff ) Indent = 0xffff;
			m_Indent = static_cast<IMF_UINT16>( Indent );
		}
	protected:
		void	FlushLine( void );
	protected:
		std::ostream*	m_pStream;	// File pointer
		IMF_UINT16		m_Indent;	// Indent size
		IMF_UINT16		m_MaxLen;	// Maximum string length per line
		std::string		m_Line;		// Line buffer
	};

	CPrintStream&	operator << ( CPrintStream& OStr, IMF_INT8   Value );
	CPrintStream&	operator << ( CPrintStream& OStr, IMF_UINT8  Value );
	CPrintStream&	operator << ( CPrintStream& OStr, IMF_INT16  Value );
	CPrintStream&	operator << ( CPrintStream& OStr, IMF_UINT16 Value );
	CPrintStream&	operator << ( CPrintStream& OStr, IMF_INT32  Value );
	CPrintStream&	operator << ( CPrintStream& OStr, IMF_UINT32 Value );
	CPrintStream&	operator << ( CPrintStream& OStr, IMF_INT64  Value );
	CPrintStream&	operator << ( CPrintStream& OStr, IMF_UINT64 Value );
	CPrintStream&	operator << ( CPrintStream& OStr, const char* pStr );
	CPrintStream&	operator << ( CPrintStream& OStr, const std::string& Str );
	CPrintStream&	operator << ( CPrintStream& OStr, bool Value );
	CPrintStream&	operator << ( CPrintStream& OStr, std::ostream& (*pf)( std::ostream& ) );
}

#endif	// IMFPRINTSTREAM_INCLUDED

// End of ImfPrintStream.h
