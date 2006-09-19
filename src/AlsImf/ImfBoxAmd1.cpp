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

Filename : ImfBoxAmd1.cpp
Project  : MPEG-A Audio Archival Multimedia Application Format
Author   : Koichi Sugiura (NTT Advanced Technology Corporation)
           Noboru Harada  (NTT)
Date     : August 31st, 2006
Contents : Box classes defined in ISO/IEC 14496-12 Amendment 1

*******************************************************************/

#include	"ImfBoxAmd1.h"

using namespace std;
using namespace NAlsImf;

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                       CBoxReaderAmd1 class                       //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//            Create a box            //
//                                    //
////////////////////////////////////////
// Type = Box type
// Return value = Pointer to newly created box object
CBox*	CBoxReaderAmd1::CreateBox( IMF_UINT32 Type )
{
	switch( Type ) {
	case	IMF_FOURCC_META:	return new CMetaBox();
	case	IMF_FOURCC_XML:		return new CXMLBox();
	case	IMF_FOURCC_BXML:	return new CBinaryXMLBox();
	case	IMF_FOURCC_ILOC:	return new CItemLocationBox();
	case	IMF_FOURCC_INFE:	return new CItemInfoEntry();
	case	IMF_FOURCC_IINF:	return new CItemInfoBox();
	}
	return CBoxReader::CreateBox( Type );
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                       CXMLBox class (xml )                       //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CXMLBox::Read( CBaseStream& Stream )
{
	if ( !CFullBox::Read( Stream ) ) return false;
	if ( !ReadString( Stream, m_xml, GetDataSize() ) ) return false;
	if ( CheckReadSize( Stream ) != 0 ) { SetLastError( E_BOX_SIZE ); return false; }
	return true;
}

////////////////////////////////////////
//                                    //
//               Write                //
//                                    //
////////////////////////////////////////
// Stream = Output stream
// Return value = true:Success / false:Error
bool	CXMLBox::Write( CBaseStream& Stream ) const
{
	IMF_UINT32	XmlLen;
	if ( !CFullBox::Write( Stream ) ) return false;
	XmlLen = static_cast<IMF_UINT32>( m_xml.length() ) + 1;
	if ( Stream.Write( m_xml.c_str(), XmlLen ) != XmlLen ) return false;
	return true;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                    CBinaryXMLBox class (bxml)                    //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CBinaryXMLBox::Read( CBaseStream& Stream )
{
	IMF_INT64	DataSize;

	// Read basic fields.
	if ( !CFullBox::Read( Stream ) ) return false;

	// [LIMITATION] Data size must be less than 2GB.
	DataSize = GetDataSize();
	if ( ( DataSize < 0 ) || ( ( DataSize >> 32 ) != 0 ) ) { SetLastError( E_BOX_SIZE ); return false; }

	// Free data.
	if ( m_data ) { delete[] m_data; m_data = NULL; }

	if ( DataSize > 0 ) {
		// Allocate buffer.
		m_data = new IMF_UINT8 [ static_cast<IMF_UINT32>( DataSize ) ];
		if ( m_data == NULL ) { SetLastError( E_MEMORY ); return false; }	// Memory error.
		// Read data.
		if ( Stream.Read( m_data, static_cast<IMF_UINT32>( DataSize ) ) != static_cast<IMF_UINT32>( DataSize ) ) { SetLastError( E_READ_STREAM ); return false; }
	}
	if ( CheckReadSize( Stream ) != 0 ) { SetLastError( E_BOX_SIZE ); return false; }
	return true;
}

////////////////////////////////////////
//                                    //
//               Write                //
//                                    //
////////////////////////////////////////
// Stream = Output stream
// Return value = true:Success / false:Error
bool	CBinaryXMLBox::Write( CBaseStream& Stream ) const
{
	IMF_INT64	DataSize;

	// [LIMITATION] Data size must be less than 2GB.
	DataSize = GetDataSize();
	if ( ( DataSize < 0 ) || ( ( DataSize >> 32 ) != 0 ) ) return false;

	// Write basic fields.
	if ( !CFullBox::Write( Stream ) ) return false;

	if ( DataSize > 0 ) {
		if ( m_data == NULL ) return false;
		// Write data.
		if ( Stream.Write( m_data, static_cast<IMF_UINT32>( DataSize ) ) != static_cast<IMF_UINT32>( DataSize ) ) return false;
	}
	return true;
}

////////////////////////////////////////
//                                    //
//        Set binary XML data         //
//                                    //
////////////////////////////////////////
// pData = Pointer to XML data
// Size = XML data size in bytes
// Return value = true:Success / false:Error
bool	CBinaryXMLBox::SetData( const void* pData, IMF_UINT64 Size )
{
	if ( m_data ) { delete[] m_data; m_data = NULL; }
	// [LIMITATION] XML data must be less than 4GB.
	if ( Size >> 32 ) { SetLastError( E_BXML_SIZE ); return false; }
	m_data = new IMF_UINT8 [ static_cast<IMF_UINT32>( Size ) ];
	if ( m_data == NULL ) { SetLastError( E_MEMORY ); return false; }
	memcpy( m_data, pData, static_cast<IMF_UINT32>( Size ) );
	return ( SetDataSize( Size ) > 0 );
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                  CItemLocationBox class (iloc)                   //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//     Read variable length value     //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Size = Value size (0 or 4 or 8)
// Value = Variable to receive the read value
// Return value = true:Success / false:Error
bool	CItemLocationBox::ReadValue( CBaseStream& Stream, IMF_UINT8 Size, IMF_UINT64& Value )
{
	IMF_UINT32	Value32;
	switch( Size ) {
	case	0:
		Value = 0;
		return true;
	case	4:
		if ( !Stream.Read32( Value32 ) ) { SetLastError( E_READ_STREAM ); return false; }
		Value = static_cast<IMF_UINT64>( Value32 );
		return true;
	case	8:
		if ( !Stream.Read64( Value ) ) { SetLastError( E_READ_STREAM ); return false; }
		return true;
	}
	SetLastError( E_ILOC_SIZE );
	return false;
}

////////////////////////////////////////
//                                    //
//    Write variable length value     //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Size = Value size (0 or 4 or 8)
// Value = Value to write
// Return value = true:Success / false:Error
bool	CItemLocationBox::WriteValue( CBaseStream& Stream, IMF_UINT8 Size, IMF_UINT64 Value ) const
{
	switch( Size ) {
	case	0:	return true;
	case	4:	return Stream.Write32( static_cast<IMF_UINT32>( Value ) );
	case	8:	return Stream.Write64( Value );
	}
	return false;
}

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CItemLocationBox::Read( CBaseStream& Stream )
{
	IMF_UINT16	Tmp, i, j, ItemCount, ExtentCount;
	ILOC_EXTENT	Extent;
	CItem		Item;

	m_items.clear();

	if ( !CFullBox::Read( Stream ) ) return false;

	try {
		if ( !Stream.Read16( Tmp ) ) throw E_READ_STREAM;
		m_offset_size = static_cast<IMF_UINT8>( Tmp >> 12 ) & 0xf;
		m_length_size = static_cast<IMF_UINT8>( Tmp >> 8 ) & 0xf;
		m_base_offset_size = static_cast<IMF_UINT8>( Tmp >> 4 ) & 0xf;
		if ( ( m_offset_size != 0 ) && ( m_offset_size != 4 ) && ( m_offset_size != 8 ) ) throw E_ILOC_OFFSET_SIZE;
		if ( ( m_length_size != 0 ) && ( m_length_size != 4 ) && ( m_length_size != 8 ) ) throw E_ILOC_LENGTH_SIZE;
		if ( ( m_base_offset_size != 0 ) && ( m_base_offset_size != 4 ) && ( m_base_offset_size != 8 ) ) throw E_ILOC_BASE_OFFSET_SIZE;
		if ( !Stream.Read16( ItemCount ) ) throw E_READ_STREAM;

		// Item loop.
		for( i=0; i<ItemCount; i++ ) {
			if ( !Stream.Read16( Item.m_item_ID ) || !Stream.Read16( Item.m_data_reference_index ) ) throw E_READ_STREAM;
			if ( !ReadValue( Stream, m_base_offset_size, Item.m_base_offset ) ) return false;
			if ( !Stream.Read16( ExtentCount ) ) throw E_READ_STREAM;
			if ( ExtentCount < 1 ) throw E_ILOC_NO_EXTENT;
			Item.m_extent_data.clear();
			for( j=0; j<ExtentCount; j++ ) {
				if ( !ReadValue( Stream, m_offset_size, Extent.m_extent_offset ) || 
					 !ReadValue( Stream, m_length_size, Extent.m_extent_length ) ) return false;
				Item.m_extent_data.push_back( Extent );
			}
			m_items.push_back( Item );
		}
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
bool	CItemLocationBox::Write( CBaseStream& Stream ) const
{
	IMF_UINT16	Tmp;
	vector<CItem>::const_iterator		iItem;
	vector<ILOC_EXTENT>::const_iterator	iExtent;

	if ( !CFullBox::Write( Stream ) ) return false;
	Tmp = ( ( static_cast<IMF_UINT16>( m_offset_size ) << 12 ) & 0xf000 ) |
		  ( ( static_cast<IMF_UINT16>( m_length_size ) <<  8 ) & 0x0f00 ) |
		  ( ( static_cast<IMF_UINT16>( m_base_offset_size ) << 4 ) & 0x00f0 );
	if ( !Stream.Write16( Tmp ) ) return false;
	// Write items.
	if ( m_items.size() > 0xffff ) return false;
	if ( !Stream.Write16( static_cast<IMF_UINT16>( m_items.size() ) ) ) return false;
	for( iItem=m_items.begin(); iItem!=m_items.end(); iItem++ ) {
		if ( !Stream.Write16( iItem->m_item_ID ) || 
			 !Stream.Write16( iItem->m_data_reference_index ) || 
			 !WriteValue( Stream, m_base_offset_size, iItem->m_base_offset ) ) return false;
		// Write extent data.
		if ( iItem->m_extent_data.size() > 0xffff ) return false;
		if ( !Stream.Write16( static_cast<IMF_UINT16>( iItem->m_extent_data.size() ) ) ) return false;
		for( iExtent=iItem->m_extent_data.begin(); iExtent!=iItem->m_extent_data.end(); iExtent++ ) {
			if ( !WriteValue( Stream, m_offset_size, iExtent->m_extent_offset ) || 
				 !WriteValue( Stream, m_length_size, iExtent->m_extent_length ) ) return false;
		}
	}
	return true;
}

////////////////////////////////////////
//                                    //
//         Calculate box size         //
//                                    //
////////////////////////////////////////
// Return value = Whole box size in bytes (-1 means error)
IMF_INT64	CItemLocationBox::CalcSize( void )
{
	IMF_INT64	Size = 4;
	vector<CItem>::const_iterator	iItem;

	for( iItem=m_items.begin(); iItem!=m_items.end(); iItem++ ) {
		Size += 6 + m_base_offset_size + iItem->m_extent_data.size() * ( m_offset_size + m_length_size );
		if ( Size < 0 ) { SetLastError( E_BOX_SET_SIZE ); return -1; }
	}
	return SetDataSize( Size );
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                   CItemInfoEntry class (infe)                    //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CItemInfoEntry::Read( CBaseStream& Stream )
{
	IMF_INT64	MaxLen;
	if ( !CFullBox::Read( Stream ) ) return false;
	if ( !Stream.Read16( m_item_ID ) || !Stream.Read16( m_item_protection_index ) ) { SetLastError( E_READ_STREAM ); return false; }
	MaxLen = GetDataSize() - 4;
	if ( !ReadString( Stream, m_item_name, MaxLen ) ) return false;
	MaxLen -= m_item_name.length() + 1;
	if ( !ReadString( Stream, m_content_type, MaxLen ) ) return false;
	MaxLen -= m_content_type.length() + 1;
	if ( MaxLen > 0 ) {
		if ( !ReadString( Stream, m_content_encoding, MaxLen ) ) return false;
	}
	if ( CheckReadSize( Stream ) != 0 ) { SetLastError( E_BOX_SIZE ); return false; }
	return true;
}

////////////////////////////////////////
//                                    //
//               Write                //
//                                    //
////////////////////////////////////////
// Stream = Output stream
// Return value = true:Success / false:Error
bool	CItemInfoEntry::Write( CBaseStream& Stream ) const
{
	IMF_UINT32	StrLen;
	if ( !CFullBox::Write( Stream ) ) return false;
	if ( !Stream.Write16( m_item_ID ) || !Stream.Write16( m_item_protection_index ) ) return false;
	StrLen = static_cast<IMF_UINT32>( m_item_name.length() ) + 1;
	if ( Stream.Write( m_item_name.c_str(), StrLen ) != StrLen ) return false;
	StrLen = static_cast<IMF_UINT32>( m_content_type.length() ) + 1;
	if ( Stream.Write( m_content_type.c_str(), StrLen ) != StrLen ) return false;
	if ( !m_content_encoding.empty() ) {
		StrLen = static_cast<IMF_UINT32>( m_content_encoding.length() ) + 1;
		if ( Stream.Write( m_content_encoding.c_str(), StrLen ) != StrLen ) return false;
	}
	return true;
}

////////////////////////////////////////
//                                    //
//         Calculate box size         //
//                                    //
////////////////////////////////////////
// Return value = Whole box size in bytes (-1 means error)
IMF_INT64	CItemInfoEntry::CalcSize( void )
{
	IMF_INT64	Size = 4 + m_item_name.length() + 1 + m_content_type.length() + 1;
	if ( !m_content_encoding.empty() ) Size += m_content_encoding.length() + 1;
	return SetDataSize( Size );
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                    CItemInfoBox class (iinf)                     //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CItemInfoBox::Read( CBaseStream& Stream )
{
	CBox*		pChild = NULL;
	IMF_UINT16	EntryCount, i;

	if ( !CFullBox::Read( Stream ) ) return false;

	try {
		m_Boxes.Clear();
		if ( !Stream.Read16( EntryCount ) ) throw E_READ_STREAM;
		for( i=0; i<EntryCount; i++ ) {
			pChild = m_pReader->Read( Stream, this );
			if ( pChild == NULL ) throw m_pReader->GetLastError();
			m_Boxes.push_back( pChild );
		}
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
bool	CItemInfoBox::Write( CBaseStream& Stream ) const
{
	CBoxVector::const_iterator	i;

	if ( !CFullBox::Write( Stream ) ) return false;
	if ( m_Boxes.size() > 0xffff ) return false;
	if ( !Stream.Write16( static_cast<IMF_UINT16>( m_Boxes.size() ) ) ) return false;
	for( i=m_Boxes.begin(); i!=m_Boxes.end(); i++ ) if ( !(*i)->Write( Stream ) ) return false;
	return true;
}

////////////////////////////////////////
//                                    //
//         Calculate box size         //
//                                    //
////////////////////////////////////////
// Return value = Whole box size in bytes (-1 means error)
IMF_INT64	CItemInfoBox::CalcSize( void )
{
	IMF_INT64	BoxSize;
	IMF_INT64	WholeSize = 2;
	CBoxVector::iterator	i;

	if ( m_Boxes.size() > 0xffff ) { SetLastError( E_IINF_ENTRY_COUNT ); return -1; }
	for( i=m_Boxes.begin(); i!=m_Boxes.end(); i++ ) {
		BoxSize = (*i)->CalcSize();
		if ( BoxSize < 0 ) { SetLastError( E_BOX_SIZE ); return -1; }
		WholeSize += BoxSize;
		if ( WholeSize < 0 ) { SetLastError( E_BOX_SIZE ); return -1; }
	}
	return SetDataSize( WholeSize );
}

// End of ImfBoxAmd1.cpp
