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

Filename : ImfBox.cpp
Project  : MPEG-4 Audio Lossless Coding
Author   : Koichi Sugiura (NTT Advanced Technology Corporation)
           Noboru Harada  (NTT)
Date     : August 31st, 2006
Contents : Box classes defined in ISO/IEC 14496-12

*******************************************************************/

/************************** Modifications *************************
 *
 * 2007/05/10 by Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - integrated ImfBoxAmd1.cpp.
 *   - supported pitm box.
 *   - updated infe box.
 *
 * 2007/05/23, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - added Print() to every class.
 *   - changed indent value type in Out() and Dump().
 *
 * 2008/10/20, Koichi Sugiura <koichi.sugiura@ntt-at.co.jp>
 *   - added CContainerFullBox class.
 *
 ******************************************************************/

#include	<iostream>
#include	<iomanip>
#include	<vector>
#include	"ImfBox.h"
#include	"ImfFileStream.h"
#include	"ImfSampleEntry.h"

using namespace std;
using namespace NAlsImf;

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                         CBoxReader class                         //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//       Peek box type and size       //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Type = Variable to receive box type
// Size = Variable to receive box size
// Return value = true:Success / false:Error
bool	CBoxReader::Peek( CBaseStream& Stream, IMF_UINT32& Type, IMF_INT64& Size )
{
	bool		Result = false;
	IMF_INT64	Offset;
	IMF_UINT32	Size32;

	SetLastError( E_NONE );

	// Save the current position.
	if ( ( Offset = Stream.Tell() ) < 0 ) { SetLastError( E_TELL_STREAM ); return false; }

	try {
		// Read size and type.
		if ( !Stream.Read32( Size32 ) || !Stream.Read32( Type ) ) throw E_READ_STREAM;
		if ( Size32 == 1 ) {
			if ( !Stream.Read64( Size ) ) throw E_READ_STREAM;
		} else {
			Size = Size32;
		}
		Result = true;
	}
	catch( IMF_UINT32 e ) {
		SetLastError( e );
	}

	// Restore the stream position.
	if ( !Stream.Seek( Offset, CBaseStream::S_BEGIN ) ) { SetLastError( E_SEEK_STREAM ); return false; }
	return Result;
}

////////////////////////////////////////
//                                    //
//             Skip a box             //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CBoxReader::Skip( CBaseStream& Stream )
{
	IMF_UINT32	Type;
	IMF_INT64	Size;

	SetLastError( E_NONE );

	// Get box info.
	if ( !Peek( Stream, Type, Size ) ) return false;		// Error code has been set in Peek().

	// Skip this box.
	if ( Size == 0 ) {
		if ( !Stream.Seek( 0, CBaseStream::S_END ) ) { SetLastError( E_SEEK_STREAM ); return false; }
	} else {
		if ( !Stream.Seek( Size, CBaseStream::S_CURRENT ) ) { SetLastError( E_SEEK_STREAM ); return false; }
	}
	return true;
}

////////////////////////////////////////
//                                    //
//             Read a box             //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// pParent = Pointer to parent object
// Return value = Created CBox object
CBox*	CBoxReader::Read( CBaseStream& Stream, CBox* pParent )
{
	IMF_UINT32	Size, BoxType;
	CBox*		pBox = NULL;

	SetLastError( E_NONE );

	try {
		// Read size and boxtype.
		if ( !Stream.Read32( Size ) || !Stream.Read32( BoxType ) ) throw E_READ_STREAM;

		// Restore the file position.
		if ( !Stream.Seek( -8, CBaseStream::S_CURRENT ) ) throw E_SEEK_STREAM;

		// Create CBox object.
		pBox = CreateBox( BoxType );
		if ( pBox == NULL ) {
			SetLastError( W_BOX_UNKNOWN );
			pBox = new CUnknownBox();
			if ( pBox == NULL ) throw E_MEMORY;	// Memory error.
		}
		pBox->m_pParent = pParent;
		pBox->m_pReader = this;

		// Read box contents.
		if ( !pBox->Read( Stream ) ) throw pBox->GetLastError();
	}
	catch( IMF_UINT32 e ) {
		SetLastError( e );
		if ( pBox ) {
			delete pBox;
			pBox = NULL;
		}
	}
	return pBox;
}

////////////////////////////////////////
//                                    //
//            Create a box            //
//                                    //
////////////////////////////////////////
// Type = Box type
// Return value = Pointer to newly created box object
CBox*	CBoxReader::CreateBox( IMF_UINT32 Type )
{
	CBox*	p = NULL;
	switch( Type ) {
	case	IMF_FOURCC_FTYP:	p = new CFileTypeBox();								break;
	case	IMF_FOURCC_MOOV:	p = new CMovieBox();								break;
	case	IMF_FOURCC_MDAT:	p = new CMediaDataBox();							break;
	case	IMF_FOURCC_MVHD:	p = new CMovieHeaderBox();							break;
	case	IMF_FOURCC_TRAK:	p = new CTrackBox();								break;
	case	IMF_FOURCC_TKHD:	p = new CTrackHeaderBox();							break;
	case	IMF_FOURCC_TREF:	p = new CTrackReferenceBox();						break;
	case	IMF_FOURCC_HINT:	p = new CTrackReferenceTypeBox( IMF_FOURCC_HINT );	break;
	case	IMF_FOURCC_CDSC:	p = new CTrackReferenceTypeBox( IMF_FOURCC_CDSC );	break;
	case	IMF_FOURCC_MDIA:	p = new CMediaBox();								break;
	case	IMF_FOURCC_MDHD:	p = new CMediaHeaderBox();							break;
	case	IMF_FOURCC_HDLR:	p = new CHandlerBox();								break;
	case	IMF_FOURCC_MINF:	p = new CMediaInformationBox();						break;
	case	IMF_FOURCC_VMHD:	p = new CVideoMediaHeaderBox();						break;
	case	IMF_FOURCC_SMHD:	p = new CSoundMediaHeaderBox();						break;
	case	IMF_FOURCC_HMHD:	p = new CHintMediaHeaderBox();						break;
	case	IMF_FOURCC_NMHD:	p = new CNullMediaHeaderBox();						break;
	case	IMF_FOURCC_DINF:	p = new CDataInformationBox();						break;
	case	IMF_FOURCC_URL:		p = new CDataEntryUrlBox();							break;
	case	IMF_FOURCC_URN:		p = new CDataEntryUrnBox();							break;
	case	IMF_FOURCC_DREF:	p = new CDataReferenceBox();						break;
	case	IMF_FOURCC_STBL:	p = new CSampleTableBox();							break;
	case	IMF_FOURCC_STTS:	p = new CTimeToSampleBox();							break;
	case	IMF_FOURCC_CTTS:	p = new CCompositionOffsetBox();					break;
	case	IMF_FOURCC_STSD:	p = new CSampleDescriptionBox();					break;
	case	IMF_FOURCC_STSZ:	p = new CSampleSizeBox();							break;
	case	IMF_FOURCC_STZ2:	p = new CCompactSampleSizeBox();					break;
	case	IMF_FOURCC_STSC:	p = new CSampleToChunkBox();						break;
	case	IMF_FOURCC_STCO:	p = new CChunkOffsetBox();							break;
	case	IMF_FOURCC_CO64:	p = new CChunkLargeOffsetBox();						break;
//	case	IMF_FOURCC_STSS:	p = new CSyncSampleBox();							break;
//	case	IMF_FOURCC_STSH:	p = new CShadowSyncSampleBox();						break;
//	case	IMF_FOURCC_STDP:	p = new CDegradationPriorityBox();					break;
//	case	IMF_FOURCC_PADB:	p = new CPaddingBitsBox();							break;
	case	IMF_FOURCC_FREE:	p = new CFreeSpaceBox( IMF_FOURCC_FREE );			break;
	case	IMF_FOURCC_SKIP:	p = new CFreeSpaceBox( IMF_FOURCC_SKIP );			break;
	case	IMF_FOURCC_EDTS:	p = new CEditBox();									break;
//	case	IMF_FOURCC_ELST:	p = new CEditListBox();								break;
	case	IMF_FOURCC_UDTA:	p = new CUserDataBox();								break;
//	case	IMF_FOURCC_CPRT:	p = new CCopyrightBox();							break;
	case	IMF_FOURCC_MVEX:	p = new CMovieExtendsBox();							break;
//	case	IMF_FOURCC_MEHD:	p = new CMovieExtendsHeaderBox();					break;
//	case	IMF_FOURCC_TREX:	p = new CTrackExtendsBox();							break;
	case	IMF_FOURCC_MOOF:	p = new CMovieFragmentBox();						break;
//	case	IMF_FOURCC_MFHD:	p = new CMovieFragmentHeaderBox();					break;
	case	IMF_FOURCC_TRAF:	p = new CTrackFragmentBox();						break;
//	case	IMF_FOURCC_TFHD:	p = new CTrackFragmentHeaderBox();					break;
//	case	IMF_FOURCC_TRUN:	p = new CTrackRunBox();								break;
	case	IMF_FOURCC_MFRA:	p = new CMovieFragmentRandomAccessBox();			break;
//	case	IMF_FOURCC_TFRA:	p = new CTrackFragmentRandomAccessBox();			break;
//	case	IMF_FOURCC_MFRO:	p = new CMovieFragmentRandomAccessOffsetBox();		break;
//	case	IMF_FOURCC_PDIN:	p = new CProgressiveDownloadInfoBox();				break;
//	case	IMF_FOURCC_SDTP:	p = new CSampleDependencyTypeBox();					break;
//	case	IMF_FOURCC_SBGP:	p = new CSampleToGroupBox();						break;
//	case	IMF_FOURCC_SGPD:	p = new CSampleGroupDescriptionBox();				break;
//	case	IMF_FOURCC_SUBS:	p = new CSubSampleInformationBox();					break;
//	case	IMF_FOURCC_IPMC:	p = new CIPMPControlBox();							break;
	case	IMF_FOURCC_META:	p = new CMetaBox();									break;
	case	IMF_FOURCC_ILOC:	p = new CItemLocationBox();							break;
//	case	IMF_FOURCC_IPRO:	p = new CItemProtectionBox();						break;
//	case	IMF_FOURCC_SINF:	p = new CProtectionSchemeInfoBox();					break;
//	case	IMF_FOURCC_FRMA:	p = new COriginalFormatBox();						break;
//	case	IMF_FOURCC_IMIF:	p = new CIPMPInfoBox();								break;
//	case	IMF_FOURCC_SCHM:	p = new CSchemeTypeBox();							break;
//	case	IMF_FOURCC_SCHI:	p = new CSchemeInformationBox();					break;
	case	IMF_FOURCC_IINF:	p = new CItemInfoBox();								break;
	case	IMF_FOURCC_INFE:	p = new CItemInfoEntry( 0 );						break;
	case	IMF_FOURCC_XML:		p = new CXMLBox();									break;
	case	IMF_FOURCC_BXML:	p = new CBinaryXMLBox();							break;
	case	IMF_FOURCC_PITM:	p = new CPrimaryItemBox();							break;
//	case	IMF_FOURCC_MECO:	p = new CAdditionalMetadataContainerBox();			break;
//	case	IMF_FOURCC_MERE:	p = new CMetaboxRelationBox();						break;
//	case	IMF_FOURCC_FIIN:	p = new CFDItemInformationBox();					break;
//	case	IMF_FOURCC_PAEN:	p = new CPartitionEntry();							break;
//	case	IMF_FOURCC_FPAR:	p = new CFilePartition();							break;
//	case	IMF_FOURCC_FECR:	p = new CFECReservoirBox();							break;
//	case	IMF_FOURCC_SEGR:	p = new CFDSessionGroupBox();						break;
//	case	IMF_FOURCC_GITN:	p = new CGroupIdToNameBox();						break;
//	case	IMF_FOURCC_TSEL:	p = new CTrackSelectionBox();						break;
	}
	return p;
}

////////////////////////////////////////
//                                    //
//     Create a sample entry box      //
//                                    //
////////////////////////////////////////
// Type = Sample entry type
// Return value = Pointer to newly created box object
CSampleEntry*	CBoxReader::CreateSampleEntry( IMF_UINT32 Type )
{
	CSampleEntry*	p = NULL;
	switch( Type ) {
	case	IMF_FOURCC_SOUN:	p = new CAudioSampleEntry( 0 );		break;
	case	IMF_FOURCC_VIDE:	p = new CVisualSampleEntry( 0 );	break;
	case	IMF_FOURCC_HINT:	p = new CHintSampleEntry( 0 );		break;
	}
	return p;
}

////////////////////////////////////////
//                                    //
//        Create a descriptor         //
//                                    //
////////////////////////////////////////
// Tag = Descriptor tag
// Return value = Pointer to newly created descriptor object
CBaseDescriptor*	CBoxReader::CreateDescriptor( DESCR_TAG Tag )
{
	CBaseDescriptor*	p = NULL;
	switch( Tag ) {
	case	T_ObjectDescrTag:			p = new CObjectDescriptor();		break;
	case	T_InitialObjectDescrTag:	p = new CInitialObjectDescriptor();	break;
	default:	break;
	}
	return p;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                            CBox class                            //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//            Constructor             //
//                                    //
////////////////////////////////////////
CBox::CBox( IMF_UINT32 type, const IMF_UINT8* usertype ) : m_size( 0 ), m_type( type ), m_largesize( 0 ), m_pParent( NULL ), m_ReadPos( 0 ), m_LastError( E_NONE )
{
	if ( ( type == IMF_FOURCC_UUID ) && ( usertype != NULL ) ) {
		memcpy( m_usertype, usertype, sizeof(m_usertype) );
	} else {
		memset( m_usertype, '\0', sizeof(m_usertype) );
	}
}

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CBox::Read( CBaseStream& Stream )
{
	IMF_UINT32	Type, MyType;
	IMF_UINT8	UserType[16];

	try {
		// Save the current position.
		if ( ( m_ReadPos = Stream.Tell() ) == -1 ) throw E_TELL_STREAM;

		// Read size and type.
		if ( !Stream.Read32( m_size ) || !Stream.Read32( Type ) ) throw E_READ_STREAM;
		if ( m_size == 0 ) {
			// [LIMITATION] Unspecified box size is not supported.
			throw E_BOX_UNSPECIFIED_SIZE;
		} else if ( m_size == 1 ) {
			if ( !Stream.Read64( m_largesize ) ) throw E_READ_STREAM;
		} else {
			m_largesize = 0;
		}

		// Check type and user type.
		MyType = GetType();
		if ( MyType == 0 ) {
			// This is an unknown box: read type and user type from stream.
			m_type = Type;
			if ( Type == IMF_FOURCC_UUID ) {
				if ( Stream.Read( m_usertype, sizeof(m_usertype) ) != sizeof(m_usertype) ) throw E_READ_STREAM;
			}
		} else {
			// This is a known box: check the type and user type.
			if ( MyType != Type ) throw E_BOX_UNEXPECTED_TYPE;
			if ( Type == IMF_FOURCC_UUID ) {
				if ( Stream.Read( UserType, sizeof(UserType) ) != sizeof(UserType) ) throw E_READ_STREAM;
				if ( memcmp( m_usertype, UserType, sizeof(m_usertype) ) != 0 ) throw E_BOX_UNEXPECTED_TYPE;
			}
		}
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
bool	CBox::Write( CBaseStream& Stream ) const
{
	if ( !Stream.Write32( m_size ) || !Stream.Write32( m_type ) ) return false;
	if ( m_size == 1 ) {
		if ( !Stream.Write64( m_largesize ) ) return false;
	}
	if ( m_type == IMF_FOURCC_UUID ) {
		if ( Stream.Write( m_usertype, sizeof(m_usertype) ) != sizeof(m_usertype) ) return false;
	}
	return true;
}

////////////////////////////////////////
//                                    //
//          Check read size           //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = -1:Not reached to end / 0:Just reached to end / 1:Overrun or error
IMF_INT8	CBox::CheckReadSize( CBaseStream& Stream ) const
{
	IMF_INT64	CurPos, EndPos;

	// Get the current position.
	if ( ( CurPos = Stream.Tell() ) < 0 ) return 1;

	// Calculate end position.
	EndPos = m_ReadPos + ( ( m_size == 1 ) ? m_largesize : m_size );
	if ( EndPos < 0 ) return 1;

	return ( CurPos == EndPos ) ? 0 : ( CurPos < EndPos ) ? -1 : 1;
}

////////////////////////////////////////
//                                    //
//           Get data size            //
//                                    //
////////////////////////////////////////
// Return value = Data size in bytes (-1 means error)
IMF_INT64	CBox::GetDataSize( void ) const
{
	IMF_INT64	Size;
	
	// Get size or largesize field.
	if ( m_size == 1 ) {
		Size = static_cast<IMF_INT64>( m_largesize );
		if ( Size < 0 ) return -1;
	} else {
		Size = static_cast<IMF_INT64>( m_size );
	}
	
	// Subtract the size of basic fields.
	Size -= 8;										// size and type
	if ( m_size == 1 ) Size -= 8;					// largesize
	if ( m_type == IMF_FOURCC_UUID ) Size -= 16;	// usertype
	return ( Size < 0 ) ? -1 : Size;
}

////////////////////////////////////////
//                                    //
//           Set data size            //
//                                    //
////////////////////////////////////////
// Size = Data size in bytes
// Return value = Whole box size in bytes (-1 means error)
IMF_INT64	CBox::SetDataSize( IMF_INT64 Size )
{
	// Size must be positive.
	if ( Size < 0 ) { SetLastError( E_BOX_SET_SIZE ); return -1; }

	// Add common header size.
	Size += 8;										// size and type
	if ( m_type == IMF_FOURCC_UUID ) Size += 16;	// usertype
	if ( Size < 0 ) { SetLastError( E_BOX_SET_SIZE ); return -1; }

	if ( Size > 0xffffffff ) {
		// Size is 64-bit.
		Size += 8;									// largesize
		if ( Size < 0 ) { SetLastError( E_BOX_SET_SIZE ); return -1; }
		m_size = 1;
		m_largesize = Size;
	} else {
		// Size is 32-bit.
		m_size = static_cast<IMF_UINT32>( Size );
		m_largesize = 0;
	}
	return Size;
}

////////////////////////////////////////
//                                    //
//             Find a box             //
//                                    //
////////////////////////////////////////
// Type = Target type
// pLast = Reference to the last found pointer (NULL means to start from the top)
// Return value = true:Found (pLast is updated) / false:Not found
bool	CBox::FindBox( IMF_UINT32 Type, CBox*& pLast )
{
	CBox*	pChild = NULL;

	// Check myself.
	if ( ( pLast == NULL ) && ( m_type == Type ) ) {
		pLast = this;
		return true;
	}
	if ( pLast == this ) pLast = NULL;

	// Looking for child boxes.
	while( ( pChild = GetNextChild( pChild ) ) ) {
		if ( pChild->FindBox( Type, pLast ) ) return true;
	}
	return false;
}

////////////////////////////////////////
//                                    //
//       Output box information       //
//                                    //
////////////////////////////////////////
// Stream = Output stream
// Indent = Indent count
void	CBox::Out( ostream& Stream, IMF_UINT16 Indent ) const
{
	char		Buf[5];
	IMF_UINT16	i;

	// Make indent.
	for( i=0; i<Indent; i++ ) Stream << ' ';

	// Show box type.
	Buf[0] = static_cast<char>( m_type >> 24 );
	Buf[1] = static_cast<char>( m_type >> 16 );
	Buf[2] = static_cast<char>( m_type >>  8 );
	Buf[3] = static_cast<char>( m_type );
	Buf[4] = '\0';
	Stream << Buf;

	// In case of 'uuid', then show user type.
	if ( m_type == IMF_FOURCC_UUID ) {
		Stream << ':' << hex << setfill( '0' );
		for( i=0; i<16; i++ ) {
			Stream << setw( 2 ) << static_cast<IMF_INT32>( m_usertype[i] );
			if ( ( i == 3 ) || ( i == 5 ) || ( i == 7 ) || ( i == 9 ) ) Stream << '-';
		}
		Stream << dec << setfill( ' ' );
	}

	// Show size.
#if defined( _MSC_VER ) && ( _MSC_VER <= 1200 )
	// VC++6 cannot compile operator << for 64-bit values.
	char	SizeBuf[64];
	sprintf( SizeBuf, " (%I64u bytes)", ( m_size == 1 ) ? m_largesize : static_cast<IMF_UINT64>( m_size ) );
	Stream << SizeBuf;
#else
	Stream << " (" << dec << ( ( m_size == 1 ) ? m_largesize : m_size ) << " bytes)";
#endif

	if ( GetType() == 0 ) Stream << " [unknown]";
	Stream << endl;
}

////////////////////////////////////////
//                                    //
//         Print box details          //
//                                    //
////////////////////////////////////////
// Stream = Output stream
// Indent = Indent count
// MaxLen = Maximum string length per line
void	CBox::Print( CPrintStream& Stream ) const
{
	char		Buf[7];

	// Show box type.
	Buf[0] = '[';
	Buf[1] = static_cast<char>( m_type >> 24 );
	Buf[2] = static_cast<char>( m_type >> 16 );
	Buf[3] = static_cast<char>( m_type >>  8 );
	Buf[4] = static_cast<char>( m_type );
	Buf[5] = ']';
	Buf[6] = '\0';
	Stream << Buf << endl;

	// Show parameters
	if ( m_size == 1 ) Stream << "largesize = " << m_largesize << endl;
	else Stream << "size = " << m_size << endl;
}

////////////////////////////////////////
//                                    //
//         Dump box contents          //
//                                    //
////////////////////////////////////////
// Stream = Output stream
// Indent = Indent count
// MaxLen = Maximum byte count to dump
void	CBox::Dump( ostream& Stream, IMF_UINT16 Indent, IMF_UINT64 MaxLen ) const
{
	IMF_UINT16	i;
	IMF_UINT64	Size;

	// Show box information.
	CBox::Out( Stream, Indent );

	// Check MaxLen.
	if ( MaxLen > 0 ) {
		Size = ( m_size == 1 ) ? m_largesize : static_cast<IMF_UINT64>( m_size );
		if ( Size > MaxLen ) {
			for( i=0; i<Indent; i++ ) Stream << ' ';
			Stream << "(This box is too big to dump.)" << endl << endl;
			return;
		}
	}

	// Dump box contents.
	CHexDumpStream	Dump;
	if ( Dump.Open( Stream, static_cast<IMF_UINT16>( Indent ) ) ) {
		Write( Dump );
		Dump.Close();
	}
	Stream << endl;
}

////////////////////////////////////////
//                                    //
//           Read a string            //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// String = String object to receive the result
// MaxLen = Maximum length in bytes, including terminating NULL character
// Return value = true:Success / false:Error
bool	CBox::ReadString( CBaseStream& Stream, string& String, IMF_INT64 MaxLen )
{
	bool		Result = false;
	IMF_UINT32	BufSize, ReadSize, i;
	char*		pBuf = NULL;
	IMF_INT64	Pos;

	// Check MaxLen.
	if ( MaxLen >> 32 ) { SetLastError( E_BOX_STRLEN_OVERFLOW ); return false; }	// MaxLen must be positive and less than 4GB.
	BufSize = static_cast<IMF_UINT32>( MaxLen );

	// Save the current position.
	if ( ( Pos = Stream.Tell() ) == -1 ) { SetLastError( E_TELL_STREAM ); return false; }

	// Allocate temporary buffer.
	pBuf = new char [ BufSize ];
	if ( pBuf == NULL ) { SetLastError( E_MEMORY ); return false; }

	try {
		// Read some data.
		ReadSize = Stream.Read( pBuf, BufSize );

		// Looking for the terminating NULL character.
		for( i=0; ( i < ReadSize ) && ( pBuf[i] != '\0' ); i++ );
		if ( i >= ReadSize ) throw E_BOX_STRING_TOO_LONG;

		// Build string object.
		String.assign( pBuf, i );

		// Seek stream pointer.
		if ( !Stream.Seek( Pos + i + 1, CBaseStream::S_BEGIN ) ) throw E_SEEK_STREAM;
		Result = true;
	}
	catch( IMF_UINT32 e ) {
		SetLastError( e );
		delete[] pBuf;
	}
	return Result;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                          CFullBox class                          //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CFullBox::Read( CBaseStream& Stream )
{
	IMF_UINT8	Buf[3];
	if ( !CBox::Read( Stream ) || !Stream.Read8( m_version ) || ( Stream.Read( Buf, sizeof(Buf) ) != sizeof(Buf) ) ) { SetLastError( E_READ_STREAM ); return false; }
	m_flags = ( static_cast<IMF_UINT32>( Buf[0] ) << 16 ) | ( static_cast<IMF_UINT32>( Buf[1] ) << 8 ) | static_cast<IMF_UINT32>( Buf[2] );
	return true;
}

////////////////////////////////////////
//                                    //
//               Write                //
//                                    //
////////////////////////////////////////
// Stream = Output stream
// Return value = true:Success / false:Error
bool	CFullBox::Write( CBaseStream& Stream ) const
{
	IMF_UINT8	Buf[3];
	Buf[0] = static_cast<IMF_UINT8>( m_flags >> 16 );
	Buf[1] = static_cast<IMF_UINT8>( m_flags >>  8 );
	Buf[2] = static_cast<IMF_UINT8>( m_flags );
	return CBox::Write( Stream ) && Stream.Write8( m_version ) && ( Stream.Write( Buf, sizeof(Buf) ) == sizeof(Buf) );
}

////////////////////////////////////////
//                                    //
//           Set data size            //
//                                    //
////////////////////////////////////////
// Size = Data size in bytes
// Return value = Whole box size in bytes (-1 means error)
IMF_INT64	CFullBox::SetDataSize( IMF_INT64 Size )
{
	if ( Size < 0 ) { SetLastError( E_BOX_SET_SIZE ); return -1; }
	return CBox::SetDataSize( Size + 4 );
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                         CBoxVector class                         //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//         Get the next item          //
//                                    //
////////////////////////////////////////
// pLast = Previously returned pointer (NULL = first call)
// Return value = Pointer to next item
CBox*	CBoxVector::GetNext( CBox* pLast )
{
	const_iterator	i = begin();
	if ( pLast ) {
		// Move to the next of pLast.
		while( ( i != end() ) && ( *i++ != pLast ) );
	}
	return ( i == end() ) ? NULL : *i;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                          CDataBox class                          //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CDataBox::Read( CBaseStream& Stream )
{
	IMF_INT64	DataSize;

	// Read basic fields.
	if ( !CBox::Read( Stream ) ) return false;

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
bool	CDataBox::Write( CBaseStream& Stream ) const
{
	IMF_INT64	DataSize;

	// [LIMITATION] Data size must be less than 2GB.
	DataSize = GetDataSize();
	if ( ( DataSize < 0 ) || ( ( DataSize >> 32 ) != 0 ) ) return false;

	// Write basic fields.
	if ( !CBox::Write( Stream ) ) return false;

	if ( DataSize > 0 ) {
		if ( m_data == NULL ) return false;
		// Write data.
		if ( Stream.Write( m_data, static_cast<IMF_UINT32>( DataSize ) ) != static_cast<IMF_UINT32>( DataSize ) ) return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                       CContainerBox class                        //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CContainerBox::Read( CBaseStream& Stream )
{
	CBox*		pChild = NULL;
	IMF_INT8	ReadStatus;

	// Read basic fields.
	if ( !CBox::Read( Stream ) ) return false;

	// Clear child boxes.
	m_Boxes.Clear();

	// Read child boxes.
	while( ( ReadStatus = CheckReadSize( Stream ) ) < 0 ) {
		// Read a child box.
		pChild = m_pReader->Read( Stream, this );
		if ( pChild == NULL ) { SetLastError( m_pReader->GetLastError() ); return false; }
		m_Boxes.push_back( pChild );
	}
	if ( ReadStatus != 0 ) { SetLastError( E_BOX_SIZE ); return false; }
	return true;
}

////////////////////////////////////////
//                                    //
//               Write                //
//                                    //
////////////////////////////////////////
// Stream = Output stream
// Return value = true:Success / false:Error
bool	CContainerBox::Write( CBaseStream& Stream ) const
{
	CBoxVector::const_iterator	i;

	// Write basic fields.
	if ( !CBox::Write( Stream ) ) return false;

	// Write child boxes.
	for( i=m_Boxes.begin(); i!=m_Boxes.end(); i++ ) if ( !(*i)->Write( Stream ) ) return false;
	return true;
}

////////////////////////////////////////
//                                    //
//         Calculate box size         //
//                                    //
////////////////////////////////////////
// Return value = Whole box size in bytes (-1 means error)
IMF_INT64	CContainerBox::CalcSize( void )
{
	IMF_INT64	BoxSize;
	IMF_INT64	WholeSize = 0;
	CBoxVector::iterator	i;

	for( i=m_Boxes.begin(); i!=m_Boxes.end(); i++ ) {
		BoxSize = (*i)->CalcSize();
		if ( BoxSize < 0 ) { SetLastError( E_BOX_SIZE ); return -1; }
		WholeSize += BoxSize;
		if ( WholeSize < 0 ) { SetLastError( E_BOX_SIZE ); return -1; }
	}
	return SetDataSize( WholeSize );
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                     CContainerFullBox class                      //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CContainerFullBox::Read( CBaseStream& Stream )
{
	CBox*		pChild = NULL;
	IMF_INT8	ReadStatus;

	// Read basic fields.
	if ( !CFullBox::Read( Stream ) ) return false;

	// Clear child boxes.
	m_Boxes.Clear();

	// Read child boxes.
	while( ( ReadStatus = CheckReadSize( Stream ) ) < 0 ) {
		// Read a child box.
		pChild = m_pReader->Read( Stream, this );
		if ( pChild == NULL ) { SetLastError( m_pReader->GetLastError() ); return false; }
		m_Boxes.push_back( pChild );
	}
	if ( ReadStatus != 0 ) { SetLastError( E_BOX_SIZE ); return false; }
	return true;
}

////////////////////////////////////////
//                                    //
//               Write                //
//                                    //
////////////////////////////////////////
// Stream = Output stream
// Return value = true:Success / false:Error
bool	CContainerFullBox::Write( CBaseStream& Stream ) const
{
	CBoxVector::const_iterator	i;

	// Write basic fields.
	if ( !CFullBox::Write( Stream ) ) return false;

	// Write child boxes.
	for( i=m_Boxes.begin(); i!=m_Boxes.end(); i++ ) if ( !(*i)->Write( Stream ) ) return false;
	return true;
}

////////////////////////////////////////
//                                    //
//         Calculate box size         //
//                                    //
////////////////////////////////////////
// Return value = Whole box size in bytes (-1 means error)
IMF_INT64	CContainerFullBox::CalcSize( void )
{
	IMF_INT64	BoxSize;
	IMF_INT64	WholeSize = 0;
	CBoxVector::iterator	i;

	for( i=m_Boxes.begin(); i!=m_Boxes.end(); i++ ) {
		BoxSize = (*i)->CalcSize();
		if ( BoxSize < 0 ) { SetLastError( E_BOX_SIZE ); return -1; }
		WholeSize += BoxSize;
		if ( WholeSize < 0 ) { SetLastError( E_BOX_SIZE ); return -1; }
	}
	return SetDataSize( WholeSize );
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                    CFileTypeBox class (ftyp)                     //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CFileTypeBox::Read( CBaseStream& Stream )
{
	IMF_UINT32	Brand;
	IMF_INT8	ReadStatus;

	// Read basic fields.
	if ( !CBox::Read( Stream ) ) return false;

	// Read major brand and minor version.
	if ( !Stream.Read32( m_major_brand ) || !Stream.Read32( m_minor_version ) ) { SetLastError( E_READ_STREAM ); return false; }

	// Clear compatible brands.
	m_compatible_brands.clear();

	// Read compatible brands.
	while( ( ReadStatus = CheckReadSize( Stream ) ) < 0 ) {
		if ( !Stream.Read32( Brand ) ) { SetLastError( E_READ_STREAM ); return false; }
		m_compatible_brands.push_back( Brand );
	}
	if ( ReadStatus != 0 ) { SetLastError( E_BOX_SIZE ); return false; }
	return true;
}

////////////////////////////////////////
//                                    //
//               Write                //
//                                    //
////////////////////////////////////////
// Stream = Output stream
// Return value = true:Success / false:Error
bool	CFileTypeBox::Write( CBaseStream& Stream ) const
{
	vector<IMF_UINT32>::const_iterator	i;

	// Write basic fields.
	if ( !CBox::Write( Stream ) ) return false;

	// Write major brand and minor version.
	if ( !Stream.Write32( m_major_brand ) ) return false;
	if ( !Stream.Write32( m_minor_version ) ) return false;

	// Write compatible brands.
	for( i=m_compatible_brands.begin(); i!=m_compatible_brands.end(); i++ ) if ( !Stream.Write32( *i ) ) return false;
	return true;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                   CMovieHeaderBox class (mvhd)                   //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//            Constructor             //
//                                    //
////////////////////////////////////////
// version = 0:32-bit / 1:64-bit
CMovieHeaderBox::CMovieHeaderBox( IMF_UINT8 version ) : CFullBox( IMF_FOURCC_MVHD, NULL, version, 0 )
{
	m_creation_time = 0;
	m_modification_time = 0;
	m_timescale	= 0;
	m_duration = 0;
	m_rate = 0x00010000;	// 1.0
	m_volume = 0x0100;		// Full volume
	memset( m_matrix, 0, sizeof(m_matrix) );
	m_matrix[0] = m_matrix[4] = 0x00010000;
	m_matrix[8] = 0x40000000;
	m_next_track_ID = 0;
}

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CMovieHeaderBox::Read( CBaseStream& Stream )
{
	IMF_UINT32	Value32[3];
	IMF_UINT32	Dummy32;
	IMF_UINT8	Dummy[10];
	int			i;

	// Read basic fields.
	if ( !CFullBox::Read( Stream ) ) return false;

	try {
		if ( m_version == 0 ) {
			if ( !Stream.Read32( Value32[0] ) || !Stream.Read32( Value32[1] ) || !Stream.Read32( m_timescale ) || !Stream.Read32( Value32[2] ) ) throw E_READ_STREAM;
			m_creation_time = Value32[0];
			m_modification_time = Value32[1];
			m_duration = Value32[2];
		} else {
			if ( !Stream.Read64( m_creation_time ) || !Stream.Read64( m_modification_time ) || !Stream.Read32( m_timescale ) || !Stream.Read64( m_duration ) ) throw E_READ_STREAM;
		}
		if ( !Stream.Read32( m_rate ) || !Stream.Read16( m_volume ) || ( Stream.Read( Dummy, 10 ) != 10 ) ) throw E_READ_STREAM;
		for( i=0; i<9; i++ ) if ( !Stream.Read32( m_matrix[i] ) ) throw E_READ_STREAM;
		for( i=0; i<6; i++ ) if ( !Stream.Read32( Dummy32 ) ) throw E_READ_STREAM;
		if ( !Stream.Read32( m_next_track_ID ) ) throw E_READ_STREAM;
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
bool	CMovieHeaderBox::Write( CBaseStream& Stream ) const
{
	int	i;

	// Write basic fields.
	if ( !CFullBox::Write( Stream ) ) return false;

	if ( m_version == 0 ) {
		if ( ( m_creation_time >> 32 ) || ( m_modification_time >> 32 ) || ( m_duration >> 32 ) ) return false;
		if ( !Stream.Write32( static_cast<IMF_UINT32>( m_creation_time ) ) || 
			 !Stream.Write32( static_cast<IMF_UINT32>( m_modification_time ) ) ||
			 !Stream.Write32( m_timescale ) || 
			 !Stream.Write32( static_cast<IMF_UINT32>( m_duration ) ) ) return false;
	} else {
		if ( !Stream.Write64( m_creation_time ) || 
			 !Stream.Write64( m_modification_time ) ||
			 !Stream.Write32( m_timescale ) || 
			 !Stream.Write64( m_duration ) ) return false;
	}
	if ( !Stream.Write32( m_rate ) || !Stream.Write16( m_volume ) ) return false;
	for( i=0; i<10; i++ ) if ( !Stream.Write8( 0 ) ) return false;
	for( i=0; i<9; i++ ) if ( !Stream.Write32( m_matrix[i] ) ) return false;
	for( i=0; i<6; i++ ) if ( !Stream.Write32( 0 ) ) return false;
	if ( !Stream.Write32( m_next_track_ID ) ) return false;
	return true;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                   CTrackHeaderBox class (tkhd)                   //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//            Constructor             //
//                                    //
////////////////////////////////////////
// version = 0:32-bit / 1:64-bit
CTrackHeaderBox::CTrackHeaderBox( IMF_UINT8 version, IMF_UINT32 flags ) : CFullBox( IMF_FOURCC_TKHD, NULL, version, flags )
{
	m_creation_time = 0;
	m_modification_time = 0;
	m_track_ID = 0;
	m_duration = 0;
	m_layer = 0;
	m_alternate_group = 0;
	m_volume = 0x0100;
	memset( m_matrix, 0, sizeof(m_matrix) );
	m_matrix[0] = m_matrix[4] = 0x00010000;
	m_matrix[8] = 0x40000000;
	m_width = m_height = 0;
}

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CTrackHeaderBox::Read( CBaseStream& Stream )
{
	IMF_UINT32	Value32[3];
	IMF_UINT32	Dummy32;
	IMF_UINT16	Dummy16;
	int			i;

	// Read basic fields.
	if ( !CFullBox::Read( Stream ) ) return false;

	try {
		if ( m_version == 0 ) {
			if ( !Stream.Read32( Value32[0] ) || !Stream.Read32( Value32[1] ) || !Stream.Read32( m_track_ID ) || !Stream.Read32( Dummy32 ) || !Stream.Read32( Value32[2] ) ) throw E_READ_STREAM;
			m_creation_time = Value32[0];
			m_modification_time = Value32[1];
			m_duration = Value32[2];
		} else {
			if ( !Stream.Read64( m_creation_time ) || !Stream.Read64( m_modification_time ) || !Stream.Read32( m_track_ID ) || !Stream.Read32( Dummy32 ) || !Stream.Read64( m_duration ) ) throw E_READ_STREAM;
		}
		if ( !Stream.Read32( Dummy32 ) || !Stream.Read32( Dummy32 ) || !Stream.Read16( m_layer ) || !Stream.Read16( m_alternate_group ) || !Stream.Read16( m_volume ) || !Stream.Read16( Dummy16 ) ) throw E_READ_STREAM;
		for( i=0; i<9; i++ ) if ( !Stream.Read32( m_matrix[i] ) ) throw E_READ_STREAM;
		if ( !Stream.Read32( m_width ) || !Stream.Read32( m_height ) ) throw E_READ_STREAM;
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
bool	CTrackHeaderBox::Write( CBaseStream& Stream ) const
{
	int	i;

	// Write basic fields.
	if ( !CFullBox::Write( Stream ) ) return false;

	if ( m_version == 0 ) {
		if ( ( m_creation_time >> 32 ) || ( m_modification_time >> 32 ) || ( m_duration >> 32 ) ) return false;
		if ( !Stream.Write32( static_cast<IMF_UINT32>( m_creation_time ) ) || 
			 !Stream.Write32( static_cast<IMF_UINT32>( m_modification_time ) ) ||
			 !Stream.Write32( m_track_ID ) || 
			 !Stream.Write32( 0 ) || 
			 !Stream.Write32( static_cast<IMF_UINT32>( m_duration ) ) ) return false;
	} else {
		if ( !Stream.Write64( m_creation_time ) || 
			 !Stream.Write64( m_modification_time ) ||
			 !Stream.Write32( m_track_ID ) || 
			 !Stream.Write32( 0 ) || 
			 !Stream.Write64( m_duration ) ) return false;
	}
	if ( !Stream.Write32( 0 ) || !Stream.Write32( 0 ) || !Stream.Write16( m_layer ) || !Stream.Write16( m_alternate_group ) || !Stream.Write16( m_volume ) || !Stream.Write16( 0 ) ) return false;
	for( i=0; i<9; i++ ) if ( !Stream.Write32( m_matrix[i] ) ) return false;
	if ( !Stream.Write32( m_width ) || !Stream.Write32( m_height ) ) return false;
	return true;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                CTrackReferenceTypeBox (hint/cdsc)                //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CTrackReferenceTypeBox::Read( CBaseStream& Stream )
{
	IMF_INT8	ReadStatus;
	IMF_UINT32	track_ID;

	// Clear track IDs.
	m_track_IDs.clear();

	// Read basic fields.
	if ( !CBox::Read( Stream ) ) return false;

	while( ( ReadStatus = CheckReadSize( Stream ) ) < 0 ) {
		if ( !Stream.Read32( track_ID ) ) { SetLastError( E_READ_STREAM ); return false; }
		if ( track_ID == 0 ) { SetLastError( E_HINTCDSC_TRACK_ID ); return false; }
		m_track_IDs.push_back( track_ID );
	}
	if ( ReadStatus != 0 ) { SetLastError( E_BOX_SIZE ); return false; }
	return true;
}

////////////////////////////////////////
//                                    //
//               Write                //
//                                    //
////////////////////////////////////////
// Stream = Output stream
// Return value = true:Success / false:Error
bool	CTrackReferenceTypeBox::Write( CBaseStream& Stream ) const
{
	vector<IMF_UINT32>::const_iterator	i;

	// Write basic fields.
	if ( !CBox::Write( Stream ) ) return false;
	for( i=m_track_IDs.begin(); i!=m_track_IDs.end(); i++ ) if ( !Stream.Write32( *i ) ) return false;
	return true;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                   CMediaHeaderBox class (mdhd)                   //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//            Constructor             //
//                                    //
////////////////////////////////////////
// version = 0:32-bit / 1:64-bit
CMediaHeaderBox::CMediaHeaderBox( IMF_UINT8 version ) : CFullBox( IMF_FOURCC_MDHD, NULL, version, 0 )
{
	m_creation_time = 0;
	m_modification_time = 0;
	m_timescale = 0;
	m_duration = 0;
	memset( m_language, 0, sizeof(m_language) );
}

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CMediaHeaderBox::Read( CBaseStream& Stream )
{
	IMF_UINT32	Value32[3];
	IMF_UINT16	Language;
	IMF_UINT16	Dummy;

	// Read basic fields.
	if ( !CFullBox::Read( Stream ) ) return false;

	try {
		if ( m_version == 0 ) {
			if ( !Stream.Read32( Value32[0] ) || !Stream.Read32( Value32[1] ) || !Stream.Read32( m_timescale ) || !Stream.Read32( Value32[2] ) ) throw E_READ_STREAM;
			m_creation_time = Value32[0];
			m_modification_time = Value32[1];
			m_duration = Value32[2];
		} else {
			if ( !Stream.Read64( m_creation_time ) || !Stream.Read64( m_modification_time ) || !Stream.Read32( m_timescale ) || !Stream.Read64( m_duration ) ) throw E_READ_STREAM;
		}
		if ( !Stream.Read16( Language ) || !Stream.Read16( Dummy ) ) throw E_READ_STREAM;
		m_language[0] = static_cast<IMF_INT8>( ( Language >> 10 ) & 0x1f );
		m_language[1] = static_cast<IMF_INT8>( ( Language >> 5 ) & 0x1f );
		m_language[2] = static_cast<IMF_INT8>( Language & 0x1f );
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
bool	CMediaHeaderBox::Write( CBaseStream& Stream ) const
{
	IMF_UINT16	Language;
	int			i;

	// Write basic fields.
	if ( !CFullBox::Write( Stream ) ) return false;

	if ( m_version == 0 ) {
		if ( ( m_creation_time >> 32 ) || ( m_modification_time >> 32 ) || ( m_duration >> 32 ) ) return false;
		if ( !Stream.Write32( static_cast<IMF_UINT32>( m_creation_time ) ) || 
			 !Stream.Write32( static_cast<IMF_UINT32>( m_modification_time ) ) ||
			 !Stream.Write32( m_timescale ) || 
			 !Stream.Write32( static_cast<IMF_UINT32>( m_duration ) ) ) return false;
	} else {
		if ( !Stream.Write64( m_creation_time ) || 
			 !Stream.Write64( m_modification_time ) ||
			 !Stream.Write32( m_timescale ) || 
			 !Stream.Write64( m_duration ) ) return false;
	}
	Language = 0;
	for( i=0; i<3; i++ ) {
		if ( m_language[i] & 0xe0 ) return false;
		Language = ( Language << 5 ) | m_language[i];
	}
	if ( !Stream.Write16( Language ) || !Stream.Write16( 0 ) ) return false;
	return true;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                     CHandlerBox class (hdlr)                     //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CHandlerBox::Read( CBaseStream& Stream )
{
	IMF_UINT32	Dummy;

	// Read basic fields.
	if ( !CFullBox::Read( Stream ) ) return false;

	try {
		if ( !Stream.Read32( Dummy ) || !Stream.Read32( m_handler_type ) ) throw E_READ_STREAM;
		if ( !Stream.Read32( Dummy ) || !Stream.Read32( Dummy ) || !Stream.Read32( Dummy ) ) throw E_READ_STREAM;
		if ( !ReadString( Stream, m_name, GetDataSize() - 20 ) ) return false;
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
bool	CHandlerBox::Write( CBaseStream& Stream ) const
{
	IMF_UINT32	NameLen;

	// Write basic fields.
	if ( !CFullBox::Write( Stream ) ) return false;

	if ( !Stream.Write32( 0 ) || !Stream.Write32( m_handler_type ) ) return false;
	if ( !Stream.Write32( 0 ) || !Stream.Write32( 0 ) || !Stream.Write32( 0 ) ) return false;
	NameLen = static_cast<IMF_UINT32>( m_name.length() ) + 1;
	if ( Stream.Write( m_name.c_str(), NameLen ) != NameLen ) return false;
	return true;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                CVideoMediaHeaderBox class (vmhd)                 //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CVideoMediaHeaderBox::Read( CBaseStream& Stream )
{
	int	i;

	// Read basic fields.
	if ( !CFullBox::Read( Stream ) ) return false;
	try {
		if ( !Stream.Read16( m_graphicsmode ) ) throw E_READ_STREAM;
		for( i=0; i<3; i++ ) if ( !Stream.Read16( m_opcolor[i] ) ) throw E_READ_STREAM;
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
bool	CVideoMediaHeaderBox::Write( CBaseStream& Stream ) const
{
	int	i;

	// Write basic fields.
	if ( !CFullBox::Write( Stream ) ) return false;
	if ( !Stream.Write16( m_graphicsmode ) ) return false;
	for( i=0; i<3; i++ ) if ( !Stream.Write16( m_opcolor[i] ) ) return false;
	return true;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                CSoundMediaHeaderBox class (smhd)                 //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CSoundMediaHeaderBox::Read( CBaseStream& Stream )
{
	IMF_UINT16	Dummy;

	// Read basic fields.
	if ( !CFullBox::Read( Stream ) ) return false;
	if ( !Stream.Read16( m_balance ) || !Stream.Read16( Dummy ) ) { SetLastError( E_READ_STREAM ); return false; }
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
bool	CSoundMediaHeaderBox::Write( CBaseStream& Stream ) const
{
	// Write basic fields.
	if ( !CFullBox::Write( Stream ) ) return false;
	if ( !Stream.Write16( m_balance ) || !Stream.Write16( 0 ) ) return false;
	return true;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                 CHintMediaHeaderBox class (hmhd)                 //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CHintMediaHeaderBox::Read( CBaseStream& Stream )
{
	// Read basic fields.
	if ( !CFullBox::Read( Stream ) ) return false;
	if ( !Stream.Read16( m_maxPDUsize ) || !Stream.Read16( m_avgPDUsize ) || 
		 !Stream.Read32( m_maxbitrate ) || !Stream.Read32( m_avgbitrate ) ) { SetLastError( E_READ_STREAM ); return false; }
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
bool	CHintMediaHeaderBox::Write( CBaseStream& Stream ) const
{
	// Write basic fields.
	if ( !CFullBox::Write( Stream ) ) return false;
	if ( !Stream.Write16( m_maxPDUsize ) || !Stream.Write16( m_avgPDUsize ) || 
		 !Stream.Write32( m_maxbitrate ) || !Stream.Write32( m_avgbitrate ) ) return false;
	return true;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                  CDataEntryUrlBox class (url )                   //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CDataEntryUrlBox::Read( CBaseStream& Stream )
{
	// Read basic fields.
	if ( !CFullBox::Read( Stream ) ) return false;
	if ( m_flags == 1 ) {
		m_location.erase();
	} else {
		if ( !ReadString( Stream, m_location, GetDataSize() ) ) return false;
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
bool	CDataEntryUrlBox::Write( CBaseStream& Stream ) const
{
	// Write basic fields.
	if ( !CFullBox::Write( Stream ) ) return false;
	return ( m_flags == 1 ) || Stream.Write( m_location.c_str(), static_cast<IMF_UINT32>( m_location.length() ) + 1 );
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                  CDataEntryUrnBox class (urn )                   //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CDataEntryUrnBox::Read( CBaseStream& Stream )
{
	IMF_INT64	DataSize;

	// Read basic fields.
	if ( !CFullBox::Read( Stream ) ) return false;
	DataSize = GetDataSize();
	if ( !ReadString( Stream, m_name, DataSize ) ) return false;
	if ( !ReadString( Stream, m_location, DataSize - m_name.length() - 1 ) ) return false;
	if ( CheckReadSize( Stream ) != 0 ) { SetLastError( E_BOX_SIZE ); }
	return true;
}

////////////////////////////////////////
//                                    //
//               Write                //
//                                    //
////////////////////////////////////////
// Stream = Output stream
// Return value = true:Success / false:Error
bool	CDataEntryUrnBox::Write( CBaseStream& Stream ) const
{
	// Write basic fields.
	if ( !CFullBox::Write( Stream ) ) return false;
	return Stream.Write( m_name.c_str(), static_cast<IMF_UINT32>( m_name.length() ) + 1 ) && Stream.Write( m_location.c_str(), static_cast<IMF_UINT32>( m_location.length() ) + 1 );
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                  CDataReferenceBox class (dref)                  //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CDataReferenceBox::Read( CBaseStream& Stream )
{
	CBox*		pBox = NULL;
	IMF_UINT32	entry_count;
	IMF_UINT32	i;

	// Clear data entries.
	m_Entries.Clear();

	// Read basic fields.
	if ( !CFullBox::Read( Stream ) ) return false;

	if ( !Stream.Read32( entry_count ) ) { SetLastError( E_READ_STREAM ); return false; }
	for( i=0; i<entry_count; i++ ) {
		// Read url or urn.
		pBox = m_pReader->Read( Stream, this );
		if ( pBox == NULL ) { SetLastError( m_pReader->GetLastError() ); return false; }
		if ( ( pBox->GetType() != IMF_FOURCC_URL ) && ( pBox->GetType() != IMF_FOURCC_URN ) ) { SetLastError( E_DREF_NO_URL_URN ); delete pBox; return false; }
		m_Entries.push_back( pBox );
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
bool	CDataReferenceBox::Write( CBaseStream& Stream ) const
{
	CBoxVector::const_iterator	i;

	// Write basic fields.
	if ( !CFullBox::Write( Stream ) ) return false;

	if ( !Stream.Write32( static_cast<IMF_UINT32>( m_Entries.size() ) ) ) return false;
	for( i=m_Entries.begin(); i!=m_Entries.end(); i++ ) if ( !(*i)->Write( Stream ) ) return false;
	return true;
}

////////////////////////////////////////
//                                    //
//         Calculate box size         //
//                                    //
////////////////////////////////////////
// Return value = Whole box size in bytes (-1 means error)
IMF_INT64	CDataReferenceBox::CalcSize( void )
{
	IMF_INT64	EntrySize;
	IMF_INT64	WholeSize = 4;
	CBoxVector::const_iterator	i;

	for( i=m_Entries.begin(); i!=m_Entries.end(); i++ ) {
		EntrySize = (*i)->CalcSize();
		if ( EntrySize < 0 ) { SetLastError( E_BOX_SET_SIZE ); return -1; }
		WholeSize += EntrySize;
		if ( WholeSize < 0 ) { SetLastError( E_BOX_SET_SIZE ); return -1; }
	}
	return SetDataSize( WholeSize );
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                  CTimeToSampleBox class (stts)                   //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CTimeToSampleBox::Read( CBaseStream& Stream )
{
	STTS_ENTRY	Entry;
	IMF_UINT32	entry_count;
	IMF_UINT32	i;

	// Clear entries.
	m_Entries.clear();

	// Read basic fields.
	if ( !CFullBox::Read( Stream ) ) return false;

	try {
		if ( !Stream.Read32( entry_count ) ) throw E_READ_STREAM;
		for( i=0; i<entry_count; i++ ) {
			if ( !Stream.Read32( Entry.m_sample_count ) || !Stream.Read32( Entry.m_sample_delta ) ) throw E_READ_STREAM;
			m_Entries.push_back( Entry );
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
bool	CTimeToSampleBox::Write( CBaseStream& Stream ) const
{
	vector<STTS_ENTRY>::const_iterator	i;

	// Write basic fields.
	if ( !CFullBox::Write( Stream ) ) return false;

	if ( !Stream.Write32( static_cast<IMF_UINT32>( m_Entries.size() ) ) ) return false;
	for( i=m_Entries.begin(); i!=m_Entries.end(); i++ ) if ( !Stream.Write32( i->m_sample_count ) || !Stream.Write32( i->m_sample_delta ) ) return false;
	return true;
}

////////////////////////////////////////
//                                    //
//             Get vector             //
//                                    //
////////////////////////////////////////
// SamplesPerFrame = Vector object to receive number of samples in each frame
void	CTimeToSampleBox::GetVector( std::vector<IMF_UINT32>& SamplesPerFrame ) const
{
	vector<STTS_ENTRY>::const_iterator	i;

	SamplesPerFrame.clear();
	for( i=m_Entries.begin(); i!=m_Entries.end(); i++ ) SamplesPerFrame.insert( SamplesPerFrame.end(), i->m_sample_count, i->m_sample_delta );
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                CCompositionOffsetBox class (ctts)                //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CCompositionOffsetBox::Read( CBaseStream& Stream )
{
	CTTS_ENTRY	Entry;
	IMF_UINT32	entry_count;
	IMF_UINT32	i;

	// Clear entries.
	m_Entries.clear();

	// Read basic fields.
	if ( !CFullBox::Read( Stream ) ) return false;

	try {
		if ( !Stream.Read32( entry_count ) ) throw E_READ_STREAM;
		for( i=0; i<entry_count; i++ ) {
			if ( !Stream.Read32( Entry.m_sample_count ) || !Stream.Read32( Entry.m_sample_offset ) ) throw E_READ_STREAM;
			m_Entries.push_back( Entry );
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
bool	CCompositionOffsetBox::Write( CBaseStream& Stream ) const
{
	vector<CTTS_ENTRY>::const_iterator	i;

	// Write basic fields.
	if ( !CFullBox::Write( Stream ) ) return false;

	if ( !Stream.Write32( static_cast<IMF_UINT32>( m_Entries.size() ) ) ) return false;
	for( i=m_Entries.begin(); i!=m_Entries.end(); i++ ) if ( !Stream.Write32( i->m_sample_count ) || !Stream.Write32( i->m_sample_offset ) ) return false;
	return true;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                CSampleDescriptionBox class (stsd)                //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CSampleDescriptionBox::Read( CBaseStream& Stream )
{
	IMF_UINT32		entry_count, i;
	CSampleEntry*	pSampleEntry = NULL;
	IMF_UINT32		handler_type;

	// Get handler type.
	handler_type = GetHandlerType();
	if ( handler_type == 0 ) { SetLastError( E_STSD_HANDLER_TYPE ); return false; }

	// Read basic fields.
	if ( !CFullBox::Read( Stream ) ) return false;

	try {
		if ( !Stream.Read32( entry_count ) ) throw E_READ_STREAM;

		for( i=0; i<entry_count; i++ ) {
			pSampleEntry = m_pReader->CreateSampleEntry( handler_type );
			if ( pSampleEntry == NULL ) throw E_STSD_SAMPLE_ENTRY;
			pSampleEntry->m_pReader = m_pReader;
			if ( !pSampleEntry->Read( Stream ) || !pSampleEntry->ReadExtension( Stream ) ) throw pSampleEntry->GetLastError();
			if ( pSampleEntry->GetType() == 0 ) throw E_STSD_SAMPLE_ENTRY_TYPE;
			m_Entries.push_back( pSampleEntry );
		}
		if ( CheckReadSize( Stream ) != 0 ) throw E_BOX_SIZE;
	}
	catch( IMF_UINT32 e ) {
		SetLastError( e );
		if ( pSampleEntry ) delete pSampleEntry;
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
bool	CSampleDescriptionBox::Write( CBaseStream& Stream ) const
{
	std::vector<CSampleEntry*>::const_iterator	i;

	// Write basic fields.
	if ( !CFullBox::Write( Stream ) ) return false;

	if ( !Stream.Write32( static_cast<IMF_UINT32>( m_Entries.size() ) ) ) return false;
	for( i=m_Entries.begin(); i!=m_Entries.end(); i++ ) if ( !(*i)->Write( Stream ) || !(*i)->WriteExtension( Stream ) ) return false;
	return true;
}

////////////////////////////////////////
//                                    //
//         Calculate box size         //
//                                    //
////////////////////////////////////////
// Return value = Whole box size in bytes (-1 means error)
IMF_INT64	CSampleDescriptionBox::CalcSize( void )
{
	IMF_INT64	EntrySize;
	IMF_INT64	WholeSize = 4;
	std::vector<CSampleEntry*>::iterator	i;

	for( i=m_Entries.begin(); i!=m_Entries.end(); i++ ) {
		EntrySize = (*i)->CalcSize();
		if ( EntrySize < 0 ) { SetLastError( E_BOX_SET_SIZE ); return false; }
		WholeSize += EntrySize;
		if ( WholeSize < 0 ) { SetLastError( E_BOX_SET_SIZE ); return false; }
	}
	return SetDataSize( WholeSize );
}

////////////////////////////////////////
//                                    //
//     Get handler type for stsd      //
//                                    //
////////////////////////////////////////
// pBox = Parent of stsd box.
// Return value = Handler type (0 means error)
IMF_UINT32	CSampleDescriptionBox::GetHandlerType( void ) const
{
	CSampleTableBox*		pStbl;
	CMediaInformationBox*	pMinf;
	CMediaBox*				pMdia;
	CHandlerBox*			pHdlr;

	// The parent of stsd must be stbl.
	if ( ( m_pParent == NULL ) || ( m_pParent->GetType() != IMF_FOURCC_STBL ) ) return 0;
	pStbl = reinterpret_cast<CSampleTableBox*>( m_pParent );

	// The parent of stbl must be minf.
	if ( ( pStbl->m_pParent == NULL ) || ( pStbl->m_pParent->GetType() != IMF_FOURCC_MINF ) ) return 0;
	pMinf = reinterpret_cast<CMediaInformationBox*>( pStbl->m_pParent );

	// The parent of minf must be mdia.
	if ( ( pMinf->m_pParent == NULL ) || ( pMinf->m_pParent->GetType() != IMF_FOURCC_MDIA ) ) return 0;
	pMdia = reinterpret_cast<CMediaBox*>( pMinf->m_pParent );

	// mdia must have hdlr.
	for( CBoxVector::const_iterator i=pMdia->m_Boxes.begin(); i!=pMdia->m_Boxes.end(); i++ ) {
		if ( ( *i != NULL ) && ( (*i)->GetType() == IMF_FOURCC_HDLR ) ) {
			// hdlr box is found!
			pHdlr = reinterpret_cast<CHandlerBox*>( *i );
			return pHdlr->m_handler_type;
		}
	}
	return 0;
}

////////////////////////////////////////
//                                    //
//               Clear                //
//                                    //
////////////////////////////////////////
void	CSampleDescriptionBox::Clear( void )
{
	for( vector<CSampleEntry*>::iterator i=m_Entries.begin(); i!=m_Entries.end(); i++ ) delete *i;
	m_Entries.clear();
}

////////////////////////////////////////
//                                    //
//               Print                //
//                                    //
////////////////////////////////////////
void	CSampleDescriptionBox::Print( CPrintStream& Stream ) const
{
	IMF_UINT32	n = 0;
	CFullBox::Print( Stream );
	for( vector<CSampleEntry*>::const_iterator i=m_Entries.begin(); i!=m_Entries.end(); i++ ) {
		Stream << "entry[" << n << "]" << endl;
		Stream.Indent( 1 );
		(*i)->Print( Stream );
		Stream.Indent( -1 );
	}
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                   CSampleSizeBox class (stsz)                    //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CSampleSizeBox::Read( CBaseStream& Stream )
{
	IMF_UINT32	i;
	IMF_UINT32	entry_size;

	// Clear entry sizes.
	m_entry_sizes.clear();

	// Read basic fields.
	if ( !CFullBox::Read( Stream ) ) return false;

	try {
		if ( !Stream.Read32( m_sample_size ) || !Stream.Read32( m_sample_count ) ) throw E_READ_STREAM;
		if ( m_sample_size == 0 ) {
			for( i=0; i<m_sample_count; i++ ) {
				if ( !Stream.Read32( entry_size ) ) throw E_READ_STREAM;
				m_entry_sizes.push_back( entry_size );
			}
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
bool	CSampleSizeBox::Write( CBaseStream& Stream ) const
{
	vector<IMF_UINT32>::const_iterator	i;

	// Write basic fields.
	if ( !CFullBox::Write( Stream ) ) return false;

	if ( !Stream.Write32( m_sample_size ) || !Stream.Write32( m_sample_count ) ) return false;
	if ( m_sample_size == 0 ) {
		if ( m_sample_count != m_entry_sizes.size() ) return false;
		for( i=m_entry_sizes.begin(); i!=m_entry_sizes.end(); i++ ) if ( !Stream.Write32( *i ) ) return false;
	}
	return true;
}

////////////////////////////////////////
//                                    //
//             Get vector             //
//                                    //
////////////////////////////////////////
// SizesPerFrame = Vector object to receive number of bytes per frame
void	CSampleSizeBox::GetVector( vector<IMF_UINT32>& SizesPerFrame ) const
{
	SizesPerFrame.clear();

	if ( m_sample_size != 0 ) {
		SizesPerFrame.assign( m_sample_count, m_sample_size );
	} else {
		SizesPerFrame = m_entry_sizes;
	}
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                CCompactSampleSizeBox class (stz2)                //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CCompactSampleSizeBox::Read( CBaseStream& Stream )
{
	IMF_UINT8	Dummy[3];
	IMF_UINT32	i;
	IMF_UINT32	sample_count;
	IMF_UINT8	Size8;
	IMF_UINT16	Size16;

	// Clear entry sizes.
	m_entry_sizes.clear();

	// Read basic fields.
	if ( !CFullBox::Read( Stream ) ) return false;

	try {
		if ( ( Stream.Read( Dummy, sizeof(Dummy) ) != sizeof(Dummy) ) || !Stream.Read8( m_field_size ) || !Stream.Read32( sample_count ) ) throw E_READ_STREAM;
		switch( m_field_size ) {
		case	4:
			for( i=0; i<sample_count; ) {
				if ( !Stream.Read8( Size8 ) ) throw E_READ_STREAM;
				m_entry_sizes.push_back( Size8 >> 4 );
				if ( ++i < sample_count ) {
					m_entry_sizes.push_back( Size8 & 0xf );
					i++;
				}
			}
			break;
		case	8:
			for( i=0; i<sample_count; i++ ) {
				if ( !Stream.Read8( Size8 ) ) throw E_READ_STREAM;
				m_entry_sizes.push_back( Size8 );
			}
			break;
		case	16:
			for( i=0; i<sample_count; i++ ) {
				if ( !Stream.Read16( Size16 ) ) throw E_READ_STREAM;
				m_entry_sizes.push_back( Size16 );
			}
			break;
		default:
			throw E_STZ2_FIELD_SIZE;
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
bool	CCompactSampleSizeBox::Write( CBaseStream& Stream ) const
{
	vector<IMF_UINT16>::const_iterator	i;
	IMF_UINT8	Size4;

	// Write basic fields.
	if ( !CFullBox::Write( Stream ) ) return false;

	if ( !Stream.Write16( 0 ) || !Stream.Write8( 0 ) || !Stream.Write8( m_field_size ) || !Stream.Write32( static_cast<IMF_UINT32>( m_entry_sizes.size() ) ) ) return false;
	switch( m_field_size ) {
	case	4:
		for( i=m_entry_sizes.begin(); i!=m_entry_sizes.end(); ) {
			Size4 = static_cast<IMF_UINT8>( *i << 4 );
			if ( ++i != m_entry_sizes.end() ) {
				Size4 |= static_cast<IMF_UINT8>( *i & 0xf );
				i++;
			}
			if ( !Stream.Write8( Size4 ) ) return false;
		}
		break;
	case	8:
		for( i=m_entry_sizes.begin(); i!=m_entry_sizes.end(); i++ ) if ( !Stream.Write8( static_cast<IMF_UINT8>( *i ) ) ) return false;
		break;
	case	16:
		for( i=m_entry_sizes.begin(); i!=m_entry_sizes.end(); i++ ) if ( !Stream.Write16( *i ) ) return false;
		break;
	default:
		return false;
	}
	return true;
}

////////////////////////////////////////
//                                    //
//         Calculate box size         //
//                                    //
////////////////////////////////////////
// Return value = Whole box size in bytes (-1 means error)
IMF_INT64	CCompactSampleSizeBox::CalcSize( void )
{
	IMF_INT64	EntrySize;

	switch( m_field_size ) {
	case	4:
		EntrySize = m_entry_sizes.size();
		if ( EntrySize & 0x1 ) EntrySize++;
		EntrySize /= 2;
		break;
	case	8:
		EntrySize = m_entry_sizes.size();
		break;
	case	16:
		EntrySize = m_entry_sizes.size() * 2;
		break;
	default:
		SetLastError( E_STZ2_FIELD_SIZE );
		return false;
	}
	return SetDataSize( 8 + EntrySize );
}

////////////////////////////////////////
//                                    //
//             Get vector             //
//                                    //
////////////////////////////////////////
// SizesPerFrame = Vector object to receive number of bytes per frame
void	CCompactSampleSizeBox::GetVector( vector<IMF_UINT32>& SizesPerFrame ) const
{
	SizesPerFrame.clear();
	for( vector<IMF_UINT16>::const_iterator i=m_entry_sizes.begin(); i!=m_entry_sizes.end(); i++ ) SizesPerFrame.push_back( static_cast<IMF_UINT32>( *i ) );
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                  CSampleToChunkBox class (stsc)                  //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CSampleToChunkBox::Read( CBaseStream& Stream )
{
	STSC_ENTRY	Entry;
	IMF_UINT32	entry_count;
	IMF_UINT32	i;

	// Clear entries.
	m_Entries.clear();

	// Read basic fields.
	if ( !CFullBox::Read( Stream ) ) return false;

	try {
		if ( !Stream.Read32( entry_count ) ) throw E_READ_STREAM;
		for( i=0; i<entry_count; i++ ) {
			if ( !Stream.Read32( Entry.m_first_chunk ) || !Stream.Read32( Entry.m_samples_per_chunk ) || !Stream.Read32( Entry.m_sample_description_index ) ) throw E_READ_STREAM;
			m_Entries.push_back( Entry );
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
bool	CSampleToChunkBox::Write( CBaseStream& Stream ) const
{
	vector<STSC_ENTRY>::const_iterator	i;

	// Write basic fields.
	if ( !CFullBox::Write( Stream ) ) return false;

	if ( !Stream.Write32( static_cast<IMF_UINT32>( m_Entries.size() ) ) ) return false;
	for( i=m_Entries.begin(); i!=m_Entries.end(); i++ ) if ( !Stream.Write32( i->m_first_chunk ) || !Stream.Write32( i->m_samples_per_chunk ) || !Stream.Write32( i->m_sample_description_index ) ) return false;
	return true;
}

////////////////////////////////////////
//                                    //
//             Get vector             //
//                                    //
////////////////////////////////////////
// FramesPerChunk = Vector object to receive number of frames in each chunk
// * Number of values in FramesPerChunk may be less than the total chunk count.
//   The last value is applied to all following chunks.
void	CSampleToChunkBox::GetVector( std::vector<IMF_UINT32>& FramesPerChunk ) const
{
	vector<STSC_ENTRY>::const_iterator	i;
	IMF_UINT32	Index = 1;
	IMF_UINT32	Frames = 0;

	FramesPerChunk.clear();
	for( i=m_Entries.begin(); i!=m_Entries.end(); i++ ) {
		while( Index < i->m_first_chunk ) {
			FramesPerChunk.push_back( Frames );
			Index++;
		}
		Frames = i->m_samples_per_chunk;
	}
	FramesPerChunk.push_back( Frames );
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                   CChunkOffsetBox class (stco)                   //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CChunkOffsetBox::Read( CBaseStream& Stream )
{
	IMF_UINT32	i;
	IMF_UINT32	chunk_offset;
	IMF_UINT32	entry_count;

	// Clear chunk offsets.
	m_chunk_offsets.clear();

	// Read basic fields.
	if ( !CFullBox::Read( Stream ) ) return false;

	try {
		if ( !Stream.Read32( entry_count ) ) throw E_READ_STREAM;
		for( i=0; i<entry_count; i++ ) {
			if ( !Stream.Read32( chunk_offset ) ) throw E_READ_STREAM;
			m_chunk_offsets.push_back( chunk_offset );
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
bool	CChunkOffsetBox::Write( CBaseStream& Stream ) const
{
	vector<IMF_UINT32>::const_iterator	i;

	// Write basic fields.
	if ( !CFullBox::Write( Stream ) ) return false;

	if ( !Stream.Write32( static_cast<IMF_UINT32>( m_chunk_offsets.size() ) ) ) return false;
	for( i=m_chunk_offsets.begin(); i!=m_chunk_offsets.end(); i++ ) if ( !Stream.Write32( *i ) ) return false;
	return true;
}

////////////////////////////////////////
//                                    //
//             Get vector             //
//                                    //
////////////////////////////////////////
// ChunkOffsets = Vector object to receive chunk offsets
void	CChunkOffsetBox::GetVector( vector<IMF_UINT64>& ChunkOffsets ) const
{
	ChunkOffsets.clear();
	for( vector<IMF_UINT32>::const_iterator i=m_chunk_offsets.begin(); i!=m_chunk_offsets.end(); i++ ) ChunkOffsets.push_back( static_cast<IMF_UINT64>( *i ) );
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                CChunkLargeOffsetBox class (co64)                 //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CChunkLargeOffsetBox::Read( CBaseStream& Stream )
{
	IMF_UINT32	i;
	IMF_UINT64	chunk_offset;
	IMF_UINT32	entry_count;

	// Clear chunk offsets.
	m_chunk_offsets.clear();

	// Read basic fields.
	if ( !CFullBox::Read( Stream ) ) return false;

	try {
		if ( !Stream.Read32( entry_count ) ) throw E_READ_STREAM;
		for( i=0; i<entry_count; i++ ) {
			if ( !Stream.Read64( chunk_offset ) ) throw E_READ_STREAM;
			m_chunk_offsets.push_back( chunk_offset );
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
bool	CChunkLargeOffsetBox::Write( CBaseStream& Stream ) const
{
	vector<IMF_UINT64>::const_iterator	i;

	// Write basic fields.
	if ( !CFullBox::Write( Stream ) ) return false;

	if ( !Stream.Write32( static_cast<IMF_UINT32>( m_chunk_offsets.size() ) ) ) return false;
	for( i=m_chunk_offsets.begin(); i!=m_chunk_offsets.end(); i++ ) if ( !Stream.Write64( *i ) ) return false;
	return true;
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

////////////////////////////////////////
//                                    //
//            Get an item             //
//                                    //
////////////////////////////////////////
// ItemID = Item ID
// Return value = Pointer to CItem object (NULL when not found)
const CItemLocationBox::CItem*	CItemLocationBox::GetItem( IMF_UINT16 ItemID ) const
{
	vector<CItem>::const_iterator	i;
	for( i=m_items.begin(); i!=m_items.end(); i++ ) if ( i->m_item_ID == ItemID ) return &*i;
	return NULL;
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

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                   CItemInfoEntry class (infe)                    //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//            Constructor             //
//                                    //
////////////////////////////////////////
CItemInfoEntry::CItemInfoEntry( IMF_UINT8 version ) : CFullBox( IMF_FOURCC_INFE, NULL, version, 0 )
{
	m_item_ID = 0;
	m_item_protection_index = 0;
	m_content_length = 0;
	m_transfer_length = 0;
}

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
	IMF_UINT8	i, EntryCount;
	IMF_UINT32	GroupId;

	if ( !CFullBox::Read( Stream ) ) return false;
	MaxLen = GetDataSize();
	if ( ( m_version == 0 ) || ( m_version == 1 ) ) {
		if ( !Stream.Read16( m_item_ID ) || !Stream.Read16( m_item_protection_index ) ) { SetLastError( E_READ_STREAM ); return false; }
		MaxLen -= 4;
		if ( !ReadString( Stream, m_item_name, MaxLen ) ) return false;
		MaxLen -= m_item_name.length() + 1;
		if ( !ReadString( Stream, m_content_type, MaxLen ) ) return false;
		MaxLen -= m_content_type.length() + 1;
	}
	if ( m_version == 0 ) {
		if ( MaxLen > 0 ) {
			if ( !ReadString( Stream, m_content_encoding, MaxLen ) ) return false;
			MaxLen -= m_content_encoding.length() + 1;
		}
		m_content_location.erase();
		m_content_MD5.erase();
		m_content_length = m_transfer_length = 0;
		m_group_id.clear();
	}
	if ( m_version == 1 ) {
		if ( !ReadString( Stream, m_content_encoding, MaxLen ) ) return false;
		MaxLen -= m_content_encoding.length() + 1;
		if ( !ReadString( Stream, m_content_location, MaxLen ) ) return false;
		MaxLen -= m_content_location.length() + 1;
		if ( !ReadString( Stream, m_content_MD5, MaxLen ) ) return false;
		MaxLen -= m_content_MD5.length() + 1;
		if ( !Stream.Read32( m_content_length ) || !Stream.Read32( m_transfer_length ) || !Stream.Read8( EntryCount ) ) { SetLastError( E_READ_STREAM ); return false; }
		for( i=1; i<=EntryCount; i++ ) {
			if ( !Stream.Read32( GroupId ) ) { SetLastError( E_READ_STREAM ); return false; }
			m_group_id.push_back( GroupId );
		}
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
	IMF_UINT8	EntryCount;
	vector<IMF_UINT32>::const_iterator	iGroupId;

	if ( !CFullBox::Write( Stream ) ) return false;
	if ( ( m_version == 0 ) || ( m_version == 1 ) ) {
		if ( !Stream.Write16( m_item_ID ) || !Stream.Write16( m_item_protection_index ) ) return false;
		StrLen = static_cast<IMF_UINT32>( m_item_name.length() ) + 1;
		if ( Stream.Write( m_item_name.c_str(), StrLen ) != StrLen ) return false;
		StrLen = static_cast<IMF_UINT32>( m_content_type.length() ) + 1;
		if ( Stream.Write( m_content_type.c_str(), StrLen ) != StrLen ) return false;
	}
	if ( m_version == 0 ) {
		if ( !m_content_encoding.empty() ) {
			StrLen = static_cast<IMF_UINT32>( m_content_encoding.length() ) + 1;
			if ( Stream.Write( m_content_encoding.c_str(), StrLen ) != StrLen ) return false;
		}
	}
	if ( m_version == 1 ) {
		StrLen = static_cast<IMF_UINT32>( m_content_encoding.length() ) + 1;
		if ( Stream.Write( m_content_encoding.c_str(), StrLen ) != StrLen ) return false;
		StrLen = static_cast<IMF_UINT32>( m_content_location.length() ) + 1;
		if ( Stream.Write( m_content_location.c_str(), StrLen ) != StrLen ) return false;
		StrLen = static_cast<IMF_UINT32>( m_content_MD5.length() ) + 1;
		if ( Stream.Write( m_content_MD5.c_str(), StrLen ) != StrLen ) return false;
		if ( !Stream.Write32( m_content_length ) || !Stream.Write32( m_transfer_length ) ) return false;
		if ( m_group_id.size() > 0xff ) return false;
		EntryCount = static_cast<IMF_UINT8>( m_group_id.size() );
		if ( !Stream.Write8( EntryCount ) ) return false;
		for( iGroupId=m_group_id.begin(); iGroupId!=m_group_id.end(); iGroupId++ ) if ( !Stream.Write32( *iGroupId ) ) return false;
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
	IMF_INT64	Size = 0;
	if ( ( m_version == 0 ) || ( m_version == 1 ) ) {
		Size += 4 + m_item_name.length() + 1 + m_content_type.length() + 1;
	}
	if ( m_version == 0 ) {
		if ( !m_content_encoding.empty() ) Size += m_content_encoding.length() + 1;
	}
	if ( m_version == 1 ) {
		Size += m_content_encoding.length() + 1 + m_content_location.length() + 1 + m_content_MD5.length() + 1 + 9 + m_group_id.size() * 4;
	}
	return SetDataSize( Size );
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
//                   CPrimaryItemBox class (pitm)                   //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CPrimaryItemBox::Read( CBaseStream& Stream )
{
	if ( !CFullBox::Read( Stream ) ) return false;
	if ( !Stream.Read16( m_item_ID ) ) { SetLastError( E_READ_STREAM ); return false; }
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
bool	CPrimaryItemBox::Write( CBaseStream& Stream ) const
{
	if ( !CFullBox::Write( Stream ) ) return false;
	if ( !Stream.Write16( m_item_ID ) ) return false;
	return true;
}

// End of ImfBox.cpp
