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

Filename : Mp4Box.cpp
Project  : MPEG-A Audio Archival Multimedia Application Format
Author   : Koichi Sugiura (NTT Advanced Technology Corporation)
           Noboru Harada  (NTT)
Date     : August 31st, 2006
Contents : Extension defined in ISO/IEC 14496-14

*******************************************************************/

#include	"Mp4Box.h"

using namespace NAlsImf;

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                       CMp4BoxReader class                        //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//            Create a box            //
//                                    //
////////////////////////////////////////
// Type = Box type
// Return value = Pointer to newly created box object
CBox*	CMp4BoxReader::CreateBox( IMF_UINT32 Type )
{
	switch( Type ) {
	case	IMF_FOURCC_IODS:	return new CObjectDescriptorBox();
	case	IMF_FOURCC_ESDS:	return new CESDBox();
	case	IMF_FOURCC_OAFI:	return new COrigAudioFileInfoBox();
	}
	return CBoxReader::CreateBox( Type );
}

////////////////////////////////////////
//                                    //
//     Create a sample entry box      //
//                                    //
////////////////////////////////////////
// Type = Handler type
// Return value = Pointer to newly created box object
CSampleEntry*	CMp4BoxReader::CreateSampleEntry( IMF_UINT32 Type )
{
	switch( Type ) {
	case	IMF_FOURCC_SOUN:	return new CMP4AudioSampleEntry();	// [LIMITATION] AudioSampleEntry must be MP4AudioSampleEntry ('mp4a').
	}
	return CBoxReader::CreateSampleEntry( Type );
}

////////////////////////////////////////
//                                    //
//        Create a descriptor         //
//                                    //
////////////////////////////////////////
// Tag = Descriptor tag
// Return value = Pointer to newly created descriptor object
CBaseDescriptor*	CMp4BoxReader::CreateDescriptor( DESCR_TAG Tag )
{
	switch( Tag ) {
	case	T_ES_ID_IncTag:				return new CES_ID_Inc();
	case	T_MP4_IOD_Tag:				return new CMp4InitialObjectDescriptor();
	case	T_DecSpecificInfoTag:		return new CDecoderSpecificInfo();
	case	T_DecoderConfigDescrTag:	return new CDecoderConfigDescriptor();
	case	T_SLConfigDescrTag:			return new CSLConfigDescriptor();
	case	T_ES_DescrTag:				return new CMp4ES_Descriptor();
	default:	break;
	}
	return CBoxReader::CreateDescriptor( Tag );
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                         CES_ID_Inc class                         //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CES_ID_Inc::Read( CBaseStream& Stream )
{
	if ( !CBaseDescriptor::Read( Stream ) ) return false;
	if ( !Stream.Read32( m_Track_ID ) ) { SetLastError( E_READ_STREAM ); return false; }
	if ( CheckReadSize( Stream ) != 0 ) { SetLastError( E_DESCR_SIZE ); return false; }
	return true;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                CMp4InitialObjectDescriptor class                 //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//            Constructor             //
//                                    //
////////////////////////////////////////
CMp4InitialObjectDescriptor::CMp4InitialObjectDescriptor( void ) : CBaseDescriptor( T_MP4_IOD_Tag )
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
bool	CMp4InitialObjectDescriptor::Read( CBaseStream& Stream )
{
	IMF_INT8			ReadStatus;
	IMF_UINT16			Tmp;
	CBaseDescriptor*	pDescr = NULL;

	// Clear m_esDescr.
	m_esDescr.Clear();

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

		while( ( ReadStatus = CheckReadSize( Stream ) ) < 0 ) {
			// [LIMITAION] m_esDescr must be a CES_ID_Inc object.
			pDescr = new CES_ID_Inc();
			if ( pDescr == NULL ) throw E_MEMORY;
			if ( !pDescr->Read( Stream ) ) { SetLastError( pDescr->GetLastError() ); delete pDescr; return false; }
			m_esDescr.push_back( pDescr );
		}
		if ( ReadStatus != 0 ) throw E_DESCR_SIZE;
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
bool	CMp4InitialObjectDescriptor::Write( CBaseStream& Stream ) const
{
	IMF_UINT16	Tmp;

	if ( !CBaseDescriptor::Write( Stream ) ) return false;
	Tmp = ( m_ObjectDescriptorID << 6 ) | ( m_URL_Flag ? 0x0020 : 0 ) | ( m_includeInlineProfileLevelFlag ? 0x0010 : 0 ) | 0x000f;
	if ( !Stream.Write16( Tmp ) ) return false;
	// [LIMITAION] URL_Flag must not be true.
	if ( m_URL_Flag ) return false;
	if ( !Stream.Write8( m_ODProfileLevelIndication ) || 
		 !Stream.Write8( m_sceneProfileLevelIndication ) || 
		 !Stream.Write8( m_audioProfileLevelIndication ) || 
		 !Stream.Write8( m_visualProfileLevelIndication ) || 
		 !Stream.Write8( m_graphicsProfileLevelIndication ) ) return false;
	// [LIMITAION] m_esDescr must be a CES_ID_Inc object.
	for( CDescriptorVector::const_iterator i=m_esDescr.begin(); i!=m_esDescr.end(); i++ ) if ( !(*i)->Write( Stream ) ) return false;
	return true;
}

////////////////////////////////////////
//                                    //
//     Calculate descriptor size      //
//                                    //
////////////////////////////////////////
// Return value = Whole descriptor size in bytes (-1 means error)
IMF_INT64	CMp4InitialObjectDescriptor::CalcSize( void )
{
	IMF_INT64	DescrSize;
	IMF_INT64	WholeSize = 2 + 5;
	CDescriptorVector::iterator	i;

	for( i=m_esDescr.begin(); i!=m_esDescr.end(); i++ ) {
		DescrSize = (*i)->CalcSize();
		if ( DescrSize < 0 ) { SetLastError( E_DESCR_SET_SIZE ); return -1; }
		WholeSize += DescrSize;
		if ( WholeSize < 0 ) { SetLastError( E_DESCR_SET_SIZE ); return -1; }
	}
	return SetDataSize( WholeSize );
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                    CDecoderSpecificInfo class                    //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CDecoderSpecificInfo::Read( CBaseStream& Stream )
{
	IMF_INT64	DataSize;

	// Clear data.
	if ( m_pData != NULL ) {
		delete[] m_pData;
		m_pData = NULL;
	}
	m_Size = 0;

	if ( !CBaseDescriptor::Read( Stream ) ) return false;

	try {
		// [LIMITATION] DecoderSpecificInfo must be less than 4GB.
		DataSize = GetDataSize();
		if ( ( DataSize < 0 ) || ( DataSize > 0xffffffff ) ) throw E_DESCR_SIZE;
		m_pData = new IMF_UINT8 [ static_cast<IMF_UINT32>( DataSize ) ];
		if ( m_pData == NULL ) throw E_MEMORY;
		m_Size = static_cast<IMF_UINT32>( DataSize );
		if ( Stream.Read( m_pData, m_Size ) != m_Size ) throw E_READ_STREAM;
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
bool	CDecoderSpecificInfo::Write( CBaseStream& Stream ) const
{
	IMF_INT64	DataSize;

	if ( !CBaseDescriptor::Write( Stream ) ) return false;
	DataSize = GetDataSize();
	// [LIMITATION] DecoderSpecificInfo must be less than 4GB.
	if ( ( DataSize < 0 ) || ( DataSize > 0xffffffff ) ) return false;
	if ( m_Size != static_cast<IMF_UINT32>( DataSize ) ) return false;
	if ( m_Size == 0 ) return true;
	if ( m_pData == NULL ) return false;
	return ( Stream.Write( m_pData, m_Size ) == m_Size );
}

////////////////////////////////////////
//                                    //
//       Set specific info data       //
//                                    //
////////////////////////////////////////
// pData = Pointer to decoder specific info
// Size = Number of bytes in decoder specific info
// Return value = true:Success / false:Error
bool	CDecoderSpecificInfo::SetData( const void* pData, IMF_UINT32 Size )
{
	// Clear existing data.
	if ( m_pData != NULL ) {
		delete[] m_pData;
		m_pData = NULL;
	}
	m_Size = 0;

	if ( Size == 0 ) return true;

	m_pData = new IMF_UINT8 [ Size ];
	if ( m_pData == NULL ) { SetLastError( E_MEMORY ); return false; }
	memcpy( m_pData, pData, Size );
	m_Size = Size;
	return true;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                  CDecoderConfigDescriptor class                  //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//            Constructor             //
//                                    //
////////////////////////////////////////
CDecoderConfigDescriptor::CDecoderConfigDescriptor( void ) : CBaseDescriptor( T_DecoderConfigDescrTag )
{
	m_objectTypeIndication = 0;
	m_streamType = 0;
	m_upStream = false;
	m_bufferSizeDB = 0;
	m_maxBitrate = 0;
	m_avgBitrate = 0;
}

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CDecoderConfigDescriptor::Read( CBaseStream& Stream )
{
	IMF_UINT32	Tmp;

	if ( !CBaseDescriptor::Read( Stream ) ) return false;

	try {
		if ( !Stream.Read8( m_objectTypeIndication ) || 
			 !Stream.Read32( Tmp ) || 
			 !Stream.Read32( m_maxBitrate ) || 
			 !Stream.Read32( m_avgBitrate ) ) throw E_READ_STREAM;
		m_streamType = static_cast<IMF_UINT8>( Tmp >> 26 ) & 0x3f;
		m_upStream = ( ( Tmp & 0x20000000 ) != 0 );
		m_bufferSizeDB = Tmp & 0x00ffffff;
		// [LIMITATION] The rest is regarded as DecoderSpecificInfo. profileLevelIndicationIndexDescriptor must not exist.
		if ( !m_decSpecificInfo.Read( Stream ) ) throw m_decSpecificInfo.GetLastError();
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
bool	CDecoderConfigDescriptor::Write( CBaseStream& Stream ) const
{
	IMF_UINT32	Tmp;
	if ( !CBaseDescriptor::Write( Stream ) ) return false;
	Tmp = ( static_cast<IMF_UINT32>( m_streamType ) << 26 ) | ( m_upStream ? 0x02000000 : 0 ) | 0x01000000 | ( m_bufferSizeDB & 0x00ffffff );
	if ( !Stream.Write8( m_objectTypeIndication ) || 
		 !Stream.Write32( Tmp ) || 
		 !Stream.Write32( m_maxBitrate ) || 
		 !Stream.Write32( m_avgBitrate ) ) return false;
	// [LIMITATION] The rest is regarded as DecoderSpecificInfo. profileLevelIndicationIndexDescriptor must not exist.
	if ( !m_decSpecificInfo.Write( Stream ) ) return false;
	return true;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                    CSLConfigDescriptor class                     //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//            Constructor             //
//                                    //
////////////////////////////////////////
CSLConfigDescriptor::CSLConfigDescriptor( void ) : CBaseDescriptor( T_SLConfigDescrTag )
{
	m_predefined = 0;
	m_useAccessUnitStartFlag = m_useAccessUnitEndFlag = m_useRandomAccessPointFlag = m_hasRandomAccessUnitsOnlyFlag = false;
	m_usePaddingFlag = m_useTimeStampsFlag = m_useIdleFlag = m_durationFlag = false;
	m_timeStampResolution = m_OCRResolution = 0;
	m_timeStampLength = m_OCRLength = m_AU_Length = m_instantBitrateLength = m_degradationPriorityLength = m_AU_seqNumLength = m_packetSeqNumLength = 0;
	m_timeScale = 0;
	m_accessUnitDuration = m_compositionUnitDuration = 0;
	m_startDecodingTimeStamp = m_startCompositionTimeStamp = 0;
}

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CSLConfigDescriptor::Read( CBaseStream& Stream )
{
	IMF_UINT8	Tmp8;
	IMF_UINT16	Tmp16;

	if ( !CBaseDescriptor::Read( Stream ) ) return false;

	try {
		if ( !Stream.Read8( m_predefined ) ) throw E_READ_STREAM;
		if ( m_predefined == 0 ) {
			if ( !Stream.Read8( Tmp8 ) ||
				 !Stream.Read32( m_timeStampResolution ) || 
				 !Stream.Read32( m_OCRResolution ) || 
				 !Stream.Read8( m_timeStampLength ) || 
				 !Stream.Read8( m_OCRLength ) || 
				 !Stream.Read8( m_AU_Length ) || 
				 !Stream.Read8( m_instantBitrateLength ) || 
				 !Stream.Read16( Tmp16 ) ) throw E_READ_STREAM;
			m_useAccessUnitStartFlag = ( ( Tmp8 & 0x80 ) != 0 );
			m_useAccessUnitEndFlag = ( ( Tmp8 & 0x40 ) != 0 );
			m_useRandomAccessPointFlag = ( ( Tmp8 & 0x20 ) != 0 );
			m_hasRandomAccessUnitsOnlyFlag = ( ( Tmp8 & 0x10 ) != 0 );
			m_usePaddingFlag = ( ( Tmp8 & 0x08 ) != 0 );
			m_useTimeStampsFlag = ( ( Tmp8 & 0x04 ) != 0 );
			m_useIdleFlag = ( ( Tmp8 & 0x02 ) != 0 );
			m_durationFlag = ( ( Tmp8 & 0x01 ) != 0 );
			m_degradationPriorityLength = static_cast<IMF_UINT8>( Tmp16 >> 12 ) & 0x0f;
			m_AU_seqNumLength = static_cast<IMF_UINT8>( Tmp16 >> 7 ) & 0x1f;
			m_packetSeqNumLength = static_cast<IMF_UINT8>( Tmp16 >> 2 ) & 0x1f;
		}
		if ( m_durationFlag ) {
			if ( !Stream.Read32( m_timeScale ) ||
				 !Stream.Read16( m_accessUnitDuration ) || 
				 !Stream.Read16( m_compositionUnitDuration ) ) throw E_READ_STREAM;
		}
		if ( !m_useTimeStampsFlag ) {
			IMF_UINT8	Bytes = ( m_timeStampLength >> 3 ) + ( ( m_timeStampLength & 0x7 ) ? 1 : 0 );
			IMF_UINT8	i;
			m_startDecodingTimeStamp = m_startCompositionTimeStamp = 0;
			for( i=0; i<Bytes; i++ ) {
				if ( !Stream.Read8( Tmp8 ) ) throw E_READ_STREAM;
				m_startDecodingTimeStamp = ( m_startDecodingTimeStamp << 8 ) | Tmp8;
			}
			for( i=0; i<Bytes; i++ ) {
				if ( !Stream.Read8( Tmp8 ) ) throw E_READ_STREAM;
				m_startCompositionTimeStamp = ( m_startCompositionTimeStamp << 8 ) | Tmp8;
			}
		}
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
bool	CSLConfigDescriptor::Write( CBaseStream& Stream ) const
{
	IMF_UINT8	Tmp8;
	IMF_UINT16	Tmp16;

	if ( !CBaseDescriptor::Write( Stream ) ) return false;
	if ( !Stream.Write8( m_predefined ) ) return false;
	if ( m_predefined == 0 ) {
		Tmp8 = ( m_useAccessUnitStartFlag ? 0x80 : 0 ) | 
			   ( m_useAccessUnitEndFlag   ? 0x40 : 0 ) | 
			   ( m_useRandomAccessPointFlag ? 0x20 : 0 ) | 
			   ( m_hasRandomAccessUnitsOnlyFlag ? 0x10 : 0 ) | 
			   ( m_usePaddingFlag ? 0x08 : 0 ) | 
			   ( m_useTimeStampsFlag ? 0x04 : 0 ) | 
			   ( m_useIdleFlag ? 0x02 : 0 ) | 
			   ( m_durationFlag ? 0x01 : 0 );
		Tmp16 = ( ( static_cast<IMF_UINT16>( m_degradationPriorityLength ) << 12 ) & 0xf000 ) | 
				( ( static_cast<IMF_UINT16>( m_AU_seqNumLength           ) <<  7 ) & 0x0f80 ) |
				( ( static_cast<IMF_UINT16>( m_packetSeqNumLength        ) <<  2 ) & 0x007c ) | 0x0003;
		if ( !Stream.Write8( Tmp8 ) || 
			 !Stream.Write32( m_timeStampResolution ) || 
			 !Stream.Write32( m_OCRResolution ) || 
			 !Stream.Write8( m_timeStampLength ) || 
			 !Stream.Write8( m_OCRLength ) || 
			 !Stream.Write8( m_AU_Length ) || 
			 !Stream.Write8( m_instantBitrateLength ) || 
			 !Stream.Write16( Tmp16 ) ) return false;
	}
	if ( m_durationFlag ) {
		if ( !Stream.Write32( m_timeScale ) || 
			 !Stream.Write16( m_accessUnitDuration ) || 
			 !Stream.Write16( m_compositionUnitDuration ) ) return false;
	}
	if ( !m_useTimeStampsFlag ) {
		IMF_UINT8	Bytes = ( m_timeStampLength >> 3 ) + ( ( m_timeStampLength & 0x7 ) ? 1 : 0 );
		IMF_UINT8	i;
		for( i=Bytes; i>0; i-- ) if ( !Stream.Write8( static_cast<IMF_UINT8>( m_startDecodingTimeStamp >> ( ( i - 1 ) * 8 ) ) ) ) return false;
		for( i=Bytes; i>0; i-- ) if ( !Stream.Write8( static_cast<IMF_UINT8>( m_startCompositionTimeStamp >> ( ( i - 1 ) * 8 ) ) ) ) return false;
	}
	return true;
}

////////////////////////////////////////
//                                    //
//     Calculate descriptor size      //
//                                    //
////////////////////////////////////////
// Return value = Whole descriptor size in bytes (-1 means error)
IMF_INT64	CSLConfigDescriptor::CalcSize( void )
{
	IMF_INT64	Size = 1;
	if ( m_predefined == 0 ) Size += 15;
	if ( m_durationFlag ) Size += 8;
	if ( !m_useTimeStampsFlag ) {
		IMF_UINT8	Bytes = ( m_timeStampLength >> 3 ) + ( ( m_timeStampLength & 0x7 ) ? 1 : 0 );
		Size += Bytes * 2;
	}
	return SetDataSize( Size );
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                     CMp4ES_Descriptor class                      //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//            Constructor             //
//                                    //
////////////////////////////////////////
CMp4ES_Descriptor::CMp4ES_Descriptor( void ) : CBaseDescriptor( T_ES_DescrTag )
{
	m_ES_ID = 0;
	m_streamDependenceFlag = m_URL_Flag = m_OCRstreamFlag = false;
	m_streamPriority = 0;
	m_dependsOn_ES_ID = 0;
	m_OCR_ES_Id = 0;
}

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CMp4ES_Descriptor::Read( CBaseStream& Stream )
{
	IMF_UINT8	Tmp;

	if ( !CBaseDescriptor::Read( Stream ) ) return false;

	try {
		if ( !Stream.Read16( m_ES_ID ) || !Stream.Read8( Tmp ) ) throw E_READ_STREAM;
		m_streamDependenceFlag = ( ( Tmp & 0x80 ) != 0 );
		m_URL_Flag = ( ( Tmp & 0x40 ) != 0 );
		m_OCRstreamFlag = ( ( Tmp & 0x20 ) != 0 );
		m_streamPriority = Tmp & 0x1f;
		if ( m_streamDependenceFlag ) {
			if ( !Stream.Read16( m_dependsOn_ES_ID ) ) throw E_READ_STREAM;
		} else {
			m_dependsOn_ES_ID = 0;
		}
		// [LIMITATION] URL_Flag must not be true.
		if ( m_URL_Flag ) throw E_DESCR_URL_FLAG;
		if ( m_OCRstreamFlag ) {
			if ( !Stream.Read16( m_OCR_ES_Id ) ) throw E_READ_STREAM;
		} else {
			m_OCR_ES_Id = 0;
		}
		if ( !m_decConfigDescr.Read( Stream ) ) throw m_decConfigDescr.GetLastError();
		if ( !m_slConfigDescr.Read( Stream ) ) throw m_slConfigDescr.GetLastError();
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
bool	CMp4ES_Descriptor::Write( CBaseStream& Stream ) const
{
	IMF_UINT8	Tmp;
	if ( !CBaseDescriptor::Write( Stream ) ) return false;
	if ( !Stream.Write16( m_ES_ID ) ) return false;
	Tmp = ( m_streamDependenceFlag ? 0x80 : 0 ) | ( m_URL_Flag ? 0x40 : 0 ) | ( m_OCRstreamFlag ? 0x20 : 0 ) | ( m_streamPriority & 0x1f );
	if ( !Stream.Write8( Tmp ) ) return false;
	if ( m_streamDependenceFlag && !Stream.Write16( m_dependsOn_ES_ID ) ) return false;
	// [LIMITATION] URL_Flag must not be true.
	if ( m_OCRstreamFlag && !Stream.Write16( m_OCR_ES_Id ) ) return false;
	if ( !m_decConfigDescr.Write( Stream ) ) return false;
	if ( !m_slConfigDescr.Write( Stream ) ) return false;
	return true;
}

////////////////////////////////////////
//                                    //
//     Calculate descriptor size      //
//                                    //
////////////////////////////////////////
// Return value = Whole descriptor size in bytes (-1 means error)
IMF_INT64	CMp4ES_Descriptor::CalcSize( void )
{
	IMF_INT64	Size = 3;
	if ( m_streamDependenceFlag ) Size += 2;
	if ( m_OCRstreamFlag ) Size += 2;
	return SetDataSize( Size + m_decConfigDescr.CalcSize() + m_slConfigDescr.CalcSize() );
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                CObjectDescriptorBox class (iods)                 //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CObjectDescriptorBox::Read( CBaseStream& Stream )
{
	if ( !CFullBox::Read( Stream ) ) return false;
	if ( !m_OD.Read( Stream ) ) { SetLastError( m_OD.GetLastError() ); return false; }
	if ( CheckReadSize( Stream ) != 0 ) { SetLastError( E_BOX_SIZE ); return false; }
	return true;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                       CESDBox class (esds)                       //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CESDBox::Read( CBaseStream& Stream )
{
	if ( !CFullBox::Read( Stream ) ) return false;
	if ( !m_ES.Read( Stream ) ) { SetLastError( m_ES.GetLastError() ); return false; }
	if ( CheckReadSize( Stream ) != 0 ) { SetLastError( E_BOX_SIZE ); return false; }
	return true;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                    CMP4AudioSampleEntry class                    //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	CMP4AudioSampleEntry::Read( CBaseStream& Stream )
{
	if ( !CAudioSampleEntry::Read( Stream ) ) return false;
	if ( !m_ES.Read( Stream ) ) { SetLastError( m_ES.GetLastError() ); return false; }
	if ( CheckReadSize( Stream ) != 0 ) { SetLastError( E_BOX_SIZE ); return false; }
	return true;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                COrigAudioFileInfoBox class (oafi)                //
//                                                                  //
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
//                                    //
//                Read                //
//                                    //
////////////////////////////////////////
// Stream = Input stream
// Return value = true:Success / false:Error
bool	COrigAudioFileInfoBox::Read( CBaseStream& Stream )
{
	IMF_UINT8	Tmp;

	if ( !CFullBox::Read( Stream ) ) return false;
	if ( !Stream.Read8( Tmp ) ||
		 !Stream.Read16( m_header_item_ID ) || 
		 !Stream.Read16( m_trailer_item_ID ) || 
		 !Stream.Read16( m_aux_item_ID ) ) { SetLastError( E_READ_STREAM ); return false; }
	m_file_type = ( Tmp >> 5 ) & 0x07;
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
bool	COrigAudioFileInfoBox::Write( CBaseStream& Stream ) const
{
	IMF_UINT8	Tmp;

	if ( !CFullBox::Write( Stream ) ) return false;
	if ( m_file_type & 0xf8 ) return false;
	Tmp = m_file_type << 5;
	if ( !Stream.Write8( Tmp ) || 
		 !Stream.Write16( m_header_item_ID ) || 
		 !Stream.Write16( m_trailer_item_ID ) || 
		 !Stream.Write16( m_aux_item_ID ) ) return false;
	return true;
}

// End of Mp4Box.cpp
