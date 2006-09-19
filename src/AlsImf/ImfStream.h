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

Filename : ImfStream.h
Project  : MPEG-A Audio Archival Multimedia Application Format
Author   : Koichi Sugiura (NTT Advanced Technology Corporation)
           Noboru Harada  (NTT)
Date     : August 31st, 2006
Contents : General stream interface

*******************************************************************/

#if !defined( IMFSTREAM_INCLUDED )
#define	IMFSTREAM_INCLUDED

#include	"ImfType.h"

namespace NAlsImf {

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                           Error codes                            //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	const IMF_UINT32	E_NONE           =  0;		// No error.
	const IMF_UINT32	E_MEMORY         =  1;		// Memory error.
	const IMF_UINT32	E_OPEN_STREAM    =  2;		// Failed to open a stream.
	const IMF_UINT32	E_CLOSE_STREAM   =  3;		// Failed to close a stream.
	const IMF_UINT32	E_READ_STREAM    =  4;		// Failed to read from a stream.
	const IMF_UINT32	E_WRITE_STREAM   =  5;		// Failed to write to a stream.
	const IMF_UINT32	E_SEEK_STREAM    =  6;		// Failed to seek a stream.
	const IMF_UINT32	E_TELL_STREAM    =  7;		// Failed to tell a stream.
	const IMF_UINT32	E_READONLY       =  8;		// This stream is read-only.
	const IMF_UINT32	E_WRITEONLY      =  9;		// This stream is write-only.
	const IMF_UINT32	E_NOT_OPENED     = 10;		// Stream is not opened yet.
	const IMF_UINT32	E_ALREADY_OPENED = 11;		// Stream is already opened.
	
	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                        CBaseStream class                         //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	class	CBaseStream {
	public:
		enum SEEK_ORIGIN {
			S_BEGIN   = 0,			// Offset from the beginning.
			S_CURRENT = 1,			// Offset from the current position.
			S_END     = 2,			// Offset from the end.
		};
		CBaseStream( void ) : m_LastError( E_NONE ) {}
		virtual	~CBaseStream( void ) {}
		virtual	IMF_UINT32	Read( void* pBuffer, IMF_UINT32 Size ) = 0;
		virtual	IMF_UINT32	Write( const void* pBuffer, IMF_UINT32 Size ) = 0;
		virtual	IMF_INT64	Tell( void ) = 0;
		virtual	bool		Seek( IMF_INT64 Offset, SEEK_ORIGIN Origin ) = 0;

		// Read 8-bit value.
		bool	Read8( IMF_INT8& Value ) { return Read8( reinterpret_cast<IMF_UINT8&>( Value ) ); }
		bool	Read8( IMF_UINT8& Value ) { return ( Read( &Value, sizeof(Value) ) == sizeof(Value) ); }

		// Read 16-bit value.
		bool	Read16( IMF_INT16& Value ) { return Read16( reinterpret_cast<IMF_UINT16&>( Value ) ); }
		bool	Read16( IMF_UINT16& Value ) {
			IMF_UINT8 Buf[2];
			if ( Read( Buf, sizeof(Buf) ) != sizeof(Buf) ) return false;
			Value = ( static_cast<IMF_UINT16>( Buf[0] ) << 8 ) | static_cast<IMF_UINT16>( Buf[1] );
			return true;
		}
		
		// Read 32-bit value.
		bool	Read32( IMF_INT32& Value ) { return Read32( reinterpret_cast<IMF_UINT32&>( Value ) ); }
		bool	Read32( IMF_UINT32& Value ) {
			IMF_UINT8	Buf[4];
			if ( Read( Buf, sizeof(Buf) ) != sizeof(Buf) ) return false;
			Value = ( static_cast<IMF_UINT32>( Buf[0] ) << 24 ) | ( static_cast<IMF_UINT32>( Buf[1] ) << 16 ) |
					( static_cast<IMF_UINT32>( Buf[2] ) <<  8 ) |   static_cast<IMF_UINT32>( Buf[3] );
			return true;
		}

		// Read 64-bit value.
		bool	Read64( IMF_INT64& Value ) { return Read64( reinterpret_cast<IMF_UINT64&>( Value ) ); }
		bool	Read64( IMF_UINT64& Value ) {
			IMF_UINT8	Buf[8];
			if ( Read( Buf, sizeof(Buf) ) != sizeof(Buf) ) return false;
			Value = ( static_cast<IMF_UINT64>( Buf[0] ) << 56 ) | ( static_cast<IMF_UINT64>( Buf[1] ) << 48 ) |
					( static_cast<IMF_UINT64>( Buf[2] ) << 40 ) | ( static_cast<IMF_UINT64>( Buf[3] ) << 32 ) |
					( static_cast<IMF_UINT64>( Buf[4] ) << 24 ) | ( static_cast<IMF_UINT64>( Buf[5] ) << 16 ) |
					( static_cast<IMF_UINT64>( Buf[6] ) <<  8 ) |   static_cast<IMF_UINT64>( Buf[7] );
			return true;
		}

		// Write 8-bit value.
		bool	Write8( IMF_UINT8 Value ) { return ( Write( &Value, sizeof(Value) ) == sizeof(Value) ); }

		// Write 16-bit value.
		bool	Write16( IMF_UINT16 Value ) {
			IMF_UINT8	Buf[2];
			Buf[0] = static_cast<IMF_UINT8>( Value >> 8 );
			Buf[1] = static_cast<IMF_UINT8>( Value );
			return ( Write( Buf, sizeof(Buf) ) == sizeof(Buf) );
		}

		// Write 32-bit value.
		bool	Write32( IMF_UINT32 Value ) {
			IMF_UINT8	Buf[4];
			Buf[0] = static_cast<IMF_UINT8>( Value >> 24 );
			Buf[1] = static_cast<IMF_UINT8>( Value >> 16 );
			Buf[2] = static_cast<IMF_UINT8>( Value >>  8 );
			Buf[3] = static_cast<IMF_UINT8>( Value );
			return ( Write( Buf, sizeof(Buf) ) == sizeof(Buf) );
		}

		// Write 64-bit value.
		bool	Write64( IMF_UINT64 Value ) {
			IMF_UINT8	Buf[8];
			Buf[0] = static_cast<IMF_UINT8>( Value >> 56 );
			Buf[1] = static_cast<IMF_UINT8>( Value >> 48 );
			Buf[2] = static_cast<IMF_UINT8>( Value >> 40 );
			Buf[3] = static_cast<IMF_UINT8>( Value >> 32 );
			Buf[4] = static_cast<IMF_UINT8>( Value >> 24 );
			Buf[5] = static_cast<IMF_UINT8>( Value >> 16 );
			Buf[6] = static_cast<IMF_UINT8>( Value >>  8 );
			Buf[7] = static_cast<IMF_UINT8>( Value );
			return ( Write( Buf, sizeof(Buf) ) == sizeof(Buf) );
		}

		// Get last error code.
		IMF_UINT32	GetLastError( void ) const { return m_LastError; }
	protected:
		void		SetLastError( IMF_UINT32 ErrCode ) { m_LastError = ErrCode; }
		IMF_UINT32	m_LastError;	// Last error code
	};
}

#endif	// IMFSTREAM_INCLUDED

// End of ImfStream.h
