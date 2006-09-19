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

Filename : ImfSampleEntry.cpp
Project  : MPEG-A Audio Archival Multimedia Application Format
Author   : Koichi Sugiura (NTT Advanced Technology Corporation)
           Noboru Harada  (NTT)
Date     : August 31st, 2006
Contents : Sample entry classes defined in ISO/IEC 14496-12

*******************************************************************/

#include	"ImfSampleEntry.h"

using namespace NAlsImf;

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                        CSampleEntry class                        //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CSampleEntry::Read( CBaseStream& Stream )
{
	IMF_UINT8	Dummy[6];
	if ( !CBox::Read( Stream ) ) return false;
	if ( ( Stream.Read( Dummy, sizeof(Dummy) ) != sizeof(Dummy) ) || !Stream.Read16( m_data_reference_index ) ) { SetLastError( E_READ_STREAM ); return false; }
	return true;
}

////////////////////////////////////////
//                                    //
//               Write                //
//                                    //
////////////////////////////////////////
// Stream = Output stream
// Return value = true:Success / false:Error
bool	CSampleEntry::Write( CBaseStream& Stream ) const
{
	return CBox::Write( Stream ) && Stream.Write32( 0 ) && Stream.Write16( 0 ) && Stream.Write16( m_data_reference_index );
}

////////////////////////////////////////
//                                    //
//        Read extension data         //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CSampleEntry::ReadExtension( CBaseStream& Stream )
{
	IMF_INT8	ReadStatus;
	CBox*		pExtBox = NULL;

	// Clear extension boxes.
	m_Boxes.Clear();

	// Read extension boxes.
	while( ( ReadStatus = CheckReadSize( Stream ) ) < 0 ) {
		pExtBox = m_pReader->Read( Stream, this );
		if ( pExtBox == NULL ) { SetLastError( m_pReader->GetLastError() ); return false; }
		m_Boxes.push_back( pExtBox );
	}
	if ( ReadStatus != 0 ) { SetLastError( E_BOX_SIZE ); return false; }
	return true;
}

////////////////////////////////////////
//                                    //
//        Write extension data        //
//                                    //
////////////////////////////////////////
// Stream = Output stream
// Return value = true:Success / false:Error
bool	CSampleEntry::WriteExtension( CBaseStream& Stream ) const
{
	CBoxVector::const_iterator	i;
	for( i=m_Boxes.begin(); i!=m_Boxes.end(); i++ ) if ( !(*i)->Write( Stream ) ) return false;
	return true;
}

////////////////////////////////////////
//                                    //
//           Set data size            //
//                                    //
////////////////////////////////////////
// Size = Data size in bytes
// Return value = Whole box size in bytes (-1 means error)
IMF_INT64	CSampleEntry::SetDataSize( IMF_INT64 Size )
{
	if ( Size < 0 ) { SetLastError( E_BOX_SET_SIZE ); return -1; }
	return CBox::SetDataSize( Size + 8 );
}

////////////////////////////////////////
//                                    //
//   Calculate extension data size    //
//                                    //
////////////////////////////////////////
// Return value = Total extension data size in bytes (-1 means error)
IMF_INT64	CSampleEntry::CalcExtensionSize( void )
{
	IMF_INT64	BoxSize;
	IMF_INT64	WholeSize = 0;
	CBoxVector::iterator	i;

	for( i=m_Boxes.begin(); i!=m_Boxes.end(); i++ ) {
		BoxSize = (*i)->CalcSize();
		if ( BoxSize < 0 ) { SetLastError( E_BOX_SET_SIZE ); return -1; }
		WholeSize += BoxSize;
		if ( WholeSize < 0 ) { SetLastError( E_BOX_SET_SIZE ); return -1; }
	}
	return WholeSize;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                      CHintSampleEntry class                      //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CHintSampleEntry::Read( CBaseStream& Stream )
{
	IMF_INT64	DataSize;

	if ( m_data ) { delete[] m_data; m_data = NULL; }

	if ( !CSampleEntry::Read( Stream ) ) return false;

	try {
		DataSize = GetDataSize();
		// [LIMITATION] DataSize must be less than 2GB.
		if ( ( DataSize < 0 ) || ( DataSize >> 32 ) ) throw E_BOX_SIZE;

		// Allocate buffer.
		m_data = new IMF_UINT8 [ static_cast<IMF_UINT32>( DataSize ) ];
		if ( m_data == NULL ) throw E_MEMORY;
		if ( Stream.Read( m_data, static_cast<IMF_UINT32>( DataSize ) ) != static_cast<IMF_UINT32>( DataSize ) ) throw E_READ_STREAM;
		if ( CheckReadSize( Stream ) != 0 ) throw E_BOX_SIZE;
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
bool	CHintSampleEntry::Write( CBaseStream& Stream ) const
{
	IMF_INT64	DataSize;

	if ( !CSampleEntry::Write( Stream ) ) return false;
	DataSize = GetDataSize();
	// [LIMITATION] DataSize must be less than 2GB.
	if ( ( DataSize < 0 ) || ( DataSize >> 32 ) ) return false;
	if ( Stream.Write( m_data, static_cast<IMF_UINT32>( DataSize ) ) != static_cast<IMF_UINT32>( DataSize ) ) return false;
	return true;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                     CVisualSampleEntry class                     //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//            Constructor             //
//                                    //
////////////////////////////////////////
CVisualSampleEntry::CVisualSampleEntry( IMF_UINT32 type, const IMF_UINT8* usertype ) : CSampleEntry( type, usertype )
{
	m_width = m_height = 0;
	m_horizresolution = m_vertresolution = 0x00480000;	// 72 dpi
	m_frame_count = 1;
	memset( m_compressorname, 0, sizeof(m_compressorname) );
	m_depth = 0x0018;
}

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CVisualSampleEntry::Read( CBaseStream& Stream )
{
	IMF_UINT16	Dummy16;
	IMF_UINT32	Dummy32;

	if ( !CSampleEntry::Read( Stream ) ) return false;

	try {
		if ( !Stream.Read16( Dummy16 ) || !Stream.Read16( Dummy16 ) ) throw E_READ_STREAM;
		for( int i=0; i<3; i++ ) if ( !Stream.Read32( Dummy32 ) ) throw E_READ_STREAM;
		if ( !Stream.Read16( m_width ) || !Stream.Read16( m_height ) || 
			 !Stream.Read32( m_horizresolution ) || !Stream.Read32( m_vertresolution ) || 
			 !Stream.Read32( Dummy32 ) || !Stream.Read16( m_frame_count ) || 
			 ( Stream.Read( m_compressorname, sizeof(m_compressorname) ) != sizeof(m_compressorname) ) || 
			 !Stream.Read16( m_depth ) || !Stream.Read16( Dummy16 ) ) throw E_READ_STREAM;
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
bool	CVisualSampleEntry::Write( CBaseStream& Stream ) const
{
	if ( !CSampleEntry::Write( Stream ) ) return false;

	if ( !Stream.Write16( 0 ) || !Stream.Write16( 0 ) ) return false;
	for( int i=0; i<3; i++ ) if ( !Stream.Write32( 0 ) ) return false;
	if ( !Stream.Write16( m_width ) || !Stream.Write16( m_height ) || 
		 !Stream.Write32( m_horizresolution ) || !Stream.Write32( m_vertresolution ) || 
		 !Stream.Write32( 0 ) || !Stream.Write16( m_frame_count ) || 
		 ( Stream.Write( m_compressorname, sizeof(m_compressorname) ) != sizeof(m_compressorname) ) || 
		 !Stream.Write16( m_depth ) || !Stream.Write16( 0xffff ) ) return false;
	return true;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                     CAudioSampleEntry class                      //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CAudioSampleEntry::Read( CBaseStream& Stream )
{
	IMF_UINT16	Dummy16;
	IMF_UINT32	Dummy32;

	if ( !CSampleEntry::Read( Stream ) ) return false;

	if ( !Stream.Read32( Dummy32 ) || !Stream.Read32( Dummy32 ) || 
		 !Stream.Read16( m_channelcount ) || !Stream.Read16( m_samplesize ) || 
		 !Stream.Read16( Dummy16 ) || !Stream.Read16( Dummy16 ) || 
		 !Stream.Read32( m_samplerate ) ) { SetLastError( E_READ_STREAM ); return false; }
	return true;
}

////////////////////////////////////////
//                                    //
//               Write                //
//                                    //
////////////////////////////////////////
// Stream = Output stream
// Return value = true:Success / false:Error
bool	CAudioSampleEntry::Write( CBaseStream& Stream ) const
{
	if ( !CSampleEntry::Write( Stream ) ) return false;

	if ( !Stream.Write32( 0 ) || !Stream.Write32( 0 ) || 
		 !Stream.Write16( m_channelcount ) || !Stream.Write16( m_samplesize ) || 
		 !Stream.Write16( 0 ) || !Stream.Write16( 0 ) || 
		 !Stream.Write32( m_samplerate ) ) return false;
	return true;
}

// End of ImfSampleEntry.cpp
