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

Filename : ImfDescriptor.cpp
Project  : MPEG-A Audio Archival Multimedia Application Format
Author   : Koichi Sugiura (NTT Advanced Technology Corporation)
           Noboru Harada  (NTT)
Date     : August 31st, 2006
Contents : Descriptor classes defined in ISO/IEC 14496-12

*******************************************************************/

#include	"ImfDescriptor.h"

using namespace NAlsImf;

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                      CBaseDescriptor class                       //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//           Set data size            //
//                                    //
////////////////////////////////////////
// Size = Data size in bytes
// Return value = Whole size in bytes (-1 = error)
IMF_INT64	CBaseDescriptor::SetDataSize( IMF_INT64 Size )
{
	int	i;
	if ( ( Size < 0 ) || ( Size > 0xffffffff ) ) { SetLastError( E_DESCR_SET_SIZE ); return -1; }
	m_size = static_cast<IMF_UINT32>( Size );
#if defined( EXPANDABLE_SIZE_4BYTES )
	for( i=4; m_size >> ( i * 7 ); i++ );
#else
	for( i=1; m_size >> ( i * 7 ); i++ );
#endif
	return static_cast<IMF_INT64>( m_size ) + 1 + i;
}

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CBaseDescriptor::Read( CBaseStream& Stream )
{
	IMF_UINT8	object_id;

	try {
		// Read object_id.
		if ( !Stream.Read8( object_id ) ) throw E_READ_STREAM;
		if ( m_object_id == 0 ) m_object_id = object_id;				// Overload object_id.
		else if ( m_object_id != object_id ) throw E_DESCR_OBJECT_ID;	// object_id does not match.

		// Read size.
		if ( !ReadSize( Stream, m_size ) ) return false;		// Error code has been set in ReadSize().

		// Save the current position.
		if ( ( m_ReadPos = Stream.Tell() ) < 0 ) throw E_TELL_STREAM;
	}
	catch( IMF_UINT32 e ) {
		SetLastError( e );
		return false;
	}
	return true;
}

////////////////////////////////////////
//                                    //
//               Write                //
//                                    //
////////////////////////////////////////
// Stream = Output stream
// Return value = true:Success / false:Error
bool	CBaseDescriptor::Write( CBaseStream& Stream ) const
{
	return Stream.Write8( m_object_id ) && WriteSize( Stream, m_size );
}

////////////////////////////////////////
//                                    //
//          Check read size           //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = -1:Not reached to end / 0:Just reached to end / 1:Overrun or error
IMF_INT8	CBaseDescriptor::CheckReadSize( CBaseStream& Stream ) const
{
	IMF_INT64	CurPos, EndPos;

	// Get the current position.
	if ( ( CurPos = Stream.Tell() ) < 0 ) return 1;

	// Calculate end position.
	EndPos = m_ReadPos + m_size;
	if ( EndPos < 0 ) return 1;

	return ( CurPos == EndPos ) ? 0 : ( CurPos < EndPos ) ? -1 : 1;
}

////////////////////////////////////////
//                                    //
//        Read expandable size        //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Size = Variable to receive the size
// Return value = true:Success / false:Error
bool	CBaseDescriptor::ReadSize( CBaseStream& Stream, IMF_UINT32& Size )
{
	IMF_UINT8	Byte;
	
	Size = 0;
	do {
		if ( !Stream.Read8( Byte ) ) { SetLastError( E_READ_STREAM ); return false; }
		if ( Size & 0xfe000000 ) { SetLastError( E_DESCR_SIZE ); return false; }	// Overflow.
		Size = ( Size << 7 ) | ( Byte & 0x7f );
	} while( Byte & 0x80 );
	return true;
}

////////////////////////////////////////
//                                    //
//       Write expandable size        //
//                                    //
////////////////////////////////////////
// Stream = Output stream
// Size = Size to write
// Return value = true:Success / false:Error
bool	CBaseDescriptor::WriteSize( CBaseStream& Stream, IMF_UINT32 Size ) const
{
	bool		Writing = false;
	int			i;
	IMF_UINT8	Byte;

	for( i=4; i>=0; i-- ) {
		Byte = static_cast<IMF_UINT8>( Size >> ( 7 * i ) ) & 0x7f;
#if defined( EXPANDABLE_SIZE_4BYTES )
		if ( ( i == 3 ) || ( Byte != 0 ) ) Writing = true;
#else
		if ( ( i == 0 ) || ( Byte != 0 ) ) Writing = true;
#endif
		if ( Writing ) {
			if ( !Stream.Write8( Byte | ( ( i == 0 ) ? 0x00 : 0x80 ) ) ) return false;
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                     CObjectDescriptor class                      //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CObjectDescriptor::Read( CBaseStream& Stream )
{
	IMF_UINT16	Tmp;
	if ( !CBaseDescriptor::Read( Stream ) ) return false;
	try {
		if ( !Stream.Read16( Tmp ) ) throw E_READ_STREAM;
		m_ObjectDescriptorID = Tmp >> 6;
		m_URL_Flag = ( ( Tmp & 0x0020 ) != 0 );
		// [LIMITATION] URL_Flag must not be true.
		if ( m_URL_Flag ) throw E_DESCR_URL_FLAG;
		// [LIMITATION] esDescr, ociDescr, ipmpDescrPtr, ipmpDescr and extDescr must not exist.
		if ( CheckReadSize( Stream ) != 0 ) throw E_DESCR_SIZE;
	}
	catch( IMF_UINT32 e ) {
		SetLastError( e );
		return false;
	}
	return true;
}

////////////////////////////////////////
//                                    //
//               Write                //
//                                    //
////////////////////////////////////////
// Stream = Output stream
// Return value = true:Success / false:Error
bool	CObjectDescriptor::Write( CBaseStream& Stream ) const
{
	IMF_UINT16	Tmp;
	if ( !CBaseDescriptor::Write( Stream ) ) return false;
	Tmp = ( m_ObjectDescriptorID << 6 ) | ( m_URL_Flag ? 0x0020 : 0 ) | 0x001f;
	if ( !Stream.Write16( Tmp ) ) return false;
	// [LIMITATION] URL_Flag must not be true.
	if ( m_URL_Flag ) return false;
	// [LIMITATION] esDescr, ociDescr, ipmpDescrPtr, ipmpDescr and extDescr must not exist.
	return true;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                  CInitialObjectDescriptor class                  //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//            Constructor             //
//                                    //
////////////////////////////////////////
CInitialObjectDescriptor::CInitialObjectDescriptor( void ) : CBaseDescriptor( T_InitialObjectDescrTag )
{
	m_ObjectDescriptorID = 0;
	m_URL_Flag = false;
	m_includeInlineProfileLevelFlag = false;
	m_ODProfileLevelIndication = 0;
	m_sceneProfileLevelIndication = 0;
	m_audioProfileLevelIndication = 0;
	m_visualProfileLevelIndication = 0;
	m_graphicsProfileLevelIndication = 0;
}

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CInitialObjectDescriptor::Read( CBaseStream& Stream )
{
	IMF_UINT16	Tmp;
	if ( !CBaseDescriptor::Read( Stream ) ) return false;

	try {
		if ( !Stream.Read16( Tmp ) ) throw E_READ_STREAM;
		m_ObjectDescriptorID = Tmp >> 6;
		m_URL_Flag = ( ( Tmp & 0x0020 ) != 0 );
		m_includeInlineProfileLevelFlag = ( ( Tmp & 0x0010 ) != 0 );
		// [LIMITATION] URL_Flag must not be true.
		if ( m_URL_Flag ) throw E_DESCR_URL_FLAG;
		if ( !Stream.Read8( m_ODProfileLevelIndication ) || 
			 !Stream.Read8( m_sceneProfileLevelIndication ) || 
			 !Stream.Read8( m_audioProfileLevelIndication ) || 
			 !Stream.Read8( m_visualProfileLevelIndication ) || 
			 !Stream.Read8( m_graphicsProfileLevelIndication ) ) throw E_READ_STREAM;
		// [LIMITATION] esDescr, ociDescr, ipmpDescrPtr, ipmpDescr, toolListDescr and extDescr must not exist.
		if ( CheckReadSize( Stream ) != 0 ) throw E_DESCR_SIZE;
	}
	catch( IMF_UINT32 e ) {
		SetLastError( e );
		return false;
	}
	return true;
}

////////////////////////////////////////
//                                    //
//               Write                //
//                                    //
////////////////////////////////////////
// Stream = Output stream
// Return value = true:Success / false:Error
bool	CInitialObjectDescriptor::Write( CBaseStream& Stream ) const
{
	IMF_UINT16	Tmp;
	if ( !CBaseDescriptor::Write( Stream ) ) return false;
	Tmp = ( m_ObjectDescriptorID << 6 ) | ( m_URL_Flag ? 0x0020 : 0 ) | ( m_includeInlineProfileLevelFlag ? 0x0010 : 0 ) | 0x000f;
	if ( !Stream.Write16( Tmp ) ) return false;
	// [LIMITATION] URL_Flag must not be true.
	if ( m_URL_Flag ) return false;
	if ( !Stream.Write8( m_ODProfileLevelIndication ) || 
		 !Stream.Write8( m_sceneProfileLevelIndication ) || 
		 !Stream.Write8( m_audioProfileLevelIndication ) || 
		 !Stream.Write8( m_visualProfileLevelIndication ) || 
		 !Stream.Write8( m_graphicsProfileLevelIndication ) ) return false;
	// [LIMITATION] esDescr, ociDescr, ipmpDescrPtr, ipmpDescr, toolListDescr and extDescr must not exist.
	return true;
}

// End of ImfDescriptor.cpp
