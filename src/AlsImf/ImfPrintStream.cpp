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

Filename : ImfPrintStream.cpp
Project  : MPEG-4 Audio Lossless Coding
Author   : Koichi Sugiura (NTT Advanced Technology Corporation)
           Noboru Harada  (NTT)
Date     : August 31st, 2006
Contents : Print stream classes

*******************************************************************/

#include	<string>
#include	"ImfPrintStream.h"

using namespace NAlsImf;

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                       CHexDumpStream class                       //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Open                //
//                                    //
////////////////////////////////////////
// OutStream = Output stream
// Indent = Indent size in characters
// Return value = true:Success / false:Error
bool	CHexDumpStream::Open( std::ostream& OutStream, IMF_UINT16 Indent )
{
	if ( m_pStream != NULL ) { SetLastError( E_ALREADY_OPENED ); return false; }
	
	// Set pointer to output stream.
	m_pStream = &OutStream;

	// Initialize member variables.
	m_Indent = Indent;
	memset( m_Data, 0, sizeof(m_Data) );
	m_Pos = 0;
	return true;
}

////////////////////////////////////////
//                                    //
//               Close                //
//                                    //
////////////////////////////////////////
// Return value = true:Success / false:Error
bool	CHexDumpStream::Close( void )
{
	if ( m_pStream == NULL ) { SetLastError( E_NOT_OPENED ); return false; }

	FlushLine();
	m_pStream = NULL;
	m_Indent = 0;
	memset( m_Data, 0, sizeof(m_Data) );
	m_Pos = 0;
	return true;
}

////////////////////////////////////////
//                                    //
//             Write data             //
//                                    //
////////////////////////////////////////
// pBuffer = Pointer to data
// Size = Data size in bytes
// Return value = Actually written byte count
IMF_UINT32	CHexDumpStream::Write( const void* pBuffer, IMF_UINT32 Size )
{
	if ( m_pStream == NULL ) { SetLastError( E_NOT_OPENED ); return 0; }

	IMF_UINT32			i;
	const IMF_UINT8*	p = reinterpret_cast<const IMF_UINT8*>( pBuffer );

	for( i=0; i<Size; i++ ) {
		m_Data[m_Pos++] = *p++;
		if ( m_Pos >= 16 ) {
			FlushLine();
			m_Pos = 0;
		}
	}
	return Size;
}

////////////////////////////////////////
//                                    //
//            Flush a line            //
//                                    //
////////////////////////////////////////
void	CHexDumpStream::FlushLine( void )
{
	IMF_UINT16	i;
	IMF_UINT8	j;
	char		StrBuf[16];

	if ( m_Pos < 1 ) return;

	// Make indent.
	for( i=0; i<m_Indent; i++ ) *m_pStream << ' ';

	// Hex dump.
	for( j=0; j<m_Pos; j++ ) { sprintf( StrBuf, "%02X ", m_Data[j] ); *m_pStream << StrBuf; }
	for( ; j<17; j++ ) *m_pStream << "   ";

	// Ascii dump.
	for( j=0; j<m_Pos; j++ ) *m_pStream << static_cast<char>( ( ( m_Data[j] >= 0x20 ) && ( m_Data[j] <= 0x7e ) ) ? m_Data[j] : '.' );
	for( ; j<16; j++ ) *m_pStream << ' ';
	*m_pStream << std::endl;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                        CPrintStream class                        //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Open                //
//                                    //
////////////////////////////////////////
// OutStream = Output stream
// Indent = Indent size in characters
// MaxLen = Maximum string length per line
// Return value = true:Success / false:Error
bool	CPrintStream::Open( std::ostream& OutStream, IMF_UINT16 Indent, IMF_UINT16 MaxLen )
{
	if ( m_pStream != NULL ) { SetLastError( E_ALREADY_OPENED ); return false; }
	
	// Set pointer to output stream.
	m_pStream = &OutStream;

	// Initialize member variables.
	m_Indent = Indent;
	m_MaxLen = MaxLen;
	m_Line.erase();
	return true;
}

////////////////////////////////////////
//                                    //
//               Close                //
//                                    //
////////////////////////////////////////
// Return value = true:Success / false:Error
bool	CPrintStream::Close( void )
{
	if ( m_pStream == NULL ) { SetLastError( E_NOT_OPENED ); return false; }

	FlushLine();
	m_pStream = NULL;
	m_Indent = 0;
	m_MaxLen = 100;
	return true;
}

////////////////////////////////////////
//                                    //
//             Write data             //
//                                    //
////////////////////////////////////////
// pBuffer = Pointer to data
// Size = Data size in bytes
// Return value = Actually written byte count
IMF_UINT32	CPrintStream::Write( const void* pBuffer, IMF_UINT32 Size )
{
	const char*	p = static_cast<const char*>( pBuffer );
	IMF_UINT32	i;

	for( i=0; i<Size; i++ ) {
		if ( p[i] == '\n' ) FlushLine();					// '\n' causes FlushLine().
		else if ( p[i] != '\r' ) m_Line.append( 1, p[i] );	// Ignore '\r'.
	}
	return Size;
}

////////////////////////////////////////
//                                    //
//            Flush a line            //
//                                    //
////////////////////////////////////////
void	CPrintStream::FlushLine( void )
{
	if ( !m_Line.empty() ) {
		// Make indent.
		for( IMF_UINT16 i=0; i<m_Indent; i++ ) *m_pStream << ' ';
		// Cut off the longer line than m_MaxLen.
		if ( ( m_MaxLen > 0 ) && ( m_Line.size() > m_MaxLen ) ) m_Line.erase( m_MaxLen ).append( "..." );
		*m_pStream << m_Line << std::endl;
		m_Line.erase();
	}
}

////////////////////////////////////////
//                                    //
//          Global operators          //
//                                    //
////////////////////////////////////////
CPrintStream&	NAlsImf::operator << ( CPrintStream& OStr, IMF_INT8   Value ) { char Buf[16]; OStr.Write( Buf, sprintf( Buf, "%d", static_cast<int>( Value ) ) ); return OStr; }
CPrintStream&	NAlsImf::operator << ( CPrintStream& OStr, IMF_UINT8  Value ) { char Buf[16]; OStr.Write( Buf, sprintf( Buf, "%u", static_cast<unsigned int>( Value ) ) ); return OStr; }
CPrintStream&	NAlsImf::operator << ( CPrintStream& OStr, IMF_INT16  Value ) { char Buf[16]; OStr.Write( Buf, sprintf( Buf, "%hd", Value ) ); return OStr; }
CPrintStream&	NAlsImf::operator << ( CPrintStream& OStr, IMF_UINT16 Value ) { char Buf[16]; OStr.Write( Buf, sprintf( Buf, "%hu", Value ) ); return OStr; }
CPrintStream&	NAlsImf::operator << ( CPrintStream& OStr, IMF_INT32  Value ) { char Buf[16]; OStr.Write( Buf, sprintf( Buf, "%d", Value ) ); return OStr; }
CPrintStream&	NAlsImf::operator << ( CPrintStream& OStr, IMF_UINT32 Value ) { char Buf[16]; OStr.Write( Buf, sprintf( Buf, "%u", Value ) ); return OStr; }
#if defined( _MSC_VER )
CPrintStream&	NAlsImf::operator << ( CPrintStream& OStr, IMF_INT64  Value ) { char Buf[16]; OStr.Write( Buf, sprintf( Buf, "%I64d", Value ) ); return OStr; }
CPrintStream&	NAlsImf::operator << ( CPrintStream& OStr, IMF_UINT64 Value ) { char Buf[16]; OStr.Write( Buf, sprintf( Buf, "%I64u", Value ) ); return OStr; }
#elif defined( __GNUC__ )
CPrintStream&	NAlsImf::operator << ( CPrintStream& OStr, IMF_INT64  Value ) { char Buf[16]; OStr.Write( Buf, sprintf( Buf, "%lld", Value ) ); return OStr; }
CPrintStream&	NAlsImf::operator << ( CPrintStream& OStr, IMF_UINT64 Value ) { char Buf[16]; OStr.Write( Buf, sprintf( Buf, "%llu", Value ) ); return OStr; }
#endif
CPrintStream&	NAlsImf::operator << ( CPrintStream& OStr, const char* pStr ) { OStr.Write( pStr, static_cast<IMF_UINT32>( strlen( pStr ) ) ); return OStr; }
CPrintStream&	NAlsImf::operator << ( CPrintStream& OStr, const std::string& Str ) { OStr.Write( Str.c_str(), static_cast<IMF_UINT32>( Str.size() ) ); return OStr; }
CPrintStream&	NAlsImf::operator << ( CPrintStream& OStr, bool Value ) { if ( Value ) OStr.Write( "true", 4 ); else OStr.Write( "false", 5 ); return OStr; }
CPrintStream&	NAlsImf::operator << ( CPrintStream& OStr, std::ostream& (*pf)( std::ostream& ) ) { OStr.Write( "\n", 1 ); return OStr; }

// End of ImfPrintStream.cpp
