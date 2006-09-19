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

Filename : Mp4Box.h
Project  : MPEG-A Audio Archival Multimedia Application Format
Author   : Koichi Sugiura (NTT Advanced Technology Corporation)
           Noboru Harada  (NTT)
Date     : August 31st, 2006
Contents : Extension defined in ISO/IEC 14496-14

*******************************************************************/

#if !defined( MP4BOX_INCLUDED )
#define	MP4BOX_INCLUDED

#include	"ImfType.h"
#include	"ImfStream.h"
#include	"ImfBox.h"
#include	"ImfSampleEntry.h"
#include	"ImfDescriptor.h"

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                       Four character codes                       //
//                                                                  //
//////////////////////////////////////////////////////////////////////
#define	IMF_FOURCC_IODS		IMF_FOURCC( 'i','o','d','s' )
#define	IMF_FOURCC_ESDS		IMF_FOURCC( 'e','s','d','s' )
#define	IMF_FOURCC_MP4A		IMF_FOURCC( 'm','p','4','a' )
#define	IMF_FOURCC_OAFI		IMF_FOURCC( 'o','a','f','i' )

namespace NAlsImf {

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                       CMp4BoxReader class                        //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	class	CMp4BoxReader : public CBoxReader {
	public:
		virtual	~CMp4BoxReader( void ) {}
		CBox*				CreateBox( IMF_UINT32 Type );
		CSampleEntry*		CreateSampleEntry( IMF_UINT32 Type );
		CBaseDescriptor*	CreateDescriptor( DESCR_TAG Tag );
	};

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                         CES_ID_Inc class                         //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	struct	CES_ID_Inc : public CBaseDescriptor {
		CES_ID_Inc( void ) : CBaseDescriptor( T_ES_ID_IncTag ), m_Track_ID( 0 ) {}
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const { return CBaseDescriptor::Write( Stream ) && Stream.Write32( m_Track_ID ); }
		IMF_INT64	CalcSize( void ) { return SetDataSize( 4 ); }
		IMF_UINT32	m_Track_ID;			// ID of the track to use.
	};

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                CMp4InitialObjectDescriptor class                 //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	struct	CMp4InitialObjectDescriptor : public CBaseDescriptor {
		CMp4InitialObjectDescriptor( void );
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void );
		IMF_UINT16	m_ObjectDescriptorID;				// 10-bit
		bool		m_URL_Flag;							//  1-bit
		bool		m_includeInlineProfileLevelFlag;	//  1-bit
		IMF_UINT8	m_ODProfileLevelIndication;			//  8-bit
		IMF_UINT8	m_sceneProfileLevelIndication;		//  8-bit
		IMF_UINT8	m_audioProfileLevelIndication;		//  8-bit
		IMF_UINT8	m_visualProfileLevelIndication;		//  8-bit
		IMF_UINT8	m_graphicsProfileLevelIndication;	//  8-bit
		CDescriptorVector	m_esDescr;
	};

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                    CDecoderSpecificInfo class                    //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	struct	CDecoderSpecificInfo : public CBaseDescriptor {
		CDecoderSpecificInfo( void ) : CBaseDescriptor( T_DecSpecificInfoTag ), m_pData( NULL ), m_Size( 0 ) {}
		virtual	~CDecoderSpecificInfo( void ) { if ( m_pData ) delete[] m_pData; }
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void ) { return SetDataSize( m_Size ); }
		bool		SetData( const void* pData, IMF_UINT32 Size );
		IMF_UINT8*	m_pData;
		IMF_UINT32	m_Size;
	};

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                  CDecoderConfigDescriptor class                  //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	struct	CDecoderConfigDescriptor : public CBaseDescriptor {
		CDecoderConfigDescriptor( void );
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void ) { return SetDataSize( 13 + m_decSpecificInfo.CalcSize() ); }
		IMF_UINT8	m_objectTypeIndication;		//  8-bit
		IMF_UINT8	m_streamType;				//  6-bit
		bool		m_upStream;					//  1-bit
		IMF_UINT32	m_bufferSizeDB;				// 24-bit
		IMF_UINT32	m_maxBitrate;				// 32-bit
		IMF_UINT32	m_avgBitrate;				// 32-bit
		CDecoderSpecificInfo	m_decSpecificInfo;
	};

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                    CSLConfigDescriptor class                     //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	struct	CSLConfigDescriptor : public CBaseDescriptor {
		CSLConfigDescriptor( void );
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void );
		IMF_UINT8	m_predefined;
		bool		m_useAccessUnitStartFlag;
		bool		m_useAccessUnitEndFlag;
		bool		m_useRandomAccessPointFlag;
		bool		m_hasRandomAccessUnitsOnlyFlag;
		bool		m_usePaddingFlag;
		bool		m_useTimeStampsFlag;
		bool		m_useIdleFlag;
		bool		m_durationFlag;
		IMF_UINT32	m_timeStampResolution;
		IMF_UINT32	m_OCRResolution;
		IMF_UINT8	m_timeStampLength;
		IMF_UINT8	m_OCRLength;
		IMF_UINT8	m_AU_Length;
		IMF_UINT8	m_instantBitrateLength;
		IMF_UINT8	m_degradationPriorityLength;
		IMF_UINT8	m_AU_seqNumLength;
		IMF_UINT8	m_packetSeqNumLength;
		IMF_UINT32	m_timeScale;
		IMF_UINT16	m_accessUnitDuration;
		IMF_UINT16	m_compositionUnitDuration;
		IMF_UINT64	m_startDecodingTimeStamp;
		IMF_UINT64	m_startCompositionTimeStamp;
	};

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                     CMp4ES_Descriptor class                      //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	struct	CMp4ES_Descriptor : public CBaseDescriptor {
		CMp4ES_Descriptor( void );
		bool	Read( CBaseStream& Stream );
		bool	Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void );
		IMF_UINT16	m_ES_ID;					// 16-bit
		bool		m_streamDependenceFlag;		//  1-bit
		bool		m_URL_Flag;					//  1-bit
		bool		m_OCRstreamFlag;			//  1-bit
		IMF_UINT8	m_streamPriority;			//  5-bit
		IMF_UINT16	m_dependsOn_ES_ID;			// 16-bit
		IMF_UINT16	m_OCR_ES_Id;				// 16-bit
		CDecoderConfigDescriptor	m_decConfigDescr;
		CSLConfigDescriptor			m_slConfigDescr;
	};

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                CObjectDescriptorBox class (iods)                 //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	struct	CObjectDescriptorBox : public CFullBox {
		CObjectDescriptorBox( void ) : CFullBox( IMF_FOURCC_IODS ) {}
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const { return CFullBox::Write( Stream ) && m_OD.Write( Stream ); }
		IMF_INT64	CalcSize( void ) { return SetDataSize( m_OD.CalcSize() ); }
		// [LIMITATION] iods box must have MP4 version of InitialObjectDescriptor.
		CMp4InitialObjectDescriptor	m_OD;
	};

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                       CESDBox class (esds)                       //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	struct	CESDBox : public CFullBox {
		CESDBox( void ) : CFullBox( IMF_FOURCC_ESDS ) {}
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const { return CFullBox::Write( Stream ) && m_ES.Write( Stream ); }
		IMF_INT64	CalcSize( void ) { return SetDataSize( m_ES.CalcSize() ); }
		CMp4ES_Descriptor	m_ES;
	};

	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                    CMP4AudioSampleEntry class                    //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	struct	CMP4AudioSampleEntry : public CAudioSampleEntry {
		CMP4AudioSampleEntry( void ) : CAudioSampleEntry( IMF_FOURCC_MP4A ) {}
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const { return CAudioSampleEntry::Write( Stream ) && m_ES.Write( Stream ); }
		IMF_INT64	CalcSize( void ) { return SetDataSize( 20 + m_ES.CalcSize() ); }
		CESDBox	m_ES;
	};
	//////////////////////////////////////////////////////////////////////
	//                                                                  //
	//                COrigAudioFileInfoBox class (oafi)                //
	//                                                                  //
	//////////////////////////////////////////////////////////////////////
	struct	COrigAudioFileInfoBox : public CFullBox {
		COrigAudioFileInfoBox( void ) : CFullBox( IMF_FOURCC_OAFI ), m_file_type( 0 ) { m_header_item_ID = m_trailer_item_ID = m_aux_item_ID = 0; }
		bool		Read( CBaseStream& Stream );
		bool		Write( CBaseStream& Stream ) const;
		IMF_INT64	CalcSize( void ) { return SetDataSize( 7 ); }
		IMF_UINT8	m_file_type;
		IMF_UINT16	m_header_item_ID;
		IMF_UINT16	m_trailer_item_ID;
		IMF_UINT16	m_aux_item_ID;
	};
}

#endif	// MP4BOX_INCLUDED

// End of Mp4Box.h
